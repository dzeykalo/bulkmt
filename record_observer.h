#pragma once
#include "observer.h"

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

  void worker(unsigned int &com, unsigned int &bkt);
public:
  record_observer(){}
  void subscribe(std::unique_ptr<handler> &ha);
  void update(std::vector<std::string>& v) override;
  std::string stop() override;
};