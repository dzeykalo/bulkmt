#pragma once
#include "observer.h"

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

  void worker();
public:
  output_observer(){}
  void subscribe(std::unique_ptr<handler> &ha);
  void update(std::vector<std::string>& v) override;
  std::string stop() override;
};