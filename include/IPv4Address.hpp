#include "Network.hpp"

#include <string>
#include <vector>

namespace ipcalctool {

class IPv4Address {
  bool COLORED = false;

  enum class valueTypes {
    MASK = 0,
    IP_ADDRESS,
  };

public:
  IPv4Address() = default;

  void processIPv4(const std::string &address, const std::string &mask) const;

  void processIPv4WithSubnets(const std::string &address,
                              const std::string &mask,
                              const std::string &subnetMask) const;

  void colorOutput(bool colored);

  static void validateMask(unsigned int mask, const std::string &maskString);

  /*
   * Return the prefix (the number of set bits) from a mask string in '/x' or
   * 'x.x.x.x' format
   *
   * Throws if input is invalid
   */
  static auto toPrefix(const std::string &maskString) -> int;

  /**
   * Convert a string in 'x.x.x.x' format to a number
   *
   * 'type' is here to make the error feedback more useful to the user
   */
  static auto toNumber(const std::string &str, const valueTypes &type)
      -> unsigned int;

  static auto toNetwork(const unsigned int &ip, const unsigned int &mask)
      -> unsigned int;

  static auto toNetmask(const unsigned int &prefix) -> unsigned int;

  static auto toBroadcast(const unsigned int &network, const unsigned int &mask)
      -> unsigned int;

  static auto getSubnetCount(const int &prefix, const int &subnetPrefix) -> int;

  void printSubnets(const std::vector<Network> &subnets) const;
};
} // namespace ipcalctool
