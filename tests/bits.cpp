// Copyright (c) 2025 ttldtor.
// SPDX-License-Identifier: BSL-1.0

#include <doctest/doctest.h>

#include <bits/bits.hpp>
#include <cstdint>
#include <cstddef>
#include <limits>

using org::ttldtor::bits::sal;
using org::ttldtor::bits::sar;

template <typename T>
constexpr std::size_t bitWidth() {
  using U = std::make_unsigned_t<T>;
  return static_cast<std::size_t>(std::numeric_limits<U>::digits);
}

TEST_CASE("sal - basic invariants") {
  SUBCASE("shift == 0 returns value") {
    CHECK(sal<std::uint32_t, int>(0x12345678u, 0) == 0x12345678u);
    CHECK(sal<std::int32_t, int>(123, 0) == 123);
    CHECK(sal<std::int32_t, int>(-456, 0) == -456);
  }

  SUBCASE("value == 0 returns 0 for any shift") {
    CHECK(sal<std::uint32_t, int>(0u, 0) == 0u);
    CHECK(sal<std::uint32_t, int>(0u, 5) == 0u);
    CHECK(sal<std::uint32_t, int>(0u, -5) == 0u);
  }
}

TEST_CASE("sal - non-negative shifts within bit width") {
  SUBCASE("unsigned V, safe positive shifts") {
    CHECK(sal<std::uint8_t, int>(0x12u, 1) == static_cast<std::uint8_t>(0x24u));
    CHECK(sal<std::uint16_t, int>(0x0003u, 2) == static_cast<std::uint16_t>(0x000Cu));
    CHECK(sal<std::uint32_t, int>(1u, 5) == 32u);
    CHECK(sal<std::uint64_t, int>(7u, 3) == 56u);
  }

  SUBCASE("signed V, use small shifts that do not lead to overflow") {
    CHECK(sal<std::int8_t, int>(3, 1) == 6);
    CHECK(sal<std::int16_t, int>(5, 2) == 20);
    CHECK(sal<std::int32_t, int>(7, 3) == 56);
    CHECK(sal<std::int64_t, int>(9, 4) == 144);
  }
}

TEST_CASE("sal - shift >= bit width returns 0") {
  SUBCASE("unsigned V") {
    constexpr auto bw8 = bitWidth<std::uint8_t>();
    constexpr auto bw32 = bitWidth<std::uint32_t>();
    constexpr auto bw64 = bitWidth<std::uint64_t>();

    CHECK(sal<std::uint8_t, int>(0xFFu, static_cast<int>(bw8)) == 0u);
    CHECK(sal<std::uint8_t, int>(0xFFu, 100) == 0u);

    CHECK(sal<std::uint32_t, int>(0xFFFFFFFFu, static_cast<int>(bw32)) == 0u);
    CHECK(sal<std::uint64_t, int>(0xFFFFFFFFFFFFFFFFull, static_cast<int>(bw64)) == 0ull);
  }

  SUBCASE("signed V (do not rely on the behavior of negative left shift)") {
    constexpr auto bw16 = bitWidth<std::int16_t>();
    CHECK(sal<std::int16_t, int>(123, static_cast<int>(bw16)) == 0);
    CHECK(sal<std::int32_t, int>(456, 1000) == 0);
  }
}

TEST_CASE("sal - negative shift delegates to sar (right arithmetic)") {
  SUBCASE("signed V, negative value: sign-extend on right shift") {
    // value = -8; shifting left by -1 => arithmetic right shift by 1 => -4
    CHECK(sal<std::int32_t, int>(-8, -1) == sar<std::int32_t, unsigned>(-8, 1));
    CHECK(sal<std::int32_t, int>(-8, -1) == -4);

    // larger magnitude
    CHECK(sal<std::int32_t, int>(-1024, -5) == sar<std::int32_t, unsigned>(-1024, 5));
    CHECK(sal<std::int32_t, int>(-1024, -5) == -32);
  }

  SUBCASE("signed V, non-negative value: behaves like logical right via sar") {
    CHECK(sal<std::int32_t, int>(1024, -5) == sar<std::int32_t, unsigned>(1024, 5));
    CHECK(sal<std::int32_t, int>(1024, -5) == 32);
  }

  SUBCASE("unsigned V: delegating negative shift performs right shift filling zeros") {
    CHECK(sal<std::uint32_t, int>(0xF0000000u, -4) == sar<std::uint32_t, unsigned>(0xF0000000u, 4));
    CHECK(sal<std::uint32_t, int>(0xF0000000u, -4) == 0x0F000000u);
  }

  SUBCASE("extreme negative shift: shift == min(S)") {
    // use the minimum possible shift value for the signed S
    constexpr int smin = std::numeric_limits<int>::min();
    // magnitude is correctly calculated in unsigned representation, resulting right shift >= bit width => 0 for
    // unsigned, -1/0 for signed inside sar
    CHECK(sal<std::uint32_t, int>(0xDEADBEEFu, smin) ==
          sar(0xDEADBEEFu, static_cast<std::make_unsigned_t<int>>(0) - static_cast<std::make_unsigned_t<int>>(smin)));
    CHECK(sal(-1, smin) ==
          sar(-1, static_cast<std::make_unsigned_t<int>>(0) - static_cast<std::make_unsigned_t<int>>(smin)));
  }
}

TEST_CASE("sal - cross-size S and V") {
  SUBCASE("S already than V (std::int8_t -> std::uint64_t)") {
    CHECK(sal<std::uint64_t, std::int8_t>(1ull, std::int8_t{5}) == 32ull);
    CHECK(sal<std::uint64_t, std::int8_t>(0xFFFFFFFFFFFFFFFFull, std::int8_t{64}) == 0ull);
  }

  SUBCASE("S wider than V (std::int64_t -> std::uint8_t)") {
    CHECK(sal<std::uint8_t, std::int64_t>(0x01u, std::int64_t{3}) == static_cast<std::uint8_t>(0x08u));
    CHECK(sal<std::uint8_t, std::int64_t>(0xFFu, std::int64_t{100}) == static_cast<std::uint8_t>(0));
  }

  SUBCASE("Unsigned S: no negative shifts, big shifts are zeroed out") {
    CHECK(sal<std::uint16_t, std::uint8_t>(0x0001u, static_cast<std::uint8_t>(4)) == static_cast<std::uint16_t>(0x0010u));
    CHECK(sal<std::uint16_t, std::uint8_t>(0xFFFFu, static_cast<std::uint8_t>(255)) == static_cast<std::uint16_t>(0));
  }
}