#include "include/net/socket.h"

#include <asm-generic/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>

#include "include/net/address.h"

namespace net {

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
  } else {
    file_descriptor_ = file_descriptor.value();
  }

  if (file_descriptor_ < 0) {
    throw SocketError("can't create socket");
  }

  try {
    SetLinger();
  } catch (const SocketError&) {
    Close();
    throw;
  }
}

Socket::Socket(Socket&& other) noexcept
    : address_family_(other.address_family_),
      socket_type_(other.socket_type_),
      protocol_(other.protocol_),
      file_descriptor_(other.file_descriptor_),
      is_unblocking_(other.is_unblocking_) {
  other.file_descriptor_ = -1;
}

Socket::~Socket() { Close(); }

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

void Socket::SetLinger(int timeout_sec) {
  struct linger linger;
  linger.l_onoff = true;
  linger.l_linger = 30;
  int status = setsockopt(file_descriptor_, SOL_SOCKET, SO_LINGER, &linger,
                          sizeof(linger));
  if (status < 0) {
    throw SocketError("can't linger socket");
  }
}

void Socket::Bind(const Address& address) {
  struct sockaddr_in address_info = address.GetAddressInfo();

  int status_code = bind(GetFileDescriptor(),
                         reinterpret_cast<struct sockaddr*>(&address_info),
                         sizeof(address_info));
  if (status_code < 0) {
    throw SocketError("can't bind socket to the address");
  }
}

void Socket::Connect(const Address& address) {
  struct sockaddr_in address_info = address.GetAddressInfo();

  int status_code = connect(GetFileDescriptor(),
                            reinterpret_cast<struct sockaddr*>(&address_info),
                            sizeof(address_info));
  if (status_code < 0) {
    throw SocketError("can't connect socket to the address");
  }
}

void Socket::Listen(int queue_size) {
  int status_code = listen(GetFileDescriptor(), queue_size);
  if (status_code < 0) {
    throw SocketError("can't listen on this socket with that queue_size");
  }
}

Socket Socket::Accept() {
  struct sockaddr_in new_address_info;
  socklen_t new_address_info_length = sizeof(new_address_info);

  int new_file_descriptor =
      accept(GetFileDescriptor(),
             reinterpret_cast<struct sockaddr*>(&new_address_info),
             &new_address_info_length);
  if (new_file_descriptor < 0) {
    throw SocketError("can't accept on this socket");
  }

  return Socket(new_file_descriptor, GetAddressFamily(), GetSocketType(),
                GetProtocol(), false);
}

Response<std::string> Socket::Receive(int timeout_msec) {
  auto response = ReadMessageLengthHeader(timeout_msec);
  if (response.status != Status::kOk) {
    return Response<std::string>{"", response.status};
  }

  size_t message_length = response.data;
  std::string result(message_length, '\0');
  size_t readed_bytes = 0;

  struct pollfd fds[1];
  fds[0].fd = GetFileDescriptor();
  fds[0].events = POLLIN;

  while (readed_bytes != message_length) {
    int status = poll(fds, 1, timeout_msec);
    if (status < 0) {
      throw SocketError("error while polling");
    }
    if (status == 0) {
      return Response<std::string>{"", Status::kTimeout};
    }

    if ((fds[0].revents & POLLHUP) || fds[0].revents & POLLERR) {
      return Response<std::string>{"", Status::kClosed};
    }

    if (fds[0].revents & POLLIN) {
      int n = recv(GetFileDescriptor(), result.data() + readed_bytes,
                   result.size() - readed_bytes, 0);
      if (n <= 0) {
        throw SocketError("error while reading from socket");
      }

      readed_bytes += n;
    }

    fds[0].revents = 0;
  }

  return Response<std::string>{result, Status::kOk};
}

Status Socket::Send(const std::string& message, int timeout_msec) {
  std::string message_length_header = std::to_string(message.size()) + ";";
  std::string message_with_header = message_length_header + message;

  struct pollfd fds[1];
  fds[0].fd = GetFileDescriptor();
  fds[0].events = POLLOUT;

  int total_sended = 0;
  while (total_sended != message_with_header.size()) {
    int status = poll(fds, 1, timeout_msec);
    if (status < 0) {
      throw SocketError("error while polling");
    }
    if (status == 0) {
      return Status::kTimeout;
    }

    if ((fds[0].revents & POLLHUP) || (fds[0].revents & POLLERR)) {
      return Status::kClosed;
    }

    if (fds[0].revents & POLLOUT) {
      int n =
          send(GetFileDescriptor(), message_with_header.c_str() + total_sended,
               message_with_header.size() - total_sended, 0);

      if (n < 0) {
        throw SocketError("error while writing to socket");
      }

      total_sended += n;
    }

    fds[0].revents = 0;
  }

  return Status::kOk;
}

void Socket::Close() noexcept {
  if (file_descriptor_ > 0) {
    close(file_descriptor_);
  }
}

Response<size_t> Socket::ReadMessageLengthHeader(int timeout_msec) {
  std::string buffer(256, '\0');
  size_t readed_bytes = 0;

  struct pollfd fds[1];
  fds[0].fd = GetFileDescriptor();
  fds[0].events = POLLIN;

  while (true) {
    int status = poll(fds, 1, timeout_msec);
    if (status < 0) {
      throw SocketError("error while polling");
    }
    if (status == 0) {
      return Response<size_t>{0, Status::kTimeout};
    }

    if ((fds[0].revents & POLLHUP) || (fds[0].revents & POLLERR)) {
      return Response<size_t>{0, Status::kClosed};
    }

    if (fds[0].revents & POLLIN) {
      int n = recv(GetFileDescriptor(), buffer.data() + readed_bytes++, 1, 0);
      if (n <= 0) {
        throw SocketError("error while reading message header length");
      }

      if (buffer[readed_bytes - 1] == ';') {
        break;
      }
    }

    fds[0].revents = 0;
  }

  buffer.resize(readed_bytes - 1);
  return Response<size_t>{std::stoull(buffer), Status::kOk};
}

}  // namespace net
