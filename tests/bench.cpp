// Copyright (c) 2025 ttldtor.
// SPDX-License-Identifier: BSL-1.0

#include <doctest/doctest.h>

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <bits/bits.hpp>
#include <cstddef>
#include <cstdint>
#include <limits>

using namespace org::ttldtor::bits;

// NOLINTNEXTLINE

TEST_CASE("tutorial_fast_v2") {
  uint64_t x = 1;
  ankerl::nanobench::Bench().run("++x", [&]() {
    ankerl::nanobench::doNotOptimizeAway(x += 1);
  });
}