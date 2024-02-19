#include "../include/IPv4Address.hpp"
#include "../include/Exception.hpp"
#include "../include/Network.hpp"
#include "../include/ipcalctool.hpp"

#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace ipcalctool {
void IPv4Address::processIPv4(const std::string &address,
                              const std::string &mask) const {
  int prefix = toPrefix(mask);
  unsigned int ip = toNumber(address, valueTypes::IP_ADDRESS);
  unsigned int netmask = toNetmask(prefix);

  Network net(ip, netmask);

  printBytes("Address:", net.ip, this->COLORED);
  printBytes("Netmask:", net.netmask, this->COLORED);
  std::cout << "CIDR prefix:\t/" << prefix << '\n';
  net.print(this->COLORED);
}

void IPv4Address::processIPv4WithSubnets(const std::string &address,
                                         const std::string &mask,
                                         const std::string &subnetMask) const {
  std::vector<Network> subnets;
  int prefix = toPrefix(mask);
  int subnetPrefix = toPrefix(subnetMask);
  unsigned int subnetNetmask = toNetmask(subnetPrefix);
  int subnetCount = getSubnetCount(prefix, subnetPrefix);

  if (subnetPrefix <= prefix) {
    throw Exception("The subnet mask must be larger than the mask!");
  }

  processIPv4(address, mask);

  std::cout << "\nSubnets after transition from /" << prefix << " to /"
            << subnetPrefix << "\n\n";
  printBytes("Netmask:", subnetNetmask, this->COLORED);
  std::cout << "CIDR prefix:\t/" << subnetPrefix << '\n';

  for (int i = 0; i < subnetCount; i++) {
    unsigned int ip = 0;
    if (i == 0) {
      ip = toNumber(address, valueTypes::IP_ADDRESS);
    } else {
      ip = subnets[i - 1].broadcast + 1;
    }
    Network net(ip, subnetNetmask);
    subnets.push_back(net);
  }

  printSubnets(subnets);
}

void IPv4Address::colorOutput(bool colored) { this->COLORED = colored; }

void IPv4Address::validateMask(unsigned int mask,
                               const std::string &maskString) {
  // Only possible values are
  // 1 if invalid
  // 0 if valid
  if (mask & (~mask >> 1)) {
    throw Exception("Invalid mask: " + maskString);
  }
}

auto IPv4Address::toPrefix(const std::string &maskString) -> int {
  int prefix = 0;
  // check if the format is x.x.x.x
  if (maskString.find('.') != std::string::npos) {
    unsigned int mask = toNumber(maskString, valueTypes::MASK);
    validateMask(mask, maskString);
    std::bitset<IPV4_BITS> bitset(mask);
    if (bitset.count() < 1 || bitset.count() > IPV4_BITS) {
      throw Exception("The mask must be <=32 and >0!");
    }
    return bitset.count();
  }

  // else assume the format is /x
  // using sscanf for simplicity
  if (sscanf(maskString.c_str(), "/%u", &prefix) != 1) {
    throw Exception("Invalid mask: " + maskString);
  }
  if (prefix <= 0 || prefix > IPV4_BITS) {
    throw Exception("The mask must be <=32 and >0!");
  }
  return prefix;
}

auto IPv4Address::toNumber(const std::string &str, const valueTypes &type)
    -> unsigned int {
  unsigned int bytes[IPV4_BYTES] = {0};
  std::string valueType;
  if (type == valueTypes::MASK) {
    valueType = "mask";
  }
  if (type == valueTypes::IP_ADDRESS) {
    valueType = "IP";
  }

  // using sscanf for simplicity
  if (sscanf(str.c_str(), "%d.%d.%d.%d", &bytes[3], &bytes[2], &bytes[1],
             &bytes[0]) != 4) {
    throw Exception("Invalid " + valueType + " format: " + str);
  }

  for (auto byte : bytes) {
    if (byte < 0 || byte > BYTE_MAX) {
      throw Exception(valueType + " bytes must be >= 0 and <= " +
                      std::to_string(BYTE_MAX) + ": " + std::to_string(byte));
    }
  }

  return bytes[0] | bytes[1] << ONE_BYTE | bytes[2] << 2 * ONE_BYTE |
         bytes[3] << 3 * ONE_BYTE;
}

auto IPv4Address::toNetwork(const unsigned int &ip, const unsigned int &mask)
    -> unsigned int {
  return ip & mask;
}

auto IPv4Address::toNetmask(const unsigned int &prefix) -> unsigned int {
  return (BASE_MASK << (IPV4_BITS - prefix)) & BASE_MASK;
}

auto IPv4Address::toBroadcast(const unsigned int &network,
                              const unsigned int &mask) -> unsigned int {
  return network + (~mask);
}

auto IPv4Address::getSubnetCount(const int &prefix, const int &subnetPrefix)
    -> int {
  return pow(2, IPV4_BITS - prefix) / pow(2, IPV4_BITS - subnetPrefix);
}

void IPv4Address::printSubnets(const std::vector<Network> &subnets) const {
  for (size_t i = 0; i < subnets.size(); i++) {
    std::cout << '\n' << i + 1 << ".\n";
    subnets[i].print(this->COLORED);
  }
}
} // namespace ipcalctool
