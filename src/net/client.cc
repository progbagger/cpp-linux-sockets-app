#include "include/net/client.h"

#include <stdexcept>

#include "include/net/socket.h"

namespace net {

ClientError::ClientError(const std::string& message)
    : std::runtime_error(message) {}

Client::Client(AddressFamilyType address_family, SocketType socket_type,
               ProtocolType protocol)
    : socket_(std::nullopt, address_family, socket_type, protocol),
      is_connected_(false) {}

void Client::Connect(const Address& address) {
  if (is_connected_) {
    throw ClientError("this client is already connected");
  }

  socket_.Connect(address);
  is_connected_ = true;
}

Status Client::Send(const std::string& message, int timeout_msec) {
  if (!is_connected_) {
    throw ClientError("client isn't connected");
  }

  return socket_.Send(message, timeout_msec);
}

Response<std::string> Client::Receive(int timeout_msec) {
  if (!is_connected_) {
    throw ClientError("client isn't connected");
  }

  return socket_.Receive(timeout_msec);
}

bool Client::IsConnected() const noexcept { return is_connected_; }

}  // namespace net
