
#include "observer.h"

  handler::~handler()
  {
    std::stringstream ss;
    ss << "main thread - " << line << " line, " << block << " block, " << command << " command" << std::endl;
    for (auto &s : view)
      ss << s->stop();
    std::cout << ss.str();
  }

  void handler::subscribe(std::shared_ptr<observer> obs) {
    view.push_back(obs);
  }
  void handler::push(std::string& str)
  {
    if (v.empty())
      setTime();

    v.push_back(str);
  }
  void handler::setTime(){ 
    tim = std::time(nullptr); 
  }
  std::string handler::getTime(){
    return std::to_string(tim);
  }
  size_t handler::size(){
    return v.size();
  }
  void handler::line_count(){ ++line; }

  void handler::show()
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
