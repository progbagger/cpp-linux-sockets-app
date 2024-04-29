#include <signal.h>

#include <exception>
#include <iostream>
#include <string>

#include "include/interrupt.h"
#include "include/net/address.h"
#include "include/net/client.h"
#include "include/net/socket.h"
#include "include/processor.h"

int main(int argc, char** argv) {
  signal(SIGINT | SIGTERM, [](int) { throw Interrupted(); });

  if (argc < 3) {
    std::cerr << "There must be two parameters: address and port" << std::endl;
    return 1;
  }

  Processor processor;
  const size_t kMaxRetries = 3;
  size_t retries = 0;

  while (retries < kMaxRetries) {
    try {
      net::Client client;
      client.Connect(net::Address(argv[1], std::stoi(argv[2])));
      std::cerr << "Connected succesfully" << std::endl;
      retries = 0;

      while (true) {
        std::cout << "Available commands: count <message> | connections | send "
                     "<client id> <message> | exit"
                  << std::endl;

        std::cout << "Input your command: ";
        std::string command;
        std::getline(std::cin, command);
        auto unpacked_command = processor.UnpackCommand(command);

        if (unpacked_command.first == "exit") {
          std::cerr << "Exiting..." << std::endl;
          break;
        }

        if (unpacked_command.first == "count" ||
            unpacked_command.first == "connections" ||
            unpacked_command.first == "send") {
          client.Send(processor.Serialize(unpacked_command.first,
                                          unpacked_command.second));
        } else {
          std::cerr << "Unknown command: " << unpacked_command.first
                    << std::endl;
          continue;
        }

        bool is_first_poll = true;
        while (true) {
          auto response = client.Receive(is_first_poll ? 100 : 0);
          is_first_poll = false;

          if (response.status == net::Status::kClosed) {
            throw net::ClientError("connection closed");
          }
          if (response.status == net::Status::kTimeout) {
            if (is_first_poll) {
              std::cerr << "Timed out for checking new messages" << std::endl;
            }
            break;
          }

          auto deserialized = processor.Deserialize(response.data);

          if (deserialized.first == "send") {
            std::cout << "Received message from client: " << deserialized.second
                      << std::endl;
          } else if (deserialized.first == "err") {
            std::cout << "Error response: " << deserialized.second << std::endl;
          } else {
            std::cout << "Reply from server:";
            if (deserialized.first == "count") {
              std::cout << std::endl << deserialized.second;
            } else {
              std::cout << ' ' << deserialized.second;
            }

            std::cout << std::endl;
          }
        }
      }

      break;
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      std::cerr << "Attempting to reconnect (" << ++retries << ")..."
                << std::endl;
    } catch (const Interrupted& e) {
      std::cerr << "Exiting... " << std::endl;
      return 1;
    }
  }

  if (retries == kMaxRetries) {
    std::cerr << "Unable to reconnect to server in " << retries - 1
              << " attempts" << std::endl;
    return 1;
  }

  return 0;
}
