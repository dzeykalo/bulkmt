#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>

class observer
{
public:
  virtual void update(std::vector<std::string>& v) = 0;
  virtual void stop() = 0;
};

class handler
{
private:
  std::vector<std::string> v;
  std::time_t tim;
  std::vector<std::shared_ptr<observer>> view;
public:
  handler(){}
  ~handler()
  {
    for (auto &s : view)
    {
      s->stop();
    }
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

  void show()
  {
    if (!v.empty())
    {
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
        std::cout << "bulk:";
        for(auto i = q.front().begin(); i != q.front().end()-1; i++)
        {
          std::cout << " " << *i;
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
  void stop() override
  {
    alive = false;
    if (thr.joinable())
      thr.join();    
  }
  
};

class record_observer : public observer, public std::enable_shared_from_this<record_observer>
{
private:
  std::array<std::thread, 2> thr;
  std::array<std::queue<std::vector<std::string>>, 2> q;
  std::mutex mtx;
  bool alive{false};
  std::atomic_int add{0};
  unsigned int count{0};

  void worker(std::queue<std::vector<std::string>> &qu)
  {
    while(alive || !qu.empty())
    {
      if (qu.empty())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }

      std::string section = std::to_string(add++);
      {
        std::lock_guard<std::mutex> lock(mtx);
        std::ofstream file("bulk" + qu.front().back() + section + ".log", std::ios::trunc | std::ios::binary );
        if (file)
        {
          for(auto i = qu.front().begin(); i != qu.front().end() - 1; i++)
          {
            file << *i << std::endl;
          }
        }
        file.close();
        qu.pop();
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
    q[count].push(v);
    if (count++ >= thr.size() - 1)
      count = 0;
    
    if (!alive)
    {
      alive = true;
      for(auto i = 0; i < thr.size(); ++i)
        thr[i] = std::thread(&record_observer::worker, this, std::ref(q[i]));
    }
  }
  void stop() override
  {
    alive = false;
    for(auto &i : thr)
    {
      if (i.joinable())
        i.join(); 
    } 
  }
};

