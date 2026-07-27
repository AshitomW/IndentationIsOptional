#include <cstdio>
#include <iio/lsp.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <unordered_map>

namespace iio::lsp{

JsonValue::JsonValue() : type_(Type::Null){}
JsonValue::JsonValue(bool v): type_(Type::Bool), bool_val_(v) {}
JsonValue::JsonValue(int v): type_(Type::Int), int_val_(v) {}
JsonValue::JsonValue(int64_t v): type_(Type::Int), int_val_(v){}
JsonValue::JsonValue(const std::string& v): type_(Type::String), str_val_(v) {}
JsonValue::JsonValue(std::string&& v): type_(Type::String), str_val_(std::move(v)){}
JsonValue::JsonValue(const char* v): type_(Type::String), str_val_(v){}
JsonValue::JsonValue(std::vector<JsonValue> arr) : type_(Type::Array), arr_val_(std::move(arr)){}
JsonValue::JsonValue(std::vector<std::pair<std::string,JsonValue>> obj): type_(Type::Object), obj_val_(std::move(obj)){}

const std::string& JsonValue::as_string() const {return str_val_;}
int64_t JsonValue::as_int() const {return int_val_;}
bool JsonValue::as_bool() const {return bool_val_;}
const std::vector<JsonValue>& JsonValue::as_array() const {
  return arr_val_;
}
const std::vector<std::pair<std::string, JsonValue>>& JsonValue::as_object() const {return obj_val_;}

const JsonValue* JsonValue::get(const std::string& key) const {
  if (type_ != Type::Object) return nullptr;
  for (const auto& [k,v] : obj_val_){
    if (k == key) return &v;
  }
  return nullptr;
}

static void escape_json_string(const std::string& s, std::string& out){
  out += '"';
  for (char c: s){
    switch(c){
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20){
          char buf[8];
          std::snprintf(buf, sizeof(buf),"\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
          out += buf;
        }else{
          out += c;
        }
    }
  }
  out += '"';
}

void JsonValue::serialize_to(std::string& out) const{
  switch(type_){
    case Type::Null:
      out += "null";
      break;
    case Type::Bool:
      out += bool_val_ ? "true" : "false";
      break;
    case Type::Int:
      out += std::to_string(int_val_);
      break;
    case Type::String:
      escape_json_string(str_val_, out);
      break;
    case Type::Array:
      out += '[';
      for (std::size_t i = 0; i < arr_val_.size(); ++i){
        if (i > 0) out += ',';
        arr_val_[i].serialize_to(out);
      }
      out += ']';
      break;
    case Type::Object:
      out += '{';
      for (std::size_t i = 0; i < obj_val_.size(); ++i){
        if (i > 0) out += ',';
        escape_json_string(obj_val_[i].first, out);
        out += ':';
        obj_val_[i].second.serialize_to(out);
      }
      out += '}';
      break;
  }
}

std::string JsonValue::serialize() const {
  std::string out;
  out.reserve(256);
  serialize_to(out);
  return out;
}

void JsonValue::skip_whitespace(std::string_view input, std::size_t& pos) {
  while (pos < input.size() && std::isspace(
             static_cast<unsigned char>(input[pos]))) {
    ++pos;
  }
}

JsonValue JsonValue::parse(std::string_view input) {
  std::size_t pos = 0;
  skip_whitespace(input, pos);
  if (pos >= input.size()) return JsonValue();
  return parse_value(input, pos);
}

JsonValue JsonValue::parse_value(std::string_view input, std::size_t& pos) {
  skip_whitespace(input, pos);
  if (pos >= input.size()) return JsonValue();
  char c = input[pos];
  if (c == '"') return parse_string_val(input, pos);
  if (c == '{') return parse_object(input, pos);
  if (c == '[') return parse_array(input, pos);
  if (c == 't' || c == 'f') {
    if (input.substr(pos, 4) == "true") {
      pos += 4;
      return JsonValue(true);
    }
    if (input.substr(pos, 5) == "false") {
      pos += 5;
      return JsonValue(false);
    }
    return JsonValue();
  }
  if (c == 'n') {
    if (input.substr(pos, 4) == "null") {
      pos += 4;
      return JsonValue();
    }
    return JsonValue();
  }
  if (c == '-' || (c >= '0' && c <= '9')) {
    return parse_number(input, pos);
  }
  return JsonValue();
}

JsonValue JsonValue::parse_string_val(std::string_view input, std::size_t& pos) {
  if (pos >= input.size() || input[pos] != '"') return JsonValue();
  ++pos;
  std::string result;
  while (pos < input.size()) {
    char c = input[pos];
    if (c == '"') {
      ++pos;
      return JsonValue(std::move(result));
    }
    if (c == '\\') {
      ++pos;
      if (pos >= input.size()) break;
      char next = input[pos];
      switch (next) {
        case '"': result += '"'; break;
        case '\\': result += '\\'; break;
        case '/': result += '/'; break;
        case 'n': result += '\n'; break;
        case 'r': result += '\r'; break;
        case 't': result += '\t'; break;
        case 'u': {
          if (pos + 4 < input.size()) {
            std::string hex(input.substr(pos + 1, 4));
            char* end = nullptr;
            long code = std::strtol(hex.c_str(), &end, 16);
            if (end == hex.c_str() + 4) {
              result += static_cast<char>(code);
              pos += 4;
            }
          }
          break;
        }
        default: result += next; break;
      }
      ++pos;
      continue;
    }
    result += c;
    ++pos;
  }
  return JsonValue(std::move(result));
}

JsonValue JsonValue::parse_number(std::string_view input, std::size_t& pos) {
  std::size_t start = pos;
  if (pos < input.size() && input[pos] == '-') ++pos;
  while (pos < input.size() && input[pos] >= '0' && input[pos] <= '9') ++pos;
  if (pos < input.size() && input[pos] == '.') {
    ++pos;
    while (pos < input.size() && input[pos] >= '0' && input[pos] <= '9') ++pos;
    std::string num_str(input.substr(start, pos - start));
    return JsonValue(static_cast<int64_t>(std::stod(num_str)));
  }
  std::string num_str(input.substr(start, pos - start));
  return JsonValue(static_cast<int64_t>(std::stoll(num_str)));
}

JsonValue JsonValue::parse_array(std::string_view input, std::size_t& pos) {
  if (pos >= input.size() || input[pos] != '[') return JsonValue();
  ++pos;
  std::vector<JsonValue> arr;
  while (pos < input.size()) {
    skip_whitespace(input, pos);
    if (pos >= input.size()) break;
    if (input[pos] == ']') {
      ++pos;
      return JsonValue(std::move(arr));
    }
    if (!arr.empty()) {
      if (input[pos] == ',') {
        ++pos;
        skip_whitespace(input, pos);
      }
    }
    arr.push_back(parse_value(input, pos));
  }
  return JsonValue(std::move(arr));
}

JsonValue JsonValue::parse_object(std::string_view input, std::size_t& pos) {
  if (pos >= input.size() || input[pos] != '{') return JsonValue();
  ++pos;
  std::vector<std::pair<std::string, JsonValue>> obj;
  while (pos < input.size()) {
    skip_whitespace(input, pos);
    if (pos >= input.size()) break;
    if (input[pos] == '}') {
      ++pos;
      return JsonValue(std::move(obj));
    }
    if (!obj.empty()) {
      if (input[pos] == ',') {
        ++pos;
        skip_whitespace(input, pos);
      }
    }
    if (pos >= input.size() || input[pos] != '"') break;
    auto key_val = parse_string_val(input, pos);
    if (key_val.type() != Type::String) break;
    skip_whitespace(input, pos);
    if (pos >= input.size() || input[pos] != ':') break;
    ++pos;
    skip_whitespace(input, pos);
    auto value = parse_value(input, pos);
    obj.emplace_back(key_val.as_string(), std::move(value));
  }
  return JsonValue(std::move(obj));
}

JsonValue make_object(
  std::initializer_list<std::pair<std::string, JsonValue>> entries) {
  std::vector<std::pair<std::string, JsonValue>> obj;
  for (auto& entry : entries) {
    obj.emplace_back(std::move(entry.first), std::move(entry.second));
  }
  return JsonValue(std::move(obj));
}

Server::Server(std::istream& input, std::ostream& output)
  : input_(input), output_(output) {}

std::optional<std::string> Server::read_message() {
  std::string header_line;
  int content_length = -1;

  while (std::getline(input_, header_line)) {
    if (header_line.back() == '\r') header_line.pop_back();
    if (header_line.empty()) break;

    if (header_line.size() > 16 &&
        header_line.substr(0, 16) == "Content-Length: ") {
      try {
        content_length = std::stoi(header_line.substr(16));
      } catch (...) {
        return std::nullopt;
      }
    }
  }

  if (content_length <= 0) return std::nullopt;

  std::string body(static_cast<std::size_t>(content_length), '\0');
  input_.read(&body[0], content_length);
  if (static_cast<int>(input_.gcount()) != content_length) {
    return std::nullopt;
  }

  return body;
}

void Server::send_message(const std::string& json) {
  output_ << "Content-Length: " << json.size() << "\r\n\r\n" << json;
  output_.flush();
}

void Server::send_response(const JsonValue& id, const JsonValue& result) {
  auto msg = make_object({
    {"jsonrpc", JsonValue("2.0")},
    {"id", id},
    {"result", result}
  });
  send_message(msg.serialize());
}

void Server::send_error(const JsonValue& id, int code, const std::string& message) {
  auto err_obj = make_object({
    {"code", JsonValue(code)},
    {"message", JsonValue(message)}
  });
  auto msg = make_object({
    {"jsonrpc", JsonValue("2.0")},
    {"id", id},
    {"error", err_obj}
  });
  send_message(msg.serialize());
}

void Server::send_notification(const std::string& method, const JsonValue& params) {
  auto msg = make_object({
    {"jsonrpc", JsonValue("2.0")},
    {"method", JsonValue(method)},
    {"params", params}
  });
  send_message(msg.serialize());
}

void Server::dispatch(const JsonValue& msg) {
  auto* method_val = msg.get("method");
  auto* id_val = msg.get("id");

  if (!method_val) return;

  const std::string& method = method_val->as_string();
  auto* params = msg.get("params");
  JsonValue empty_params = make_object({});
  const JsonValue& p = params ? *params : empty_params;

  if (method == "initialize") {
    if (id_val) handle_initialize(*id_val, p);
  } else if (method == "initialized") {
    handle_initialized();
  } else if (method == "shutdown") {
    if (id_val) handle_shutdown(*id_val);
  } else if (method == "exit") {
    handle_exit();
  } else if (method == "textDocument/didOpen") {
    handle_did_open(p);
  } else if (method == "textDocument/didChange") {
    handle_did_change(p);
  } else if (method == "textDocument/didClose") {
    handle_did_close(p);
  } else if (method == "textDocument/hover") {
    if (id_val) handle_hover(*id_val, p);
  }
}

int Server::run() {
  while (!shutdown_requested_) {
    auto msg_body = read_message();
    if (!msg_body) {
      if (input_.eof()) break;
      continue;
    }
    auto msg = JsonValue::parse(*msg_body);
    dispatch(msg);
  }
  return 0;
}

void Server::handle_initialize(const JsonValue& id, const JsonValue& /*params*/) {
  auto capabilities = make_object({
    {"textDocumentSync", JsonValue(1)},
    {"hoverProvider", JsonValue(true)}
  });
  auto result = make_object({
    {"capabilities", capabilities},
    {"serverInfo", make_object({
      {"name", JsonValue("iio-lsp")},
      {"version", JsonValue("1.0.0")}
    })}
  });
  send_response(id, result);
  initialized_ = true;
}

void Server::handle_initialized() {}

void Server::handle_shutdown(const JsonValue& id) {
  send_response(id, JsonValue(nullptr));
  shutdown_requested_ = true;
}

void Server::handle_exit() {
  shutdown_requested_ = true;
}

void Server::handle_did_open(const JsonValue& params) {
  auto* text_doc = params.get("textDocument");
  if (!text_doc) return;
  auto* uri = text_doc->get("uri");
  auto* text = text_doc->get("text");
  auto* version_val = text_doc->get("version");
  if (!uri || !text) return;

  DocumentState doc;
  doc.uri = uri->as_string();
  doc.content = text->as_string();
  doc.version = version_val ? static_cast<int>(version_val->as_int()) : 0;
  documents_[doc.uri] = std::move(doc);

  validate_document(uri->as_string());
}

void Server::handle_did_change(const JsonValue& params) {
  auto* text_doc = params.get("textDocument");
  if (!text_doc) return;
  auto* uri = text_doc->get("uri");
  if (!uri) return;

  auto* content_changes = params.get("contentChanges");
  if (!content_changes || content_changes->as_array().empty()) return;

  auto& it = documents_[uri->as_string()];
  auto* text_val = content_changes->as_array().back().get("text");
  if (!text_val) return;
  it.content = text_val->as_string();
  it.uri = uri->as_string();
  if (auto* version_val = text_doc->get("version")) {
    it.version = static_cast<int>(version_val->as_int());
  }

  validate_document(uri->as_string());
}

void Server::handle_did_close(const JsonValue& params) {
  auto* text_doc = params.get("textDocument");
  if (!text_doc) return;
  auto* uri = text_doc->get("uri");
  if (!uri) return;
  documents_.erase(uri->as_string());
}

void Server::handle_hover(const JsonValue& id, const JsonValue& /*params*/) {
  auto contents = make_object({
    {"kind", JsonValue("markdown")},
    {"value", JsonValue("IndentationIsOptional (.iio) file\n"
                        "Use `{` to open and `}` to close blocks.\n"
                        "Braces must be the last non-whitespace on a line.")}
  });
  auto result = make_object({
    {"contents", contents}
  });
  send_response(id, result);
}

void Server::validate_document(const std::string& uri) {
  auto it = documents_.find(uri);
  if (it == documents_.end()) return;

  Checker checker;
  std::istringstream stream(it->second.content);
  std::string line;
  std::vector<Diagnostic> diags;

  while (std::getline(stream, line)) {
    auto err = checker.feed_line(line);
    if (err) {
      diags.push_back(*err);
      break;
    }
  }

  auto final_err = checker.finalize();
  if (final_err) {
    diags.push_back(*final_err);
  }

  it->second.diagnostics = std::move(diags);
  publish_diagnostics(uri, it->second.diagnostics);
}

void Server::publish_diagnostics(const std::string& uri, const std::vector<Diagnostic>& diags) {
  std::vector<JsonValue> json_diags;
  for (const auto& d : diags) {
    json_diags.push_back(make_object({
      {"range", make_object({
        {"start", make_object({
          {"line", JsonValue(std::max(0, d.line - 1))},
          {"character", JsonValue(d.column)}
        })},
        {"end", make_object({
          {"line", JsonValue(std::max(0, d.line - 1))},
          {"character", JsonValue(d.column + 1)}
        })}
      })},
      {"severity", JsonValue(1)},
      {"message", JsonValue(d.message)}
    }));
  }

  auto params = make_object({
    {"uri", JsonValue(uri)},
    {"diagnostics", JsonValue(std::move(json_diags))}
  });

  send_notification("textDocument/publishDiagnostics", params);
}

}
