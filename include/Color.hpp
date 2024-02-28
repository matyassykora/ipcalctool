#pragma once
#include <iostream>

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

/*
 * a helper for coloring text with ANSI color codes
 *
 * example: std::cout << Color::FG_LIGHT_BLUE << "some text" <<
 * Color::FG_DEFAULT;
 */
auto operator<<(std::ostream &os, ANSICode code) -> std::ostream & {
  return os << "\033[" << static_cast<int>(code) << "m";
}

} // namespace Color
