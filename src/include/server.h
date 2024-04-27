#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_SERVER_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_SERVER_H_

#include <sys/socket.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/connection.h"
#include "include/socket.h"

namespace std {

template <>
struct std::hash<Socket> {
  size_t operator()(const Socket& socket) const noexcept {
    return socket.GetFileDescriptor();
  }
};

template <>
struct std::hash<net::Connection> {
  size_t operator()(const net::Connection& connection) const noexcept {
    return std::hash<std::shared_ptr<Socket>>()(connection.GetSocket());
  }
};

}  // namespace std

namespace net {

bool operator==(const Socket& l, const Socket& r) noexcept;

class ServerError : public std::runtime_error {
 public:
  explicit ServerError(const std::string& message);
};

class Server {
 public:
  using ResponseProcessor = std::function<std::string(const std::string&)>;

  explicit Server(AddressFamilyType listener_address_family = AF_UNSPEC,
                  SocketType listener_socket_type = SOCK_STREAM,
                  ProtocolType listener_protocol = 0);

  virtual ~Server() = default;

  void Serve(AddressType address, PortType port,
             const ResponseProcessor& response_processor,
             int timeout_msec = 60'000);

 private:
  void ProcessConnection(Connection connection,
                         const ResponseProcessor& response_processor);

  Socket listener_;
  std::vector<Connection> connections_;
};

}  // namespace net

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_SERVER_H_
