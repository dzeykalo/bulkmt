#include "record_observer.h"
#include "hard.h"

  void record_observer::worker(unsigned int &com, unsigned int &bkt)
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
  void record_observer::subscribe(std::unique_ptr<handler> &ha)
  {
    ha->subscribe(shared_from_this());
  }
  void record_observer::update(std::vector<std::string>& v)
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
  std::string record_observer::stop()
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