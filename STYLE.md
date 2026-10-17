# C++ style

The Google C++ Style Guide (https://google.github.io/styleguide/cppguide.html)
with the mechanical settings replaced by `.clang-format`, lint by
`.clang-tidy`, and the naming rules loosened as listed below. Where this file
is silent, Google applies.

The reference for "in practice" is the lk kernel, whose newer C++ (the fs
layer, the AHCI and PCI drivers, `lib/lktl`, `lib/minip`, `arch/test`) uses
this style and whose `.clang-format` and `.clang-tidy` this tree carries
verbatim. lk's older code is snake_case C and is out of scope here.

## Formatting

Fixed by `.clang-format` (LLVM base); `./codestyle <files>` applies it.

| | this style | Google |
|---|---|---|
| Indent | 4 spaces, no tabs | 2 spaces |
| Line length | 100 | 80 |
| Pointers and references | bind to the name: `T *p`, `T &r` | bind to the type: `T* p`, `T& r` |
| Braces | attached; every `if`/`for`/`while` body braced, even one statement | attached; a one-statement body may stay on the `if` line unbraced |
| Access specifiers | indented 2 (`  public:`) | indented 1 (` public:`) |
| Braced lists | spaces inside: `{ 1, 2, 3 }` | `{1, 2, 3}` |
| Case labels | indented one level | same |
| Consecutive macros, bit-fields | values aligned in columns | not aligned |
| One-line functions | only inside a class body | anywhere |
| Header guard | `#pragma once` | `#ifndef PROJECT_PATH_FILE_H_` |
| File extension | `.cpp`, `.h` | `.cc`, `.h` |
| Includes | every include is the path from the repo root in angle brackets (`<ray/scene.h>`, `<libvec/vector3.h>`), a file's own header too; one sorted block with system and project headers interleaved, or the own header alone in a first block (clang-format pins only a quoted main header, so a blank line does it) | four groups: related header, C system, C++ standard, other libraries, this project |
| Namespace close | `} // namespace x` | `}  // namespace x` |

## Naming

| | this style | Google | in practice |
|---|---|---|---|
| Class data members | `name_` | same | firm, followed everywhere |
| Struct fields (plain data) | `name` | same | firm |
| Constants (`constexpr`, `static const`) | `kName` | same | newer code; a few older classes use `MAX_CMD_COUNT` |
| Accessors | `name()`, `set_name()` | same | `get_name()` also appears |
| Namespaces | short, lowercase (`pci`, `lk`) | same | |
| Macros | `UPPER_CASE` | same | |
| Template parameters | `T`, `Args` | same | |
| Types (class, struct, enum, alias) | snake_case (`ahci_port`, `tcp_socket`, `lk::function_ref`) | `CamelCase` | firm: reads like the C around it and the STL |
| Functions and methods | snake_case (`queue_command`) | `CamelCase()` | firm |
| Enumerators | no settled rule: `Legacy`/`Msi`, `HALT`/`NEXT`, `read_only`, register names verbatim | `kName` | loose |
| Files | snake_case (`bus_mgr.cpp`; hyphens in device names, `virtio-block.cpp`) | snake_case | |
| Output parameters | last, by pointer; `out`, `out_x` or `x_out` | last, by pointer | |
| Local variables and parameters | snake_case, never the name of their type: `ray r`, `hit h`, `scene s`, `frame f` | snake_case | with snake_case types this is the one real cost; a variable that takes its type's name hides the type for the rest of the scope, and inside a class body it is an error |

A plain struct's field may carry its type's name, with the type qualified so
the declaration still names the type once the field exists:

```cpp
struct shader_args {
    const ::scene &scene;
    ::tracer &tracer;
    const ::ray &ray;
    const ::hit &hit;
};
```

`.clang-tidy` carries these as `readability-identifier-naming` options, so
the lint reports a name that breaks them. Local constants stay snake_case
(the `k` prefix is for constants with static storage), and a public field
has no suffix.

## Class layout

Google's declaration order, with the copy-control block written as a macro:

```cpp
class tile_queue final {
  public:
    tile_queue(framebuffer &surface, tile_order order, uint32_t seed);
    ~tile_queue();

    DISALLOW_COPY_ASSIGN_AND_MOVE(tile_queue);

    // false once every tile has been handed out
    bool get_work_unit(work_unit *out);
    void return_work_unit(const work_unit &unit);

    size_t tiles_x() const { return tiles_x_; }
    tile_order order() const { return order_; }

    static constexpr int kWorkUnitSize = 16;

  private:
    struct tile {
        uint32_t x = 0;
        uint32_t y = 0;
    };

    void fill_unit(const tile &t, work_unit *out) const;

    // members
    framebuffer &surface_;
    tile_order order_;
    size_t tiles_x_ = 0;
    std::vector<tile> tiles_;
    std::mutex lock_;
    size_t next_ = 0;
};
```

- `public:`, then `protected:`, then `private:`. Within a section: types and
  aliases, constructors and assignment, destructor, other methods, accessors,
  static constants, data members last. Constants sit either with the types or
  after the accessors.
- Copy and move are deleted explicitly with `DISALLOW_COPY_ASSIGN_AND_MOVE`
  (or `DISALLOW_COPY_AND_ASSIGN_ALLOW_MOVE`) unless the class is meant to be
  copied; `= default` where the defaults are right. In lk the macros come from
  `lk/cpp.h`; a hosted tree carries its own copy.
- One-line accessors live in the class body. Anything longer goes out of line:
  `inline` below the class in the header, or in the `.cpp`, in declaration
  order.
- `final` on classes nothing derives from, `explicit` on single-argument
  constructors, `override` (not `virtual`) on overriders, no default arguments
  on virtual functions.
- Data members are private and default-initialized in the declaration with
  `=` (`int x_ = 0;`, not `{0}`). Long classes group them at the end under a
  `// members` comment; other section comments (`// constants`) are common.
- `struct` is for passive data: public fields, no trailing underscore, at most
  trivial helpers. A value type whose public fields are its interface
  (`vector3::x`) stays a `class` when it carries real operators.

## Language

- lk is freestanding C++17 (no exceptions, no RTTI, `std::` from `lib/libcpp`).
  A hosted tree writes C++20 and still keeps exceptions and RTTI out of its
  own code.
- The `modernize-*` set applies: `nullptr`, `using` aliases over `typedef`,
  range-for, `constexpr`, `enum class`, default member initializers,
  `std::unique_ptr` for ownership, `std::optional` for maybe-values. Not
  forced: `auto` (write the type when it says something; `auto` for iterators
  and obvious initializers), trailing return types, `[[nodiscard]]`. C arrays
  and `<stdint.h>`-style headers are fine.
- Non-const reference parameters are fine (Google dropped the ban too).
- No `using namespace` in headers. File-local code goes in an anonymous
  namespace in the `.cpp`; `static` functions are fine too.
- Named casts (`static_cast`, `reinterpret_cast`) in new code. C-style casts
  survive in hardware-facing code (`google-readability-casting` is on, so the
  lint reports them).
- Fixed-width integers (`uint32_t`) rather than `short`/`long`.
- Preconditions are asserted near the top of a function (`DEBUG_ASSERT` in lk,
  `assert` in a hosted tree).
- `//` comments everywhere except the license block; terse; say why, not
  what. A `TODO` needs no owner.

## Lint

`.clang-tidy` is lk's file plus the naming options above. It enables `bugprone-*`,
`clang-diagnostic-*`, `google-*`, `misc-*`, `modernize-*`, `performance-*` and
`readability-*`, then switches off:

- `google-runtime-references` (non-const refs are fine),
  `google-readability-todo` (no owner), `google-readability-braces-around-statements`
  (clang-format's `InsertBraces` does it).
- `modernize-use-auto`, `-use-nodiscard`, `-use-trailing-return-type`,
  `-avoid-c-arrays`, `-deprecated-headers`, `-raw-string-literal`,
  `-return-braced-init-list`, `-use-equals-delete`.
- `readability-magic-numbers`, `-identifier-length`, `-implicit-bool-conversion`,
  `-else-after-return`, `-isolate-declaration`, `-math-missing-parentheses`,
  `-named-parameter`, `-qualified-auto`, `-uppercase-literal-suffix`,
  `-function-cognitive-complexity`.
- `misc-const-correctness`, `-include-cleaner`, `-no-recursion`,
  `-unused-parameters`, `-noexcept*`; `bugprone-easily-swappable-parameters`,
  `-reserved-identifier`.

`WarningsAsErrors` is off: the lint is advisory. clangd runs it in the editor
(`.clangd` points at `build/`), and `run-clang-tidy -p build` runs it over the
tree.
