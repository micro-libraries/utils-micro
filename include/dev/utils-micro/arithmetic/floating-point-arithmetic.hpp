/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_FLOATING_POINT_ARITHMETIC_H
#define UTILS_MICRO_FLOATING_POINT_ARITHMETIC_H

namespace utils_micro::arithmetic {

template <typename BaseT_>
class FloatingPointArithmetic {
public:
    using BaseT = BaseT_;

    static_assert(std::is_floating_point_v<BaseT>);

    static BaseT add(BaseT a, BaseT b) { return a+b; }

    static BaseT sub(BaseT a, BaseT b) { return a-b; }
};

}  // namespace utils_micro::arithmetic

#endif  // UTILS_MICRO_FLOATING_POINT_ARITHMETIC_H
