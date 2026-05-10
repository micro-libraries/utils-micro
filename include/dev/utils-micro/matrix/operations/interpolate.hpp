/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_OPERATIONS_INTERPOLATE_H
#define UTILS_MICRO_MATRIX_OPERATIONS_INTERPOLATE_H
#include <utils-micro/span.hpp>
#include <utils-micro/matrix/traits.hpp>

namespace utils_micro::matrix {

template <typename T, class MatrixT>
inline void interpolate(const span<const float> dstX, const span<const float> dstY, const typename Traits<MatrixT>::View &input, span<T> &output)
{
    // linear
    const int h = input.getHeight(), w = input.getWidth();
    const int n = output.size();
    error_handling::assert(dstX.size() == n);
    error_handling::assert(dstY.size() == n);

    for (int i = 0; i < n; i++) {
        int xlo = ROUND2INT(floorf(dstX[i]));
        int ylo = ROUND2INT(floorf(dstY[i]));
        if (ylo < 0 || xlo < 0 || ylo >= h || xlo >= w) {
            output[i] = 0;
            continue;
        }
        int xup = i_min(w - 1, xlo + 1);
        int yup = i_min(h - 1, ylo + 1);
        float dx = dstX[i] - xlo;
        float dy = dstY[i] - ylo;
        auto v00 = input[ylo][xlo];
        auto v01 = input[ylo][xup];
        auto v10 = input[yup][xlo];
        auto v11 = input[yup][xup];
        output[i] = v11 * dx * dy + v10 * (1 - dx) * dy + v01 * dx * (1 - dy) + v00 * (1 - dx) * (1 - dy);
    }
}

}

#endif  // UTILS_MICRO_MATRIX_OPERATIONS_INTERPOLATE_H
