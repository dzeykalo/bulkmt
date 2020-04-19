#include "output_observer.h"
#include "hard.h"

  void output_observer::worker()
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

  void output_observer::subscribe(std::unique_ptr<handler> &ha)
  {
    ha->subscribe(shared_from_this());
  }
  void output_observer::update(std::vector<std::string>& v)
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
  std::string output_observer::stop()
  {
    alive = false;
    cv.notify_all();
    std::stringstream ss;
    if (thr.joinable())
      thr.join();

    ss << "log thread - " << block << " block, " << command << " command" << std::endl;
    return ss.str(); 
  }