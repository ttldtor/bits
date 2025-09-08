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
using namespace std::literals;

// NOLINTNEXTLINE

TEST_CASE("bench_sal_sar_vs_builtin") {
  constexpr size_t N = 1u << 15;

  std::mt19937_64 rng{0xB00B00};
  // Для сравнения с builtin << используем беззнаковые типы и неотрицательные сдвиги
  std::uniform_int_distribution<uint32_t> u32dist{0u, 0x7FFF'FFFFu};
  std::uniform_int_distribution<uint64_t> u64dist{0ull, (1ull << 50)};
  std::uniform_int_distribution<unsigned>  shldist{0u, 63u};

  // Для сравнения с builtin >> используем знаковые типы и неотрицательные сдвиги
  std::uniform_int_distribution<int32_t>  s32dist{-(1 << 29), (1 << 29)};
  std::uniform_int_distribution<int64_t>  s64dist{-(1ll << 50), (1ll << 50)};
  std::uniform_int_distribution<unsigned> shrdist{0u, 63u};

  std::vector<uint32_t>  u32(N);
  std::vector<unsigned>  shl32(N);
  std::vector<uint64_t>  u64(N);
  std::vector<unsigned>  shl64(N);

  std::vector<int32_t>   s32(N);
  std::vector<unsigned>  shr32(N);
  std::vector<int64_t>   s64(N);
  std::vector<unsigned>  shr64(N);

  for (size_t i = 0; i < N; ++i) {
    u32[i]  = u32dist(rng);
    shl32[i]= shldist(rng) % 32;
    u64[i]  = u64dist(rng);
    shl64[i]= shldist(rng) % 64;

    s32[i]  = s32dist(rng);
    shr32[i]= shrdist(rng) % 32;
    s64[i]  = s64dist(rng);
    shr64[i]= shrdist(rng) % 64;
  }

  ankerl::nanobench::Bench bench;
  bench.title("Compare sal/sar vs builtin shifts")
       .unit("op")
       .batch(N)
       .warmup(10)
#if defined(WIN32)
       .minEpochTime(150ms)
#endif
       .relative(false)
       .performanceCounters(true);

  // sal vs builtin << (беззнаковые типы, чтобы избежать UB для <<)
  bench.run("sal<uint32_t>", [&] {
    uint32_t acc = 0;
    for (size_t i = 0; i < N; ++i) {
      acc ^= sal(u32[i], static_cast<unsigned>(shl32[i]));
    }
    ankerl::nanobench::doNotOptimizeAway(acc);
  });

  bench.run("builtin << (uint32_t)", [&] {
    uint32_t acc = 0;
    for (size_t i = 0; i < N; ++i) {
      acc ^= static_cast<uint32_t>(u32[i] << shl32[i]);
    }
    ankerl::nanobench::doNotOptimizeAway(acc);
  });

  bench.run("sal<uint64_t>", [&] {
    uint64_t acc = 0;
    for (size_t i = 0; i < N; ++i) {
      acc ^= sal(u64[i], static_cast<unsigned>(shl64[i]));
    }
    ankerl::nanobench::doNotOptimizeAway(acc);
  });

  bench.run("builtin << (uint64_t)", [&] {
    uint64_t acc = 0;
    for (size_t i = 0; i < N; ++i) {
      acc ^= static_cast<uint64_t>(u64[i] << shl64[i]);
    }
    ankerl::nanobench::doNotOptimizeAway(acc);
  });

  // sar vs builtin >> (знаковые типы, неотрицательные сдвиги)
  bench.run("sar<int32_t>", [&] {
    int32_t acc = 0;
    for (size_t i = 0; i < N; ++i) {
      acc ^= sar(s32[i], static_cast<unsigned>(shr32[i]));
    }
    ankerl::nanobench::doNotOptimizeAway(acc);
  });

  bench.run("builtin >> (int32_t)", [&] {
    int32_t acc = 0;
    for (size_t i = 0; i < N; ++i) {
      acc ^= static_cast<int32_t>(s32[i] >> shr32[i]);
    }
    ankerl::nanobench::doNotOptimizeAway(acc);
  });

  bench.run("sar<int64_t>", [&] {
    int64_t acc = 0;
    for (size_t i = 0; i < N; ++i) {
      acc ^= sar(s64[i], static_cast<unsigned>(shr64[i]));
    }
    ankerl::nanobench::doNotOptimizeAway(acc);
  });

  bench.run("builtin >> (int64_t)", [&] {
    int64_t acc = 0;
    for (size_t i = 0; i < N; ++i) {
      acc ^= static_cast<int64_t>(s64[i] >> shr64[i]);
    }
    ankerl::nanobench::doNotOptimizeAway(acc);
  });

}