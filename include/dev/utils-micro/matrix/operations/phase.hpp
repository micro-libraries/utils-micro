/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_OPERATIONS_PHASE_H
#define UTILS_MICRO_MATRIX_OPERATIONS_PHASE_H
#include <complex>
#include <utils-micro/matrix/coordinates.hpp>
#include <utils-micro/matrix/shape.hpp>
#include <utils-micro/matrix/traits.hpp>

namespace utils_micro::matrix::operations {

enum class PhaseAlgorithm {
    Default
};

template <PhaseAlgorithm Algorithm = PhaseAlgorithm::Default>
class Phase {
public:
    template <class InputMatrixT, class OutputMatrixT>
    static void phase(const typename Traits<InputMatrixT>::View & input, typename Traits<OutputMatrixT>::MutableView & output) {
        error_handling::assertion(equalShape(input, output), error_handling::AssertionType::ShapeMismatch, "Rotating cannot change shape.");

        auto h = input.getHeight();
        auto w = input.getWidth();

        for (CoordinateT i = 0; i < h; ++i) {
            const auto * input_row_data = input.getRowData(i);
            auto * output_row_data = output.getMutableRowData(i);
            for (CoordinateT j = 0; j < w; ++j) {
                output_row_data[j] = std::arg(input_row_data[j]);
            };
        }
    }

    template <class InputMatrixT, class OutputMatrixT>
    static OutputMatrixT phase(const typename Traits<InputMatrixT>::View & input) {
        OutputMatrixT output{input.getShape()};
        phase(input, output);
        return output;
    }
};

}  // namespace utils_micro::matrix

#endif  // UTILS_MICRO_MATRIX_OPERATIONS_PHASE_H
