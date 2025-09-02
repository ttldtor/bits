# Copyright (c) 2025 ttldtor.
# SPDX-License-Identifier: BSL-1.0

include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.12
)
FetchContent_MakeAvailable(doctest)

add_executable(bits_tests
    main.cpp
    bits.cpp
)

target_link_libraries(bits_tests PRIVATE bits::bits)
target_include_directories(bits_tests PRIVATE ${doctest_SOURCE_DIR})

add_test(NAME bits_tests COMMAND bits_tests)