#include <cctype>
#include <iio/utils.hpp>
#include <sstream>
#include <algorithm>
#include <string>



namespace iio{
std::string Diagnostic::format() const {
  std::string result = "error:" + std::to_string(line);
  if (column >0){
    result += ":" + std::to_string(column);
  }
  result += ": " + message;
  return result;
};


std::string make_indent(int level){
  if(level <= 0) return {};
   return std::string(static_cast<std::size_t>(level * INDENT_WIDTH), ' ');
}

std::string_view trim_left(std::string_view sv){
    auto it = std::find_if_not(sv.begin(), sv.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    sv.remove_prefix(static_cast<std::size_t>(
        std::distance(sv.begin(), it)));
    return sv;
}

std::string_view trim_right(std::string_view sv){
  auto it = std::find_if_not(sv.rbegin(),sv.rend(),[](unsigned char c){
    return std::isspace(c);
  });
  sv.remove_suffix(static_cast<std::size_t>(std::distance(sv.rbegin(),it)));
  return sv;
}


std::string_view trim(std::string_view sv){
  return trim_left(trim_right(sv));
}

bool is_blank(std::string_view sv){
  return trim(sv).empty();
}

std::size_t leading_whitespace_count(std::string_view sv){
  auto it = std::find_if_not(sv.begin(), sv.end(),[](unsigned char c){
    return std::isspace(c);
  });

  return static_cast<std::size_t>(std::distance(sv.begin(),it));
}


bool is_inside_string(std::string_view line, std::size_t pos){
  if (pos >= line.size()) return false;
  bool in_single = false;
  bool in_double = false;
  bool escaped = false;

  for (std::size_t i = 0; i < pos; ++i){
    char c = line[i];
    if(escaped){
      escaped = false;
      continue;
    }
    if (c == '\\'){
      escaped = true;
      continue;
    }

    if (c=='\'' && !in_double){
      in_single = !in_single;
    }else if(c == '"' && !in_single){
      in_double = !in_double;
    }
  }

  return in_single || in_double;

}

std::size_t find_brace_outside_string(std::string_view line, char brace){
    bool in_single = false;
    bool in_double = false;
    bool escaped = false;

    for (std::size_t i = 0; i< line.size(); ++i){
      char c = line[i];
      if (escaped) {
        escaped = false;
        continue;
      }
      if (c == '\\'){
        escaped = true;
        continue;
      }
      if (c == '\'' && !in_double){
        in_single = !in_single;
      }else if(c == '"' && !in_single){
        in_double = !in_double;
      } else if (c == brace && !in_single && !in_double){
        return i;
      }
    }

    return std::string::npos;
}



int count_braces_outside_string(std::string_view line , char brace){
  int count = 0;
  bool in_single = false;
  bool in_double = false;
  bool escaped = false;


  for (std::size_t i = 0; i< line.size(); ++i){
    char c = line[i];
    if (escaped) {
      escaped = false;
      continue;
    }


    if (c == '\\'){
      escaped = true;
      continue;
    }
    if (c == '\'' && !in_double){
      in_single = !in_single;
    }else if (c == '"' && !in_single){
      in_double = !in_double;
    } else if (c == brace && !in_single && !in_double){
      ++count;
    }


  }
  return count;
}

};



