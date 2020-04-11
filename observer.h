#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <thread>
#include <memory>
#include <queue>

class observer
{
public:
  virtual void update(std::vector<std::string>& v) = 0;
};

class handler
{
private:
  std::vector<std::string> v;
  std::queue<std::vector<std::string>> q;
  std::time_t tim;
  std::vector<std::shared_ptr<observer>> view;
public:
  handler(){}

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
      q.push(v);
      v.clear();
    }
    if (!q.empty())
    {
      for (auto &s : view)
      {
        s->update(q.front());
      }
      q.pop();
    }
  }
};

class output_observer : public observer, public std::enable_shared_from_this<output_observer>
{
public:
  output_observer(){}
  void subscribe(std::unique_ptr<handler> &ha)
  {
    ha->subscribe(shared_from_this());
  }
  void update(std::vector<std::string>& v) override
  {
    std::cout << "bulk:";
    for(auto i = v.begin(); i != v.end()-1; i++)
    {
      std::cout << " " << *i;
    }
    std::cout << std::endl;
  }
};

class record_observer : public observer, public std::enable_shared_from_this<record_observer>
{
public:
  record_observer(){}
  void subscribe(std::unique_ptr<handler> &ha)
  {
    ha->subscribe(shared_from_this());
  }
  void update(std::vector<std::string>& v) override
  {
    // std::string t = std::to_string(tim);
    std::ofstream file("bulk" + v.back() + ".log", std::ios::trunc | std::ios::binary );
    if (file)
    {
      for(auto i = v.begin(); i != v.end() - 1; i++)
      {
        file << *i << std::endl;
      }
    }
    file.close();
  }
};

