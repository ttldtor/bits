// Copyright (c) 2025 ttldtor.
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <climits>
#include <concepts>
#include <type_traits>

namespace org::ttldtor::bits {

namespace detail {
template <typename T, typename U>
using Gt = std::conditional_t<sizeof(T) >= sizeof(U), T, U>;

template <typename T, typename...>
struct MaxImpl {
  using Type = T;
};

template <typename T, typename U, typename... Ts>
struct MaxImpl<T, U, Ts...> {
  using Type = MaxImpl<Gt<T, U>, Ts...>::Type;
};
}  // namespace detail

/**
 * Returns max type by size (first is better)
 */
template <typename... Ts>
using Max = detail::MaxImpl<Ts...>::Type;

template <std::unsigned_integral T>
static constexpr T bitsAreSet(T sourceBits, T bitMaskToCheck) {
  return (sourceBits & bitMaskToCheck) != 0;
}

template <std::integral SB, std::integral M>
static constexpr SB bitsAreSet(SB sourceBits, M bitMaskToCheck) {
  using MaxType = Max<SB, M>;

  if constexpr (std::is_signed_v<SB> || std::is_signed_v<M>) {
    using U = std::make_unsigned_t<MaxType>;

    return static_cast<SB>(bitsAreSet(static_cast<U>(sourceBits), static_cast<U>(bitMaskToCheck)));
  } else {
    return static_cast<SB>(bitsAreSet(static_cast<MaxType>(sourceBits), static_cast<MaxType>(bitMaskToCheck)));
  }
}

template <std::unsigned_integral T>
static constexpr T setBits(T sourceBits, T bitMaskToSet) {
  return sourceBits | bitMaskToSet;
}

template <std::integral SB, std::integral M>
static constexpr SB setBits(SB sourceBits, M bitMaskToSet) {
  using MaxType = Max<SB, M>;

  if constexpr (std::is_signed_v<SB> || std::is_signed_v<M>) {
    using U = std::make_unsigned_t<MaxType>;

    return static_cast<SB>(setBits(static_cast<U>(sourceBits), static_cast<U>(bitMaskToSet)));
  } else {
    return static_cast<SB>(setBits(static_cast<MaxType>(sourceBits), static_cast<MaxType>(bitMaskToSet)));
  }
}

template <std::unsigned_integral T>
static constexpr T resetBits(T sourceBits, T bitMaskToReset) {
  return sourceBits & ~bitMaskToReset;
}

template <std::integral SB, std::integral M>
// ReSharper disable once CppDFAConstantParameter
static constexpr SB resetBits(SB sourceBits, M bitMaskToReset) {
  using MaxType = Max<SB, M>;

  if constexpr (std::is_signed_v<SB> || std::is_signed_v<M>) {
    using U = std::make_unsigned_t<MaxType>;

    return static_cast<SB>(resetBits(static_cast<U>(sourceBits), static_cast<U>(bitMaskToReset)));
  } else {
    return static_cast<SB>(resetBits(static_cast<MaxType>(sourceBits), static_cast<MaxType>(bitMaskToReset)));
  }
}

/**
 * Performs a right arithmetic bit shift operation (>> in Java, C, etc.). The sign bit is extended to preserve the
 * signedness of the number.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::sal() "left arithmetic shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then, if the `value` is
 * negative (a signed integer type), `-1` will be returned, and if positive, then `0` will be returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted.
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V sar(V value, S shift) noexcept;

/**
 * Performs a left arithmetic bit shift operation (<< in Java, C, etc.).
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::sar() "right arithmetic shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V leftArithmeticShift(V value, S shift) noexcept {
  using UnsignedShift = std::make_unsigned_t<S>;

  if constexpr (std::is_signed_v<S>) {
    if (shift < 0) {
      const auto magnitude = UnsignedShift{} - static_cast<UnsignedShift>(shift);

      return sar(value, magnitude);
    }
  }

  if (shift == 0 || value == 0) {
    return value;
  }

  const auto unsignedShift = static_cast<UnsignedShift>(shift);
  const auto bitWidth = static_cast<UnsignedShift>(sizeof(V) * CHAR_BIT);

  if (unsignedShift >= bitWidth) {
    return V{0};
  }

  return static_cast<V>(value << unsignedShift);
}

/**
 * Performs a left arithmetic bit shift operation (<< in Java, C, etc.).
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::sar() "right arithmetic shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted.
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V sal(V value, S shift) noexcept {
  return leftArithmeticShift(value, shift);
}

/**
 * Performs a right arithmetic bit shift operation (>> in Java, C, etc.). The sign bit is extended to preserve the
 * signedness of the number.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::sal() "left arithmetic shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then, if the `value` is
 * negative (a signed integer type), `-1` will be returned, and if positive, then `0` will be returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V rightArithmeticShift(V value, S shift) noexcept {
  using UnsignedShift = std::make_unsigned_t<S>;

  if constexpr (std::is_signed_v<S>) {
    if (shift < 0) {
      const UnsignedShift magnitude = UnsignedShift{} - static_cast<UnsignedShift>(shift);

      return sal(value, magnitude);
    }
  }

  if (shift == 0 || value == 0) {
    return value;
  }

  const auto unsignedShift = static_cast<UnsignedShift>(shift);
  const auto bitWidth = static_cast<UnsignedShift>(sizeof(V) * CHAR_BIT);

  if (unsignedShift >= bitWidth) {
    return value < 0 ? static_cast<V>(-1) : V{0};
  }

  return static_cast<V>(value >> unsignedShift);
}

/**
 * Performs a right arithmetic bit shift operation (>> in Java, C, etc.). The sign bit is extended to preserve the
 * signedness of the number.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::sal() "left arithmetic shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then, if the `value` is
 * negative (a signed integer type), `-1` will be returned, and if positive, then `0` will be returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted.
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V sar(V value, S shift) noexcept {
  return rightArithmeticShift(value, shift);
}

/**
 * Performs a right logical bit shift operation (>>> in Java). Fills the left bits by zero.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::shl() "left logical shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted.
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V shr(V value, S shift) noexcept;

/**
 * Performs a left logical bit shift operation.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::shr() "right logical shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V leftLogicalShift(V value, S shift) noexcept {
  using UnsignedShift = std::make_unsigned_t<S>;

  if constexpr (std::is_signed_v<S>) {
    if (shift < 0) {
      return shr(value, -shift);
    }
  }

  if (shift == 0 || value == 0) {
    return value;
  }

  auto unsignedShift = static_cast<std::make_unsigned_t<S>>(shift);

  if (unsignedShift >= sizeof(V) * CHAR_BIT) {
    return 0;
  }

  return value << unsignedShift;
}

/**
 * Performs a left logical bit shift operation.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::shr() "right logical shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted.
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V shl(V value, S shift) noexcept {
  return leftLogicalShift(value, shift);
}

/**
 * Performs a right logical bit shift operation (>>> in Java). Fills the left bits by zero.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::shl() "left logical shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V rightLogicalShift(V value, S shift) noexcept {
  if constexpr (std::is_signed_v<S>) {
    if (shift < 0) {
      return shl(value, -shift);
    }
  }

  if (shift == 0 || value == 0) {
    return value;
  }

  auto unsignedShift = static_cast<std::make_unsigned_t<S>>(shift);

  if (unsignedShift >= sizeof(V) * CHAR_BIT) {
    return 0;
  }

  return static_cast<V>(static_cast<std::make_unsigned_t<V>>(value) >> unsignedShift);
}

/**
 * Performs a right logical bit shift operation (>>> in Java). Fills the left bits by zero.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::shl() "left logical shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam S The type of `shift`
 * @param value The value to be shifted.
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::integral S>
static constexpr V shr(V value, S shift) noexcept {
  return rightLogicalShift(value, shift);
}

template <std::integral A, std::integral B>
static constexpr A andOp(A a, B b) noexcept {
  using Common = std::make_unsigned_t<Max<A, B>>;

  return static_cast<A>(static_cast<Common>(a) & static_cast<Common>(b));
}

template <std::integral A, std::integral B>
static constexpr A orOp(A a, B b) noexcept {
  using Common = std::make_unsigned_t<Max<A, B>>;

  return static_cast<A>(static_cast<Common>(a) | static_cast<Common>(b));
}

template <std::integral A, std::integral B>
static constexpr A xorOp(A a, B b) noexcept {
  using Common = std::make_unsigned_t<Max<A, B>>;

  return static_cast<A>(static_cast<Common>(a) ^ static_cast<Common>(b));
}

}  // namespace org::ttldtor::bits