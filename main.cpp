#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

void printHelp(const std::string &message = "") {
  if (!message.empty()) {
    std::cout << message << '\n';
  }
  std::cout << "Usage: ipcalc -a <ip address> -m [/prefix] -s [/prefix] "
               "{options}\n\n"
            << "Options:\n"
            << "  -c\tDisplay colored output.\n"
            << "  -h\tPrint this help.\n\n"
            << "Example inputs:\n"
            << "  ipcalc -a 192.168.0.1 -m /24 -c\n"
            << "  ipcalc -a 192.168.0.1 -m /17 -s /19\n"
            << std::endl;
}

class parseInput {
public:
  parseInput(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
      this->args.push_back(std::string(argv[i]));
    }
  }

  bool optionExists(const std::string &option) {
    return std::find(this->args.begin(), this->args.end(), option) !=
           this->args.end();
  }

  const std::string &getOption(const std::string &option) {
    std::vector<std::string>::const_iterator iter;
    iter = std::find(this->args.begin(), this->args.end(), option);

    if (iter != this->args.end() && ++iter != this->args.end()) {
      return *iter;
    }

    static const std::string empty = "";
    return empty;
  }

private:
  std::vector<std::string> args;
};

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
      printHelp("Subnet prefix must be larger than the prefix!");
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
    for (int i = bytes.size() - 1; i >= 0; i--) {
      if (i != 0)
        std::cout << bytes[i].to_ulong() << '.';
      else
        std::cout << bytes[i].to_ulong() << "\t\t";
    }
    for (int i = bytes.size() - 1; i >= 0; i--) {
      if (i != 0)
        std::cout << bytes[i] << '.';
      else
        std::cout << bytes[i] << '\n';
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
  parseInput input(argc, argv);
  IPv4Address ip;

  if (input.optionExists("-h") || input.optionExists("--help")) {
    printHelp();
    exit(0);
  }

  const std::string &address = input.getOption("-a");
  const std::string &mask = input.getOption("-m");
  const std::string &subnetMask = input.getOption("-s");

  if (address.empty()) {
    printHelp("No IP address provided!");
    exit(1);
  }

  if (mask.empty()) {
    printHelp("No mask provided!");
    exit(1);
  }

  if (!subnetMask.empty()) {
    ip.processSubnets(address, mask, subnetMask);
  } else {
    ip.processIPv4(address, mask);
  }
}
