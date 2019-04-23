# Bachelor Thesis

This repository contains the source files for my bachelor thesis at the [HTW Dresden](https://www.htw-dresden.de).
The thesis itself is written in German and discusses the vectorization of line drawings via photographs.

## Building the Library

It depends on:
* A C++17 compatible compiler
* OpenCV 3.4 or later (including 4.x)

Git submodules are being used for dependencies. You can...
* clone this project using `git clone --recursive https://github.com/lhecker/bachelor.git`
* check them out retroactively using `git submodule update --init`

Afterwards the library can be built like any other CMake project.
The resulting `lhecker_bachelor_cli` target acts as an example CLI for the library.

### Windows

* If the library fails to build with a `LNK2019` error in the `RelWithDebInfo` configuration, try setting `CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO` to `RELEASE`.
  See [here](https://github.com/opencv/opencv/issues/5564#issuecomment-195324087) for instance.
* If the tests fail to build please try setting the `gtest_force_shared_crt` variable to `ON`.

## Building the Thesis

The thesis is built using LuaTeX and `latexmk`, as well as several popular packages.
It depends on:
* LuaTeX
* latexmk
* Python and `pygments` for the use with `minted`
* Inkscape for the use with `svg`
* (Plus a number of commonly found TeX packages)

To build the thesis `cd` into the `/thesis` directory and run something like this:
```sh
latexmk -synctex=1 main.tex
```
