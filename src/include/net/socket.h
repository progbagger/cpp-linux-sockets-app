#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_NET_SOCKET_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_NET_SOCKET_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "include/net/address.h"

namespace net {

using SocketType = __socket_type;
using FileDescriptorType = int;
using ProtocolType = int;

class SocketError : public std::logic_error {
 public:
  explicit SocketError(const std::string& message);

  virtual ~SocketError() = default;
};

enum class Status { kOk, kTimeout, kClosed };

template <class T>
struct Response {
  T data;
  Status status;
};

class Socket {
 public:
  constexpr static int kDefaultTimeoutMsec = 5'000;

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket& operator=(Socket&&) noexcept = delete;

  explicit Socket(
      std::optional<FileDescriptorType> file_descriptor = std::nullopt,
      AddressFamilyType address_family = AF_INET,
      SocketType socket_type = SOCK_STREAM, ProtocolType protocol = 0,
      bool is_unblocking = false);

  Socket(Socket&&) noexcept;

  virtual ~Socket();

  AddressFamilyType GetAddressFamily() const noexcept;
  SocketType GetSocketType() const noexcept;
  ProtocolType GetProtocol() const noexcept;
  FileDescriptorType GetFileDescriptor() const noexcept;

  void MakeUnblocking();
  void SetLinger(int timeout_sec = kDefaultTimeoutMsec);
  void SetReusable();

  void Bind(const Address& address);
  void Connect(const Address& address);

  void Listen(int queue_size = 1);

  Socket Accept();

  Response<std::string> Receive(int timeout_msec = kDefaultTimeoutMsec);
  Status Send(const std::string& message,
              int timeout_msec = kDefaultTimeoutMsec);

 private:
  void Close() noexcept;

  Response<size_t> ReadMessageLengthHeader(int timeout_msec);

  AddressFamilyType address_family_;
  SocketType socket_type_;
  ProtocolType protocol_;
  FileDescriptorType file_descriptor_;

  bool is_unblocking_;
};

}  // namespace net

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_NET_SOCKET_H_
