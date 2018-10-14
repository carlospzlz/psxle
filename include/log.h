#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <iostream>

namespace psxle
{
class Logger
{
public:
  enum mode
  {
    Info = 0,
    Warning = 1,
    Error = 2
  };
  static void setMode(mode m);

private:
  static mode current_mode;
  friend mode operator<<(mode, std::ostream &(*manip)(std::ostream &));
  template <typename T>
  friend mode operator<<(mode, const T &);
};

Logger::mode operator<<(Logger::mode log,
                        std::ostream &(*manip)(std::ostream &));

template <typename T>
Logger::mode operator<<(Logger::mode log, const T &val)
{
  if (log >= Logger::current_mode)
  {
    std::cerr << val;
  }
  return log;
}

}  // namespace psxle
#endif
