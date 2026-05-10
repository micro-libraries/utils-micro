/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_COMPLEX_NUMBER_ARITHMETIC_H
#define UTILS_MICRO_COMPLEX_NUMBER_ARITHMETIC_H
#include <type_traits>
#include <complex>

namespace utils_micro::arithmetic {

template <typename FloatingPointT>
class ComplexNumberArithmetic {
public:
    static_assert(std::is_floating_point_v<FloatingPointT>);

    using BaseT = std::complex<FloatingPointT>;

    static BaseT add(BaseT a, BaseT b) { return a+b; }

    static BaseT sub(BaseT a, BaseT b) { return a-b; }

    static BaseT mul(BaseT a, BaseT b) { return a * b; }

    static BaseT div(BaseT a, BaseT b) { return a / b; }

};

}  // namespace utils_micro::arithmetic

#endif  // UTILS_MICRO_COMPLEX_NUMBER_ARITHMETIC_H
