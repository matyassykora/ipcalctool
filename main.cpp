#include "CppClip.hpp"
#include <bitset>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

const int IPV4_BITS = 32;
const int ONE_BYTE = 8;
const int IPV4_BYTES = IPV4_BITS / ONE_BYTE;
const int BYTE_MAX = 0xFF;
const unsigned int BASE_MASK = 0xFFFFFFFF;

namespace Color {
enum ANSICode {
  FG_DEFAULT = 39,
  FG_BLACK = 30,
  FG_RED = 31,
  FG_GREEN = 32,
  FG_YELLOW = 33,
  FG_BLUE = 34,
  FG_MAGENTA = 35,
  FG_CYAN = 36,
  FG_LIGHT_GRAY = 37,
  FG_DARK_GRAY = 90,
  FG_LIGHT_RED = 91,
  FG_LIGHT_GREEN = 92,
  FG_LIGHT_YELLOW = 93,
  FG_LIGHT_BLUE = 94,
  FG_LIGHT_MAGENTA = 95,
  FG_LIGHT_CYAN = 96,
  FG_WHITE = 97,

  BG_DEFAULT = 49,
  BG_RED = 41,
  BG_GREEN = 42,
  BG_BLUE = 44,
  BG_LIGHT_GRAY = 100,
  BG_LIGHT_RED = 101,
  BG_LIGHT_GREEN = 102,
  BG_LIGHT_YELLOW = 103,
  BG_LIGHT_BLUE = 104,
  BG_LIGHT_MAGENTA = 105,
  BG_LIGHT_CYAN = 106,
  BG_WHITE = 107,
};

auto operator<<(std::ostream &os, ANSICode code) -> std::ostream & {
  return os << "\033[" << static_cast<int>(code) << "m";
}

} // namespace Color

class IPv4Address {
  bool COLORED = false;

  class Exception : public std::exception {
  public:
    explicit Exception(std::string_view message) : m_message(message) {}

    [[nodiscard]] auto what() const noexcept -> const char * override {
      return m_message.c_str();
    }

  private:
    std::string m_message;
  };

  struct network {
    unsigned int ip;
    unsigned int netmask;
    unsigned int network;
    unsigned int hostMin;
    unsigned int hostMax;
    unsigned int broadcast;
    unsigned int hostCount;
  };

public:
  IPv4Address() = default;

  auto processIPv4(const std::string &address, const std::string &mask)
      -> void {
    network net{};
    int prefix = toPrefix(mask);
    unsigned int ip = toNumber(address);
    unsigned int netmask = toNetmask(prefix);

    assignNetwork(net, ip, netmask);

    printBytes("Address:", net.ip);
    printBytes("Netmask:", net.netmask);
    std::cout << "CIDR prefix:\t/" << prefix << '\n';
    printNetwork(net);
  }

  auto processIPv4WithSubnets(const std::string &address,
                              const std::string &mask,
                              const std::string &subnetMask) -> void {
    std::vector<network> subnets;
    int prefix = toPrefix(mask);
    unsigned int netmask = toNetmask(prefix);
    int subnetPrefix = toPrefix(subnetMask);
    int subnetCount = getSubnetCount(prefix, subnetPrefix);

    if (subnetPrefix <= prefix) {
      throw Exception("The subnet mask must be larger than the mask!");
    }

    processIPv4(address, mask);

    std::cout << "\nSubnets after transition from " << mask << " to "
              << subnetMask << "\n\n";
    netmask = toNetmask(subnetPrefix);
    printBytes("Netmask:", netmask);
    std::cout << "CIDR prefix:\t/" << subnetPrefix << '\n';

    for (int i = 0; i < subnetCount; i++) {
      network net{};
      unsigned int ip = 0;
      if (i == 0) {
        ip = toNumber(address);
      } else {
        ip = subnets[i - 1].broadcast + 1;
      }
      assignNetwork(net, ip, netmask);
      subnets.push_back(net);
    }

    printSubnets(subnets);
  }

  auto colorOutput(bool colored) -> void { this->COLORED = colored; }

  static auto isContinuous(std::bitset<IPV4_BITS> bitset) -> bool {
    for (int i = 0; i < IPV4_BITS; i++) {
      if (bitset.test(i)) {
        for (int j = i; j < IPV4_BITS - i; j++) {
          if (!bitset.test(j)) {
            return false;
          }
        }
      }
    }
    return true;
  }

  static auto toPrefix(const std::string &cidr) -> int {
    int prefix = 0;
    // check if the format is x.x.x.x
    if (cidr.find('.') != std::string::npos) {
      unsigned int mask = toNumber(cidr, "mask");
      std::bitset<IPV4_BITS> bitset(mask);
      if (!isContinuous(bitset)) {
        throw Exception("The mask is non-continuous!");
      }
      if (bitset.count() < 1 || bitset.count() > IPV4_BITS) {
        throw Exception("The mask must be <=32 and >0!");
      }
      return bitset.count();
    }

    // else assume the format is /x
    if (sscanf(cidr.c_str(), "/%u", &prefix) != 1) {
      throw Exception("Invalid mask: " + cidr);
    }
    if (prefix <= 0 || prefix > IPV4_BITS) {
      throw Exception("The mask must be <=32 and >0!");
    }
    return prefix;
  }

  // 'type' is here to make the error feedback more useful to the user
  static auto toNumber(const std::string &str, const std::string &type = "IP")
      -> unsigned int {
    unsigned int bytes[IPV4_BYTES] = {0};

    if (sscanf(str.c_str(), "%d.%d.%d.%d", &bytes[3], &bytes[2], &bytes[1],
               &bytes[0]) != 4) {
      throw Exception("Invalid " + type + " format: " + str);
    }

    for (auto byte : bytes) {
      if (byte < 0 || byte > BYTE_MAX) {
        throw Exception(type + " bytes must be >= 0 and <= " +
                        std::to_string(BYTE_MAX) + ": " + std::to_string(byte));
      }
    }

    return bytes[0] | bytes[1] << ONE_BYTE | bytes[2] << 2 * ONE_BYTE |
           bytes[3] << 3 * ONE_BYTE;
  }

  static auto toNetwork(const unsigned int &ip, const unsigned int &mask)
      -> unsigned int {
    return ip & mask;
  }

  static auto toNetmask(const unsigned int &prefix) -> unsigned int {
    return (BASE_MASK << (IPV4_BITS - prefix)) & BASE_MASK;
  }

  static auto toBroadcast(const unsigned int &network, const unsigned int &mask)
      -> unsigned int {
    return network + (~mask);
  }

  auto printBytes(const std::string &message, const unsigned int &thing) const
      -> void {
    std::vector<std::bitset<ONE_BYTE>> bytes(IPV4_BYTES);

    for (int i = 0; i < 4; i++) {
      bytes[i] = thing >> i * ONE_BYTE;
    }
    std::cout << message << "\t";
    if (this->COLORED) {
      std::cout << Color::FG_LIGHT_BLUE;
    }
    for (int i = bytes.size() - 1; i >= 0; i--) {
      if (i != 0) {
        std::cout << bytes[i].to_ulong() << '.';
      } else {
        std::cout << bytes[i].to_ulong() << "\t\t";
      }
    }
    if (this->COLORED) {
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
    if (this->COLORED) {
      std::cout << Color::FG_DEFAULT;
    }
  }

  static auto getSubnetCount(const int &prefix, const int &subnetPrefix)
      -> int {
    return pow(2, IPV4_BITS - prefix) / pow(2, IPV4_BITS - subnetPrefix);
  }

  auto printNetwork(const network &net) -> void {
    printBytes("Network:", net.network);
    printBytes("HostMin:", net.hostMin);
    printBytes("HostMax:", net.hostMax);
    printBytes("Broadcast:", net.broadcast);
    std::cout << "Hosts/Net:\t" << net.hostCount << '\n';
  }

  static auto assignNetwork(network &net, const unsigned int &ip,
                            const unsigned int &netmask) -> void {
    net.ip = ip;
    net.netmask = netmask;
    net.network = toNetwork(net.ip, net.netmask);
    net.hostMin = net.network + 1;
    net.broadcast = toBroadcast(net.network, net.netmask);
    net.hostMax = net.broadcast - 1;
    net.hostCount = net.broadcast - net.hostMin;
  }

  auto printSubnets(const std::vector<network> &subnets) -> void {
    for (int i = 0; i < subnets.size(); i++) {
      std::cout << '\n' << i + 1 << ".\n";
      printNetwork(subnets[i]);
    }
  }
};

auto main(int argc, char *argv[]) -> int {
  IPv4Address ip;
  CppClip::ArgumentParser ipcalctool("ipcalctool");
  ipcalctool.add("subnet").help("A subnet mask in '/xx' format");
  ipcalctool.add("mask").help("A mask in '/xx' format").nargs(1);
  ipcalctool.add("address").help("An IPv4 address").nargs(1);
  ipcalctool.add("-c", "--colored").help("Color output stream");
  ipcalctool.add("-h", "--help").help("Display help");
  ipcalctool.addEpilogue("Example inputs:\n"
                         "  ipcalctool 192.168.0.1 /24 /25 -c\n"
                         "  ipcalctool 192.168.0.1 /17 /19");
  try {
    ipcalctool.parse(argc, argv);

    if (ipcalctool.isSet("-h") || ipcalctool.argsEmpty()) {
      ipcalctool.printHelp();
      exit(0);
    }

    if (ipcalctool.isSet("-c")) {
      ip.colorOutput(true);
    }

    const std::vector<std::string> &address =
        ipcalctool.getPositional("address");
    const std::vector<std::string> &mask = ipcalctool.getPositional("mask");
    const std::vector<std::string> &subnetMask =
        ipcalctool.getPositional("subnet");

    if (!subnetMask.empty()) {
      ip.processIPv4WithSubnets(address.at(0), mask.at(0), subnetMask.at(0));
      exit(0);
    }
    ip.processIPv4(address.at(0), mask.at(0));

  } catch (const std::exception &e) {
    ipcalctool.printHelp();
    std::cerr << "\nError: " << e.what() << '\n';
    exit(1);
  }
}
