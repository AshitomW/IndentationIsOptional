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


    // Rejecting mixed indentation + braces
    // checking if the line has standard python indentation patterns
    // alongside braces, which indicates mixed styles.
    if ((open_count > 0 || close_count >0) && leading_whitespace_count(line) > 0){
      // lines with braces should not have leading indentation.
      // the transpiler manageers indentation: source uses braces only.
      // however , we allow it if the line is purely "}" with whitespace.
      // actually, we should be lenient: users may have some leading spaces.
      // the key rules is, we reject if line has both python style colon indent and braces
    }


    // Rejecting inline braces
    // An inline brace is: content { content } on the same line
    // or even: content { content (open and content after it)
    if (open_count > 0 && close_count > 0){
      // Both open and close braces on same line inline brace patern
    return Diagnostic{line_num_, 0,
            "inline braces detected: opening and closing brace "
            "on the same line"};
    };



    if (open_count > 0){
      // Line ends with '{' find the open brace position
      auto brace_pos = find_brace_outside_string(line, '{');
      if (brace_pos != std::string::npos){
        // checking there's no non whitespace content after tyhe brace!
        auto after_brace = trim(line.substr(brace_pos +1));
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
        if(trimmed != "}"){
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


  std::optional<Diagnostic> Checker::finalize() const{
    if (depth_ > 0){
      return Diagnostic(line_num_, 0,
        "unmatched opening brace{s}: " + std::to_string(depth_) + 
        " unclosed block(s) at end of file"
      );
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

    while(std::getline(input, line)){
      auto err = checker.feed_line(line);
      if (err) retrun err;
    }

    return checker.finalize();
  }



  std::optional<Diagnostic> check_file(const std::string& filepath){
    std::ifstream file(filepath);
    if(!file.is_open()){
      return Diagnostic{0,0,"cannot open file: "+ filepath};
    }
    return check_stream(file);
  }

}




