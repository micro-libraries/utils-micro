/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_OPERATIONS_H
#define UTILS_MICRO_MATRIX_OPERATIONS_H
/*
Matrix data storage and view. Stored in contiguous memory with optional custom alignment.
*/
#include <vector>
#include <utils-micro/span.hpp>
#include <utils-micro/matrix/traits.hpp>

namespace utils_micro::matrix {

// NOTE: immutable "view"
// NOTE: row-major matrix representation

template <typename T, class InputMatrixT, class OutputMatrixT>
inline void levels(
        const typename Traits<InputMatrixT>::View &input,
        typename Traits<OutputMatrixT>::MutableView &output)
{
    auto [h, w] = input.getShape();
    error_handling::assert(h != 0);
    error_handling::assert(equalShape(input, output));

    std::vector< std::pair<T, std::pair<int, int> > > values(h * w);
    for (SizeT i = 0; i < h; i++)
    {
        for (SizeT j = 0; j < w; j++)
            values[i * w + j] = {input.get(i, j), {i, j}};
    }
    std::sort(values.begin(), values.end());
    for (int i = 0; i < h * w; i++)
    {
        int r = values[i].second.first;
        int c = values[i].second.second;
        output.set(r, c, i * 256 / (h * w));
    }
}

}

#endif  // UTILS_MICRO_MATRIX_OPERATIONS_H
