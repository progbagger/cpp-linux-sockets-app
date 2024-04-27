#include "include/socket.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdexcept>

SocketError::SocketError(const std::string& message)
    : std::logic_error(message) {}

Socket::Socket(std::optional<FileDescriptorType> file_descriptor,
               AddressFamilyType address_family, SocketType socket_type,
               ProtocolType protocol, bool is_unblocking)
    : address_family_(address_family),
      socket_type_(socket_type),
      protocol_(protocol),
      file_descriptor_(),
      is_unblocking_(is_unblocking) {
  if (!file_descriptor.has_value()) {
    file_descriptor_ = socket(address_family, socket_type, 0);
  }

  if (file_descriptor_ < 0) {
    throw SocketError("can't create socket");
  }
}

Socket::Socket(Socket&& other) noexcept
    : address_family_(other.address_family_),
      socket_type_(other.socket_type_),
      protocol_(other.protocol_),
      file_descriptor_(other.file_descriptor_) {
  other.file_descriptor_ = -1;
}

Socket::~Socket() { Close(); }

void Socket::Close() noexcept {
  if (file_descriptor_ > 0) {
    close(file_descriptor_);
    file_descriptor_ = -1;
  }
}

AddressFamilyType Socket::GetAddressFamily() const noexcept {
  return address_family_;
}

SocketType Socket::GetSocketType() const noexcept { return socket_type_; }

ProtocolType Socket::GetProtocol() const noexcept { return protocol_; }

FileDescriptorType Socket::GetFileDescriptor() const noexcept {
  return file_descriptor_;
}

void Socket::MakeUnblocking() {
  if (is_unblocking_) {
    return;
  }

  int status_code = fcntl(file_descriptor_, F_SETFL, O_NONBLOCK);
  if (status_code < 0) {
    throw SocketError("can't make unblocking socket");
  }

  is_unblocking_ = true;
}

void Socket::Bind(AddressType address, PortType port) {
  struct sockaddr_in address_info;
  address_info.sin_port = htons(port);
  address_info.sin_addr.s_addr = htonl(address);
  address_info.sin_family = GetAddressFamily();

  int status_code = bind(GetFileDescriptor(),
                         reinterpret_cast<struct sockaddr*>(&address_info),
                         sizeof(address_info));
  if (status_code < 0) {
    throw SocketError("can't bind socket to the address");
  }
}

void Socket::Listen(int queue_size) {
  int status_code = listen(GetFileDescriptor(), queue_size);
  if (status_code < 0) {
    throw SocketError("can't listen on this socket with that queue_size");
  }
}

Socket Socket::Accept() {
  struct sockaddr_in address_info;
  int new_file_descriptor =
      accept(GetFileDescriptor(),
             reinterpret_cast<struct sockaddr*>(&address_info), nullptr);
  if (new_file_descriptor < 0) {
    throw SocketError("can't accept on this socket");
  }

  return Socket(new_file_descriptor, address_info.sin_family, GetSocketType(),
                GetProtocol(), false);
}

std::string Socket::Receive() {
  std::string result;
  std::string buffer(256, '\0');

  while (true) {
    int n = recv(GetFileDescriptor(), buffer.data(), buffer.capacity(), 0);
    if (n < 0) {
      throw SocketError("error while reading from socket");
    }

    if (n == 0) {
      break;
    }

    result.append(buffer.begin(), buffer.begin() + n);
  }

  return result;
}
