#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_SOCKET_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_SOCKET_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <exception>
#include <stdexcept>

using AddressFamilyType = int;
using SocketType = __socket_type;
using FileDescriptorType = int;
using ProtocolType = int;
using PortType = in_port_t;
using AddressType = in_addr_t;

class SocketError : public std::logic_error {
 public:
  explicit SocketError(const std::string& message);

  virtual ~SocketError() = default;
};

class Socket {
 public:
  Socket() = delete;
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket& operator=(Socket&&) noexcept = delete;

  explicit Socket(AddressFamilyType address_family = AF_UNSPEC,
                  SocketType socket_type = SOCK_STREAM,
                  ProtocolType protocol = 0, bool is_unblocking = false);

  Socket(Socket&&) noexcept;

  virtual ~Socket();

  void Close() noexcept;

  AddressFamilyType GetAddressFamily() const noexcept;
  SocketType GetSocketType() const noexcept;
  ProtocolType GetProtocol() const noexcept;
  FileDescriptorType GetFileDescriptor() const noexcept;

  void MakeUnblocking();

  void Bind(AddressType address, PortType port);
  void Listen(int queue_size = 1);

  Socket Accept();

  std::string Receive();

 private:
  AddressFamilyType address_family_;
  SocketType socket_type_;
  ProtocolType protocol_;
  FileDescriptorType file_descriptor_;

  bool is_unblocking_;
};

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_SOCKET_H_
