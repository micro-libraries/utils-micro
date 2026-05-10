/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MODULAR_RING_ARITHMETIC_H
#define UTILS_MICRO_MODULAR_RING_ARITHMETIC_H
#include <cstdint>
#include <limits>
#include <utils-micro/assertion-handler.hpp>

namespace utils_micro::arithmetic {

// Arithmetic corresponding to the typical implementation of integers modulo a compile-time constant,
// stored in a base type such as a 32-bit or 64-bit unsigned integer.
// The modulus must be small enough that a sum of two values fits within the base type.
// During multiplication, operands are extended to a type with double width, which may impact performance.
template <typename BaseT_, BaseT_ MOD_>
class ModularRingArithmetic {
public:
    using BaseT = BaseT_;

    constexpr static auto MOD = MOD_;
    static_assert(MOD > 0);
    static_assert(MOD < std::numeric_limits<BaseT>::max() / 2);

    static BaseT add(const BaseT a, const BaseT b) {
        auto result = a + b;
        if (result >= MOD) { result -= MOD; }
        return result;
    }

    static BaseT sub(const BaseT a, const BaseT b) {
        return (a >= b) ? (a - b) : (a + MOD - b);
    }

    template <typename T = BaseT>
    static std::enable_if_t<std::is_same_v<BaseT, std::uint64_t>, BaseT> mul(const T a, const T b) {
        // Yes, this is slow.
        const auto result = static_cast<__uint128_t>(a) * b % MOD;
        return static_cast<T>(result);
    }

    template <typename T = BaseT>
    static std::enable_if_t<std::is_same_v<BaseT, std::uint32_t>, BaseT> mul(const T a, const T b) {
        // Yes, this is slow.
        const auto result = static_cast<std::uint64_t>(a) * b % MOD;
        return static_cast<T>(result);
    }

    static bool isZero(const BaseT a) {
        return a == 0;
    }

    static bool isOne(const BaseT a) {
        return a == 1;
    }

    static BaseT pow(const BaseT a, const std::uint64_t e) {
        if (e == 0) { return 1; }
        auto result = pow(a, e / 2);
        result = mul(result, result);
        if (e % 2 != 0) { result = mul(result, a); }
        return result;
    }
};

// If the modulus is prime, every non-zero element has an inverse given by Fermat's little theorem a^(MOD-1) = 1.
// There is a generator g such that the set {g^0...MOD-2} is {1...MOD-1}.
template <typename BaseT_, BaseT_ MOD_>
class ModularFieldArithmetic : public ModularRingArithmetic<BaseT_, MOD_> {
    // Only possible if the modulus is prime. Validating that is too complicated to perform here.
public:
    using BaseT = BaseT_;

    constexpr static auto MOD = MOD_;

    static BaseT inv(const BaseT a) {
        // Calculates the multiplicative inverse of `a`.
        // Assumes that `a` is not the zero element; isZero(a) is provided to check it if necessary.
        return ModularRingArithmetic<BaseT_, MOD>::pow(a, MOD - 2);
    }

    static BaseT calculatePrimitiveRoot(const std::uint64_t order) {
        // Primitive root of order n satisfies r^order == 1, r^1...order-1 != 1.
        BaseT r = 1;
        while (r != MOD) {
            if (!ModularRingArithmetic<BaseT, MOD>::isOne(ModularRingArithmetic<BaseT, MOD>::pow(r, order))) {
                ++r;
                continue;
            }
            bool ok = true;
            BaseT r_current_power = 1;
            for (int i = 1; i < order; ++i) {
                r_current_power = ModularRingArithmetic<BaseT, MOD>::mul(r_current_power, r);
                if (ModularRingArithmetic<BaseT, MOD>::isOne(r_current_power)) {
                    ok = false;
                    break;
                }
            }
            if (ok) { break; }
            ++r;
        }

        error_handling::assert(
            r != MOD,
            error_handling::AssertionType::SanityCheckFailed,
            "Primitive root of given order not found."
        );

        return r;
    }
};

}

#endif  // UTILS_MICRO_MODULAR_RING_ARITHMETIC_H
