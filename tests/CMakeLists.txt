# Copyright (c) 2025 ttldtor.
# SPDX-License-Identifier: BSL-1.0

include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.12)
FetchContent_MakeAvailable(doctest)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG v4.1.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nanobench)

add_executable(bits_tests
    main.cpp
    bits.cpp
)

target_compile_features(bits_tests PRIVATE cxx_std_20)
target_link_libraries(bits_tests PRIVATE bits::bits)
target_include_directories(bits_tests PRIVATE ${doctest_SOURCE_DIR})

add_test(NAME bits_tests COMMAND bits_tests)

add_executable(bits_bench
        main.cpp
        bench.cpp
)

target_compile_features(bits_bench PRIVATE cxx_std_20)
target_link_libraries(bits_bench PRIVATE bits::bits nanobench)
target_include_directories(bits_bench PRIVATE ${doctest_SOURCE_DIR})

add_test(NAME bits_bench COMMAND bits_bench)