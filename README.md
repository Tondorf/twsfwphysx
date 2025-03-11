[![Documentation](https://github.com/tondorf/twsfwphysx/actions/workflows/deploy_docs.yml/badge.svg)](https://tondorf.github.io/twsfwphysx/)
[![Test coverage](https://codecov.io/gh/tondorf/twsfwphysx/graph/badge.svg?token=1197KZQ0AR)](https://codecov.io/gh/tondorf/twsfwphysx)
[![Unit tests](https://github.com/tondorf/twsfwphysx/actions/workflows/validate.yml/badge.svg)](https://codecov.io/gh/tondorf/twsfwphysx)

# twsfwphysx

This is the twsfwphysx project, a physics engine
for [twsfw (the 'w' stands for WASM)](https://github.com/Tondorf/twsfw).

## 📚 Documentation

Find the documentation of the API [here](https://tondorf.github.io/twsfwphysx/).

The library is a C header-only library, implemented
in [include/twsfwphysx/twsfwphysx.h](https://github.com/Tondorf/twsfwphysx/blob/main/include/twsfwphysx/twsfwphysx.h).
Despite using functions from `<math.h>`, no further dependencies are need for this library, making it straightforward to
embed the engine in other software projects.

In order to build the library, define `TWSFWPHYSX_IMPLEMENTATION` in **one** source file before including
`twsfwphysx.h`. Without this defintion, `twsfwphysx.h` is a canonical header file that declares the API but does not
provide any implementation.

More advanced build instructions are given in [BUILDING](BUILDING.md).

## 🐍 Python Binding

Do you prefer Python? Checkout our official [Python binding of twsfwphysx](python-binding)! This is a thin ([Cython][1])
wrapper around the C-API. Read [its documentation][2] to learn more.

## ⚙️ And what about WASM?

As stated above, this is a C-only library with minimal dependencies. Hence, it is easy to embed it into a WASM module!
In fact, we already do this for you, automatically, for each
new [release](https://github.com/Tondorf/twsfwphysx/releases). See [wasm-binding/](wasm-binding) for details.

## 👷 Contributing

If you plan to contribute to the library, you might find the instructions under [BUILDING](BUILDING.md) helpful.
Make sure to read our [CODE_OF_CONDUCT](CODE_OF_CONDUCT.md) document and find our advanced instructions
in [HACKING](HACKING.md).
document.

Please make sure to always check the [formatting](.clang-format) of your contribution, either by
running [clang-format][3] manually, or
by using our `format-check` and `format-fix` build targets!

[1]: https://cython.org/

[2]: https://tondorf.github.io/twsfwphysx/python-binding/

[3]: https://clang.llvm.org/docs/ClangFormat.html
