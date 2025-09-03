# bits

Library for bit manipulation.

## Examples of usage

### xmake

```lua
add_rules("mode.debug", "mode.release")

add_repositories("ttldtor https://github.com/ttldtor/xmake-repo.git")
add_requires("bits 0.1.0")

target("test_bits")
    set_kind("binary")
    add_packages("bits")
    add_files("src/*.cpp")
    set_languages("c++20")
```

### CMake

```cmake
cmake_minimum_required(VERSION 3.16)
project(test_bits LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  bits
  GIT_REPOSITORY https://github.com/ttldtor/bits.git
  GIT_TAG        v0.1.0
)
FetchContent_MakeAvailable(bits)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE bits)
```

### Code

```c++
#include <iostream>
#include <bits/bits.hpp>

using namespace org::ttldtor::bits;

int main(int argc, char** argv) {
    std::cout << "hello world!" << std::endl;
    std::cout << shl(-5, 4) << std::endl;

    return 0;
}
```
