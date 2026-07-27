#include <iio/transpiler.hpp>
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

void test_simple_function() {
  TEST("simple function") {
    std::istringstream input(
      "def main() {\n"
      "    print(\"hello\")\n"
      "}\n"
      "main()\n"
    );
    std::ostringstream output;
    auto err = iio::transpile_stream(input, output);
    assert_no_error(err);

    std::string result = output.str();
    if (result.find("def main():") == std::string::npos) {
      throw std::runtime_error("missing def main():");
    }
    if (result.find("print(\"hello\")") == std::string::npos) {
      throw std::runtime_error("missing print");
    }
    if (result.find("main()") == std::string::npos) {
      throw std::runtime_error("missing main() call");
    }
  } END_TEST;
}

void test_class_transpile() {
  TEST("class transpilation") {
    std::istringstream input(
      "class Animal {\n"
      "    def __init__(self, name) {\n"
      "        self.name = name\n"
      "    }\n"
      "}\n"
    );
    std::ostringstream output;
    auto err = iio::transpile_stream(input, output);
    assert_no_error(err);

    std::string result = output.str();
    if (result.find("class Animal:") == std::string::npos) {
      throw std::runtime_error("missing class Animal:");
    }
    if (result.find("def __init__(self, name):") == std::string::npos) {
      throw std::runtime_error("missing __init__");
    }
    if (result.find("self.name = name") == std::string::npos) {
      throw std::runtime_error("missing self.name");
    }
  } END_TEST;
}

void test_indentation() {
  TEST("indentation levels") {
    std::istringstream input(
      "if a {\n"
      "    if b {\n"
      "        pass\n"
      "    }\n"
      "}\n"
    );
    std::ostringstream output;
    auto err = iio::transpile_stream(input, output);
    assert_no_error(err);

    std::string result = output.str();
    if (result.find("if a:") == std::string::npos) throw std::runtime_error("no if a:");
    if (result.find("    if b:") == std::string::npos) throw std::runtime_error("no indented if b:");
    if (result.find("        pass") == std::string::npos) throw std::runtime_error("no double indented pass");
  } END_TEST;
}

void test_blank_lines() {
  TEST("blank lines preserved") {
    std::istringstream input(
      "def foo() {\n"
      "\n"
      "    pass\n"
      "}\n"
    );
    std::ostringstream output;
    auto err = iio::transpile_stream(input, output);
    assert_no_error(err);

    std::string result = output.str();
    if (result.find("def foo():") == std::string::npos) throw std::runtime_error("no def foo():");
    if (result.find("pass") == std::string::npos) throw std::runtime_error("no pass");
    if (result.find("def foo():\n\n") == std::string::npos) throw std::runtime_error("blank line not preserved");
  } END_TEST;
}

void test_inline_error() {
  TEST("inline braces error propagated") {
    std::istringstream input(
      "def foo() {\n"
      "    if x { do() }\n"
      "}\n"
    );
    std::ostringstream output;
    auto err = iio::transpile_stream(input, output);
    assert_error(err);
  } END_TEST;
}

void test_unmatched_error() {
  TEST("unmatched brace error propagated") {
    std::istringstream input(
      "def foo() {\n"
      "    print(\"hello\")\n"
    );
    std::ostringstream output;
    auto err = iio::transpile_stream(input, output);
    assert_error(err);
  } END_TEST;
}

void test_lru_cache() {
  TEST("LRU cache transpilation") {
    std::istringstream input(
      "class Node {\n"
      "    def __init__(self, key, val) {\n"
      "        self.key = key\n"
      "        self.val = val\n"
      "        self.prev = None\n"
      "        self.next = None\n"
      "    }\n"
      "}\n"
      "\n"
      "class LRUCache {\n"
      "    def __init__(self, capacity) {\n"
      "        self.cap = capacity\n"
      "        self.cache = {}\n"
      "        self.head = Node(0, 0)\n"
      "        self.tail = Node(0, 0)\n"
      "        self.head.next = self.tail\n"
      "        self.tail.prev = self.head\n"
      "    }\n"
      "}\n"
    );
    std::ostringstream output;
    auto err = iio::transpile_stream(input, output);
    assert_no_error(err);
    std::string result = output.str();
    if (result.find("class Node:") == std::string::npos) throw std::runtime_error("no class Node:");
    if (result.find("class LRUCache:") == std::string::npos) throw std::runtime_error("no class LRUCache:");
    if (result.find("self.cache = {}") == std::string::npos) throw std::runtime_error("no cache dict");
    if (result.find("def __init__(self, capacity):") == std::string::npos) throw std::runtime_error("no init");
  } END_TEST;
}

void test_derive_output_path() {
  TEST("derive output path from .iio") {
    std::string out = iio::derive_output_path("/path/to/foo.iio");
    if (out != "/path/to/foo.py") {
      throw std::runtime_error("got: " + out);
    }
  } END_TEST;

  TEST("derive output path from other ext") {
    std::string out = iio::derive_output_path("script.txt");
    if (out != "script.py") {
      throw std::runtime_error("got: " + out);
    }
  } END_TEST;
}

int main() {
  std::cout << "Transpiler tests:\n";

  test_simple_function();
  test_class_transpile();
  test_indentation();
  test_blank_lines();
  test_inline_error();
  test_unmatched_error();
  test_lru_cache();
  test_derive_output_path();

  std::cout << "\n" << tests_run << " tests, "
            << tests_failed << " failed\n";

  return tests_failed > 0 ? 1 : 0;
}
