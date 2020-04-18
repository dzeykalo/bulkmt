#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <condition_variable>
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
  std::condition_variable cv;

  void worker()
  {
    while(alive || !q.empty())
    {
      std::unique_lock<std::mutex> ulk(mtx);
      cv.wait(ulk, [&](){ return !q.empty() || !alive; });
      if (!q.empty()){
        auto qf = std::move(q.front());
        q.pop();
        ulk.unlock();

        command += qf.size()-1;
        ++block;
        std::cout << "bulk:";
        for(auto i = qf.begin(); i != std::prev(qf.end()); i++)
          std::cout << " " << fa(*i);
        std::cout << std::endl;
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
    if (!alive)
    {
      alive = true;
      thr = std::thread(&output_observer::worker, this);
    }
    {
      std::lock_guard<std::mutex> lock(mtx);
      q.push(v);
    }
    cv.notify_one();
  }
  std::string stop() override
  {
    alive = false;
    cv.notify_all();
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
  std::mutex q_mtx;
  std::mutex f_mtx;
  std::atomic_bool alive{false};
  std::atomic_uint add{0};
  std::array<unsigned int, size> command{{0,0}};
  std::array<unsigned int, size> block{{0,0}};
  std::condition_variable cv;

  void worker(unsigned int &com, unsigned int &bkt)
  {     
    while(alive || !q.empty()){
      std::unique_lock<std::mutex> ulk(q_mtx);
      cv.wait(ulk, [&](){ return !q.empty() || !alive; });
      if (!q.empty())
      {
        auto qf = std::move(q.front());
        q.pop();
        ulk.unlock();

        com += qf.size()-1;
        ++bkt;
        std::stringstream fname;
        fname << "bulk" << qf.back() << ++add << ".log";
        std::stringstream data;
        for(auto i = qf.begin(); i != std::prev(qf.end()); i++)
          data << fi(*i) << std::endl;

        {
          std::lock_guard<std::mutex> lock(f_mtx);
          std::ofstream file( fname.str(), std::ios::trunc | std::ios::binary );
          if (file)
            file << data.str();
          file.close();
        }
      }
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
    if (!alive){
      alive = true;
      for(int i = 0; i < size; i++)
        thr[i] = std::thread(&record_observer::worker, this, std::ref(command[i]), std::ref(block[i]));
    }
    {
      std::lock_guard<std::mutex> lock(q_mtx);
      q.push(v);
    }
    cv.notify_one();
  }
  std::string stop() override
  {
    alive = false;
    cv.notify_all();
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

