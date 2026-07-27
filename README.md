# IndentationIsOptional

A C++17 transpiler that converts brace-based Python-like syntax (`.iio` files) into standard indentation-based Python (`.py`).

```
Input (.iio)              Output (.py)
─────────────────          ─────────────────
def main() {        →      def main():
    print("hi")     →          print("hi")
}                   →
main()              →      main()
```

## Features

- **Transpile** — Convert `{}` blocks to indented Python with `:`
- **Check** — Standalone syntax validation (inline braces, unmatched braces, depth limits, string-safe)
- **Run** — Transpile + execute in one step
- **LSP** — Language Server Protocol support for real-time diagnostics in editors
- **Streaming** — Single-pass O(n) processing, memory proportional to line size
- **Memory-safe** — Modern C++17, no raw pointers, automatic memory management

## CLI Usage

```
Usage:
  indentationisoptional build <input>        Transpile .iio to .py
  indentationisoptional run <input>          Transpile and execute
  indentationisoptional check <input>        Validate syntax only
  indentationisoptional --lsp                Run LSP server (stdin/stdout)
  indentationisoptional -h, --help           Show help

Options:
  -o <file>    Output path (default: replace .iio with .py)
```

### Examples

```bash
# Check syntax
indentationisoptional check script.iio

# Transpile
indentationisoptional build script.iio

# Transpile to custom path
indentationisoptional build script.iio -o out.py

# Transpile and run
indentationisoptional run script.iio
```

## Build & Install

### Prerequisites

- C++17 compiler (Clang 7+, GCC 7+, MSVC 2019+)
- CMake 3.20+
- Python 3 (for `run` command)

### Build

```bash
git clone <repo>
cd IndentationIsOptional
cmake -S . -B build
cmake --build build
```

### Install

```bash
cmake --install build
```

Or run directly from the build directory:

```bash
./build/indentationisoptional --help
```

## Syntax Rules

### Block Structure

- `{` at the end of a line opens a block (converted to `:`)
- `}` on its own line closes a block (decreases indentation)
- Indentation uses 4 spaces per level

### Valid

```iio
def greet(name) {
    print("hello, " + name)
}

class Foo {
    def bar(self) {
        pass
    }
}

if x {
    for y in items {
        process(y)
    }
}
```

### Invalid (rejected by `check`)

| Pattern                     | Why                            |
| --------------------------- | ------------------------------ |
| `if x { do() }`             | Inline braces on same line     |
| `if x {`<br>`do()`<br>`  }` | Content after `{`              |
| `def foo() {`<br>`}`<br>`}` | Unmatched closing brace        |
| `def foo() {`               | Unmatched opening brace at EOF |
| `if x {{`                   | Multiple opening braces        |
| `def foo() {`<br>`}}`       | Multiple closing braces        |

### String Safety

Braces inside `'...'` or `"..."` are ignored:

```iio
msg = "this { has a brace } inside"
# Perfectly valid — the braces are in a string
```

## LSP Setup

IndentationIsOptional includes a built-in LSP server for real-time diagnostics.

### Neovim

Using `vim.lsp.start`:

```lua
vim.api.nvim_create_autocmd("BufNewFile,BufRead", {
  pattern = "*.iio",
  callback = function()
    vim.lsp.start({
      name = "iio-lsp",
      cmd = { "/path/to/indentationisoptional", "--lsp" },
    })
  end,
})
```

### VS Code

Create `.vscode/iio-language-server.json` or add to your settings:

```json
{
  "lsp": [
    {
      "command": "/path/to/indentationisoptional",
      "args": ["--lsp"],
      "filetypes": ["iio"]
    }
  ]
}
```

### Sublime Text

Install the LSP package and add:

```json
{
  "clients": {
    "IIO Language Server": {
      "command": ["/path/to/indentationisoptional", "--lsp"],
      "selector": "source.iio"
    }
  }
}
```

## Project Structure

```
├── CMakeLists.txt       # Build system (C++17, static lib + executable)
├── include/iio/         # Public headers
│   ├── transpiler.hpp   # Brace → Python transpiler
│   ├── checker.hpp      # Syntax validator
│   ├── lsp.hpp          # LSP server (JSON-RPC, diagnostics)
│   └── utils.hpp        # Shared utilities (trim, brace-scan, Diagnostic)
├── src/                 # Implementation
│   ├── main.cpp         # CLI entry point
│   ├── transpiler/      # Transpilation logic
│   ├── checker/         # Syntax validation
│   ├── lsp/             # Language server
│   └── utils/           # Shared utilities
└── tests/               # Unit tests (Catch2-free, minimal test framework)
    ├── CMakeLists.txt
    ├── test_transpiler.cpp
    ├── test_checker.cpp
    └── test_samples/    # .iio fixtures
```

## Architecture

The tool uses a single-pass streaming architecture:

1. **`Checker`** — State machine tracking brace depth, validates each line as it arrives
2. **`Transpiler`** — Wraps `Checker`, classifies lines (open/close/regular/blank), emits Python output
3. **`Server`** — LSP server reading JSON-RPC 2.0 from stdin, publishing diagnostics via notifications

All modules share:

- `trim()` / `trim_left()` / `trim_right()` — whitespace utilities
- `find_brace_outside_string()` — brace scanning with string awareness
- `count_braces_outside_string()` — same for brace counting
- `Diagnostic` — structured error (line, column, message)

## License

MIT
