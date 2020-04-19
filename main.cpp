#include "observer.h"
#include "output_observer.h"
#include "record_observer.h"

int main(int argc, char* argv[])
{
  int N = 3;
  if (argc > 1)
    N = atoi(argv[1]);
  if (N <= 0)
    N = 1;

  std::string input;
  int bkt = 0;

  auto hand = std::make_unique<handler>();

  auto out = std::make_shared<output_observer>();
  auto rec = std::make_shared<record_observer>();
  out->subscribe(hand);
  rec->subscribe(hand);


  while(std::getline(std::cin, input))
  {
    hand->line_count();
    switch (bkt)
    {
      case 0:
        if (input.find('{') != std::string::npos)
          ++bkt;
        else
          hand->push(input);

        if ( hand->size() >= N || bkt)
          hand->show();
        break;

      default:
        if (input.find('{') != std::string::npos)
          ++bkt;
        else if (input.find('}') != std::string::npos)
        {
          if (--bkt < 1 )
          {
            hand->show();
            bkt = 0;
          }
        }
        else
          hand->push(input);
        break;
    }
  }
  if (!bkt)
    hand->show();

  return 0;
}