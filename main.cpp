#include "CppClip.hpp"
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

bool COLORED = false;

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

std::ostream &operator<<(std::ostream &os, ANSICode code) {
  return os << "\033[" << static_cast<int>(code) << "m";
}

} // namespace Color

class IPv4Address {
public:
  IPv4Address(){};

  void processIPv4(const std::string &address, const std::string &mask) {
    network net;
    int prefix = getPrefix(mask);
    unsigned long ip = getNum(address);
    unsigned long netmask = (0xFFFFFFFF << (32 - getPrefix(mask))) & 0xFFFFFFFF;
    assignNetwork(net, ip, netmask);

    printBytes("Address:", net.ip);
    printBytes("Netmask:", net.netmask);
    std::cout << "CIDR prefix:\t/" << prefix << '\n';
    printNetwork(net);
  }

  void processSubnets(const std::string &address, const std::string &mask,
                      const std::string &subnetMask) {
    std::vector<network> subnets;
    unsigned long netmask = (0xFFFFFFFF << (32 - getPrefix(mask))) & 0xFFFFFFFF;
    int prefix = getPrefix(mask);
    int subnetPrefix = getPrefix(subnetMask);
    int subnetCount = getSubnetCount(prefix, subnetPrefix);

    if (subnetPrefix <= prefix) {
      std::cerr << "Subnet prefix must be larger than the prefix!\n";
      exit(1);
    }

    processIPv4(address, mask);

    std::cout << "\nSubnets after transition from " << mask << " to "
              << subnetMask << "\n\n";
    netmask = (0xFFFFFFFF << (32 - subnetPrefix)) & 0xFFFFFFFF;
    printBytes("Netmask:", netmask);
    std::cout << "CIDR prefix:\t/" << subnetPrefix << '\n';

    for (int i = 0; i < subnetCount; i++) {
      network net;
      unsigned long ip;
      if (i == 0) {
        ip = getNum(address);
      } else {
        ip = subnets[i - 1].broadcast + 1;
      }
      assignNetwork(net, ip, netmask);
      subnets.push_back(net);
    }

    printSubnets(subnets);
  }

private:
  struct network {
    unsigned long ip;
    unsigned long netmask;
    unsigned long network;
    unsigned long hostMin;
    unsigned long hostMax;
    unsigned long broadcast;
    int hostCount;
  };

  int getPrefix(const std::string &cidr) {
    int prefix;
    sscanf(cidr.c_str(), "/%u", &prefix);
    return prefix;
  }

  unsigned long getNum(const std::string &ip) {
    unsigned int bytes[4] = {0};
    sscanf(ip.c_str(), "%d.%d.%d.%d", &bytes[3], &bytes[2], &bytes[1],
           &bytes[0]);
    return bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;
  }

  unsigned long getNetwork(const unsigned long &ip, const unsigned long &mask) {
    return ip & mask;
  }

  unsigned long getBroadcast(const unsigned long &network,
                             const unsigned long &mask) {
    return network + (~mask);
  }

  void printBytes(const std::string &message, const unsigned long &thing) {
    std::vector<std::bitset<8>> bytes(4);
    for (int i = 0; i < 4; i++) {
      bytes[i] = thing >> i * 8;
    }
    std::cout << message << "\t";
    if (COLORED) {
      std::cout << Color::FG_LIGHT_BLUE;
    }
    for (int i = bytes.size() - 1; i >= 0; i--) {
      if (i != 0)
        std::cout << bytes[i].to_ulong() << '.';
      else
        std::cout << bytes[i].to_ulong() << "\t\t";
    }
    if (COLORED) {
      std::cout << Color::FG_DEFAULT;
      std::cout << Color::FG_LIGHT_YELLOW;
    }
    for (int i = bytes.size() - 1; i >= 0; i--) {
      if (i != 0)
        std::cout << bytes[i] << '.';
      else
        std::cout << bytes[i] << '\n';
    }
    if (COLORED) {
      std::cout << Color::FG_DEFAULT;
    }
  }

  int getSubnetCount(const int &prefix, const int &subnetPrefix) {
    int a = std::pow(2, 32 - prefix) / pow(2, 32 - subnetPrefix);
    return a;
  }

  void printNetwork(const network &net) {
    printBytes("Network:", net.network);
    printBytes("HostMin:", net.hostMin);
    printBytes("HostMax:", net.hostMax);
    printBytes("Broadcast:", net.broadcast);
    std::cout << "Hosts/Net:\t" << net.hostCount << '\n';
  }

  void assignNetwork(network &net, const unsigned long &ip,
                     const unsigned long &netmask) {
    net.ip = ip;
    net.netmask = netmask;
    net.network = getNetwork(net.ip, net.netmask);
    net.hostMin = net.network + 1;
    net.broadcast = getBroadcast(net.network, net.netmask);
    net.hostMax = net.broadcast - 1;
    net.hostCount = net.broadcast - net.hostMin;
  }

  void printSubnets(const std::vector<network> &subnets) {
    for (int i = 0; i < subnets.size(); i++) {
      std::cout << '\n' << i + 1 << ".\n";
      printNetwork(subnets[i]);
    }
  }
};

int main(int argc, char *argv[]) {
  IPv4Address ip;
  ArgumentParser ipcalctool("ipcalctool");
  ipcalctool.add("subnet").help("A subnet mask in '/xx' format");
  ipcalctool.add("mask").help("A mask in '/xx' format");
  ipcalctool.add("address").help("An IPv4 address");
  ipcalctool.add("-c", "--colored").help("Color output stream");
  ipcalctool.add("-h", "--help").help("Display help");
  ipcalctool.addEpilogue("Example inputs:\n"
                         "  ipcalctool 192.168.0.1 /24 /25 -c\n"
                         "  ipcalctool 192.168.0.1 /17 /19");
  ipcalctool.parse(argc, argv);

  if (ipcalctool.isSet("-h") || ipcalctool.argsEmpty()) {
    ipcalctool.printHelp();
    exit(0);
  }

  if (ipcalctool.isSet("-c")) {
    COLORED = true;
  }

  const std::vector<std::string> &address = ipcalctool.getPositional("address");
  const std::vector<std::string> &mask = ipcalctool.getPositional("mask");
  const std::vector<std::string> &subnetMask =
      ipcalctool.getPositional("subnet");

  for (const std::string &s : address) {
  }

  if (!subnetMask.empty()) {
    ip.processSubnets(address.at(0), mask.at(0), subnetMask.at(0));
  } else {
    ip.processIPv4(address.at(0), mask.at(0));
  }
}
