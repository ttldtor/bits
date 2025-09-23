// Copyright (c) 2025 ttldtor.
// SPDX-License-Identifier: BSL-1.0

#include <doctest/doctest.h>

#include <bits/bits.hpp>
#include <cstddef>
#include <cstdint>
#include <limits>

using namespace org::ttldtor::bits;

template <typename T>
constexpr std::size_t bitWidth() {
  using U = std::make_unsigned_t<T>;

  return static_cast<std::size_t>(std::numeric_limits<U>::digits);
}

TEST_CASE("sal - basic invariants") {
  SUBCASE("shift == 0 returns value") {
    CHECK_EQ(sal<std::uint32_t, int>(0x12345678u, 0), 0x12345678u);
    CHECK_EQ(sal<std::int32_t, int>(123, 0), 123);
    CHECK_EQ(sal<std::int32_t, int>(-456, 0), -456);
  }

  SUBCASE("value == 0 returns 0 for any shift") {
    CHECK_EQ(sal<std::uint32_t, int>(0u, 0), 0u);
    CHECK_EQ(sal<std::uint32_t, int>(0u, 5), 0u);
    CHECK_EQ(sal<std::uint32_t, int>(0u, -5), 0u);
  }
}

TEST_CASE("sal - non-negative shifts within bit width") {
  SUBCASE("unsigned V, safe positive shifts") {
    CHECK_EQ(sal<std::uint8_t, int>(0x12u, 1), static_cast<std::uint8_t>(0x24u));
    CHECK_EQ(sal<std::uint16_t, int>(0x0003u, 2), static_cast<std::uint16_t>(0x000Cu));
    CHECK_EQ(sal<std::uint32_t, int>(1u, 5), 32u);
    CHECK_EQ(sal<std::uint64_t, int>(7u, 3), 56u);
  }

  SUBCASE("signed V, use small shifts that do not lead to overflow") {
    CHECK_EQ(sal<std::int8_t, int>(3, 1), 6);
    CHECK_EQ(sal<std::int16_t, int>(5, 2), 20);
    CHECK_EQ(sal<std::int32_t, int>(7, 3), 56);
    CHECK_EQ(sal<std::int64_t, int>(9, 4), 144);
  }
}

TEST_CASE("sal - shift >= bit width returns 0") {
  SUBCASE("unsigned V") {
    constexpr auto BIT_WIDTH_8 = bitWidth<std::uint8_t>();
    constexpr auto BIT_WIDTH_32 = bitWidth<std::uint32_t>();
    constexpr auto BIT_WIDTH_64 = bitWidth<std::uint64_t>();

    CHECK_EQ(sal<std::uint8_t, int>(0xFFu, static_cast<int>(BIT_WIDTH_8)), 0u);
    CHECK_EQ(sal<std::uint8_t, int>(0xFFu, 100), 0u);

    CHECK_EQ(sal<std::uint32_t, int>(0xFFFFFFFFu, static_cast<int>(BIT_WIDTH_32)), 0u);
    CHECK_EQ(sal<std::uint64_t, int>(0xFFFFFFFFFFFFFFFFull, static_cast<int>(BIT_WIDTH_64)), 0ull);
  }

  SUBCASE("signed V (do not rely on the behavior of negative left shift)") {
    constexpr auto BIT_WIDTH_16 = bitWidth<std::int16_t>();

    CHECK_EQ(sal<std::int16_t, int>(123, static_cast<int>(BIT_WIDTH_16)), 0);
    CHECK_EQ(sal<std::int32_t, int>(456, 1000), 0);
  }
}

TEST_CASE("sal - negative shift delegates to sar (right arithmetic)") {
  SUBCASE("signed V, negative value: sign-extend on right shift") {
    // value = -8; shifting left by -1 => arithmetic right shift by 1 => -4
    CHECK_EQ(sal<std::int32_t, int>(-8, -1), sar<std::int32_t, unsigned>(-8, 1));
    CHECK_EQ(sal<std::int32_t, int>(-8, -1), -4);

    // larger magnitude
    CHECK_EQ(sal<std::int32_t, int>(-1024, -5), sar<std::int32_t, unsigned>(-1024, 5));
    CHECK_EQ(sal<std::int32_t, int>(-1024, -5), -32);
  }

  SUBCASE("signed V, non-negative value: behaves like logical right via sar") {
    CHECK_EQ(sal<std::int32_t, int>(1024, -5), sar<std::int32_t, unsigned>(1024, 5));
    CHECK_EQ(sal<std::int32_t, int>(1024, -5), 32);
  }

  SUBCASE("unsigned V: delegating negative shift performs right shift filling zeros") {
    CHECK_EQ(sal<std::uint32_t, int>(0xF0000000u, -4), sar<std::uint32_t, unsigned>(0xF0000000u, 4));
    CHECK_EQ(sal<std::uint32_t, int>(0xF0000000u, -4), 0x0F000000u);
  }

  SUBCASE("extreme negative shift: shift == min(S)") {
    // use the minimum possible shift value for the signed S
    constexpr int MIN_SHIFT = std::numeric_limits<int>::min();
    // magnitude is correctly calculated in unsigned representation, resulting right shift >= bit width => 0 for
    // unsigned, -1/0 for signed inside sar
    CHECK_EQ(
      sal<std::uint32_t, int>(0xDEADBEEFu, MIN_SHIFT),
      sar(0xDEADBEEFu, static_cast<std::make_unsigned_t<int>>(0) - static_cast<std::make_unsigned_t<int>>(MIN_SHIFT)));
    CHECK_EQ(sal(-1, MIN_SHIFT),
             sar(-1, static_cast<std::make_unsigned_t<int>>(0) - static_cast<std::make_unsigned_t<int>>(MIN_SHIFT)));
  }
}

TEST_CASE("sal - cross-size S and V") {
  SUBCASE("S already than V (std::int8_t -> std::uint64_t)") {
    CHECK_EQ(sal<std::uint64_t, std::int8_t>(1ull, std::int8_t{5}), 32ull);
    CHECK_EQ(sal<std::uint64_t, std::int8_t>(0xFFFFFFFFFFFFFFFFull, std::int8_t{64}), 0ull);
  }

  SUBCASE("S wider than V (std::int64_t -> std::uint8_t)") {
    CHECK_EQ(sal<std::uint8_t, std::int64_t>(0x01u, std::int64_t{3}), static_cast<std::uint8_t>(0x08u));
    CHECK_EQ(sal<std::uint8_t, std::int64_t>(0xFFu, std::int64_t{100}), static_cast<std::uint8_t>(0));
  }

  SUBCASE("Unsigned S: no negative shifts, big shifts are zeroed out") {
    CHECK_EQ(sal<std::uint16_t, std::uint8_t>(0x0001u, static_cast<std::uint8_t>(4)),
             static_cast<std::uint16_t>(0x0010u));
    CHECK_EQ(sal<std::uint16_t, std::uint8_t>(0xFFFFu, static_cast<std::uint8_t>(255)), static_cast<std::uint16_t>(0));
  }
}

TEST_CASE("sar - basic invariants") {
  SUBCASE("shift == 0 returns value") {
    CHECK_EQ(sar<std::uint32_t, int>(0x12345678u, 0), 0x12345678u);
    CHECK_EQ(sar<std::int32_t, int>(123, 0), 123);
    CHECK_EQ(sar<std::int32_t, int>(-456, 0), -456);
  }

  SUBCASE("value == 0 returns 0 for any shift") {
    CHECK_EQ(sar<std::uint32_t, int>(0u, 0), 0u);
    CHECK_EQ(sar<std::uint32_t, int>(0u, 5), 0u);
    CHECK_EQ(sar<std::uint32_t, int>(0u, -5), 0u);
  }
}

TEST_CASE("sar - non-negative shifts within bit width") {
  SUBCASE("unsigned V behaves like logical right shift") {
    CHECK_EQ(sar<std::uint8_t, int>(0x80u, 1), static_cast<std::uint8_t>(0x40u));
    CHECK_EQ(sar<std::uint16_t, int>(0x00F0u, 4), static_cast<std::uint16_t>(0x000Fu));
    CHECK_EQ(sar<std::uint32_t, int>(0x80000000u, 31), 1u);
    CHECK_EQ(sar<std::uint64_t, int>(0xF000000000000000ull, 4), 0x0F00000000000000ull);
  }

  SUBCASE("signed V sign-extends on right shift") {
    CHECK_EQ(sar<std::int8_t, int>(-2, 1), static_cast<std::int8_t>(-1));
    CHECK_EQ(sar<std::int16_t, int>(-8, 1), static_cast<std::int16_t>(-4));
    CHECK_EQ(sar<std::int32_t, int>(-1024, 5), -32);
    CHECK_EQ(sar<std::int64_t, int>(-9, 3), -2);
  }
}

TEST_CASE("sar - shift >= bit width returns 0 for non-negative and -1 for negative") {
  SUBCASE("unsigned V") {
    constexpr auto BIT_WIDTH_8 = bitWidth<std::uint8_t>();
    constexpr auto BIT_WIDTH_32 = bitWidth<std::uint32_t>();
    constexpr auto BIT_WIDTH_64 = bitWidth<std::uint64_t>();

    CHECK_EQ(sar<std::uint8_t, int>(0xFFu, static_cast<int>(BIT_WIDTH_8)), 0u);
    CHECK_EQ(sar<std::uint8_t, int>(0xFFu, 100), 0u);

    CHECK_EQ(sar<std::uint32_t, int>(0xFFFFFFFFu, static_cast<int>(BIT_WIDTH_32)), 0u);
    CHECK_EQ(sar<std::uint64_t, int>(0xFFFFFFFFFFFFFFFFull, static_cast<int>(BIT_WIDTH_64)), 0ull);
  }

  SUBCASE("signed V") {
    constexpr auto BIT_WIDTH_16 = bitWidth<std::int16_t>();

    CHECK_EQ(sar<std::int16_t, int>(123, static_cast<int>(BIT_WIDTH_16)), 0);
    CHECK_EQ(sar<std::int16_t, int>(-123, static_cast<int>(BIT_WIDTH_16)), static_cast<std::int16_t>(-1));

    CHECK_EQ(sar<std::int32_t, int>(456, 1000), 0);
    CHECK_EQ(sar<std::int32_t, int>(-456, 1000), static_cast<std::int32_t>(-1));
  }
}

TEST_CASE("sar - negative shift delegates to sal (left arithmetic)") {
  SUBCASE("unsigned V: negative shift becomes left logical via sal delegation") {
    CHECK_EQ(sar<std::uint32_t, int>(0x0000000Fu, -4), sal<std::uint32_t, unsigned>(0x0000000Fu, 4));
    CHECK_EQ(sar<std::uint32_t, int>(0x0000000Fu, -4), 0x000000F0u);
  }

  SUBCASE("signed V, negative value") {
    // right arithmetic by -(-shift) means left shift by magnitude
    CHECK_EQ(sar<std::int32_t, int>(-8, -1), sal<std::int32_t, unsigned>(-8, 1));
    CHECK_EQ(sar<std::int32_t, int>(-8, -1), -16);
  }

  SUBCASE("signed V, non-negative value") {
    CHECK_EQ(sar<std::int32_t, int>(1024, -5), sal<std::int32_t, unsigned>(1024, 5));
    CHECK_EQ(sar<std::int32_t, int>(1024, -5), 32768);
  }

  SUBCASE("extreme negative shift: shift == min(S)") {
    constexpr int MIN_SHIFT = std::numeric_limits<int>::min();
    using U = std::make_unsigned_t<int>;
    constexpr auto magnitude = U{} - static_cast<U>(MIN_SHIFT);

    CHECK_EQ(sar<std::uint32_t, int>(0x00000001u, MIN_SHIFT), sal<std::uint32_t, U>(0x00000001u, magnitude));
    CHECK_EQ(sar<std::int32_t, int>(1, MIN_SHIFT), sal<std::int32_t, U>(1, magnitude));
  }
}

TEST_CASE("sar - cross-size S and V") {
  SUBCASE("S narrower than V (std::int8_t -> std::uint64_t)") {
    CHECK_EQ(sar<std::uint64_t, std::int8_t>(0x8000000000000000ull, std::int8_t{63}), 1ull);
    CHECK_EQ(sar<std::uint64_t, std::int8_t>(0x00000000000000FFull, std::int8_t{7}), 1ull);
  }

  SUBCASE("S wider than V (std::int64_t -> std::uint8_t)") {
    CHECK_EQ(sar<std::uint8_t, std::int64_t>(0x80u, std::int64_t{7}), static_cast<std::uint8_t>(0x01u));
    CHECK_EQ(sar<std::uint8_t, std::int64_t>(0xFFu, std::int64_t{100}), static_cast<std::uint8_t>(0));
  }

  SUBCASE("Unsigned S: no negative shifts, big shifts are zeroed out or -1 per sign") {
    CHECK_EQ(sar<std::uint16_t, std::uint8_t>(0x00F0u, static_cast<std::uint8_t>(4)),
             static_cast<std::uint16_t>(0x000Fu));
    CHECK_EQ(sar<std::uint16_t, std::uint8_t>(0xFFFFu, static_cast<std::uint8_t>(255)), static_cast<std::uint16_t>(0));
    CHECK_EQ(sar<std::int16_t, std::uint8_t>(-1, static_cast<std::uint8_t>(255)), static_cast<std::int16_t>(-1));
  }
}

TEST_CASE("shl - basic invariants") {
  SUBCASE("shift == 0 returns value") {
    CHECK_EQ(shl<std::uint32_t, int>(0x12345678u, 0), 0x12345678u);
    CHECK_EQ(shl<std::int32_t, int>(123, 0), 123);
    CHECK_EQ(shl<std::int32_t, int>(0, 0), 0);
  }

  SUBCASE("value == 0 returns 0 for any shift") {
    CHECK_EQ(shl<std::uint32_t, int>(0u, 0), 0u);
    CHECK_EQ(shl<std::uint32_t, int>(0u, 5), 0u);
    CHECK_EQ(shl<std::uint32_t, int>(0u, -5), 0u);
  }
}

TEST_CASE("shl - non-negative shifts within bit width") {
  SUBCASE("unsigned V, safe positive shifts") {
    CHECK_EQ(shl<std::uint8_t, int>(0x12u, 1), static_cast<std::uint8_t>(0x24u));
    CHECK_EQ(shl<std::uint16_t, int>(0x0003u, 2), static_cast<std::uint16_t>(0x000Cu));
    CHECK_EQ(shl<std::uint32_t, int>(1u, 5), 32u);
    CHECK_EQ(shl<std::uint64_t, int>(7u, 3), 56u);
  }

  SUBCASE("signed V, use small shifts and non-negative values") {
    CHECK_EQ(shl<std::int8_t, int>(3, 1), 6);
    CHECK_EQ(shl<std::int16_t, int>(5, 2), 20);
    CHECK_EQ(shl<std::int32_t, int>(7, 3), 56);
    CHECK_EQ(shl<std::int64_t, int>(9, 4), 144);
  }
}

TEST_CASE("shl - shift >= bit width returns 0") {
  SUBCASE("unsigned V") {
    constexpr auto BIT_WIDTH_8 = bitWidth<std::uint8_t>();
    constexpr auto BIT_WIDTH_32 = bitWidth<std::uint32_t>();
    constexpr auto BIT_WIDTH_64 = bitWidth<std::uint64_t>();

    CHECK_EQ(shl<std::uint8_t, int>(0xFFu, static_cast<int>(BIT_WIDTH_8)), 0u);
    CHECK_EQ(shl<std::uint8_t, int>(0xFFu, 100), 0u);

    CHECK_EQ(shl<std::uint32_t, int>(0xFFFFFFFFu, static_cast<int>(BIT_WIDTH_32)), 0u);
    CHECK_EQ(shl<std::uint64_t, int>(0xFFFFFFFFFFFFFFFFull, static_cast<int>(BIT_WIDTH_64)), 0ull);
  }

  SUBCASE("signed V") {
    constexpr auto BIT_WIDTH_16 = bitWidth<std::int16_t>();

    CHECK_EQ(shl<std::int16_t, int>(123, static_cast<int>(BIT_WIDTH_16)), 0);
    CHECK_EQ(shl<std::int32_t, int>(456, 1000), 0);
  }
}

TEST_CASE("shl - negative shift delegates to shr (right logical)") {
  SUBCASE("unsigned V: negative shift becomes right logical via shr") {
    CHECK_EQ(shl<std::uint32_t, int>(0xF0000000u, -4), shr<std::uint32_t, unsigned>(0xF0000000u, 4));
    CHECK_EQ(shl<std::uint32_t, int>(0xF0000000u, -4), 0x0F000000u);
  }

  SUBCASE("signed V, non-negative value: right logical via shr") {
    CHECK_EQ(shl<std::int32_t, int>(1024, -5), shr<std::int32_t, unsigned>(1024, 5));
    CHECK_EQ(shl<std::int32_t, int>(1024, -5), 32);
  }

  SUBCASE("extreme negative shift: shift == min(S)") {
    constexpr int MIN_SHIFT = std::numeric_limits<int>::min();
    using U = std::make_unsigned_t<int>;
    constexpr auto MAGNITUDE = U{} - static_cast<U>(MIN_SHIFT);

    CHECK_EQ(shl<std::uint32_t, int>(0xDEADBEEFu, MIN_SHIFT), shr<std::uint32_t, U>(0xDEADBEEFu, MAGNITUDE));
    CHECK_EQ(shl<std::int32_t, int>(1, MIN_SHIFT), shr<std::int32_t, U>(1, MAGNITUDE));
  }
}

TEST_CASE("shl - cross-size S and V") {
  SUBCASE("S narrower than V (std::int8_t -> std::uint64_t)") {
    CHECK_EQ(shl<std::uint64_t, std::int8_t>(1ull, std::int8_t{5}), 32ull);
    CHECK_EQ(shl<std::uint64_t, std::int8_t>(0xFFFFFFFFFFFFFFFFull, std::int8_t{64}), 0ull);
  }

  SUBCASE("S wider than V (std::int64_t -> std::uint8_t)") {
    CHECK_EQ(shl<std::uint8_t, std::int64_t>(0x01u, std::int64_t{3}), static_cast<std::uint8_t>(0x08u));
    CHECK_EQ(shl<std::uint8_t, std::int64_t>(0xFFu, std::int64_t{100}), static_cast<std::uint8_t>(0));
  }

  SUBCASE("Unsigned S: no negative shifts, big shifts are zeroed out") {
    CHECK_EQ(shl<std::uint16_t, std::uint8_t>(0x0001u, static_cast<std::uint8_t>(4)),
             static_cast<std::uint16_t>(0x0010u));
    CHECK_EQ(shl<std::uint16_t, std::uint8_t>(0xFFFFu, static_cast<std::uint8_t>(255)), static_cast<std::uint16_t>(0));
  }
}

TEST_CASE("shr - basic invariants") {
  SUBCASE("shift == 0 returns value") {
    CHECK_EQ(shr<std::uint32_t, int>(0x12345678u, 0), 0x12345678u);
    CHECK_EQ(shr<std::int32_t, int>(123, 0), 123);
    CHECK_EQ(shr<std::int32_t, int>(-456, 0), -456);
  }

  SUBCASE("value == 0 returns 0 for any shift") {
    CHECK_EQ(shr<std::uint32_t, int>(0u, 0), 0u);
    CHECK_EQ(shr<std::uint32_t, int>(0u, 5), 0u);
    CHECK_EQ(shr<std::uint32_t, int>(0u, -5), 0u);
  }
}

TEST_CASE("shr - non-negative shifts within bit width") {
  SUBCASE("unsigned V behaves like built-in >> with zero-fill") {
    CHECK_EQ(shr<std::uint8_t, int>(0x80u, 1), static_cast<std::uint8_t>(0x40u));
    CHECK_EQ(shr<std::uint16_t, int>(0x00F0u, 4), static_cast<std::uint16_t>(0x000Fu));
    CHECK_EQ(shr<std::uint32_t, int>(0x80000000u, 31), 1u);
    CHECK_EQ(shr<std::uint64_t, int>(0xF000000000000000ull, 4), 0x0F00000000000000ull);
  }

  SUBCASE("signed V also zero-fills (logical right)") {
    CHECK_EQ(shr<std::int8_t, int>(static_cast<std::int8_t>(-128), 1), static_cast<std::int8_t>(0x40));
    CHECK_EQ(shr<std::int16_t, int>(static_cast<std::int16_t>(-1), 1), static_cast<std::int16_t>(0x7FFF));
    CHECK_EQ(shr<std::int32_t, int>(static_cast<std::int32_t>(-1024), 5), static_cast<std::int32_t>(0x07FFFFE0));
    CHECK_EQ(shr<std::int64_t, int>(static_cast<std::int64_t>(-9), 3), static_cast<std::int64_t>(0x1FFFFFFFFFFFFFFE));
  }
}

TEST_CASE("shr - shift >= bit width returns 0") {
  SUBCASE("unsigned V") {
    constexpr auto BIT_WIDTH_8 = bitWidth<std::uint8_t>();
    constexpr auto BIT_WIDTH_32 = bitWidth<std::uint32_t>();
    constexpr auto BIT_WIDTH_64 = bitWidth<std::uint64_t>();

    CHECK_EQ(shr<std::uint8_t, int>(0xFFu, static_cast<int>(BIT_WIDTH_8)), 0u);
    CHECK_EQ(shr<std::uint8_t, int>(0xFFu, 100), 0u);

    CHECK_EQ(shr<std::uint32_t, int>(0xFFFFFFFFu, static_cast<int>(BIT_WIDTH_32)), 0u);
    CHECK_EQ(shr<std::uint64_t, int>(0xFFFFFFFFFFFFFFFFull, static_cast<int>(BIT_WIDTH_64)), 0ull);
  }

  SUBCASE("signed V") {
    constexpr auto BIT_WIDTH_16 = bitWidth<std::int16_t>();

    CHECK_EQ(shr<std::int16_t, int>(-1, static_cast<int>(BIT_WIDTH_16)), 0);
    CHECK_EQ(shr<std::int32_t, int>(-456, 1000), 0);
    CHECK_EQ(shr<std::int32_t, int>(456, 1000), 0);
  }
}

TEST_CASE("shr - negative shift delegates to shl (left logical)") {
  SUBCASE("unsigned V: negative shift becomes left logical via shl") {
    CHECK_EQ(shr<std::uint32_t, int>(0x0000000Fu, -4), shl<std::uint32_t, unsigned>(0x0000000Fu, 4));
    CHECK_EQ(shr<std::uint32_t, int>(0x0000000Fu, -4), 0x000000F0u);
  }

  SUBCASE("signed V, non-negative value") {
    CHECK_EQ(shr<std::int32_t, int>(1024, -5), shl<std::int32_t, unsigned>(1024, 5));
    CHECK_EQ(shr<std::int32_t, int>(1024, -5), 32768);
  }

  SUBCASE("extreme negative shift: shift == min(S)") {
    constexpr int MIN_SHIFT = std::numeric_limits<int>::min();
    using U = std::make_unsigned_t<int>;
    constexpr auto MAGNITUDE = U{} - static_cast<U>(MIN_SHIFT);

    CHECK_EQ(shr<std::uint32_t, int>(0x00000001u, MIN_SHIFT), shl<std::uint32_t, U>(0x00000001u, MAGNITUDE));
    CHECK_EQ(shr<std::int32_t, int>(1, MIN_SHIFT), shl<std::int32_t, U>(1, MAGNITUDE));
  }
}

TEST_CASE("shr - cross-size S and V") {
  SUBCASE("S narrower than V (std::int8_t -> std::uint64_t)") {
    CHECK_EQ(shr<std::uint64_t, std::int8_t>(0x8000000000000000ull, std::int8_t{63}), 1ull);
    CHECK_EQ(shr<std::uint64_t, std::int8_t>(0x00000000000000FFull, std::int8_t{8}), 0ull);
  }

  SUBCASE("S wider than V (std::int64_t -> std::uint8_t)") {
    CHECK_EQ(shr<std::uint8_t, std::int64_t>(0x80u, std::int64_t{7}), static_cast<std::uint8_t>(0x01u));
    CHECK_EQ(shr<std::uint8_t, std::int64_t>(0xFFu, std::int64_t{100}), static_cast<std::uint8_t>(0));
  }

  SUBCASE("Unsigned S: no negative shifts, big shifts are zeroed out") {
    CHECK_EQ(shr<std::uint16_t, std::uint8_t>(0x00F0u, static_cast<std::uint8_t>(4)),
             static_cast<std::uint16_t>(0x000Fu));
    CHECK_EQ(shr<std::uint16_t, std::uint8_t>(0xFFFFu, static_cast<std::uint8_t>(255)), static_cast<std::uint16_t>(0));
    CHECK_EQ(shr<std::int16_t, std::uint8_t>(-1, static_cast<std::uint8_t>(15)), static_cast<std::int16_t>(0x0001));
  }
}
