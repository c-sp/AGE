
# AGE Desktop Application Source Code

## Structure

The AGE source code is structured into _groups_ of coherent code.
While code inside a _group_ may be tightly coupled,
code of different _groups_ is only loosely coupled (if at all).
Loose coupling is achieved by including shared C++ header files found at
`include`.

Current _groups_:

* **`age_common`** contains basic code used throughout the AGE project like
    common data types.
    This _group_ does not require any library beside the
    [C++14 STL](https://en.cppreference.com/w/cpp/14).
* **`age_emulator_gb`** is the actual Game Boy emulator.
    This _group_ does not require any library beside the
    [C++14 STL](https://en.cppreference.com/w/cpp/14).
* **`age_qt_emu_test`** is an emulator test runner created with
    [Qt](https://www.qt.io/).
* **`age_qt_gui`** is the AGE desktop application created with
    [Qt](https://www.qt.io/).
* **`age_wasm`** is a [WebAssembly](https://webassembly.org/) interface to
    `age_emulator_gb`.
    It is built with [emscripten](https://kripken.github.io/emscripten-site/)
    and [CMake](https://cmake.org/).


## C++ Code Quality

1. Do not violate the **single responsibility principle**.
    Unrelated functionality must not be grouped together
    (i.e. in the same file).

### Namespaces

1. **Don't put `using namespace` statements in header files.**
    All consumers of this header file would be forced to live with that
    namespace usage as it cannot be undone.
1. **Avoid `using namespace`** statements in cpp files.

### Includes

1. **Use include guards** in every header file.
    Use the file's name converted to upper snake case as include guard,
    e.g. use `AGE_FOO_HPP` for guarding `age_foo.hpp`.
1. **Do not include more than what a file needs.**
    Code required only for a cpp file must not bloat header files.
1. **Include everything a file needs.**
    Don't rely on transitive includes or include order in cpp files.

### Data Types

1. **Assume `int` to be at least 32 bits wide**.
    While the C++ standard requires an `int` to be
    [at least 16 bits wide](https://en.cppreference.com/w/cpp/language/types#Properties),
    for current data models `int` width usually is 32 bits.
    AGE static-asserts `int` to be at least 32 bits wide.
1. **Use `int` for arithmetic** until you have a specific reason not to do so.
    Arithmetic operators will cause smaller integral values
    [to be promoted to `int`](https://en.cppreference.com/w/cpp/language/implicit_conversion#Integral_promotion)
    anyway,
    so the result of most operations is an `int`.
1. **Allocate fixed width integers**.
    Don't allocate more memory than necessary.
    Allocating non-fixed width integers like `int` or `std::int_fast##_t` can
    cause more memory to be reserved than actually required.
    This might increase cache misses and thus can decrease performance.
1. **Minimize the size of class data members** while avoiding casts.
    E.g. if `int8_t` is sufficient and does not require additional casts,
    use it in favor of `int`.

**signed vs. unsigned**

1. **Do not use `unsigned`** for
    [ensuring that a value is non-negative](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-nonnegative).
1. **Use `signed`** until there is a very specific reason to not do so.
    Most arithmetic is [assumed to be `signed`](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es102-use-signed-types-for-arithmetic).
    `signed` integer overflow being undefined
    [enables several compiler optimizations](http://blog.llvm.org/2011/05/what-every-c-programmer-should-know.html)
    (`unsigned` overflow being well-defined may prevent compiler optimizations
    though).

AGE code uses `unsigned` only for values representing the emulated hardware
(e.g. Game Boy CPU registers, Game Boy memory)
and when interacting with STL containers
(e.g. `size_t std::vector::size()` or `std::vector::operator[size_t]`).
