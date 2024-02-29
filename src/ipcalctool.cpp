#include "../include/ipcalctool.hpp"
#include "../include/Color.hpp"
#include "../include/Exception.hpp"

#include <bitset>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace ipcalctool {

auto toNetwork(const unsigned int &ip, const unsigned int &mask)
    -> unsigned int {
  return ip & mask;
}

auto toBroadcast(const unsigned int &network, const unsigned int &mask)
    -> unsigned int {
  return network + (~mask);
}

auto toNetmask(const unsigned int &prefix) -> unsigned int {
  return (BASE_MASK << (IPV4_BITS - prefix)) & BASE_MASK;
}

void printBytes(const std::string &message, const unsigned int &thing,
                bool colored) {
  std::vector<std::bitset<ONE_BYTE>> bytes(IPV4_BYTES);

  for (int i = 0; i < 4; i++) {
    bytes[i] = thing >> i * ONE_BYTE;
  }
  std::cout << message << "\t";
  if (colored) {
    std::cout << Color::FG_LIGHT_BLUE;
  }
  for (int i = bytes.size() - 1; i >= 0; i--) {
    if (i != 0) {
      std::cout << bytes[i].to_ulong() << '.';
    } else {
      std::cout << bytes[i].to_ulong() << "\t\t";
    }
  }
  if (colored) {
    std::cout << Color::FG_DEFAULT;
    std::cout << Color::FG_LIGHT_YELLOW;
  }
  for (int i = bytes.size() - 1; i >= 0; i--) {
    if (i != 0) {
      std::cout << bytes[i] << '.';
    } else {
      std::cout << bytes[i] << '\n';
    }
  }
  if (colored) {
    std::cout << Color::FG_DEFAULT;
  }
}

auto toNumber(const std::string &str) -> unsigned int {
  unsigned int bytes[IPV4_BYTES] = {0};

  // using sscanf for simplicity
  if (sscanf(str.c_str(), "%d.%d.%d.%d", &bytes[3], &bytes[2], &bytes[1],
             &bytes[0]) != 4) {
    throw Exception("Invalid format: " + str);
  }

  for (auto byte : bytes) {
    if (byte < 0 || byte > BYTE_MAX) {
      throw Exception("Octets must be >= 0 and <= " + std::to_string(BYTE_MAX) +
                      ": " + std::to_string(byte));
    }
  }

  return bytes[0] | bytes[1] << ONE_BYTE | bytes[2] << 2 * ONE_BYTE |
         bytes[3] << 3 * ONE_BYTE;
}

void validateMask(unsigned int mask, const std::string &maskString) {
  // Only possible values are
  // 1 if invalid
  // 0 if valid
  if (mask & (~mask >> 1)) {
    throw Exception("Invalid mask: " + maskString);
  }
}

auto toPrefix(const std::string &maskString) -> int {
  int prefix = 0;
  // check if the format is x.x.x.x
  if (maskString.find('.') != std::string::npos) {
    unsigned int mask = toNumber(maskString);
    validateMask(mask, maskString);
    std::bitset<IPV4_BITS> bitset(mask);
    if (bitset.count() < 1 || bitset.count() > IPV4_BITS) {
      throw Exception("The mask must be <=32 and >0: " + maskString);
    }
    return bitset.count();
  }

  // else assume the format is /x
  // using sscanf for simplicity
  if (sscanf(maskString.c_str(), "/%u", &prefix) != 1) {
    throw Exception("Invalid mask: " + maskString);
  }
  if (prefix <= 0 || prefix > IPV4_BITS) {
    throw Exception("The mask must be <=32 and >0: " + maskString);
  }
  return prefix;
}

IPv4Network::IPv4Network(const uint32_t &ip, const uint32_t &netmask) {
  address = ip;
  mask = netmask;
  networkAddress = toNetwork(ip, mask);
  hostMin = networkAddress + 1;
  broadcast = toBroadcast(networkAddress, mask);
  hostMax = broadcast - 1;
  hostCount = broadcast - hostMin;
  std::bitset<IPV4_BITS> maskBits(mask);
  cidrPrefix = maskBits.count();
}

IPv4Network::IPv4Network(const std::string &ip, const std::string &netmask) {
  address = toNumber(ip);
  cidrPrefix = toPrefix(netmask);
  mask = toNetmask(cidrPrefix);
  networkAddress = toNetwork(address, mask);
  hostMin = networkAddress + 1;
  broadcast = toBroadcast(networkAddress, mask);
  hostMax = broadcast - 1;
  hostCount = broadcast - hostMin;
}

void IPv4Network::print(bool colored, bool extended) const {
  if (extended) {
    printBytes("Address:", this->address, colored);
    printBytes("Netmask:", this->mask, colored);
    std::cout << "CIDR prefix:\t/" << cidrPrefix << '\n';
  }
  printBytes("Network:", this->networkAddress, colored);
  printBytes("HostMin:", this->hostMin, colored);
  printBytes("HostMax:", this->hostMax, colored);
  printBytes("Broadcast:", this->broadcast, colored);
  std::cout << "Hosts/Net:\t" << this->hostCount << '\n';
}

auto calculateSubnets(const std::string &ip, const std::string &netmask,
                      const std::string &subnetMask)
    -> std::vector<IPv4Network> {
  std::vector<IPv4Network> subnets;
  int prefix = toPrefix(netmask);
  int subnetPrefix = toPrefix(subnetMask);
  int subnetCount = exp2(IPV4_BITS - prefix) / exp2(IPV4_BITS - subnetPrefix);
  uint32_t subneMaskInt = toNetmask(subnetPrefix);

  if (prefix >= subnetPrefix) {
    throw Exception("The prefix must be smaller than the subnet prefix!");
  }

  for (int i = 0; i < subnetCount; i++) {
    uint32_t address = 0;
    IPv4Network subnet(ip, netmask);
    if (i == 0) {
      address = toNumber(ip);
    } else {
      address = subnets.at(i - 1).broadcast + 1;
    }

    subnet = IPv4Network(address, subneMaskInt);
    subnets.push_back(subnet);
  }
  return subnets;
}

void printSubnetTransitionInfo(IPv4Network &net, IPv4Network &subnet,
                               bool colored) {
  std::cout << "\nSubnets after transition from /" << net.cidrPrefix << " to /"
            << subnet.cidrPrefix << "\n\n";
  ipcalctool::printBytes("Netmask:", subnet.mask, colored);
  std::cout << "CIDR prefix:\t/" << subnet.cidrPrefix << '\n';
}

}; // namespace ipcalctool
