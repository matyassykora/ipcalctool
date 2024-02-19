#include "../include/Network.hpp"
#include "../include/ipcalctool.hpp"
#include "../include/IPv4Address.hpp"

#include <iostream>

namespace ipcalctool {

Network::Network(unsigned int ip, unsigned int netmask) {
  this->ip = ip;
  this->netmask = netmask;
  this->network = IPv4Address::toNetwork(this->ip, this->netmask);
  this->hostMin = this->network + 1;
  this->broadcast = IPv4Address::toBroadcast(this->network, this->netmask);
  this->hostMax = this->broadcast - 1;
  this->hostCount = this->broadcast - this->hostMin;
}

void Network::print(bool colored) const {
  printBytes("Network:", this->network, colored);
  printBytes("HostMin:", this->hostMin, colored);
  printBytes("HostMax:", this->hostMax, colored);
  printBytes("Broadcast:", this->broadcast, colored);
  std::cout << "Hosts/Net:\t" << this->hostCount << '\n';
}

} // namespace ipcalctool
