#include "../include/ipcalctool.hpp"
#include "../include/Color.hpp"
#include "../include/IPv4Address.hpp"

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

namespace ipcalctool {

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
} // namespace ipcalctool
