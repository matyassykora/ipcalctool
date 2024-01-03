#pragma once

#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace CppClip {

class Exception : public std::exception {
public:
  explicit Exception(std::string_view message) : m_message(message) {}

  [[nodiscard]] auto what() const noexcept -> const char * override {
    return m_message.c_str();
  }

private:
  std::string m_message;
};

struct Argument {
  std::string shortOpt;
  std::string longOpt;
  std::string positionalOpt;
  std::string helpMessage;
  int nargs = 0;
  bool isSet = false;
  bool isPositional = false;
  bool isOptional = true;
};

class ArgumentParser {
public:
  ArgumentParser(std::string_view programName) : programName(programName){};

  auto add(const std::string &option, const std::string &longOpt = "")
      -> ArgumentParser & {

    if (option.empty()) {
      throw Exception("You must add at least one argument");
    }

    this->currentOption = option;

    Argument arg;
    argumentMap.try_emplace(option, arg);
    Argument &argument = argumentMap.at(option);

    if (isPositionalOpt(option)) {
      if (!longOpt.empty()) {
        throw Exception("When setting a positional option, the second option "
                        "must not be set");
      }
      argument.positionalOpt = option;
      argument.isPositional = true;
      argument.nargs = 1;
      return *this;
    }

    // both long and short
    if (!longOpt.empty()) {
      if (longOpt.find("--") == std::string::npos) {
        throw Exception("Long option must start with --");
      }
      if (option.find("--") != std::string::npos) {
        throw Exception("When setting both options, the first option must not "
                        "start with --");
      }
      argument.shortOpt = option;
      argument.longOpt = longOpt;
      return *this;
    }

    // only long
    if (option.find("--") != std::string::npos) {
      argument.longOpt = option;
      return *this;
    }

    // only short
    if (!option.empty()) {
      argument.shortOpt = option;
      return *this;
    }

    throw Exception("Something went wrong when adding an argument");
  }

  auto help(const std::string &message) -> ArgumentParser & {
    if (message.empty()) {
      return *this;
    }
    argumentMap.at(currentOption).helpMessage = message;
    return *this;
  }

  auto nargs(const int &nargs) -> ArgumentParser & {
    if (nargs <= 0) {
      throw Exception("Number of arguments must be >0");
    }
    auto &arg = argumentMap.at(currentOption);
    arg.nargs = nargs;
    arg.isPositional = true;
    arg.isOptional = false;
    return *this;
  }

  static auto isPositionalOpt(std::string_view option) -> bool {
    return option.at(0) != '-';
  }

  void addDescription(std::string_view description) {
    if (description.empty()) {
      return;
    }
    this->programDescription = description;
  }

  void addEpilogue(std::string_view epilogue) {
    this->programEpilogue = epilogue;
  }

  void parse(const int &argc, char **argv) {
    for (int i = 1; i < argc; i++) {
      this->args.emplace_back(argv[i]);
    }

    for (const auto &arg : this->args) {
      if (arg.find("--") != std::string::npos) {
        checkUnrecognized(arg);
        findLong(arg);
        continue;
      }
      parseShort(arg);
    }
  }

  auto getPositional(std::string_view option)
      -> std::vector<std::string> {
    std::vector<std::string> vec;
    std::vector<std::string> all = getAllPositional();
    for (const auto &[key, opt] : argumentMap) {
      if (!opt.isPositional) {
        continue;
      }
      if (option != opt.positionalOpt) {
        continue;
      }
      for (int j = 0; j < opt.nargs; j++) {
        if (positionalIndex >= all.size() && opt.isOptional) {
          continue;
        }
        if (positionalIndex >= all.size() && !opt.isOptional) {
          throw Exception("Not enough arguments to " + opt.positionalOpt);
        }
        vec.push_back(all.at(positionalIndex));
        positionalIndex++;
      }
    }

    return vec;
  }

  auto existsInMap(std::string_view option) -> bool {
    return std::any_of(
        argumentMap.begin(), argumentMap.end(), [&](const auto &pair) {
          return option == pair.second.shortOpt ||
                 option == pair.second.longOpt ||
                 option == pair.second.positionalOpt || pair.first == option;
        });
  }

  auto isSet(std::string_view option) -> bool {
    auto iter = std::find_if(
        argumentMap.begin(), argumentMap.end(), [&](const auto &pair) {
          return option.size() == pair.second.shortOpt.size() &&
                 option == pair.second.shortOpt && pair.second.isSet;
        });
    return iter != argumentMap.end();
  }

  void printHelp(bool expl = true) {
    std::cout << "Usage: " << programName << " ";
    if (expl) {
      printShortOptions();
      printLongOptions();
      printPositionalOptions();
    } else {
      std::cout << "[OPTION]... [ARGUMENT]...";
    }
    std::cout << '\n';

    printProgramDescription();

    std::cout << "\nOptions:\n";
    printOptions();

    std::cout << "\nPositional arguments:\n";
    printPositionalArguments();

    printProgramEpilogue();
  }

  void printShortOptions() {
    if (argumentMap.empty()) {
      return;
    }
    std::cout << "[-";
    for (const auto &[key, opt] : argumentMap) {
      if (opt.shortOpt.empty()) {
        continue;
      }
      for (const char &c : opt.shortOpt) {
        if (c == '-') {
          continue;
        }
        std::cout << c;
      }
    }
    std::cout << "] ";
  }

  void printLongOptions() {
    for (const auto &[key, opt] : argumentMap) {
      if (opt.shortOpt.empty() && !opt.longOpt.empty()) {
        std::cout << "[" << opt.longOpt << "] ";
      }
    }
  }

  void printPositionalOptions() {
    for (const auto &[key, opt] : argumentMap) {
      if (opt.positionalOpt.empty()) {
        continue;
      }
      if (opt.isOptional) {
        std::cout << "[" << opt.positionalOpt << "] ";
        continue;
      }
      std::cout << opt.positionalOpt << " ";
    }
  }

  void printProgramDescription() {
    if (!this->programDescription.empty()) {
      std::cout << '\n' << programDescription << '\n';
    }
  }

  void printOptions() {
    for (const auto &[key, opt] : argumentMap) {
      std::stringstream optionStream;
      std::stringstream helpStream;
      optionStream << "  " << opt.shortOpt;
      if (!opt.longOpt.empty()) {
        optionStream << ", " << opt.longOpt;
      }

      if (opt.isPositional) {
        continue;
      }
      std::cout << std::setw(FORMAT_WIDTH) << std::left << optionStream.str()
                << std::right << opt.helpMessage << "\n";
    }
  }

  void printPositionalArguments() {
    for (const auto &[key, opt] : argumentMap) {
      std::stringstream positional;
      if (!opt.isPositional) {
        continue;
      }
      if (opt.isOptional) {
        positional << "  [" << opt.positionalOpt << "]";
      } else {
        positional << "  " << opt.positionalOpt;
      }
      positional << " (" << opt.nargs << ')';
      std::cout << std::setw(FORMAT_WIDTH) << std::left << positional.str()
                << std::right << opt.helpMessage << '\n';
    }
  }

  void printProgramEpilogue() {
    if (!this->programEpilogue.empty()) {
      std::cout << '\n' << this->programEpilogue << '\n';
    }
  }

  auto argsEmpty() -> bool { return args.empty(); }

  auto getArgument(const std::string &option) -> Argument & {
    return argumentMap.at(option);
  }

  void allowUnrecognized() { unrecognizedAllowed = true; }

private:
  auto getAllPositional() -> std::vector<std::string> {
    std::vector<std::string> vec;
    for (const auto &arg : args) {
      if (arg.front() != '-') {
        vec.push_back(arg);
      }
    }
    return vec;
  }

  void findLong(const std::string &arg) {
    for (auto &[key, opt] : this->argumentMap) {
      if (opt.longOpt == arg || key == arg) {
        opt.isSet = true;
      }
    }
  }

  void parseShort(const std::string &arg) {
    if (arg.front() != '-') {
      return;
    }
    for (const char &c : arg) {
      if (c != '-') {
        checkUnrecognized(std::string(1, '-').append(1, c));
      }
      for (auto &[key, opt] : this->argumentMap) {
        if (opt.shortOpt.size() <= 1) {
          continue;
        }
        if (opt.shortOpt.at(1) == c) {
          opt.isSet = true;
        }
      }
    }
  }

  void checkUnrecognized(const std::string &arg) {
    if (!existsInMap(arg)) {
      this->unrecognizedFound = true;
    }
    if (this->unrecognizedFound && !this->unrecognizedAllowed) {
      throw Exception("Unrecognized option: " + arg);
    }
  }

  static const int FORMAT_WIDTH = 30;

  bool unrecognizedFound = false;
  bool unrecognizedAllowed = false;
  unsigned long positionalIndex = 0;

  std::vector<std::string> args;
  std::string programDescription;
  std::string programEpilogue;

  std::string currentOption;

  std::string programName;
  std::map<std::string, Argument> argumentMap;
};
} // namespace CppClip
