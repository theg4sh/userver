#include <userver/engine/io/sockaddr.hpp>

#include <arpa/inet.h>

#include <array>
#include <limits>
#include <ostream>

#include <fmt/format.h>

#include <userver/engine/io/exception.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/assert.hpp>

namespace engine::io {

bool Sockaddr::HasPort() const {
  switch (Data()->sa_family) {
    case AF_INET:
    case AF_INET6:
      return true;

    default:
      return false;
  }
}

int Sockaddr::Port() const {
  switch (Data()->sa_family) {
    case AF_INET:
      // NOLINTNEXTLINE(hicpp-no-assembler)
      return ntohs(As<struct sockaddr_in>()->sin_port);

    case AF_INET6:
      // NOLINTNEXTLINE(hicpp-no-assembler)
      return ntohs(As<struct sockaddr_in6>()->sin6_port);

    default:
      throw AddrException(fmt::format(
          "Cannot determine port for address family {}", Data()->sa_family));
  }
}

void Sockaddr::SetPort(int port) {
  using PortLimits = std::numeric_limits<in_port_t>;
  if (port < PortLimits::min() || port > PortLimits::max()) {
    throw AddrException(fmt::format("Cannot set invalid port {}", port));
  }
  // NOLINTNEXTLINE(hicpp-no-assembler)
  const auto net_port = htons(port);
  switch (Data()->sa_family) {
    case AF_INET:
      As<struct sockaddr_in>()->sin_port = net_port;
      break;
    case AF_INET6:
      As<struct sockaddr_in6>()->sin6_port = net_port;
      break;

    default:
      throw AddrException(
          fmt::format("Unable to set port {} for an unknown address family {}",
                      port, Data()->sa_family));
  }
}

std::string Sockaddr::PrimaryAddressString() const {
  std::array<char, std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)> buf{};

  switch (Domain()) {
    case AddrDomain::kInet: {
      const auto* inet_addr = As<struct sockaddr_in>();
      if (!inet_ntop(AF_INET, &inet_addr->sin_addr, buf.data(), buf.size())) {
        int err_value = errno;
        throw IoSystemError(err_value)
            << "Cannot convert IPv4 address to string";
      }
      return buf.data();
    } break;

    case AddrDomain::kInet6: {
      const auto* inet6_addr = As<struct sockaddr_in6>();
      if (!inet_ntop(AF_INET6, &inet6_addr->sin6_addr, buf.data(),
                     buf.size())) {
        int err_value = errno;
        throw IoSystemError(err_value)
            << "Cannot convert IPv6 address to string";
      }
      return buf.data();
    } break;

    case AddrDomain::kUnix:
      return As<struct sockaddr_un>()->sun_path;
      break;

    case AddrDomain::kUnspecified:
      break;
  }
  return "[unknown address]";
}

logging::LogHelper& operator<<(logging::LogHelper& lh, const Sockaddr& sa) {
  return lh << fmt::to_string(sa);
}

}  // namespace engine::io
