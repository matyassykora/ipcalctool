#pragma once
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class ArgumentParser {
public:
  ~ArgumentParser() = default;
  ArgumentParser(const std::string &programName) {
    this->programName = programName;
  };

  // FIX: needing to add arguments in reverse?
  ArgumentParser &add(const std::string &option,
                      const std::string &longOpt = "") {
    id++;

    // positional
    if (option.at(0) != '-') {
      a[id].positionalOpt = option;
      a[id].isPositional = true;
      a[id].nargs = 1;
      return *this;
    }

    // only long
    if (option.find("--") != std::string::npos) {
      a[id].longOpt = option;
      return *this;
    }

    // both long and short
    if (!longOpt.empty()) {
      a[id].shortOpt = option;
      a[id].longOpt = longOpt;
      return *this;
    }

    // only short
    if (!option.empty()) {
      a[id].shortOpt = option;
    }

    return *this;
  }

  ArgumentParser &help(const std::string &message) {
    a[id].helpMessage = message;
    return *this;
  }

  ArgumentParser &nargs(int nargs) {
    a[id].nargs = nargs;
    a[id].isPositional = true;
    return *this;
  }

  void addDescription(const std::string &description) {
    this->description = description;
  }

  void addEpilogue(const std::string &epilogue) { this->epilogue = epilogue; }

  void parse(const int &argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
      this->args.push_back(std::string(argv[i]));
    }
    for (const auto &arg : args) {
      for (const char &c : arg) {
        for (auto &pair : a) {
          if (pair.second.shortOpt.size() <= 1) {
            continue;
          }
          if (pair.second.shortOpt.at(1) == c) {
            pair.second.isSet = true;
          }
        }
      }
    }
  }

  std::vector<std::string> getPositional(const std::string &option) {
    std::vector<std::string> vec;
    std::vector<std::string> all = getAllPositional();
    for (auto i : a) {
      if (!i.second.isPositional) {
        continue;
      }
      if (option != i.second.positionalOpt) {
        continue;
      }
      for (int j = 0; j < i.second.nargs; j++) {
        if (positionalIndex >= all.size()) {
          std::cout << "Not enough arguments to \"" << i.second.positionalOpt
                    << "\"!\n";
          exit(1);
        }
        vec.push_back(all.at(positionalIndex));
        positionalIndex++;
      }
    }

    return vec;
  }

  bool isSet(std::string option) {
    for (auto pair : a) {
      if (option.size() != pair.second.shortOpt.size()) {
        continue;
      }
      if (option == pair.second.shortOpt && pair.second.isSet) {
        return true;
      }
    }
    return false;
  }

  void printHelp() {
    std::cout << "Usage: " << programName << " ";

    // print short options
    for (auto i : a) {
      if (i.second.shortOpt.empty()) {
        continue;
      }
      std::cout << "[" << i.second.shortOpt << "] ";
    }

    // print long options
    for (auto i : a) {
      if (i.second.shortOpt.empty() && !i.second.longOpt.empty()) {
        std::cout << "[" << i.second.longOpt << "] ";
      }
    }

    // print positional options
    for (auto i : a) {
      if (i.second.positionalOpt.empty()) {
        continue;
      }
      std::cout << i.second.positionalOpt << " ";
    }

    std::cout << '\n';

    if (!this->description.empty()) {
      std::cout << "\n" << description << "\n";
    }

    std::cout << "\nOptions:\n";
    for (const auto &i : a) {
      std::stringstream optionStream;
      std::stringstream helpStream;
      optionStream << "  " << i.second.shortOpt;
      if (!i.second.longOpt.empty()) {
        optionStream << ", " << i.second.longOpt;
      }

        if (i.second.isPositional) {
          continue;
        }
        std::cout << std::setw(30) << std::left << optionStream.str()
                  << std::right << i.second.helpMessage << "\n";
    }

    std::cout << "\nPositional arguments:\n";
    for (const auto &i : a) {
      std::stringstream positional;
      if (!i.second.isPositional) {
        continue;
      }
      positional << "  " << i.second.positionalOpt << " (" << i.second.nargs
                 << ')';
      std::cout << std::setw(30) << std::left << positional.str() << std::right
                << i.second.helpMessage << "\n";
    }

    if (!this->epilogue.empty()) {
      std::cout << '\n' << this->epilogue << "\n";
    }
  }

  bool argsEmpty() { return args.size() == 0; }

  // TODO: make sure arguments get correctly consumed after running this
  std::vector<std::string> getArgsAfter(const std::string &option) {
    std::vector<std::string> vec;
    std::vector<std::string>::const_iterator itr;
    int id = mapIDFromArg(option);
    const auto &map = a[id];
    int nargs = map.nargs;
    if (id == -1) {
      std::cout << "These arguments don't exist!\n";
      exit(1);
    }
    itr = std::find(this->args.begin(), this->args.end(), option);
    for (int i = 0; i < nargs; i++) {
      if (itr != this->args.end() && ++itr != this->args.end()) {
        vec.push_back(*itr);
      }
    }
    return vec;
  }

private:
  int mapIDFromArg(const std::string &j) {
    int id = -1;

    for (const std::pair<const int, argument> &i : a) {
      const std::string &opts = i.second.shortOpt;
      if (i.second.shortOpt == j) {
        id = i.first;
        return id;
      }
    }

    return id;
  }

  std::vector<std::string> getAllPositional() {
    std::vector<std::string> vec;
    for (auto i : args) {
      if (i[0] != '-') {
        vec.push_back(i);
      }
    }
    return vec;
  }

private:
  struct argument {
    std::string shortOpt = "";
    bool isSet = false;
    std::string longOpt = "";
    std::string positionalOpt = "";
    std::vector<std::string> options;
    std::string helpMessage = "";
    bool isPositional = false;
    int nargs = 0;
    std::string metavar = "";
    std::vector<std::string> data;
  };

  int positionalIndex = 0;

  std::vector<std::string> args;
  std::string description = "";
  std::string epilogue = "";

  std::string programName;
  int id = -1;
  std::unordered_map<int, argument> a;
};
