
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

  template <typename T>
  [[nodiscard]] inline const T& get_ok(const Result<T>& r){
    return std::get<T>(r);
  }


  template <typename T>
  [[nodiscard]] inline const Diagnostic& get_err(const Result<T>& r){
    return std::get<Diagnostic>(r);
  }

  [[nodiscard]] std::string make_indent(int level);
  [[nodiscard]] std::string_view trim_left(std::string_view sv);
  [[nodiscard]] std::string_view trim_right(std::string_view sv);
  [[nodiscard]] std::string_view trim(std::string_view sv);
  [[nodiscard]] bool is_blank(std::string_view sv);
  [[nodiscard]] std::size_t leading_whitespace_count(std::string_view sv);
  [[nodiscard]] bool is_inside_string(std::string_view line, std::size_t pos);
  [[nodiscard]] std::size_t find_brace_outside_string(std::string_view line, char brace);
  [[nodiscard]] int count_braces_outside_string(std::string_view line,char brace);
}