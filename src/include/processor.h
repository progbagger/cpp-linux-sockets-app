#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_PROCESSOR_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_PROCESSOR_H_

#include <istream>
#include <string>

class Processor {
 public:
  std::string Serialize(const std::string& command,
                        const std::string& input) const;

  std::pair<std::string, std::string> Deserialize(
      const std::string& input) const;

  std::pair<std::string, std::string> UnpackCommand(
      const std::string& command) const;
};

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_PROCESSOR_H_
