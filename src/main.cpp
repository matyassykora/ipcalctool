#include "../include/CppClip.hpp"
#include "../include/IPv4Address.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  ipcalctool::IPv4Address ip;
  CppClip::ArgumentParser ipcalctool("ipcalctool");
  ipcalctool.add("subnet").help("A subnet mask in '/x' or 'x.x.x.x' format");
  ipcalctool.add("mask").help("A mask in '/x' or 'x.x.x.x' format").nargs(1);
  ipcalctool.add("address").help("An IPv4 address").nargs(1);
  ipcalctool.add("-c", "--colored").help("Color output stream");
  ipcalctool.add("-h", "--help").help("Display help");
  ipcalctool.addEpilogue("Example inputs:\n"
                         "  ipcalctool 192.168.0.1 /24 \n"
                         "  ipcalctool 192.168.0.1 /24 /25 -c\n"
                         "  ipcalctool 192.168.0.1 255.255.0.0 /17 ");
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
