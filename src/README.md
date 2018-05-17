
# AGE Source Code

## Structure

AGE source code is structured into _groups_ of coherent code.
While code within a _group_ may be tightly coupled,
code of different _groups_ is only loosely coupled (if at all).
Loose coupling is achieved by including shared C++ header files found at
`include`.

The following _groups_ exist:

* **`age_common`** contains basic code used throughout the AGE project,
    like for example AGE-specific data types.
    This _group_ does not require any library beside the
    [C++14 STL](http://en.cppreference.com/w/cpp).
* **`age_emulator_gb`** contains the actual gameboy emulation code.
    This _group_ does not require any library beside the
    [C++14 STL](http://en.cppreference.com/w/cpp).
* **`age_js`** contains the AGE browser application created with
    [Angular](https://angular.io).
    This _group_ requires the `age_wasm` binaries to run the emulator within
    the browser.
* **`age_qt_emu_test`** contains an emulator test runner created with
    [Qt](https://www.qt.io/).
* **`age_qt_gui`** contains the AGE desktop application created with
    [Qt](https://www.qt.io/).
* **`age_wasm`** contains a [WebAssembly](https://webassembly.org/) interface
    to `age_emulator_gb`.
    It depends on [emscripten](http://kripken.github.io/emscripten-site/) for
    making specific C++ methods callable from JavaScript.


## Code Quality

AGE code has to stick to the following rules.

### C++

1. **Don't put `using namespace` statements in header files.**
    All consumers of this header file would be forced to live with that
    namespace usage as it cannot be undone.
1. **Avoid `using namespace`** statements in cpp files.
1. Do not violate the **single responsibility principle**.
    Unrelated functionality must not be grouped together
    (i.e. in the same file).

#### includes

1. **Use include guards** in every header file.
    Use the file's name converted to "screaming snake case" for it's include
    guard,
    e.g. use the include guard `AGE_FOO_HPP` for a file named `age_foo.hpp`.
1. **Do not include more than a file needs.**
    Code required just for cpp files must not bloat header files.
1. **Include everything a file needs.**
    Don't rely on transitive includes or the include order in cpp files.

