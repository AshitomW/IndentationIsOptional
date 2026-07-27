#include <iio/checker.hpp>
#include <iio/utils.hpp>
#include <iostream>
#include <sstream>
#include <cassert>
#include <string>

static int tests_run = 0;
static int tests_failed = 0;

#define TEST(name) do { \
  ++tests_run; \
  std::cout << "  " << name << "... "; \
  try {

#define END_TEST \
    std::cout << "PASS\n"; \
  } catch (const std::exception& e) { \
    std::cout << "FAIL: " << e.what() << "\n"; \
    ++tests_failed; \
  } catch (...) { \
    std::cout << "FAIL: unexpected exception\n"; \
    ++tests_failed; \
  } \
} while(0)

static void assert_no_error(const std::optional<iio::Diagnostic>& d) {
  if (d) throw std::runtime_error("unexpected error: " + d->format());
}

static void assert_error(const std::optional<iio::Diagnostic>& d) {
  if (!d) throw std::runtime_error("expected an error but got none");
}

void test_valid_hello() {
  TEST("valid hello world") {
    std::istringstream input(
      "def main() {\n"
      "    print(\"hello, world!\")\n"
      "}\n"
      "main()\n"
    );
    auto err = iio::check_stream(input);
    assert_no_error(err);
  } END_TEST;
}

void test_valid_class() {
  TEST("valid class") {
    std::istringstream input(
      "class Animal {\n"
      "    def __init__(self, name) {\n"
      "        self.name = name\n"
      "    }\n"
      "}\n"
    );
    auto err = iio::check_stream(input);
    assert_no_error(err);
  } END_TEST;
}

void test_inline_braces() {
  TEST("inline braces rejected") {
    std::istringstream input(
      "def foo() {\n"
      "    if x { do() }\n"
      "}\n"
    );
    auto err = iio::check_stream(input);
    assert_error(err);
  } END_TEST;
}

void test_unmatched_open() {
  TEST("unmatched opening brace") {
    std::istringstream input(
      "def foo() {\n"
      "    print(\"hello\")\n"
    );
    auto err = iio::check_stream(input);
    assert_error(err);
  } END_TEST;
}

void test_unmatched_close() {
  TEST("unmatched closing brace") {
    std::istringstream input(
      "def foo() {\n"
      "    print(\"hello\")\n"
      "}\n"
      "}\n"
    );
    auto err = iio::check_stream(input);
    assert_error(err);
  } END_TEST;
}

void test_close_on_own_line() {
  TEST("close brace must be own line") {
    std::istringstream input(
      "def foo() {\n"
      "    print(\"hello\") }\n"
    );
    auto err = iio::check_stream(input);
    assert_error(err);
  } END_TEST;
}

void test_content_after_open() {
  TEST("content after opening brace") {
    std::istringstream input(
      "def foo() { extra\n"
    );
    auto err = iio::check_stream(input);
    assert_error(err);
  } END_TEST;
}

void test_string_safe() {
  TEST("braces inside strings ignored") {
    std::istringstream input(
      "x = \"hello { world\"\n"
      "y = 'test } here'\n"
    );
    auto err = iio::check_stream(input);
    assert_no_error(err);
  } END_TEST;
}

void test_multiple_open() {
  TEST("multiple opening braces rejected") {
    std::istringstream input(
      "def foo() {{\n"
    );
    auto err = iio::check_stream(input);
    assert_error(err);
  } END_TEST;
}

void test_multiple_close() {
  TEST("multiple closing braces rejected") {
    std::istringstream input(
      "def foo() {\n"
      "}}\n"
    );
    auto err = iio::check_stream(input);
    assert_error(err);
  } END_TEST;
}

void test_empty_file() {
  TEST("empty file") {
    std::istringstream input("");
    auto err = iio::check_stream(input);
    assert_no_error(err);
  } END_TEST;
}

void test_nested_blocks() {
  TEST("deeply nested valid blocks") {
    std::istringstream input(
      "if a {\n"
      "    if b {\n"
      "        if c {\n"
      "            pass\n"
      "        }\n"
      "    }\n"
      "}\n"
    );
    auto err = iio::check_stream(input);
    assert_no_error(err);
  } END_TEST;
}

void test_blank_lines() {
  TEST("blank lines handled") {
    std::istringstream input(
      "def foo() {\n"
      "\n"
      "    pass\n"
      "}\n"
    );
    auto err = iio::check_stream(input);
    assert_no_error(err);
  } END_TEST;
}

void test_dict_literal() {
  TEST("empty dict literal allowed") {
    std::istringstream input(
      "d = {}\n"
    );
    auto err = iio::check_stream(input);
    assert_no_error(err);
  } END_TEST;
}

void test_file_not_found() {
  TEST("file not found") {
    auto err = iio::check_file("/nonexistent/path.iio");
    assert_error(err);
  } END_TEST;
}

int main() {
  std::cout << "Checker tests:\n";

  test_valid_hello();
  test_valid_class();
  test_inline_braces();
  test_unmatched_open();
  test_unmatched_close();
  test_close_on_own_line();
  test_content_after_open();
  test_string_safe();
  test_multiple_open();
  test_multiple_close();
  test_empty_file();
  test_nested_blocks();
  test_blank_lines();
  test_dict_literal();
  test_file_not_found();

  std::cout << "\n" << tests_run << " tests, "
            << tests_failed << " failed\n";

  return tests_failed > 0 ? 1 : 0;
}
