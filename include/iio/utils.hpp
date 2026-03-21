
#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <variant>
#include <optional>


namespace iio{
  inline constexpr int INDENT_WIDTH = 4;
  inline constexpr int MAX_INDENT_DEPTH = 256;
  struct Diagnostic{
    int line;  
    int column;
    std::string message;
    [[nodiscard]] std::string format() const;
  };
  template <typename T>
  using Result = std::variant<T,Diagnostic>;


  template <typename T>
  [[nodiscard]] inline bool is_ok(const Result<T>& r){
    return std::holds_alternative<T>(r);
  }

}