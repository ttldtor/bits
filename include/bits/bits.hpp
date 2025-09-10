// Copyright (c) 2025 ttldtor.
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <climits>
#include <concepts>
#include <limits>
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
constexpr V sar(V value, S shift) noexcept;

/**
 * Performs a left arithmetic bit shift operation (<< in Java, C, etc.). The `shift` is unsigned.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam US The type of `shift`
 * @param value The value to be shifted
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::unsigned_integral US>
constexpr V leftArithmeticShift(V value, US shift) noexcept {
  using UV = std::make_unsigned_t<std::remove_cv_t<V>>;
  constexpr US width = static_cast<US>(std::numeric_limits<UV>::digits);

  if (shift < width) {
    return static_cast<V>(static_cast<UV>(value) << shift);
  }

  return V{0};
}

/**
 * Performs a left arithmetic bit shift operation (<< in Java, C, etc.). The `shift` is signed.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::sar() "right arithmetic shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then `0` will be
 * returned.
 *
 * @tparam V The type of `value`
 * @tparam SS The type of `shift`
 * @param value The value to be shifted
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::signed_integral SS>
constexpr V leftArithmeticShift(V value, SS shift) noexcept {
  using US = std::make_unsigned_t<std::remove_cv_t<SS>>;

  if (shift < 0) {
    const US magnitude = US{0} - static_cast<US>(shift);

    return sar(value, magnitude);
  }

  using UV = std::make_unsigned_t<std::remove_cv_t<V>>;
  constexpr US width = static_cast<US>(std::numeric_limits<UV>::digits);
  const US unsignedShift = static_cast<US>(shift);

  if (unsignedShift < width) {
    return static_cast<V>(static_cast<UV>(value) << unsignedShift);
  }

  return V{0};
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
constexpr V sal(V value, S shift) noexcept {
  return leftArithmeticShift(value, shift);
}

/**
 * Performs a right arithmetic bit shift operation (>> in Java, C, etc.). The `shift` is unsigned.
 * The sign bit is extended to preserve the signedness of the number.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then, if the `value` is
 * negative (a signed integer type), `-1` will be returned, and if positive, then `0` will be returned.
 *
 * @tparam V The type of `value`
 * @tparam US The type of `shift`
 * @param value The value to be shifted
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::unsigned_integral US>
constexpr V rightArithmeticShift(V value, US shift) noexcept {
  using UV = std::make_unsigned_t<std::remove_cv_t<V>>;
  constexpr US width = static_cast<US>(std::numeric_limits<UV>::digits);

  if (shift < width) {
    return static_cast<V>(value >> shift);
  }

  return value < 0 ? static_cast<V>(-1) : V{0};
}

/**
 * Performs a right arithmetic bit shift operation (>> in Java, C, etc.). The `shift` is signed.
 * The sign bit is extended to preserve the signedness of the number.
 *
 * The result of the shift will be of the same type as the `value` being shifted.
 * If the shift is a negative number of bits, then a @ref ::sal() "left arithmetic shift" will be performed.
 * If the shift size is greater than or equal to the number of bits in the shifted `value`, then, if the `value` is
 * negative (a signed integer type), `-1` will be returned, and if positive, then `0` will be returned.
 *
 * @tparam V The type of `value`
 * @tparam SS The type of `shift`
 * @param value The value to be shifted
 * @param shift The shift in bits
 * @return The shifted `value`
 */
template <std::integral V, std::signed_integral SS>
constexpr V rightArithmeticShift(V value, SS shift) noexcept {
  using US = std::make_unsigned_t<std::remove_cv_t<SS>>;

  if (shift < 0) {
    const US magnitude = US{0} - static_cast<US>(shift);

    return sal(value, magnitude);
  }

  using UV = std::make_unsigned_t<std::remove_cv_t<V>>;
  constexpr US width = static_cast<US>(std::numeric_limits<UV>::digits);
  const US unsignedShift = static_cast<US>(shift);

  if (unsignedShift < width) {
    return static_cast<V>(value >> unsignedShift);
  }

  return value < 0 ? static_cast<V>(-1) : V{0};
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
constexpr V sar(V value, S shift) noexcept {
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
constexpr V shr(V value, S shift) noexcept;

/**
 * Performs a left logical bit shift operation (shl).
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
constexpr V leftLogicalShift(V value, S shift) noexcept {
  using UnsignedShift = std::make_unsigned_t<std::remove_cv_t<S>>;

  if constexpr (std::is_signed_v<S>) {
    if (shift < 0) {
      const auto magnitude = UnsignedShift{} - static_cast<UnsignedShift>(shift);

      return shr(value, magnitude);
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
constexpr V shl(V value, S shift) noexcept {
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
constexpr V rightLogicalShift(V value, S shift) noexcept {
  using UnsignedValue = std::make_unsigned_t<std::remove_cv_t<V>>;
  using UnsignedShift = std::make_unsigned_t<std::remove_cv_t<S>>;

  if constexpr (std::is_signed_v<S>) {
    if (shift < 0) {
      const auto magnitude = UnsignedShift{} - static_cast<UnsignedShift>(shift);

      return shl(value, magnitude);
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

  return static_cast<V>(static_cast<UnsignedValue>(value) >> unsignedShift);
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
constexpr V shr(V value, S shift) noexcept {
  return rightLogicalShift(value, shift);
}

/**
 * Performs a bitwise AND operation between two values of possibly different types
 * and ensures the result is cast back to the type of the first argument.
 *
 * @tparam A The type of the first argument.
 * @tparam B The type of the second argument.
 * @param a The first operand of type A.
 * @param b The second operand of type B.
 * @return The result of the bitwise AND operation, cast to type A.
 */
template <std::integral A, std::integral B>
constexpr A andOp(A a, B b) noexcept {
  using Common = std::make_unsigned_t<Max<A, B>>;

  return static_cast<A>(static_cast<Common>(a) & static_cast<Common>(b));
}

/**
 * Performs a bitwise OR operation between two values of possibly different types
 * and ensures the result is cast back to the type of the first argument.
 *
 * @tparam A The type of the first argument.
 * @tparam B The type of the second argument.
 * @param a The first operand of type A.
 * @param b The second operand of type B.
 * @return The result of the bitwise OR operation, cast to type A.
 */
template <std::integral A, std::integral B>
constexpr A orOp(A a, B b) noexcept {
  using Common = std::make_unsigned_t<Max<A, B>>;

  return static_cast<A>(static_cast<Common>(a) | static_cast<Common>(b));
}

/**
 * Performs a bitwise XOR operation between two values of possibly different types
 * and ensures the result is cast back to the type of the first argument.
 *
 * @tparam A The type of the first argument.
 * @tparam B The type of the second argument.
 * @param a The first operand of type A.
 * @param b The second operand of type B.
 * @return The result of the bitwise XOR operation, cast to type A.
 */
template <std::integral A, std::integral B>
constexpr A xorOp(A a, B b) noexcept {
  using Common = std::make_unsigned_t<Max<A, B>>;

  return static_cast<A>(static_cast<Common>(a) ^ static_cast<Common>(b));
}

/**
 * Determines if the specified bits are set in the source value.
 *
 * @tparam T the type of arguments.
 * @param sourceBits The value to check for the presence of specific bits.
 * @param bitMaskToCheck The mask of bits to check against the source value.
 * @return `true` if the specified bits are set in the source value.
 */
template <std::unsigned_integral T>
constexpr T bitsAreSet(T sourceBits, T bitMaskToCheck) {
  return andOp(sourceBits, bitMaskToCheck) != 0;
}

/**
 * Determines if the specified bits are set in the source value.
 *
 * @tparam SB The type of the source bits (e.g., an integer type).
 * @tparam M The type of the bit mask (e.g., an integer type).
 * @param sourceBits The value to check for the presence of specific bits.
 * @param bitMaskToCheck The mask of bits to check against the source value.
 * @return `true` if the specified bits are set in the source value.
 */
template <std::integral SB, std::integral M>
constexpr SB bitsAreSet(SB sourceBits, M bitMaskToCheck) {
  using MaxType = Max<SB, M>;

  if constexpr (std::is_signed_v<SB> || std::is_signed_v<M>) {
    using U = std::make_unsigned_t<MaxType>;

    return static_cast<SB>(bitsAreSet(static_cast<U>(sourceBits), static_cast<U>(bitMaskToCheck)));
  } else {
    return static_cast<SB>(bitsAreSet(static_cast<MaxType>(sourceBits), static_cast<MaxType>(bitMaskToCheck)));
  }
}

/**
 * Sets specific bits in the source value using a bitmask.
 *
 * @tparam T The type of arguments.
 * @param sourceBits The original bits to modify.
 * @param bitMaskToSet The bitmask representing the bits to be set.
 * @return The resulting value after setting the specified bits.
 */
template <std::unsigned_integral T>
constexpr T setBits(T sourceBits, T bitMaskToSet) {
  return orOp(sourceBits, bitMaskToSet);
}

/**
 * Sets specific bits in the source value using a bitmask.
 *
 * @tparam SB The type of the source bits (e.g., an integer type).
 * @tparam M The type of the bit mask (e.g., an integer type).
 * @param sourceBits The original bits to modify.
 * @param bitMaskToSet The bitmask representing the bits to be set.
 * @return The resulting value after setting the specified bits.
 */
template <std::integral SB, std::integral M>
constexpr SB setBits(SB sourceBits, M bitMaskToSet) {
  using MaxType = Max<SB, M>;

  if constexpr (std::is_signed_v<SB> || std::is_signed_v<M>) {
    using U = std::make_unsigned_t<MaxType>;

    return static_cast<SB>(setBits(static_cast<U>(sourceBits), static_cast<U>(bitMaskToSet)));
  } else {
    return static_cast<SB>(setBits(static_cast<MaxType>(sourceBits), static_cast<MaxType>(bitMaskToSet)));
  }
}

/**
 * Resets (clears) specific bits in a bitmask.
 *
 * @tparam T The type of the source bits and the bitmask. Should be an integral type.
 * @param sourceBits The original bitmask containing the bits to modify.
 * @param bitMaskToReset The bitmask specifying the bits to reset.
 * @return The resulting value after resetting the specified bits.
 */
template <std::unsigned_integral T>
constexpr T resetBits(T sourceBits, T bitMaskToReset) {
  return andOp(sourceBits, ~bitMaskToReset);
}

/**
 * Resets (clears) specific bits in a bitmask.
 *
 * @tparam SB The type of the source bits (e.g., an integer type).
 * @tparam M The type of the bit mask (e.g., an integer type).
 * @param sourceBits The original bitmask containing the bits to modify.
 * @param bitMaskToReset The bitmask specifying the bits to reset.
 * @return The resulting value after resetting the specified bits.
 */
template <std::integral SB, std::integral M>
// ReSharper disable once CppDFAConstantParameter
constexpr SB resetBits(SB sourceBits, M bitMaskToReset) {
  using MaxType = Max<SB, M>;

  if constexpr (std::is_signed_v<SB> || std::is_signed_v<M>) {
    using U = std::make_unsigned_t<MaxType>;

    return static_cast<SB>(resetBits(static_cast<U>(sourceBits), static_cast<U>(bitMaskToReset)));
  } else {
    return static_cast<SB>(resetBits(static_cast<MaxType>(sourceBits), static_cast<MaxType>(bitMaskToReset)));
  }
}

}  // namespace org::ttldtor::bits