#pragma once

#include <string>
#include <string_view>
#include <exception>

class Exception : public std::exception {
public:
  explicit Exception(std::string_view message) : m_message(message) {}

  [[nodiscard]] auto what() const noexcept -> const char * override {
    return m_message.c_str();
  }

private:
  std::string m_message;
};
