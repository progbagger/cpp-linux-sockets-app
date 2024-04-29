#include "include/processor.h"

#include <algorithm>
#include <cctype>
#include <utility>

std::string Processor::Serialize(const std::string& command,
                                 const std::string& input) const {
  return command + ";" + input;
}

std::pair<std::string, std::string> Processor::Deserialize(
    const std::string& input) const {
  size_t pos = input.find(';');
  if (pos == std::string::npos || input.empty()) {
    return {"", ""};
  }

  std::string command = input.substr(0, pos);

  std::string data;
  if (pos != input.size() - 1) {
    data = input.substr(pos + 1);
  }

  return {command, data};
}

std::pair<std::string, std::string> Processor::UnpackCommand(
    const std::string& command) const {
  auto command_start = std::find_if_not(command.begin(), command.end(),
                                        [](char c) { return std::isspace(c); });
  auto command_end = std::find_if(command_start, command.end(),
                                  [](char c) { return std::isspace(c); });

  auto arg_start = std::find_if_not(command_end, command.end(),
                                    [](char c) { return std::isspace(c); });

  return {std::string(command_start, command_end),
          std::string(arg_start, command.end())};
}
