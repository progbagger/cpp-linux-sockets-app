#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_CONNECTION_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_CONNECTION_H_

#include <chrono>
#include <memory>
#include <stdexcept>

#include "include/socket.h"

namespace net {

using TimePointType =
    std::chrono::time_point<std::chrono::high_resolution_clock,
                            std::chrono::milliseconds>;

class ConnectionError : public std::runtime_error {
 public:
  explicit ConnectionError(const std::string& message);
};

class Connection {
 public:
  explicit Connection(std::shared_ptr<Socket> socket);

  std::shared_ptr<Socket>& GetSocket() noexcept;
  const std::shared_ptr<Socket>& GetSocket() const noexcept;

  TimePointType GetLastAccessTime() const noexcept;

  bool IsExpired(int expiration_time,
                 TimePointType time_point =
                     std::chrono::time_point_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now())) const;

  TimePointType UpdateLastAccessTime() noexcept;

 private:
  std::shared_ptr<Socket> socket_;
  TimePointType last_access_time_;
};

}  // namespace net

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_CONNECTION_H_
