# Pre Setup
This Shell progrem uses DC libraries for error handling and error reporting for your POSIX application

Clone the [dc_scripts](https://github.com/bcitcstdatacomm/dc_scripts) repository then go into dc_scripts folder and run `./dc-update work gcc g++` this will install the DC libraries into a folder called work. If you have to update go to dc_scripts folder and run `./dc-update work` to update the libraries.
## Setup
```
mkdir cmake-build-debug
```

## Source Code Additions
Add your source files to the CMakeLists.txt:

```
set(HEADER_LIST [files])
set(SOURCE_LIST [files])
```

```
cmake -DCMAKE_C_COMPILER="gcc" -DCMAKE_CXX_COMPILER="g++" -S . -B cmake-build-debug
```
or:

```
cmake -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" -S . -B cmake-build-debug
```

## Build 
Examples:
```
cmake --build cmake-build-debug
cmake --build cmake-build-debug --clean-first
cmake --build cmake-build-debug --target docs
cmake --build cmake-build-debug --target format
```
