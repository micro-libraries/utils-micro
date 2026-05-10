/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_OPERATIONS_ROTATE_H
#define UTILS_MICRO_MATRIX_OPERATIONS_ROTATE_H
#include <algorithm>
#include <utils-micro/matrix/traits.hpp>
#include <utils-micro/assertion-handler.hpp>

namespace utils_micro::matrix {

enum class RotationAlgorithm {
    Default
};

template <RotationAlgorithm = RotationAlgorithm::Default>
class Rotation {
public:
    template <class InputMatrixT, class OutputMatrixT>
    static void rotateHalf(const typename Traits<InputMatrixT>::View & input, typename Traits<OutputMatrixT>::MutableView & output) {
        error_handling::assert(equalShape(input, output), error_handling::AssertionType::ShapeMismatch, "Rotating cannot change shape.");

        auto h = input.getHeight();
        auto w = input.getWidth();
        // we require both dimensions to be even
        error_handling::assert(
            h % 2 == 0 && w % 2 == 0,
            error_handling::AssertionType::OtherPreconditionFailure,
            "Rotating a matrix by half is only supported if its dimensions are even");

        for (CoordinateT i = 0; i < h / 2; ++i) {
            const auto * input_row_data = input.getRowData(i);
            auto * output_row_data = output.getMutableRowData(i + h / 2);
            std::copy_n(input_row_data, w / 2, output_row_data + w / 2);
            std::copy_n(input_row_data + w / 2, w / 2, output_row_data);
        }
        for (CoordinateT i = h / 2; i < h; ++i) {
            const auto * input_row_data = input.getRowData(i + h / 2);
            auto * output_row_data = output.getMutableRowData(i);
            std::copy_n(input_row_data, w / 2, output_row_data + w / 2);
            std::copy_n(input_row_data + w / 2, w / 2, output_row_data);
        }
    }

    template <class InputMatrixT, class OutputMatrixT>
    static OutputMatrixT rotateHalf(const typename Traits<InputMatrixT>::View & input) {
        OutputMatrixT output{input.getShape()};
        rotateHalf(input, output);
        return output;
    }
};

}  // namespace utils_micro::matrix

#endif  // UTILS_MICRO_MATRIX_OPERATIONS_ROTATE_H
