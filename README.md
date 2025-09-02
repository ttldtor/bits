# bits

Library for bit manipulation.

## Examples of usage

### xmake

```lua
add_requires("bits default", { 
    urls = "https://github.com/ttldtor/bits.git" 
})

target("myapp")
    set_kind("binary")
    add_files("src/main.cpp")
    add_packages("bits")
```

### CMake

```cmake
cmake_minimum_required(VERSION 3.16)
project(myapp LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  bits
  GIT_REPOSITORY https://github.com/ttldtor/bits.git
  GIT_TAG        default
)
FetchContent_MakeAvailable(bits)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE bits)
```

