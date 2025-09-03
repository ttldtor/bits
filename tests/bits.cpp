// Copyright (c) 2025 ttldtor.
// SPDX-License-Identifier: BSL-1.0

#include <doctest/doctest.h>

#include <bits/bits.hpp>
#include <cstdint>
#include <cstddef>
#include <limits>

using namespace org::ttldtor::bits;

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

TEST_CASE("sar - basic invariants") {
  SUBCASE("shift == 0 returns value") {
    CHECK(sar<std::uint32_t, int>(0x12345678u, 0) == 0x12345678u);
    CHECK(sar<std::int32_t, int>(123, 0) == 123);
    CHECK(sar<std::int32_t, int>(-456, 0) == -456);
  }

  SUBCASE("value == 0 returns 0 for any shift") {
    CHECK(sar<std::uint32_t, int>(0u, 0) == 0u);
    CHECK(sar<std::uint32_t, int>(0u, 5) == 0u);
    CHECK(sar<std::uint32_t, int>(0u, -5) == 0u);
  }
}

TEST_CASE("sar - non-negative shifts within bit width") {
  SUBCASE("unsigned V behaves like logical right shift") {
    CHECK(sar<std::uint8_t, int>(0x80u, 1) == static_cast<std::uint8_t>(0x40u));
    CHECK(sar<std::uint16_t, int>(0x00F0u, 4) == static_cast<std::uint16_t>(0x000Fu));
    CHECK(sar<std::uint32_t, int>(0x80000000u, 31) == 1u);
    CHECK(sar<std::uint64_t, int>(0xF000000000000000ull, 4) == 0x0F00000000000000ull);
  }

  SUBCASE("signed V sign-extends on right shift") {
    CHECK(sar<std::int8_t, int>(-2, 1) == static_cast<std::int8_t>(-1));
    CHECK(sar<std::int16_t, int>(-8, 1) == static_cast<std::int16_t>(-4));
    CHECK(sar<std::int32_t, int>(-1024, 5) == -32);
    CHECK(sar<std::int64_t, int>(-9, 3) == -2);
  }
}

TEST_CASE("sar - shift >= bit width returns 0 for non-negative and -1 for negative") {
  SUBCASE("unsigned V") {
    constexpr auto bw8 = bitWidth<std::uint8_t>();
    constexpr auto bw32 = bitWidth<std::uint32_t>();
    constexpr auto bw64 = bitWidth<std::uint64_t>();

    CHECK(sar<std::uint8_t, int>(0xFFu, static_cast<int>(bw8)) == 0u);
    CHECK(sar<std::uint8_t, int>(0xFFu, 100) == 0u);

    CHECK(sar<std::uint32_t, int>(0xFFFFFFFFu, static_cast<int>(bw32)) == 0u);
    CHECK(sar<std::uint64_t, int>(0xFFFFFFFFFFFFFFFFull, static_cast<int>(bw64)) == 0ull);
  }

  SUBCASE("signed V") {
    constexpr auto bw16 = bitWidth<std::int16_t>();
    constexpr auto bw32 = bitWidth<std::int32_t>();

    CHECK(sar<std::int16_t, int>(123, static_cast<int>(bw16)) == 0);
    CHECK(sar<std::int16_t, int>(-123, static_cast<int>(bw16)) == static_cast<std::int16_t>(-1));

    CHECK(sar<std::int32_t, int>(456, 1000) == 0);
    CHECK(sar<std::int32_t, int>(-456, 1000) == static_cast<std::int32_t>(-1));
  }
}

TEST_CASE("sar - negative shift delegates to sal (left arithmetic)") {
  SUBCASE("unsigned V: negative shift becomes left logical via sal delegation") {
    CHECK(sar<std::uint32_t, int>(0x0000000Fu, -4) == sal<std::uint32_t, unsigned>(0x0000000Fu, 4));
    CHECK(sar<std::uint32_t, int>(0x0000000Fu, -4) == 0x000000F0u);
  }

  SUBCASE("signed V, negative value") {
    // right arithmetic by -(-shift) means left shift by magnitude
    CHECK(sar<std::int32_t, int>(-8, -1) == sal<std::int32_t, unsigned>(-8, 1));
    CHECK(sar<std::int32_t, int>(-8, -1) == -16);
  }

  SUBCASE("signed V, non-negative value") {
    CHECK(sar<std::int32_t, int>(1024, -5) == sal<std::int32_t, unsigned>(1024, 5));
    CHECK(sar<std::int32_t, int>(1024, -5) == 32768);
  }

  SUBCASE("extreme negative shift: shift == min(S)") {
    constexpr int smin = std::numeric_limits<int>::min();
    using U = std::make_unsigned_t<int>;
    const auto magnitude = U{} - static_cast<U>(smin);

    CHECK(sar<std::uint32_t, int>(0x00000001u, smin) == sal<std::uint32_t, U>(0x00000001u, magnitude));
    CHECK(sar<std::int32_t, int>(1, smin) == sal<std::int32_t, U>(1, magnitude));
  }
}

TEST_CASE("sar - cross-size S and V") {
  SUBCASE("S narrower than V (std::int8_t -> std::uint64_t)") {
    CHECK(sar<std::uint64_t, std::int8_t>(0x8000000000000000ull, std::int8_t{63}) == 1ull);
    CHECK(sar<std::uint64_t, std::int8_t>(0x00000000000000FFull, std::int8_t{7}) == 1ull);
  }

  SUBCASE("S wider than V (std::int64_t -> std::uint8_t)") {
    CHECK(sar<std::uint8_t, std::int64_t>(0x80u, std::int64_t{7}) == static_cast<std::uint8_t>(0x01u));
    CHECK(sar<std::uint8_t, std::int64_t>(0xFFu, std::int64_t{100}) == static_cast<std::uint8_t>(0));
  }

  SUBCASE("Unsigned S: no negative shifts, big shifts are zeroed out or -1 per sign") {
    CHECK(sar<std::uint16_t, std::uint8_t>(0x00F0u, static_cast<std::uint8_t>(4)) == static_cast<std::uint16_t>(0x000Fu));
    CHECK(sar<std::uint16_t, std::uint8_t>(0xFFFFu, static_cast<std::uint8_t>(255)) == static_cast<std::uint16_t>(0));
    CHECK(sar<std::int16_t, std::uint8_t>(-1, static_cast<std::uint8_t>(255)) == static_cast<std::int16_t>(-1));
  }
}

TEST_CASE("shl - basic invariants") {
  SUBCASE("shift == 0 returns value") {
    CHECK(shl<std::uint32_t, int>(0x12345678u, 0) == 0x12345678u);
    CHECK(shl<std::int32_t, int>(123, 0) == 123);
    CHECK(shl<std::int32_t, int>(0, 0) == 0);
  }

  SUBCASE("value == 0 returns 0 for any shift") {
    CHECK(shl<std::uint32_t, int>(0u, 0) == 0u);
    CHECK(shl<std::uint32_t, int>(0u, 5) == 0u);
    CHECK(shl<std::uint32_t, int>(0u, -5) == 0u);
  }
}

TEST_CASE("shl - non-negative shifts within bit width") {
  SUBCASE("unsigned V, safe positive shifts") {
    CHECK(shl<std::uint8_t, int>(0x12u, 1) == static_cast<std::uint8_t>(0x24u));
    CHECK(shl<std::uint16_t, int>(0x0003u, 2) == static_cast<std::uint16_t>(0x000Cu));
    CHECK(shl<std::uint32_t, int>(1u, 5) == 32u);
    CHECK(shl<std::uint64_t, int>(7u, 3) == 56u);
  }

  SUBCASE("signed V, use small shifts and non-negative values") {
    CHECK(shl<std::int8_t, int>(3, 1) == 6);
    CHECK(shl<std::int16_t, int>(5, 2) == 20);
    CHECK(shl<std::int32_t, int>(7, 3) == 56);
    CHECK(shl<std::int64_t, int>(9, 4) == 144);
  }
}

TEST_CASE("shl - shift >= bit width returns 0") {
  SUBCASE("unsigned V") {
    constexpr auto bw8 = bitWidth<std::uint8_t>();
    constexpr auto bw32 = bitWidth<std::uint32_t>();
    constexpr auto bw64 = bitWidth<std::uint64_t>();

    CHECK(shl<std::uint8_t, int>(0xFFu, static_cast<int>(bw8)) == 0u);
    CHECK(shl<std::uint8_t, int>(0xFFu, 100) == 0u);

    CHECK(shl<std::uint32_t, int>(0xFFFFFFFFu, static_cast<int>(bw32)) == 0u);
    CHECK(shl<std::uint64_t, int>(0xFFFFFFFFFFFFFFFFull, static_cast<int>(bw64)) == 0ull);
  }

  SUBCASE("signed V") {
    constexpr auto bw16 = bitWidth<std::int16_t>();
    CHECK(shl<std::int16_t, int>(123, static_cast<int>(bw16)) == 0);
    CHECK(shl<std::int32_t, int>(456, 1000) == 0);
  }
}

TEST_CASE("shl - negative shift delegates to shr (right logical)") {
  SUBCASE("unsigned V: negative shift becomes right logical via shr") {
    CHECK(shl<std::uint32_t, int>(0xF0000000u, -4)
          == shr<std::uint32_t, unsigned>(0xF0000000u, 4));
    CHECK(shl<std::uint32_t, int>(0xF0000000u, -4) == 0x0F000000u);
  }

  SUBCASE("signed V, non-negative value: right logical via shr") {
    CHECK(shl<std::int32_t, int>(1024, -5)
          == shr<std::int32_t, unsigned>(1024, 5));
    CHECK(shl<std::int32_t, int>(1024, -5) == 32);
  }

  SUBCASE("extreme negative shift: shift == min(S)") {
    constexpr int smin = std::numeric_limits<int>::min();
    using U = std::make_unsigned_t<int>;
    const auto magnitude = U{} - static_cast<U>(smin);

    CHECK(shl<std::uint32_t, int>(0xDEADBEEFu, smin)
          == shr<std::uint32_t, U>(0xDEADBEEFu, magnitude));
    CHECK(shl<std::int32_t, int>(1, smin)
          == shr<std::int32_t, U>(1, magnitude));
  }
}

TEST_CASE("shl - cross-size S and V") {
  SUBCASE("S narrower than V (std::int8_t -> std::uint64_t)") {
    CHECK(shl<std::uint64_t, std::int8_t>(1ull, std::int8_t{5}) == 32ull);
    CHECK(shl<std::uint64_t, std::int8_t>(0xFFFFFFFFFFFFFFFFull, std::int8_t{64}) == 0ull);
  }

  SUBCASE("S wider than V (std::int64_t -> std::uint8_t)") {
    CHECK(shl<std::uint8_t, std::int64_t>(0x01u, std::int64_t{3}) == static_cast<std::uint8_t>(0x08u));
    CHECK(shl<std::uint8_t, std::int64_t>(0xFFu, std::int64_t{100}) == static_cast<std::uint8_t>(0));
  }

  SUBCASE("Unsigned S: no negative shifts, big shifts are zeroed out") {
    CHECK(shl<std::uint16_t, std::uint8_t>(0x0001u, static_cast<std::uint8_t>(4)) == static_cast<std::uint16_t>(0x0010u));
    CHECK(shl<std::uint16_t, std::uint8_t>(0xFFFFu, static_cast<std::uint8_t>(255)) == static_cast<std::uint16_t>(0));
  }
}

TEST_CASE("shr - basic invariants") {
  SUBCASE("shift == 0 returns value") {
    CHECK(shr<std::uint32_t, int>(0x12345678u, 0) == 0x12345678u);
    CHECK(shr<std::int32_t, int>(123, 0) == 123);
    CHECK(shr<std::int32_t, int>(-456, 0) == -456);
  }

  SUBCASE("value == 0 returns 0 for any shift") {
    CHECK(shr<std::uint32_t, int>(0u, 0) == 0u);
    CHECK(shr<std::uint32_t, int>(0u, 5) == 0u);
    CHECK(shr<std::uint32_t, int>(0u, -5) == 0u);
  }
}

TEST_CASE("shr - non-negative shifts within bit width") {
  SUBCASE("unsigned V behaves like built-in >> with zero-fill") {
    CHECK(shr<std::uint8_t, int>(0x80u, 1) == static_cast<std::uint8_t>(0x40u));
    CHECK(shr<std::uint16_t, int>(0x00F0u, 4) == static_cast<std::uint16_t>(0x000Fu));
    CHECK(shr<std::uint32_t, int>(0x80000000u, 31) == 1u);
    CHECK(shr<std::uint64_t, int>(0xF000000000000000ull, 4) == 0x0F00000000000000ull);
  }

  SUBCASE("signed V also zero-fills (logical right)") {
    CHECK(shr<std::int8_t, int>(static_cast<std::int8_t>(-128), 1) == static_cast<std::int8_t>(0x40));
    CHECK(shr<std::int16_t, int>(static_cast<std::int16_t>(-1), 1) == static_cast<std::int16_t>(0x7FFF));
    CHECK(shr<std::int32_t, int>(static_cast<std::int32_t>(-1024), 5) == static_cast<std::int32_t>(0x07FFFFE0));
    CHECK(shr<std::int64_t, int>(static_cast<std::int64_t>(-9), 3) == static_cast<std::int64_t>(0x1FFFFFFFFFFFFFFE));
  }
}

TEST_CASE("shr - shift >= bit width returns 0") {
  SUBCASE("unsigned V") {
    constexpr auto bw8 = bitWidth<std::uint8_t>();
    constexpr auto bw32 = bitWidth<std::uint32_t>();
    constexpr auto bw64 = bitWidth<std::uint64_t>();

    CHECK(shr<std::uint8_t, int>(0xFFu, static_cast<int>(bw8)) == 0u);
    CHECK(shr<std::uint8_t, int>(0xFFu, 100) == 0u);

    CHECK(shr<std::uint32_t, int>(0xFFFFFFFFu, static_cast<int>(bw32)) == 0u);
    CHECK(shr<std::uint64_t, int>(0xFFFFFFFFFFFFFFFFull, static_cast<int>(bw64)) == 0ull);
  }

  SUBCASE("signed V") {
    constexpr auto bw16 = bitWidth<std::int16_t>();
    constexpr auto bw32 = bitWidth<std::int32_t>();
    CHECK(shr<std::int16_t, int>(-1, static_cast<int>(bw16)) == 0);
    CHECK(shr<std::int32_t, int>(-456, 1000) == 0);
    CHECK(shr<std::int32_t, int>(456, 1000) == 0);
  }
}

TEST_CASE("shr - negative shift delegates to shl (left logical)") {
  SUBCASE("unsigned V: negative shift becomes left logical via shl") {
    CHECK(shr<std::uint32_t, int>(0x0000000Fu, -4)
          == shl<std::uint32_t, unsigned>(0x0000000Fu, 4));
    CHECK(shr<std::uint32_t, int>(0x0000000Fu, -4) == 0x000000F0u);
  }

  SUBCASE("signed V, non-negative value") {
    CHECK(shr<std::int32_t, int>(1024, -5)
          == shl<std::int32_t, unsigned>(1024, 5));
    CHECK(shr<std::int32_t, int>(1024, -5) == 32768);
  }

  SUBCASE("extreme negative shift: shift == min(S)") {
    constexpr int smin = std::numeric_limits<int>::min();
    using U = std::make_unsigned_t<int>;
    const auto magnitude = U{} - static_cast<U>(smin);

    CHECK(shr<std::uint32_t, int>(0x00000001u, smin)
          == shl<std::uint32_t, U>(0x00000001u, magnitude));
    CHECK(shr<std::int32_t, int>(1, smin)
          == shl<std::int32_t, U>(1, magnitude));
  }
}

TEST_CASE("shr - cross-size S and V") {
  SUBCASE("S narrower than V (std::int8_t -> std::uint64_t)") {
    CHECK(shr<std::uint64_t, std::int8_t>(0x8000000000000000ull, std::int8_t{63}) == 1ull);
    CHECK(shr<std::uint64_t, std::int8_t>(0x00000000000000FFull, std::int8_t{8}) == 0ull);
  }

  SUBCASE("S wider than V (std::int64_t -> std::uint8_t)") {
    CHECK(shr<std::uint8_t, std::int64_t>(0x80u, std::int64_t{7}) == static_cast<std::uint8_t>(0x01u));
    CHECK(shr<std::uint8_t, std::int64_t>(0xFFu, std::int64_t{100}) == static_cast<std::uint8_t>(0));
  }

  SUBCASE("Unsigned S: no negative shifts, big shifts are zeroed out") {
    CHECK(shr<std::uint16_t, std::uint8_t>(0x00F0u, static_cast<std::uint8_t>(4)) == static_cast<std::uint16_t>(0x000Fu));
    CHECK(shr<std::uint16_t, std::uint8_t>(0xFFFFu, static_cast<std::uint8_t>(255)) == static_cast<std::uint16_t>(0));
    CHECK(shr<std::int16_t, std::uint8_t>(-1, static_cast<std::uint8_t>(15)) == static_cast<std::int16_t>(0x0001));
  }
}
