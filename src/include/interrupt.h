#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_INTERRUPT_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_INTERRUPT_H_

#include <stdexcept>
#include <string>

class Interrupted {
 public:
  virtual ~Interrupted() = default;

  std::string what() const;
};

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_INTERRUPT_H_
