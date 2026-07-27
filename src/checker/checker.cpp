#include <iio/checker.hpp>
#include <fstream>
#include <algorithm>

namespace iio{

  std::optional<Diagnostic> Checker::feed_line(std::string_view line){
    ++line_num_;
    auto trimmed = trim(line);
    if (trimmed.empty()) return std::nullopt;

    int open_count = count_braces_outside_string(line,'{');
    int close_count = count_braces_outside_string(line,'}');

    if (open_count > 0 && close_count > 0 && open_count == close_count) {
      if (has_inline_block_braces(line)) {
        return Diagnostic{line_num_, 0,
          "inline braces detected: opening and closing brace "
          "on the same line"};
      }
      return std::nullopt;
    }

    if (open_count > 0){
      auto brace_pos = find_brace_outside_string(line, '{');
      if (brace_pos != std::string::npos){
        auto after_brace = trim(line.substr(brace_pos + 1));
        if (!after_brace.empty()){
          return Diagnostic{line_num_,
              static_cast<int>(brace_pos + 2),
              "content found after opening brace; "
              "opening brace must be the last non-whitespace character"};
        }
      }

      if (open_count > 1){
        return Diagnostic{line_num_, 0,
              "multiple opening braces on a single line"};
      }

      depth_ += 1;

      if (depth_ > MAX_INDENT_DEPTH){
        return Diagnostic{line_num_, 0,
            "maximum nesting depth exceeded (" +
            std::to_string(MAX_INDENT_DEPTH) + ")"};
      }
    }

    if (close_count > 0){
      if (trimmed != "}"){
        return Diagnostic{line_num_, 0,
            "closing brace '}' must be on its own line"};
      }
      if (close_count > 1){
        return Diagnostic{line_num_, 0,
            "multiple closing braces on a single line"};
      }

      depth_ -= 1;

      if (depth_ < 0){
        return Diagnostic{line_num_, 0,
            "unmatched closing brace '}' — "
            "no corresponding opening brace"};
      }
    }

    return std::nullopt;
  }

  bool Checker::has_inline_block_braces(std::string_view line) const {
    bool in_single = false;
    bool in_double = false;
    bool escaped = false;
    int brace_depth = 0;
    std::size_t last_open = std::string::npos;

    for (std::size_t i = 0; i < line.size(); ++i) {
      char c = line[i];
      if (escaped) {
        escaped = false;
        continue;
      }
      if (c == '\\') {
        escaped = true;
        continue;
      }
      if (c == '\'' && !in_double) {
        in_single = !in_single;
        continue;
      }
      if (c == '"' && !in_single) {
        in_double = !in_double;
        continue;
      }
      if (in_single || in_double) continue;

      if (c == '{') {
        if (brace_depth == 0) {
          last_open = i;
        }
        ++brace_depth;
      } else if (c == '}') {
        --brace_depth;
        if (brace_depth == 0 && last_open != std::string::npos) {
          auto between = trim(line.substr(last_open + 1, i - last_open - 1));
          if (!between.empty()) {
            return true;
          }
          last_open = std::string::npos;
        }
      }
    }
    return false;
  }

  std::optional<Diagnostic> Checker::finalize() const{
    if (depth_ > 0){
      return Diagnostic{line_num_, 0,
        "unmatched opening brace(s): " + std::to_string(depth_) +
        " unclosed block(s) at end of file"
      };
    }
    return std::nullopt;
  }

  void Checker::reset() noexcept{
    depth_ = 0;
    line_num_ = 0;
  }

  std::optional<Diagnostic> check_stream(std::istream& input){
    Checker checker;
    std::string line;
    while (std::getline(input, line)){
      auto err = checker.feed_line(line);
      if (err) return err;
    }
    return checker.finalize();
  }

  std::optional<Diagnostic> check_file(const std::string& filepath){
    std::ifstream file(filepath);
    if (!file.is_open()){
      return Diagnostic{0, 0, "cannot open file: " + filepath};
    }
    return check_stream(file);
  }

}
