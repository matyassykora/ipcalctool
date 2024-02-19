#pragma once

namespace ipcalctool {

class Network {
public:
  unsigned int ip;
  unsigned int netmask;
  unsigned int network;
  unsigned int hostMin;
  unsigned int hostMax;
  unsigned int broadcast;
  unsigned int hostCount;

  Network(unsigned int ip, unsigned int netmask);

  void print(bool colored) const;
};

} // namespace ipcalctool
