#ifndef CPP_LINUX_SOCKETS_APP_INCLUDE_NET_ADDRESS_H_
#define CPP_LINUX_SOCKETS_APP_INCLUDE_NET_ADDRESS_H_

#include <netinet/in.h>

#include <stdexcept>
#include <string>

namespace net {

using AddressType = in_addr_t;
using PortType = in_port_t;
using AddressFamilyType = int;

static const std::string kAny = "any";
static const std::string kLocalhost = "localhost";

class AddressError : public std::logic_error {
 public:
  explicit AddressError(const std::string& message);
};

class Address {
 public:
  static Address FromString(const std::string& address,
                            AddressFamilyType address_family = AF_INET);

  Address(const std::string& ip, unsigned port,
          AddressFamilyType address_family = AF_INET);

  virtual ~Address() = default;

  AddressType GetAddress() const noexcept;
  PortType GetPort() const noexcept;
  AddressFamilyType GetAddressFamily() const noexcept;

  struct sockaddr_in GetAddressInfo() const noexcept;

 private:
  struct sockaddr_in address_info_;
};

}  // namespace net

#endif  // CPP_LINUX_SOCKETS_APP_INCLUDE_NET_ADDRESS_H_
