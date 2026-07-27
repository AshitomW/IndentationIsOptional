#include "iio/utils.hpp"
#include <iio/transpiler.hpp>
#include <fstream>
#include <optional>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace iio{
  LineType Transpiler::classify_line(
    std::string_view trimmed, std::string_view original
  ){
    if (trimmed.empty()){
      return LineType::BLANK;
    }

    if (trimmed == "}"){
      return LineType::CloseBrace;
    }

    if (!trimmed.empty() && trimmed.back() == '{'){
      auto pos = find_brace_outside_string(original,'{');
      if (pos != std::string::npos){
        auto after = trim(original.substr(pos + 1));
        if (after.empty()){
          return LineType::OpenBrace;
        }
      }
    }
    return LineType::Regular;
  }

  std::optional<Diagnostic> Transpiler::transpile_line(
    std::string_view line, std::ostream& output
  ){
    auto err = checker_.feed_line(line);
    if (err) return err;
    auto trimmed = trim(line);

    auto line_type = classify_line(trimmed, line);

    switch(line_type){
      case LineType::BLANK:
        output << "\n";
        break;

      case LineType::OpenBrace:{
        int output_depth = checker_.depth() - 1;
        auto content = trim_right(trimmed.substr(0, trimmed.size() - 1));
        content = trim(content);
        output << make_indent(output_depth) << content << ":\n";
        break;
      }

      case LineType::CloseBrace:
        break;

      case LineType::Regular:
        output << make_indent(checker_.depth()) << trimmed << '\n';
        break;
    }

    return std::nullopt;
  }

  std::optional<Diagnostic> Transpiler::finalize() const {
    return checker_.finalize();
  }

  void Transpiler::reset() noexcept{
    checker_.reset();
  }

  std::optional<Diagnostic> transpile_stream(std::istream &input, std::ostream &output){
    Transpiler transpiler;
    std::string line;

    while (std::getline(input, line)){
      auto err = transpiler.transpile_line(line, output);
      if (err) return err;
    }
    return transpiler.finalize();
  }

  std::optional<Diagnostic> transpile_file(
    const std::string& input_path, const std::string& output_path
  ){
    std::ifstream infile(input_path);
    if (!infile.is_open()){
      return Diagnostic{0, 0, "cannot open input file: " + input_path};
    }

    std::ofstream outfile(output_path);
    if (!outfile.is_open()){
      return Diagnostic{0, 0, "cannot open output file: " + output_path};
    }

    auto err = transpile_stream(infile, outfile);
    if (err) return err;

    if (!outfile.good()){
      return Diagnostic{0, 0, "error writing output file: " + output_path};
    }

    return std::nullopt;
  }

  std::string derive_output_path(const std::string &input_path){
    namespace fs = std::filesystem;
    fs::path p(input_path);
    if (p.extension() == ".iio"){
      p.replace_extension(".py");
    } else{
      auto new_name = p.stem().string() + ".py";
      p = p.parent_path() / new_name;
    }
    return p.string();
  }
}
