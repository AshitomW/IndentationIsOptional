#pragma once


#include <iio/utils.hpp>
#include <string>
#include <string_view>
#include <istream>
#include <optional>
#include <vector>



namespace iio{
  class Checker{

    private:
    int depth_ = 0; 
    int line_num_ = 0;


    public:
      Checker() = default;
      Checker(const Checker&) = delete;
      Checker& operator=(const Checker&) = delete;
      Checker(Checker&&) = default;
      Checker& operator=(Checker&&) = default;

    
      [[nodiscard]] std::optional<Diagnostic> feed_line(std::string_view line);
      [[nodiscard]] std::optional<Diagnostic> finalize() const;
      [[nodiscard]] int depth() const noexcept {return depth_;}
      [[nodiscard]] int line_number() const noexcept {return line_num_;}
      void reset() noexcept;

    private:
      [[nodiscard]] bool has_inline_block_braces(std::string_view line) const;

  };


  [[nodiscard]] std::optional<Diagnostic> check_stream(std::istream& input);
  [[nodiscard]] std::optional<Diagnostic> check_file(const std::string& filepath);

}