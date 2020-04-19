#include "hard.h"

std::string fa(std::string &s)
{
  try
  {
    int num = std::stoi(s);
    return std::to_string(factorial<int>(num));
  }
  catch(const std::exception)
  {
    return s;
  }
}

std::string fi(std::string &s)
{
  try
  {
    int num = std::stoi(s);
    return std::to_string(fibonacci<int>(num));
  }
  catch(const std::exception)
  {
    return s;
  }
}

template <typename T>
T factorial (T n)
{
  T res;

  res = 1;
  for (int i = 1; i <= n; i++)
  {
    res = res * i;
  }
  return  res;
}

template <typename T>
T fibonacci (T n)
{
  if (n <= 1)
    return n;
  return fibonacci(n-1) + fibonacci(n-2);
}