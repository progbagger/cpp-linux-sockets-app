#include "include/net/server.h"

#include <poll.h>
#include <sys/poll.h>

#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/net/socket.h"

namespace net {

ServerError::ServerError(const std::string& message)
    : std::runtime_error(message) {}

Server::Server(AddressFamilyType listener_address_family,
               SocketType listener_socket_type, ProtocolType listener_protocol)
    : listener_(std::nullopt, listener_address_family, listener_socket_type,
                listener_protocol),
      connections_(),
      is_serving_(false) {
  listener_.MakeUnblocking();
}

void Server::Serve(const Address& address,
                   const ResponseProcessor& response_processor,
                   int timeout_msec) {
  if (is_serving_) {
    throw ServerError("this server is already serving");
  }

  listener_.Bind(address);
  listener_.Listen(5);

  is_serving_ = true;

  while (true) {
    std::cerr << "Active connections: " << connections_.size() << std::endl;

    // forming file descriptors for polling
    std::vector<struct pollfd> descriptors;
    descriptors.reserve(connections_.size() + 1);

    struct pollfd poll_file_descriptor {};
    poll_file_descriptor.fd = listener_.GetFileDescriptor();
    poll_file_descriptor.events = POLLIN;
    descriptors.push_back(poll_file_descriptor);

    for (auto i = connections_.begin(); i != connections_.end(); ++i) {
      poll_file_descriptor.fd = i->get()->GetFileDescriptor();
      poll_file_descriptor.events = POLLIN;
      descriptors.push_back(poll_file_descriptor);
    }

    int status_code =
        poll(descriptors.data(), descriptors.size(), timeout_msec);
    if (status_code < 0) {
      // error while polling

      connections_.clear();
      is_serving_ = false;

      throw ServerError("error while serving");
    }
    if (status_code == 0) {
      // no events - skipping
      continue;
    }

    std::vector<std::shared_ptr<Socket>> active_connections;
    active_connections.reserve(connections_.size());

    if (descriptors.front().revents & POLLIN) {
      // new client wants to connect

      active_connections.emplace_back(
          std::make_shared<Socket>(listener_.Accept()));
      active_connections.back()->SetLinger();
      active_connections.back()->MakeUnblocking();
    }

    for (auto i = ++descriptors.begin(); i != descriptors.end(); ++i) {
      auto connection_iter =
          connections_.begin() + std::distance(descriptors.begin(), i) - 1;

      if (i->revents & POLLIN) {
        auto future =
            std::async(&Server::ProcessConnection, this, *connection_iter,
                       std::ref(response_processor));

        try {
          future.get();
        } catch (...) {
          // this connection is closed also!!!!
          continue;
        }
      } else if (i->revents & POLLHUP) {
        // closed connection case
        continue;
      }

      active_connections.emplace_back(*connection_iter);
    }

    connections_ = std::move(active_connections);
  }

  connections_.clear();
  is_serving_ = false;
}

bool Server::IsServing() const noexcept { return is_serving_; }

void Server::ProcessConnection(std::shared_ptr<Socket> connection,
                               const ResponseProcessor& response_processor) {
  auto response = connection->Receive();
  if (response.status != Status::kOk) {
    return;
  }

  std::string processed = response_processor(response.data);
  if (!processed.empty()) {
    connection->Send(processed);
  }
}

}  // namespace net
