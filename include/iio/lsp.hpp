#pragma once


#include <iio/utils.hpp>
#include <iio/checker.hpp>

#include <string>
#include <unordered_map>
#include <optional>
#include <istream>
#include <ostream>
#include <sstream>
#include <vector>


namespace iio::lsp{
  class JsonValue{
    public:
    enum class Type{ Null, Bool, Int, String, Array, Object};
    JsonValue();
    explicit JsonValue(bool v);
    explicit JsonValue(int v);
    explicit JsonValue(int64_t v);
    explicit JsonValue(const std::string& v);
    explicit JsonValue(std::string&& v);
    explicit JsonValue(const char* v);
    explicit JsonValue(std::vector<JsonValue> arr);
    explicit JsonValue(std::vector<std::pair<std::string,JsonValue>> obj);
    [[nodiscard]] Type type() const noexcept {return type_;}
    [[nodiscard]] bool is_null() const noexcept{
      return type_ == Type::Null;
    }

    [[nodiscard]] const std::string& as_string() const;
    [[nodiscard]] int64_t as_int() const;
    [[nodiscard]] bool as_bool() const;
    [[nodiscard]] const std::vector<JsonValue>& as_array() const;
    [[nodiscard]] const std::vector<std::pair<std::string, JsonValue>>& as_object() const;

    [[nodiscard]] const JsonValue* get(const std::string& key) const;
    [[nodiscard]] std::string seialize() const;
    [[nodiscard]] static JsonValue parse(std::string_view input);

    private:
     Type type_;
     bool bool_val_ = false;
     int64_t int_val_ = 0;
     std::string str_val_;
     std::vector<JsonValue> arr_val_;
     std::vector<std::pair<std::string, JsonValue>> obj_val_;


     void serialize_to(std::string& out) const;
     static JsonValue parse_value(std::string_view input, std::size_t& pos);
     static JsonValue parse_string_val(std::string_view input, std::size_t& pos);
     static JsonValue parse_number(std::string_view input, std::size_t& post);
     static JsonValue parse_array(std::string_view input, std::size_t& post);
     static JsonValue parse_object(std::string_view input, std::size_t& post);
     static JsonValue skip_whitespace(std::string_view input, std::size_t& post);
  };

  JsonValue make_object(
    std::initializer_list<std::pair<std::string, JsonValue>> entries);

  struct DocumentState{
    std::string uri;
    std::string content;
    std::vector<Diagnostic> diagnostics;
    int version = 0;
  };

  
class Server{
public:
  Server(std::istream& input, std::ostream& output);
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;
  int run();

private:
  std::istream& input_;
  std::ostream& output_;
  bool initialized_ = false;
  bool shutdown_requested_ = false;
  std::unordered_map<std::string, DocumentState> documents_;

  [[nodiscard]] std::optional<std::string> read_message();
  void send_message(const std::string& json);
  void send_response(const JsonValue& id, const JsonValue& result);
  void send_error(const JsonValue& id, int code, const std::string& message);
  void send_notification(const std::string& method, const JsonValue& params);
  void dispatch(const JsonValue& msg);
  

  void handle_initialize(const JsonValue& id, const JsonValue& params);
  void handle_initialized();
  void handle_shutdown(const JsonValue& id);
  void handle_exit();
  void handle_did_open(const JsonValue& params);
  void handle_did_change(const JsonValue& params);
  void handle_did_close(const JsonValue& params);
  void handle_hover(const JsonValue& id, const JsonValue& params);
  
  void validate_document(const std::string& uri);
  void publish_diagnostics(const std::string& uri, const std::vector<Diagnostic>& diags);

};

}
