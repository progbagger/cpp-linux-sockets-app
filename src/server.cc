#include "include/server.h"

#include <poll.h>
#include <sys/poll.h>

#include <functional>
#include <future>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/connection.h"
#include "include/socket.h"

namespace net {

bool operator==(const Socket& l, const Socket& r) noexcept {
  return l.GetFileDescriptor() == r.GetFileDescriptor();
}

ServerError::ServerError(const std::string& message)
    : std::runtime_error(message) {}

Server::Server(AddressFamilyType listener_address_family,
               SocketType listener_socket_type, ProtocolType listener_protocol)
    : listener_(std::nullopt, listener_address_family, listener_socket_type,
                listener_protocol),
      connections_() {
  listener_.MakeUnblocking();
}

void Server::Serve(AddressType address, PortType port,
                   const ResponseProcessor& response_processor,
                   int timeout_msec) {
  listener_.Bind(address, port);
  listener_.Listen(5);

  while (true) {
    // forming file descriptors for polling
    std::vector<struct pollfd> descriptors;
    descriptors.reserve(connections_.size() + 1);

    descriptors.emplace_back(POLLIN, listener_.GetFileDescriptor());

    for (auto i = connections_.begin(); i != connections_.end();) {
      if (i->IsExpired(timeout_msec)) {
        // if expired - remove
        i = connections_.erase(i);
        continue;
      }

      // else - push into polling array
      descriptors.emplace_back(POLLIN, i->GetSocket()->GetFileDescriptor());
      ++i;
    }

    int status_code =
        poll(descriptors.data(), descriptors.size(), timeout_msec);
    if (status_code < 0) {
      throw ServerError("error while serving");
    }
    if (status_code == 0) {
      // no messages from clients, clearing connections
      connections_.clear();
      continue;
    }

    if (descriptors.front().revents & POLLIN) {
      // new client wants to connect
      connections_.emplace_back(std::make_shared<Socket>(listener_.Accept()));
      continue;
    }

    for (auto i = ++descriptors.begin(); i != descriptors.end(); ++i) {
      // checking opened input connections

      auto connection_iter =
          connections_.begin() + std::distance(descriptors.begin(), i) - 1;

      if (i->revents & POLLIN) {
        connection_iter->UpdateLastAccessTime();

        static_cast<void>(std::async(&Server::ProcessConnection, this,
                                     *connection_iter,
                                     std::ref(response_processor)));

        continue;
      }
    }
  }
}

}  // namespace net
