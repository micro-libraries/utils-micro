/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_FOURIER_H
#define UTILS_MICRO_FOURIER_H
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <complex>
#include <vector>
#include <utils-micro/span.hpp>
#include <utils-micro/matrix.hpp>
#include <utils-micro/inline.h>

namespace utils_micro::fourier {

template <class MatrixT>
void transposeSquare(typename matrix::Traits<MatrixT>::MutableView & matrix) noexcept {
    assert(matrix.getHeight() == matrix.getWidth());
    int size = matrix.getHeight();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < i; j++) {
            std::swap(matrix.getMutable(i, j), matrix.getMutable(j, i));
        }
    }
}

/*
  Following functions differ by output storage.
  - vector of vector: output must be resized to `h x w` before function is called
  - vector of array: output must be resized to `SIZE` before function is called, stored in free store (heap)
  - array of array: automatic storage (on stack)

  We're operating on vectors and matrices. A non-allocating variant takes an input view
  of a user-defined type `InputT` and a mutable output view of an operation-defined type `ScalarT`,
  which is the scalar type used for internal computations.
  An allocating variant creates and returns a container of `ScalarT` instead.
  Note that a transforming a non-square matrix can't be done in place under Matrix constraints,
  so there's a square variant and a non-square variant that allocates temporary storage.
  The view and container types are:
  - vectors: `span<const InputT>` -> `span<ScalarT>, ::std::vector<ScalarT, Allocator>`
  - matrices: `matrix::[Specify]MatrixView<InputT> -> matrix::Mutable[Specify]MatrixView<ScalarT>, matrix::[Specify]Matrix<ScalarT, Allocator>`

  TODO explore performance effects of a different implementation: there's a fixed "line" object
  that data gets copied to, then in place transformed and copied back
*/

namespace detail {

template <typename ScalarT, class FourierTransform>
class TwoDimensionalTransform {
public:
    template <typename InputT, SizeT SIZE>
    void run(const matrix::FixedShapeMatrixView<InputT, SIZE, SIZE> & input,
             matrix::MutableFixedShapeMatrixView<ScalarT, SIZE, SIZE> & output, bool inverse) noexcept {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) { output.set(i, j, static_cast<ScalarT>(input.get(i, j))); }
            static_cast<FourierTransform *>(this)->template runInPlace<SIZE>(output.getMutableRowData(i).data(), inverse);
        }
        transposeSquare(output);
        for (int i = 0; i < SIZE; i++) {
            static_cast<FourierTransform *>(this)->template runInPlace<SIZE>(output.getMutableRowData(i).data(), inverse);
        }
        transposeSquare(output);
    }

    // provided purely for naming consistency
    template <typename InputT, SizeT SIZE>
    void runSquare(const matrix::FixedShapeMatrixView<InputT, SIZE, SIZE> & input,
         matrix::MutableFixedShapeMatrixView<ScalarT, SIZE, SIZE> & output, bool inverse) noexcept {
        this->run<InputT, SIZE>(input, output, inverse);
    }

    template <typename InputT, SizeT SIZE, class Allocator>
    auto runSquare(const matrix::FixedShapeMatrixView<InputT, SIZE, SIZE> & input, bool inverse) {
        return this->template run<InputT, SIZE, Allocator>(input, inverse);
    }

    template <typename InputT, unsigned HEIGHT, unsigned WIDTH, class Allocator>
    void run(const matrix::FixedShapeMatrixView<InputT, HEIGHT, WIDTH> & input,
             matrix::MutableFixedShapeMatrixView<ScalarT, HEIGHT, WIDTH> & output, bool inverse) {
        // using `output` as temporary output for horizontal pass
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) { output.set(i, j, static_cast<ScalarT>(input.get(i, j))); }
            static_cast<FourierTransform *>(this)->template runInPlace<WIDTH>(output.getMutableRowData(i).data(), inverse);
        }
        matrix::FixedShapeMatrix<ScalarT, WIDTH, HEIGHT, Allocator> tmp;
        transpose(output, tmp);
        // using `tmp` as input-output for vertical pass
        for (int i = 0; i < WIDTH; i++) {
            static_cast<FourierTransform *>(this)->template runInPlace<HEIGHT>(tmp.getMutableRowData(i).data(), inverse);
        }
        transpose(tmp, output);
    }

    template <typename InputT, unsigned SIZE>
    void runSquare(const matrix::FixedWidthMatrixView<InputT, SIZE> & input, matrix::MutableFixedWidthMatrixView<ScalarT, SIZE> & output, bool inverse) noexcept {
        assert(input.getHeight() == SIZE);
        assert(output.getHeight() == SIZE);

        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) { output.set(i, j, static_cast<ScalarT>(input.get(i, j))); }
            static_cast<FourierTransform *>(this)->template runInPlace<SIZE>(output.getMutableRowData(i).data(), inverse);
        }
        transposeSquare(output);
        for (int i = 0; i < SIZE; i++) {
            static_cast<FourierTransform *>(this)->template runInPlace<SIZE>(output.getMutableRowData(i).data(), inverse);
        }
        transposeSquare(output);
    }

    template <typename InputT, unsigned SIZE, class Allocator>
    auto runSquare(const matrix::FixedWidthMatrixView<InputT, SIZE> & input, bool inverse) {
        matrix::FixedWidthMatrix<InputT, SIZE, Allocator> output;
        run(input, output, inverse);
        return output;
    }

    template <typename InputT, unsigned SIZE>
    void run(const matrix::FixedWidthMatrixView<InputT, SIZE> & input, matrix::MutableFixedWidthMatrixView<ScalarT, SIZE> & output, bool inverse) {
        alignas(32) ScalarT fftLine[SIZE];

        matrix::FixedWidthMatrix<ScalarT, SIZE> tmp(SIZE);
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) { fftLine[j] = static_cast<ScalarT>(input.get(i, j)); }
            static_cast<FourierTransform *>(this)->template runInPlace<SIZE>(fftLine, inverse);
            memcpy(tmp[i].data(), fftLine, sizeof(fftLine));
        }
        transpose(tmp);
        for (int i = 0; i < SIZE; i++) {
            memcpy(fftLine, tmp[i].data(), sizeof(fftLine));
            static_cast<FourierTransform *>(this)->template runInPlace<SIZE>(fftLine, inverse);
            memcpy(output[i].data(), fftLine, sizeof(fftLine));
        }
        transpose(output);
    }

    template <typename InputT>
    void run(const matrix::NestedMatrixView<InputT> & input, matrix::MutableNestedMatrixView<ScalarT> & output, bool inverse) {
        assert(input.getShape() == output.getShape());
        int h = input.getHeight();
        int w = input.getWidth();
        assert(h != 0 && w != 0);

        std::vector<ScalarT> fftLine(std::max(h, w));

        matrix::NestedMatrix<ScalarT> tmp(input.getShape());
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                fftLine[j] = static_cast<ScalarT>(input.get(matrix::Row{i}, matrix::Column{j}));
            }
            static_cast<FourierTransform *>(this)->runInPlace(utils_micro::span<ScalarT>{fftLine.data(), static_cast<std::size_t>(w)}, inverse);
            for (int j = 0; j < w; j++) {
                tmp.set(matrix::Row{i}, matrix::Column{j}, fftLine[j]);
            }
        }
        for (matrix::CoordinateT i = 0; i < w; i++) {
            for (matrix::CoordinateT j = 0; j < h; j++) {
                fftLine[j] = tmp.get(matrix::Row{j}, matrix::Column{i});
            }
            static_cast<FourierTransform *>(this)->runInPlace(utils_micro::span<ScalarT>{fftLine.data(), static_cast<std::size_t>(h)}, inverse);
            for (int j = 0; j < h; j++) {
                output.set(matrix::Row{j}, matrix::Column{i}, fftLine[j]);
            }
        }
    }

    template <class MatrixT>
    auto runSquare(const typename matrix::Traits<MatrixT>::View & input, bool inverse) -> typename MatrixT::template MatrixOf<ScalarT> {
        MatrixT output{input.getShape()};
        this->run(input, output, inverse);
        return output;
    }

    template <class MatrixT>
    auto run(const typename matrix::Traits<MatrixT>::View & input, bool inverse) -> typename MatrixT::template MatrixOf<ScalarT> {
        typename MatrixT::template MatrixOf<ScalarT> output{input.getShape()};
        this->run(input, output, inverse);
        return output;
    }
};

template <typename>
struct is_std_complex : std::false_type {};

template <typename T>
struct is_std_complex<std::complex<T>> : std::true_type {};

template <typename T>
constexpr bool is_std_complex_v = is_std_complex<T>::value;

}

template <class ScalarArithmetic>
class RingArithmetic : public ScalarArithmetic {
public:
    using ScalarT = typename ScalarArithmetic::BaseT;

    template <unsigned N, typename T = ScalarT>
    constexpr static std::enable_if_t<detail::is_std_complex_v<T>, T>
    calculatePrimitiveRoot() {
        // exp(-2*i*pi/N) is a primitive N-th root of unity in complex numbers
        using BaseT = typename T::value_type;
        auto minus_pi = std::acos(BaseT{-1});
        BaseT two{2};
        return std::polar(BaseT{1}, two * minus_pi / BaseT{N});
    }

    template <unsigned N, typename T = ScalarT>
    constexpr static std::enable_if_t<std::is_integral_v<T> && ((N & (N-1)) == 0), T>
    calculatePrimitiveRoot() {
        // Predicting roots of unity in a modular ring is a difficult problem.
        // We just check random numbers until one is found.
        // TODO
    }
};

// // slow algorithm for small general sizes
// // TODO: basis for future common slowdft/slowdct
// template <unsigned SIZE, class RingArithmetic, class Allocator = std::allocator<typename RingArithmetic::ScalarT>>
// class CosineTransform : detail::TwoDimensionalTransform<CosineTransform<SIZE, RingArithmetic, Allocator>> {
//     static_assert(SIZE >= 1);
//
//     using ScalarT = typename RingArithmetic::ScalarT;
//
//     static auto calculateWeights() {
//         matrix::FixedShapeMatrix<ScalarT, SIZE, SIZE, Allocator> output;
//         for (int i = 0; i < SIZE; i++) {
//             for (int j = 0; j < SIZE; j++) {
//                 auto value = static_cast<ScalarT>(std::cos(M_PI * (2 * j + 1) * i / (2 * SIZE)));
//                 output.set<i, j>(value);
//             }
//         }
//         return output;
//     }
//
//     static auto weights = calculateWeights();
//
//     // This implementation isn't inplace because it doesn't rely on butterfly diagram.
//     void innerLoop(const ScalarT * __restrict ptr_in, ScalarT & out, const ScalarT * __restrict ptr_weight) const noexcept {
//         ptr_in = __builtin_assume_aligned(ptr_in, Allocator::getAlignment());
//         ptr_weight = __builtin_assume_aligned(ptr_weight, Allocator::getAlignment());
//
//         out = 0;
//         for (int j = 0; j < SIZE; j++) {
//             out += RingArithmetic::add_mul(ptr_in + j, ptr_weight + j, out);
//         }
//     }
//
// public:
//     template <typename InputT>
//     void run(span<const InputT> input, span<ScalarT> output, bool inverse) {
//         assert(input.size() == SIZE);
//         assert(output.size() == SIZE);
//
//         alignas(Allocator::getAlignment()) std::array<ScalarT, SIZE> tmp;
//         for (int i = 0; i < SIZE; i++) { tmp[i] = static_cast<ScalarT>(input[i]); }
//
//         for (int i = 0; i < SIZE; i++) {
//             this->innerLoop(tmp.data(), output[i], weights[i].data());
//         }
//         for (int i = 0; i < SIZE; i++) {
//             ScalarT scale = ((i == 0) ? 1 : std::sqrt(2.0)) / std::sqrt(SIZE);
//             output[i] = RingArithmetic::mul(output[i], scale);
//         }
//         return output;
//     }
//
//     template <typename InputT>
//     auto run(span<const InputT> input, bool inverse) {
//         std::vector<ScalarT> output(SIZE);
//         run(input, output, inverse);
//         return output;
//     }
// };

// TODO future basis for fastdft/fastdct
template <unsigned MAX_BITS, class RingArithmetic>
class FastFourierTransform : public detail::TwoDimensionalTransform<typename RingArithmetic::ScalarT, FastFourierTransform<MAX_BITS, RingArithmetic>> {
    static_assert(MAX_BITS >= 1);

public:
    static constexpr unsigned MAX_SIZE = 1<<MAX_BITS;

    using ScalarT = typename RingArithmetic::ScalarT;

private:
    // jagged array with O(MAX_SIZE) space requirements
    using Weights = std::array<std::vector<ScalarT>, MAX_BITS + 1>;

    template <typename T = ScalarT>
    static std::enable_if_t<detail::is_std_complex_v<T>, Weights>
    calculateWeights() {
        using BaseT = typename T::value_type;
        constexpr auto minus_pi = std::acos(BaseT{-1});
        constexpr BaseT two{2};
        Weights output;
        for (int d = 1; d <= MAX_BITS; d++) {
            auto phase = two * minus_pi / std::pow(two, static_cast<BaseT>(d));
            auto root = std::polar(BaseT{1}, phase);
            output[d].resize(1 << d);
            output[d][0] = 1;
            for (int j = 1; j < (1 << d); j++) { output[d][j] = RingArithmetic::mul(output[d][j - 1], root); }
        }
        return output;
    }

    template <typename T = ScalarT>
    static std::enable_if_t<!detail::is_std_complex_v<T>, Weights>
    calculateWeights() {
        Weights output;
        auto root = RingArithmetic::template calculatePrimitiveRoot<MAX_SIZE>(); // initially for 2^MAX_BITS, update so it's always for 2^d
        for (int d = MAX_BITS; d > 0; d--, root = RingArithmetic::mul(root, root)) {
            output[d].resize(1 << d);
            output[d][0] = static_cast<T>(1);
            for (int j = 1; j < (1 << d); j++) { output[d][j] = RingArithmetic::mul(output[d][j - 1], root); }
        }
        return output;
    }

    static inline const auto W = calculateWeights();

    FORCE_INLINE_PRAGMA
    template <unsigned SIZE>
    FORCE_INLINE std::enable_if_t<detail::is_std_complex_v<ScalarT>, void>
    innerLoop(ScalarT * __restrict ptr_l, ScalarT * __restrict ptr_r, const ScalarT * __restrict ptr_weight) const noexcept {
        using BaseT = typename ScalarT::value_type;
        /*
            for (int k = 0; k < SIZE; k++, ptrL++, ptrR++, ptrWeight++)
            {
                std::complex<FftT> u = *ptrL, v = *ptrR * (*ptrWeight);
                *ptrL = u + v;
                *ptrR = u - v;
            }
        */

        if (SIZE == 1) {
            ScalarT a = *ptr_l, b = *ptr_r;
            *ptr_r = a - b;
            *ptr_l = a + b;
            return;
        }

        auto * __restrict ptr2_l = reinterpret_cast<BaseT *>(ptr_l);
        auto * __restrict ptr2_r = reinterpret_cast<BaseT *>(ptr_r);
        const auto * __restrict ptr2_weight = reinterpret_cast<const BaseT *>(ptr_weight);

    #ifdef USE_SSE
        for (int k = 0; k < SIZE / 2; k++) {
            auto tmpA = _mm_loadu_ps(ptr2_r);
            auto tmpB = _mm_set_ps(*(ptr2_weight + 2), *(ptr2_weight + 2), *(ptr2_weight), *(ptr2_weight));
            auto tmpC = _mm_set_ps(*(ptr2_weight + 3), *(ptr2_weight + 3), *(ptr2_weight + 1), *(ptr2_weight + 1));
            auto tmpD = _mm_mul_ps(tmpA, tmpB);
            auto tmpE = _mm_mul_ps(tmpA, tmpC);
            tmpE = _mm_shuffle_ps(tmpE, tmpE, _MM_SHUFFLE(2, 3, 0, 1));
            tmpD = _mm_addsub_ps(tmpD, tmpE);

            auto tmpF = _mm_loadu_ps(ptr2_l);
            _mm_storeu_ps(ptr2_r, _mm_sub_ps(tmpF, tmpD));
            _mm_storeu_ps(ptr2_l, _mm_add_ps(tmpF, tmpD));
            ptr2_l += 4, ptr2_r += 4, ptr2_weight += 4;
        }
    // TODO: NEON (how to test??)
    #else
        for (int k = 0; k < SIZE; k++) {
            auto tmpA = (*ptr2_r) * (*ptr2_weight);
            auto tmpB = (*(ptr2_r + 1)) * (*ptr2_weight);
            auto tmpC = (*ptr2_r) * (*(ptr2_weight + 1));
            auto tmpD = (*(ptr2_r + 1)) * (*(ptr2_weight + 1));
            tmpA -= tmpD;
            tmpC += tmpB;
            *ptr2_r = *ptr2_l - tmpA;
            *(ptr2_r + 1) = *(ptr2_l + 1) - tmpC;
            *ptr2_l += tmpA;
            *(ptr2_l + 1) += tmpC;
            ptr2_l += 2, ptr2_r += 2, ptr2_weight += 2;
        }
    #endif
    }

    template <unsigned SIZE, unsigned N>
    std::enable_if_t<SIZE == N, void>
    outerLoop(ScalarT * __restrict, const std::vector<ScalarT> * __restrict) const noexcept {}

    template <unsigned SIZE, unsigned N>
    std::enable_if_t<SIZE != N, void>
    outerLoop(ScalarT * __restrict a, const std::vector<ScalarT> * __restrict ptrW) const noexcept {
        const auto * __restrict const weights = ptrW->data();
        for (int j = 0; j < N; j += 2 * SIZE) {
            this->template innerLoop<SIZE>(a + j, a + j + SIZE, weights);
        }
        this->template outerLoop<2 * SIZE, N>(a, ptrW + 1);
    }

    template <unsigned SIZE>
    void outerLoop(ScalarT * __restrict a, const std::vector<ScalarT> * __restrict ptrW, SizeT n) const noexcept {
        if (SIZE == n) { return; }
        const auto * __restrict const weights = ptrW->data();
        for (int j = 0; j < n; j += 2 * SIZE) {
            this->template innerLoop<SIZE>(a + j, a + j + SIZE, weights);
        }
        this->template outerLoop<2 * SIZE>(a, ptrW + 1, n);
    }

public:
    void runInPlace(span<ScalarT> a, const bool inverse) const noexcept {
        int n = a.size();
        error_handling::assertion((n & (n - 1)) == 0, error_handling::AssertionType::OtherPreconditionFailure, "We require dimensions to be powers of 2.");
        error_handling::assertion(n <= (1<<MAX_BITS), error_handling::AssertionType::OtherPreconditionFailure, "n must be a power of 2 to at most MAX_BITS.");

        for (int i = 0, j = 0; i < n; ++i) {
            if (i < j) std::swap(a[i], a[j]);
            for (int k = n >> 1; (j ^= k) < k; k >>= 1) {}
        }
        this->outerLoop<1>(a.data(), this->W.data(), n);
        if (inverse) {
            std::reverse(a.begin() + 1, a.end());
            ScalarT nInv = RingArithmetic::div(static_cast<ScalarT>(1), static_cast<ScalarT>(n));
            for (int i = 0; i < n; ++i) a[i] = RingArithmetic::mul(a[i], nInv);
        }
    }

    template <unsigned N>
    void runInPlace(ScalarT * __restrict a, const bool inverse) const noexcept {
        // N is a power of 2 to at most MAX_BITS
        static_assert(N >= 1);
        static_assert((N & (N - 1)) == 0);
        static_assert(N <= (1<<MAX_BITS));

        for (int i = 0, j = 0; i < N; ++i) {
            if (i < j) std::swap(a[i], a[j]);
            for (int k = N >> 1; (j ^= k) < k; k >>= 1) {}
        }
        this->template outerLoop<1, N>(a, this->W.data() + 1);
        if (inverse) {
            std::reverse(a + 1, a + N);
            ScalarT nInv = RingArithmetic::div(static_cast<ScalarT>(1), static_cast<ScalarT>(N));
            for (int i = 0; i < N; ++i) a[i] = RingArithmetic::mul(a[i], nInv);
        }
    }

    template <typename InputT>
    void run(span<const InputT> input, span<ScalarT> output, bool inverse) const noexcept {
        int n = input.size();
        assert (n != 0);
        assert (output.size() == n);
        // we require size to be power of 2
        assert ((n & (n - 1)) == 0);

        for (int i = 0; i < n; i++) { output[i] = static_cast<ScalarT>(input[i]); }
        this->runInPlace(output.data(), n, inverse);
    }

    template <typename InputT>
    auto run(span<const InputT> input, bool inverse) const {
        auto output = std::vector<ScalarT>(input.size());
        this->run(input, output, inverse);
        return output;
    }

    using detail::TwoDimensionalTransform<ScalarT, FastFourierTransform>::run;
};

// template <class ScalarArithmetic>
// class RadonTransform {
//     /*
//     Fast Radon transform based on slice theorem.
//     */
// public:
//     using FrtT = float;
//
//     template <typename InputT, class Allocator = std::allocator<InputT>>
//     void frt(const matrix::NestedMatrixView<InputT> & input, matrix::NestedMatrix<FrtT> & output) {
//         // input is 2D array (size x size)
//         // size is an even power of 2
//         // output is 2D array (angleResolution x size)
//         int size = input.getHeight();
//         int angleResolution = static_cast<int>(output.getHeight());
//         assert(size > 1 && angleResolution > 0);
//         assert(input.getShape() == output.getShape());
//         assert((size & (size-1)) == 0);
//
//         // expecting zero of input to be in the middle (size/2, size/2)
//         matrix::NestedMatrix<InputT> inputUnshifted(size, std::vector<InputT>(size));
//         shift2D(input, inputUnshifted);
//
//         matrix::NestedMatrix<std::complex<FrtT>> fft2Unshifted(matrix::Shape(size, size));
//         fft2D.run<InputT>(inputUnshifted, fft2Unshifted, /*rev=*/false);
//         matrix::NestedMatrix<std::complex<FrtT>> fft2Shifted(matrix::Shape(size, size));
//         shift2D(fft2Unshifted, fft2Shifted);
//
//         // coordinates to interpolate in 2D Fourier space
//         std::vector<float> dstX(angleResolution * size), dstY(angleResolution * size);
//         for (int i = 0; i < angleResolution; i++)
//             for (int j = 0; j < size; j++) {
//                 dstX[i * size + j] = (size / 2) + (j - size / 2) * cosf(i * M_PI / angleResolution);
//                 dstY[i * size + j] = (size / 2) + (j - size / 2) * sinf(i * M_PI / angleResolution);
//             }
//
//         // interpolate the central slices of 2D Fourier space grid
//         matrix::FlatMatrix<std::complex<FrtT>> slices(matrix::Shape(angleResolution, size));
//         interpolate2D<std::complex<FrtT>>(dstX, dstY, fft2Shifted, slices);
//
//         matrix::NestedMatrix<std::complex<FrtT>> ifftSlices(matrix::Shape(angleResolution, size));
//         for (int i = 0; i < angleResolution; i++) {
//             for (int j = 0; j < size / 2; j++) ifftSlices.set(i, j + size / 2, slices.get(i, j));
//             for (int j = size / 2; j < size; j++) ifftSlices.set(i, j - size / 2, slices.get(i, j));
//             fft.runInPlace(ifftSlices[i].data(), size, /*rev=*/true);
//         }
//
//         matrix::NestedMatrix<std::complex<FrtT>> radon(matrix::Shape(angleResolution, size));
//         for (int i = 0; i < angleResolution; i++) {
//             for (int j = 0; j < size / 2; j++) radon.set(i, j + size / 2, ifftSlices.get(i, j));
//             for (int j = size / 2; j < size; j++) radon.set(i, j - size / 2, ifftSlices.get(i, j));
//         }
//
//         for (int i = 0; i < angleResolution; i++)
//             for (int j = 0; j < size; j++)
//                 output.set(i, j, radon.get(i, j).real());
//     }
// };

}  // namespace utils_micro::fourier

#endif  // UTILS_MICRO_FOURIER_H
