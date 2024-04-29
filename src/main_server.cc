#include <cctype>
#include <csignal>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "include/interrupt.h"
#include "include/net/address.h"
#include "include/net/server.h"
#include "include/net/socket.h"
#include "include/processor.h"

struct CustomResponseProcessor {
  std::vector<std::shared_ptr<net::Socket>>& connections;
  Processor processor;

  std::string operator()(const std::string& message) const {
    auto deserialized = processor.Deserialize(message);

    if (deserialized.first == "connections") {
      return processor.Serialize("connections",
                                 std::to_string(connections.size()));
    }

    if (deserialized.first == "count") {
      // pretty-print table of letters
      std::unordered_map<char, size_t> counts;
      for (char c : deserialized.second) {
        if (std::isalpha(c)) {
          ++counts[c];
        }
      }

      std::string message_header = "Message";

      std::stringstream ss;
      ss << message_header << " | " << deserialized.second << "\n";
      bool comma = false;
      for (char c : deserialized.second) {
        auto pos = counts.find(c);
        if (pos == counts.end()) {
          continue;
        }

        if (comma) {
          ss << "\n";
        }
        comma = true;

        ss << c;
        for (size_t i = 0; i < message_header.size() - 1; ++i) {
          ss << ' ';
        }
        ss << " | " << pos->second;
        counts.erase(pos);
      }

      return processor.Serialize("count", ss.str());
    }

    if (deserialized.first == "send") {
      size_t client_id;
      try {
        client_id = std::stoull(deserialized.second);
      } catch (const std::exception& e) {
        return processor.Serialize("error", "can't convert id to number");
      }

      if (client_id > connections.size() - 1) {
        return processor.Serialize("error", "this id is not connected");
      }

      auto pos = deserialized.second.find_first_of(" \t");
      pos = deserialized.second.find_first_not_of(" \t", pos);

      return processor.Serialize("send", deserialized.second.substr(pos));
    }

    return "";
  }
};

class CustomServer final : public net::Server {
 public:
  CustomServer() : net::Server() {}

 protected:
  virtual void ProcessConnection(
      std::shared_ptr<net::Socket> connection,
      const ResponseProcessor& response_processor) override {
    CustomResponseProcessor processor{connections_};
    return net::Server::ProcessConnection(connection, processor);
  }
};

int main(int argc, char** argv) {
  signal(SIGINT, [](int) { throw Interrupted(); });

  if (argc < 2) {
    std::cerr << "There must be only one parameter - port" << std::endl;
    return 1;
  }

  try {
    CustomServer server;
    server.Serve(net::Address("any", std::stoi(argv[1])),
                 [](const std::string& message) { return message; });
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (const Interrupted& e) {
    std::cerr << "Exiting..." << std::endl;
    return 1;
  }

  return 0;
}
