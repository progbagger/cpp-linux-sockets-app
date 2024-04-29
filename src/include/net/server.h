#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_NET_SERVER_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_NET_SERVER_H_

#include <sys/socket.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/net/address.h"
#include "include/net/socket.h"

namespace net {

class ServerError : public std::runtime_error {
 public:
  explicit ServerError(const std::string& message);
};

class Server {
 public:
  using ResponseProcessor = std::function<std::string(const std::string&)>;

  explicit Server(AddressFamilyType listener_address_family = AF_INET,
                  SocketType listener_socket_type = SOCK_STREAM,
                  ProtocolType listener_protocol = 0);

  virtual ~Server() = default;

  void Serve(const Address& address,
             const ResponseProcessor& response_processor,
             int timeout_msec = 60'000);

  bool IsServing() const noexcept;

 protected:
  virtual void ProcessConnection(std::shared_ptr<Socket> connection,
                                 const ResponseProcessor& response_processor);

  Socket listener_;
  std::vector<std::shared_ptr<Socket>> connections_;

  bool is_serving_;
};

}  // namespace net

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_NET_SERVER_H_
