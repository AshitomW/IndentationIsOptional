#pragma once

#include <iio/utils.hpp>
#include <iio/checker.hpp>
#include <string>
#include <string_view>
#include <istream>
#include <ostream>
#include <functional>


namespace iio{
  enum class LineType{
    BLANK,
    OpenBrace,
    ClsoeBrace,
    Regular
  };

  class Transpiler{

    private:
      Checker checker_;
      [[nodiscard]] static LineType classify_line(std::string_view trimmed, std::string_view original);

    public:
      Transpiler() = default;
      Transpiler(const Transpiler&) = delete;
      Transpiler& operator=(const Transpiler&) = delete;
      Transpiler(Transpiler&&) = default;
      Transpiler& operator=(Transpiler&&) = default;



      [[nodiscard]] std::optional<Diagnostic> transpile_line(std::string_view line, std::ostream& output);
      [[nodiscard]] std::optional<Diagnostic> finalize() const;
      [[nodiscard]] int depth() const noexcept {return checker_.depth();}
      [[nodiscard]] int line_number() const noexcept{
        return checker_.line_number();
      }
      void reset() noexcept;
  };


[[nodiscard]] std::optional<Diagnostic> transpile_stream(std::istream& input, std::ostream& output);
[[nodiscard]] std::optional<Diagnostic> transpile_file(const std::string& input_path, const std::string& output_path);
[[nodiscard]] std::string derive_output_path(const std::string& input_path);
};



