#include "include/connection.h"

#include <chrono>
#include <memory>
#include <stdexcept>

#include "include/socket.h"

namespace net {

ConnectionError::ConnectionError(const std::string& message)
    : std::runtime_error(message) {}

Connection::Connection(std::shared_ptr<Socket> socket)
    : socket_(socket),
      last_access_time_(std::chrono::time_point_cast<std::chrono::milliseconds>(
          std::chrono::high_resolution_clock::now())) {}

std::shared_ptr<Socket>& Connection::GetSocket() noexcept { return socket_; }

const std::shared_ptr<Socket>& Connection::GetSocket() const noexcept {
  return socket_;
}

bool Connection::IsExpired(int expiration_time,
                           TimePointType time_point) const {
  return std::chrono::duration(time_point - last_access_time_) >=
         std::chrono::milliseconds(expiration_time);
}

TimePointType Connection::UpdateLastAccessTime() noexcept {
  last_access_time_ = std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now());

  return last_access_time_;
}

}  // namespace net
