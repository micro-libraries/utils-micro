/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_OPERATIONS_ABS_H
#define UTILS_MICRO_MATRIX_OPERATIONS_ABS_H
#include <utils-micro/assertion-handler.hpp>
#include <utils-micro/matrix/traits.hpp>

namespace utils_micro::matrix::operations {

enum class AbsoluteValueAlgorithm {
    Default
};

template <AbsoluteValueAlgorithm Algorithm = AbsoluteValueAlgorithm::Default>
class AbsoluteValue {
public:
    template <class InputMatrixT, class OutputMatrixT>
    static void abs(const typename Traits<InputMatrixT>::View & input, typename Traits<OutputMatrixT>::MutableView & output) {
        error_handling::assertion(equalShape(input, output), error_handling::AssertionType::ShapeMismatch, "Rotating cannot change shape.");

        auto h = input.getHeight();
        auto w = input.getWidth();

        for (CoordinateT i = 0; i < h; ++i) {
            const auto * input_row_data = input.getRowData(i);
            auto * output_row_data = output.getMutableRowData(i);
            for (CoordinateT j = 0; j < w; ++j) {
                output_row_data[j] = std::abs(input_row_data[j]);
            };
        }
    }

    template <class InputMatrixT, class OutputMatrixT>
    static OutputMatrixT abs(const typename Traits<InputMatrixT>::View & input) {
        OutputMatrixT output{input.getShape()};
        abs(input, output);
        return output;
    }
};

}  // namespace utils_micro::matrix

#endif  // UTILS_MICRO_MATRIX_OPERATIONS_ABS_H
