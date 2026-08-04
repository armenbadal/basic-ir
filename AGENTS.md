# AGENTS.md

## What this is
A BASIC → LLVM IR compiler written in C++23 (CMake + Ninja, LLVM 21, Catch2). The language grammar lives in `cherry-syntax.md`. All comments, diagnostic messages, docs, and test assertions are in **Armenian** — keep new user-facing strings and comments in Armenian. Project is developed on the `semantic-analysis` branch.

## Build & test
- Configure (build dir `build/` already configured as Debug/Ninja): `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug`
- Build everything: `cmake --build build` — this also generates `build/basic_ir_lib.ll` (clang compiles `src/basic_ir_lib.c` via a custom command).
- Tests: `./build/basic-ir-tests` — there is **no ctest** (CMakeLists.txt has no `add_test`/`enable_testing`), run the binary directly. Filter with Catch2 tags, e.g. `./build/basic-ir-tests "[parser]"` or `"[recovery]"`.
- Smoke test: `build/basic-ir cases/case00.bas --lisp` writes `cases/case00.lisp` (ignored by git).

## Pipeline
`main.cxx` → `compile()` in `compiler.cxx`:
1. `scanner.cxx` (lexer) → `parser.cxx` (recursive-descent, Wirth-style, builds AST from `ast.hxx`; visitor dispatch via CRTP `astvisitor.hxx`)
2. `semantic.cxx` (symbol table in `symbol.hxx`, type system in `types.hxx`) — two-pass: declares all subroutines first, then checks bodies
3. Output: `--lisp` via `aslisp.cxx`, or LLVM IR via `iremitter.cxx`

## Gotchas
- **`--ir` is currently a no-op.** The IR-emission block in `compiler.cxx` is commented out and `iremitter.cxx` is NOT listed in CMakeLists.txt, so it never compiles. Only `--lisp` works. Re-enabling IR output means wiring `iremitter.cxx` into the build and uncommenting `compiler.cxx`. (Also note `iremitter.hxx` declares `ASTVisitorBase`-based API while `astvisitor.hxx` defines `ASTVisitor<Derived, ReturnType>` — an inconsistency to resolve when re-enabling.)
- **Parser never returns a null tree on errors.** It recovers (sync points, Wirth cascade suppression) and returns a partial AST; callers must check `Diagnostics::errors()` before using it (`compiler.cxx` does this between phases).
- **Diagnostic messages are asserted verbatim in tests** as Armenian substrings (e.g. `parser_test.cxx` checks `"Սպասվում է"`, `"THEN"`, etc.). Changing wording breaks tests.
- Statement-terminated language: every statement is on its own line (newline is significant); comments start with `'`. Types: `REAL`/`TEXT`/`BOOL`; `TRUE`/`FALSE` literals; text concat is `&`.

## Style
- clang-format config is `_clang-format` (clang-format detects this filename, so plain `clang-format -i <files>` works).
- Code uses C++23 `std::format`, ranges, `std::shared_ptr` AST nodes.
