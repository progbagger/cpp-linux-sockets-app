#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_NET_CLIENT_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_NET_CLIENT_H_

#include <sys/socket.h>

#include <stdexcept>
#include <string>

#include "include/net/address.h"
#include "include/net/socket.h"

namespace net {

class ClientError : public std::runtime_error {
 public:
  explicit ClientError(const std::string& message);
};

class Client {
 public:
  explicit Client(AddressFamilyType address_family = AF_INET,
                  SocketType socket_type = SOCK_STREAM,
                  ProtocolType protocol = 0);

  void Connect(const Address& address);

  Status Send(const std::string& message,
              int timeout_msec = Socket::kDefaultTimeoutMsec);
  Response<std::string> Receive(int timeout_msec = Socket::kDefaultTimeoutMsec);

  bool IsConnected() const noexcept;

 private:
  Socket socket_;

  bool is_connected_;
};

}  // namespace net

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_NET_CLIENT_H_
