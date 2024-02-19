#pragma once

#include <string>

namespace ipcalctool {

void printBytes(const std::string &message, const unsigned int &thing,
                bool colored);

const int IPV4_BITS = 32;
const int ONE_BYTE = 8;
const int IPV4_BYTES = IPV4_BITS / ONE_BYTE;
const int BYTE_MAX = 0xFF;
const unsigned int BASE_MASK = 0xFFFFFFFF;
} // namespace ipcalctool
