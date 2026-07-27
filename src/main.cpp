#include <iio/transpiler.hpp>
#include <iio/checker.hpp>
#include <iio/lsp.hpp>
#include <iostream>
#include <cstdlib>
#include <string>

static void print_usage(const char* prog) {
  std::cerr << "IndentationIsOptional — brace-based Python transpiler\n"
            << "\n"
            << "Usage:\n"
            << "  " << prog << " build <input>        Transpile .iio to .py\n"
            << "  " << prog << " run <input>          Transpile and execute\n"
            << "  " << prog << " check <input>        Validate syntax only\n"
            << "  " << prog << " --lsp                Run LSP server (stdin/stdout)\n"
            << "\n"
            << "Options:\n"
            << "  -o <file>          Output path (default: replace .iio with .py)\n"
            << "  -h, --help         Show this help\n";
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::string cmd = argv[1];

  if (cmd == "-h" || cmd == "--help") {
    print_usage(argv[0]);
    return 0;
  }

  if (cmd == "--lsp") {
    iio::lsp::Server server(std::cin, std::cout);
    return server.run();
  }

  if (cmd == "build" || cmd == "run" || cmd == "check") {
    if (argc < 3) {
      std::cerr << "error: missing input file\n";
      return 1;
    }

    std::string input_path = argv[2];
    std::string output_path;
    if (argc >= 4 && std::string(argv[3]) == "-o") {
      if (argc < 5) {
        std::cerr << "error: -o requires a filename\n";
        return 1;
      }
      output_path = argv[4];
    } else if (cmd == "build" || cmd == "run") {
      output_path = iio::derive_output_path(input_path);
    }

    if (cmd == "check") {
      auto err = iio::check_file(input_path);
      if (err) {
        std::cerr << err->format() << "\n";
        return 1;
      }
      std::cout << "syntax OK\n";
      return 0;
    }

    auto err = iio::transpile_file(input_path, output_path);
    if (err) {
      std::cerr << err->format() << "\n";
      return 1;
    }

    if (cmd == "build") {
      std::cout << "wrote " << output_path << "\n";
      return 0;
    }

    std::string py_cmd = "python3 " + output_path;
    int ret = std::system(py_cmd.c_str());
    if (ret != 0) {
      std::cerr << "error: python execution failed with code " << ret << "\n";
    }
    return ret;
  }

  std::cerr << "error: unknown command '" << cmd << "'\n";
  print_usage(argv[0]);
  return 1;
}
