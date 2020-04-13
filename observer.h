#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include "hard.h"

class observer
{
public:
  virtual void update(std::vector<std::string>& v) = 0;
  virtual std::string stop() = 0;
};

class handler
{
private:
  std::vector<std::string> v;
  std::time_t tim;
  std::vector<std::shared_ptr<observer>> view;
  unsigned int line{0};
  unsigned int command{0};
  unsigned int block{0};
public:
  handler(){}
  ~handler()
  {
    std::stringstream ss;
    ss << "main thread - " << line << " line, " << block << " block, " << command << " command" << std::endl;
    for (auto &s : view)
    {
      ss << s->stop();
    }
    std::cout << ss.str();
  }

  void subscribe(std::shared_ptr<observer> obs) {
    view.push_back(obs);
  }
  void push(std::string& str)
  {
    if (v.empty())
      setTime();

    v.push_back(str);
  }
  void setTime(){ 
    tim = std::time(nullptr); 
  }
  std::string getTime(){
    return std::to_string(tim);
  }
  auto size(){
    return v.size();
  }
  void line_count(){ ++line; }

  void show()
  {
    if (!v.empty())
    {
      command += v.size();
      ++block;
      v.push_back(getTime());
      for (auto &s : view)
      {
        s->update(v);
      }
      v.clear();
    }      
  }
};

class output_observer : public observer, public std::enable_shared_from_this<output_observer>
{
private:
  std::mutex mtx;
  std::thread thr;
  std::queue<std::vector<std::string>> q;
  bool alive = false;
  unsigned int command{0};
  unsigned int block{0};

  void worker()
  {
    while(alive || !q.empty())
    {
      if (q.empty())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      
      {
        std::lock_guard<std::mutex> lock(mtx);
        command += q.front().size()-1;
        ++block;
        std::cout << "bulk:";
        for(auto i = q.front().begin(); i != q.front().end()-1; i++)
        {
          std::cout << " " << fa(*i);
        }
        std::cout << std::endl;
        q.pop();
      }
    }
  }
public:
  output_observer(){}
  void subscribe(std::unique_ptr<handler> &ha)
  {
    ha->subscribe(shared_from_this());
  }
  void update(std::vector<std::string>& v) override
  {
    q.push(v);

    if (!alive)
    {
      alive = true;
      thr = std::thread(&output_observer::worker, this);
    }
  }
  std::string stop() override
  {
    alive = false;
    std::stringstream ss;
    if (thr.joinable())
      thr.join();

    ss << "log thread - " << block << " block, " << command << " command" << std::endl;
    return ss.str(); 
  }
  
};

class record_observer : public observer, public std::enable_shared_from_this<record_observer>
{
private:
  static const int size = 2;
  std::array<std::thread, size> thr;
  std::queue<std::vector<std::string>> q;
  std::mutex mtx;
  bool alive{false};
  unsigned int add{0};
  std::array<unsigned int, size> command{{0,0}};
  std::array<unsigned int, size> block{{0,0}};

  void worker(unsigned int &com, unsigned int &bkt)
  {     
    while(alive || !q.empty())
    {
      
      if (q.empty())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      
      {
        std::lock_guard<std::mutex> lock(mtx);
        if (q.empty()) continue; 
        com += q.front().size()-1;
        ++bkt;
        std::string section = std::to_string(add++);
        std::ofstream file("bulk" + q.front().back() + section + ".log", std::ios::trunc | std::ios::binary );
        if (file)
        {
          for(auto i = q.front().begin(); i != q.front().end() - 1; i++)
          {
            file << fi(*i) << std::endl;
          }
        }
        file.close();
        q.pop();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  }
public:
  record_observer(){}
  void subscribe(std::unique_ptr<handler> &ha)
  {
    ha->subscribe(shared_from_this());
  }
  void update(std::vector<std::string>& v) override
  {
    
    q.push(v);
    
    if (!alive)
    {
      alive = true;
      for(int i = 0; i < size; i++)
        thr[i] = std::thread(&record_observer::worker, this, std::ref(command[i]), std::ref(block[i]));
    }
  }
  std::string stop() override
  {
    alive = false;
    std::stringstream ss;
    for(int i = 0; i < size; i++)
    {
      if (thr[i].joinable())
        thr[i].join();
      ss << "file" << i+1 << " thread - " << block[i] << " block, " << command[i] << " command" << std::endl;
    }
    return ss.str();
  }
};

