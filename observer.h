#pragma once
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
  ~handler();
  void subscribe(std::shared_ptr<observer> obs);
  void push(std::string& str);
  void setTime();
  std::string getTime();
  size_t size();
  void line_count();
  void show();
};