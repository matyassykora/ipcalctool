#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ipcalctool {

const int IPV4_BITS = 32;
const int ONE_BYTE = 8;
const int IPV4_BYTES = IPV4_BITS / ONE_BYTE;
const int BYTE_MAX = 0xFF;
const unsigned int BASE_MASK = 0xFFFFFFFF;

auto toNetwork(const unsigned int &ip, const unsigned int &mask)
    -> unsigned int;

auto toBroadcast(const unsigned int &network, const unsigned int &mask)
    -> unsigned int;

auto toNetmask(const unsigned int &prefix) -> unsigned int;

void printBytes(const std::string &message, const unsigned int &thing,
                bool colored);

auto toNumber(const std::string &str) -> unsigned int;

void validateMask(unsigned int mask, const std::string &maskString);

auto toPrefix(const std::string &maskString) -> int;

class IPv4Network {
public:
  IPv4Network(const uint32_t &ip, const uint32_t &netmask);

  IPv4Network(const std::string &ip, const std::string &netmask);

  void print(bool colored, bool extended = false) const;

  uint32_t address;
  uint32_t mask;
  uint32_t networkAddress;
  uint32_t hostMin;
  uint32_t broadcast;
  uint32_t hostMax;
  uint32_t hostCount;
  uint32_t cidrPrefix;
};

auto calculateSubnets(const std::string &ip, const std::string &netmask,
                      const std::string &subnetMask)
    -> std::vector<IPv4Network>;

} // namespace ipcalctool
