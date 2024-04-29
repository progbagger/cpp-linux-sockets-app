#include "include/net/address.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>

#include <stdexcept>
#include <string>

namespace net {

AddressError::AddressError(const std::string& message)
    : std::logic_error(message) {}

Address Address::FromString(const std::string& address,
                            AddressFamilyType address_family) {
  auto dots_pos = address.find(':');
  if (dots_pos == std::string::npos) {
    return Address(address, 0, address_family);
  }

  std::string address_str = address.substr(0, dots_pos);
  std::string port_str = address.substr(dots_pos + 1);

  return Address(address_str, std::stoi(port_str), address_family);
}

Address::Address(const std::string& ip, unsigned port,
                 AddressFamilyType address_family)
    : address_info_() {
  bzero((char*)&address_info_, sizeof(address_info_));

  int status_code;

  if (ip == "localhost") {
    status_code = inet_aton("127.0.0.1", &address_info_.sin_addr);
  } else if (ip == "any") {
    address_info_.sin_addr.s_addr = INADDR_ANY;
    status_code = 1;
  } else {
    status_code = inet_aton(ip.c_str(), &address_info_.sin_addr);
  }

  if (status_code == 0) {
    throw AddressError("invalid address");
  }

  address_info_.sin_port = port;
  address_info_.sin_family = address_family;
}

AddressType Address::GetAddress() const noexcept {
  return address_info_.sin_addr.s_addr;
}

PortType Address::GetPort() const noexcept { return address_info_.sin_port; }

struct sockaddr_in Address::GetAddressInfo() const noexcept {
  return address_info_;
}

}  // namespace net
