/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_SIMD_ARITHMETIC_H
#define UTILS_MICRO_SIMD_ARITHMETIC_H
#include <cmath>
#include <limits>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utils-micro/inline.h>

// Microsoft Visual Studio doesn’t set the __SSEn__ macros (but they do set __AVX__ and __AVX2__)
#ifdef _MSC_VER
#define __SSE__ 1
#endif

// x86 platform specific header
#if defined(__SSE__) || defined(__AVX__) || defined(__AVX2__)
#include <immintrin.h>
#endif // defined(__SSE__) || defined(__AVX__) || defined(__AVX2__)

// android arm specific header
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#include <arm_neon.h>
#endif // defined(__ARM_NEON__) || defined(__ARM_NEON)

namespace utils_micro::arithmetic {

/*
SimdArithmetic classes are purely static and define essential arithmetic functions for array transformations:
- default "zero" element
- addition, subtraction
- multiplication
- FMA
- min, max
- SIMD type load (from array of base type) / store (to array of base type) / set (repeated value of base type)
- horizontal sum
- conversions to/from different base types, particularly floating point <-> integral

Operations are typically binary. Horizontal means unary, applied on multiple/all elements in one array.

The goal of these classes is DRY for algorithms that work equivalently over multiple algebraic fields,
requiring specific abstract operations but not depending on their implementation or hardware capabilities.
All members are simple and inlined, removing overhead.

*/

// disable SIMD types warning: ignoring attributes on template argument ‘__m128’ [-Wignored-attributes]
//                             std::is_same_v<T, __m128>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"

template <typename BaseT_, typename SimdT_, int INT_SHIFT>
class SimdArithmetic {
public:
    /*
     * make template parameters publicly accessible
     */
    using BaseT = BaseT_;
    using SimdT = SimdT_;
    static constexpr auto getIntShift() { return INT_SHIFT; }

    /*
     * check the base type and the simd type size constraints
     */
    static_assert(sizeof(BaseT) <= sizeof(SimdT), "SimdT size must be greater than or equal to BaseT");

    /*
     * check that int shift is used with integer arithmetic only
     */
    static_assert(std::is_integral_v<BaseT> || INT_SHIFT == 0, "INT_SHIFT can be used with integer arithmetic only");

    /*
     * supported data types (allowed base type and SIMD type combinations)
     */
    static_assert((std::is_same_v<BaseT, float> && std::is_same_v<SimdT, float>)  // generic float
    || (std::is_same_v<BaseT, int16_t> && std::is_same_v<SimdT, int16_t>)  // generic INT16
    || (std::is_same_v<BaseT, int32_t> && std::is_same_v<SimdT, int32_t>)  // generic INT32
#ifdef __SSE__
    || (std::is_same_v<BaseT, float> && std::is_same_v<SimdT, __m128>)  // SSE float
    || (std::is_same_v<BaseT, int32_t> && std::is_same_v<SimdT, __m128i>)  // SSE INT32
#endif  // __SSE__
#ifdef __AVX__
    || (std::is_same_v<BaseT, float> && std::is_same_v<SimdT, __m256>)  // AVX float
#endif  // __AVX__
#ifdef __AVX2__
    || (std::is_same_v<BaseT, int32_t> && std::is_same_v<SimdT, __m256i>)  // AVX2 INT32
#endif  // __AVX2__
#ifdef __AVX512F__
    || (std::is_same_v<BaseT, float> && std::is_same_v<SimdT, __m512>)  // AVX-512 float
#endif  // __AVX512F__
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
    || (std::is_same_v<BaseT, float> && std::is_same_v<SimdT, float32x4_t>)  // NEON float
#endif  // defined(__ARM_NEON__) || defined(__ARM_NEON)
    ,
    "unsupported arithmetic type");

    static constexpr auto getSimdElementsCount() { return sizeof(SimdT) / sizeof(BaseT); }

    /*
     * use 32-bit accumulator for 16-bit SIMD type (performance optimization)
     */
    using LineResultT = std::conditional_t<std::is_same_v<SimdT, int16_t>, int32_t, SimdT>;

    /*
     * SIMD type alignment
     * ARM int16_t code uses 32-bit accumulator so calculate it from LineResultT.
     */
    static constexpr auto getAlignment() { return sizeof(LineResultT); }

    /*
     * the lowest base type value
     */
    static constexpr BaseT lowest() { return std::numeric_limits<BaseT>::lowest(); }

    /*
     * from/to float conversion functions
     */
    template <typename T = BaseT>
    static constexpr std::enable_if_t<std::is_floating_point_v<T>, T> fromFloat(const float a) {
        return a;
    }

    template <typename T = BaseT>
    static constexpr std::enable_if_t<std::is_floating_point_v<T>, float> toFloat(const T a) {
        return a;
    }

    template <typename T = BaseT>
    static constexpr std::enable_if_t<std::is_integral_v<T>, T> fromFloat(const float a) {
        const auto result = std::round(a * (1 << INT_SHIFT));
        assert(result >= static_cast<float>(std::numeric_limits<BaseT>::lowest()) &&
               result <= static_cast<float>(std::numeric_limits<BaseT>::max()));
        return result;
    }

    template <typename T = BaseT>
    static constexpr std::enable_if_t<std::is_integral_v<T>, float> toFloat(const T a) {
        return static_cast<float>(a) / (1 << INT_SHIFT);
    }

    /*
     * generic float arithmetic
     * generic calculation functions are enabled for all float types including SIMD to process remaining data after SIMD calculation
     */

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> add(const float a, const float b) {
        return a + b;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> sub(const float a, const float b) {
        return a - b;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> max(const float a, const float b) {
        return std::max(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> min(const float a, const float b) {
        return std::min(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> mul(const float a, const float b) {
        return a * b;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> add_mul(const float a, const float b, const float c)
    {
        return a + b * c;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> set(const float a) {
        return a;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> load(const float *p) {
        return *p;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, void> store(float *p, const float a) {
        *p = a;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> zero() {
        return 0.0F;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float>, float> horizontal_sum(const float a) {
        return a;
    }

    /*
     * generic INT16 arithmetic
     * uses 32-bit intermediate data types for better accuracy
     * generic calculation functions are enabled for all float types including SIMD to process remaining data after SIMD calculation
     */

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int32_t> add(const int32_t a, const int32_t b) {
        return a + b;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int32_t> sub(const int32_t a, const int32_t b) {
        return a - b;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int32_t> max(const int32_t a, const int32_t b)
    {
        return std::max(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int32_t> min(const int32_t a, const int32_t b)
    {
        return std::min(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int32_t> mul(const int32_t a, const int32_t b)
    {
        return (a * b) / (1 << INT_SHIFT);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int32_t> mul_no_shift(const int32_t a, const int32_t b)
    {
        return a * b;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int32_t> add_mul(const int32_t a, const int32_t b, const int32_t c)
    {
        return a + ((b * c) / (1 << INT_SHIFT));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int32_t> add_mul_no_shift(const int32_t a, const int32_t b, const int32_t c)
    {
        return a + b * c;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int16_t> set(
            const int16_t a)
    {
        return a;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int16_t> load(const int16_t *p)
    {
        return *p;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, void> store(int16_t *p, const int16_t a)
    {
        *p = a;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int16_t>, int16_t> zero()
    {
        return 0;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int16_t> horizontal_sum(const int32_t a)
    {
        return a;
    }

    /*
     * generic INT32 arithmetic
     * generic calculation functions are enabled for all float types including SIMD to process remaining data after SIMD calculation
     */

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> add(const int32_t a, const int32_t b)
    {
        return a + b;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> sub(const int32_t a, const int32_t b)
    {
        return a - b;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> max(const int32_t a, const int32_t b)
    {
        return std::max(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> min(const int32_t a, const int32_t b)
    {
        return std::min(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> mul(const int32_t a, const int32_t b)
    {
        // use 64-bit intermediate multiplication result for better accuracy
        return static_cast<int32_t>((static_cast<int64_t>(a) * b) / (1 << INT_SHIFT));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = BaseT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> add_mul(const int32_t a, const int32_t b, const int32_t c) {
        // use 64-bit intermediate multiplication result for better accuracy
        return a + static_cast<int32_t>((static_cast<int64_t>(b) * c) / (1 << INT_SHIFT));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> set(const int32_t a)
    {
        return a;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> load(const int32_t *p)
    {
        return *p;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, void> store(int32_t *p, const int32_t a)
    {
        *p = a;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> zero() {
        return 0;
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, int32_t>, int32_t> horizontal_sum(const int32_t a)
    {
        return a;
    }

#ifdef __SSE__
    /*
     * SSE float arithmetic
     */

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> add(const __m128 a, const __m128 b)
    {
        return _mm_add_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> sub(const __m128 a, const __m128 b)
    {
        return _mm_sub_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> max(const __m128 a, const __m128 b)
    {
        return _mm_max_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> min(const __m128 a, const __m128 b)
    {
        return _mm_min_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> mul(const __m128 a, const __m128 b)
    {
        return _mm_mul_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> add_mul(const __m128 a, const __m128 b, const __m128 c)
    {
        return _mm_add_ps(a, _mm_mul_ps(b, c));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> set(const float a)
    {
        return _mm_set1_ps(a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> load(const float *p)
    {
        return _mm_loadu_ps(p);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, void> store(float *p, const __m128 a)
    {
        _mm_storeu_ps(p, a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, __m128> zero() {
        return _mm_setzero_ps();
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128>, float> horizontal_sum(const __m128 a) {
        // https://stackoverflow.com/questions/6996764/fastest-way-to-do-horizontal-sse-vector-sum-or-other-reduction
        // a = [ D C | B A ]
        __m128 shuf = _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 3, 0, 1)); // [ C D | A B ]
        __m128 sums = _mm_add_ps(a, shuf);                           // sums = [ D+C C+D | B+A A+B ]
        shuf = _mm_movehl_ps(
            shuf, sums); //  [   C   D | D+C C+D ]  // let the compiler avoid a mov by reusing shuf
        sums = _mm_add_ss(sums, shuf);
        return _mm_cvtss_f32(sums);
    }

    /*
     * SSE INT32 arithmetic
     */

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> add(const __m128i a, const __m128i b)
    {
        return _mm_add_epi32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> sub(const __m128i a, const __m128i b)
    {
        return _mm_sub_epi32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> max(const __m128i a, const __m128i b)
    {
        return _mm_max_epi32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> min(const __m128i a, const __m128i b)
    {
        return _mm_min_epi32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> mul(const __m128i a, const __m128i b) {
        // use 64-bit intermediate multiplication result for better accuracy

        // first half (2x 32-bit * 32-bit -> 2x 64-bit result)
        const auto r1 = _mm_mul_epi32(a, b);

        // second half (2x 32-bit * 32-bit -> 2x 64-bit result)
        const auto a2 = _mm_shuffle_epi32(a, _MM_SHUFFLE(0, 3, 0, 1));
        const auto b2 = _mm_shuffle_epi32(b, _MM_SHUFFLE(0, 3, 0, 1));
        const auto r2 = _mm_mul_epi32(a2, b2);

        // neither the 64-bit arithmetic right shift nor the integer division is available in SSE :-(
        alignas(alignment) int64_t tmp[4];

        _mm_storeu_si128(reinterpret_cast<__m128i *>(tmp), r1);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(tmp + 2), r2);

        for (size_t i = 0; i < 4; ++i)
        {
            tmp[i] /= (1 << INT_SHIFT);
        }

        // values in tmp[i] are small enough to cast to 32-bit now
        return _mm_setr_epi32(static_cast<int32_t>(tmp[0]), static_cast<int32_t>(tmp[2]),
                              static_cast<int32_t>(tmp[1]), static_cast<int32_t>(tmp[3]));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> add_mul(const __m128i a, const __m128i b, const __m128i c)
    {
        return add(a, mul(b, c));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> set(const int32_t a)
    {
        return _mm_set1_epi32(a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> load(const int32_t *p) {
        return _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, void> store(int32_t *p, const __m128i a)
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(p), a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, int32_t> horizontal_sum(const T a)
    {
        // https://stackoverflow.com/questions/6996764/fastest-way-to-do-horizontal-sse-vector-sum-or-other-reduction
        __m128i hi64 = _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2));
        __m128i sum64 = _mm_add_epi32(hi64, a);
        __m128i hi32 = _mm_shufflelo_epi16(sum64, _MM_SHUFFLE(1, 0, 3, 2));
        __m128i sum32 = _mm_add_epi32(sum64, hi32);
        return _mm_cvtsi128_si32(sum32);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m128i>, __m128i> zero()
    {
        return _mm_setzero_si128();
    }
#endif //__SSE__

#ifdef __AVX__
    /*
     * AVX float arithmetic
     */

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> add(const __m256 a, const __m256 b)
    {
        return _mm256_add_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> sub(const __m256 a, const __m256 b)
    {
        return _mm256_sub_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> max(const __m256 a, const __m256 b)
    {
        return _mm256_max_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> min(const __m256 a, const __m256 b)
    {
        return _mm256_min_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> mul(const __m256 a, const __m256 b)
    {
        return _mm256_mul_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> add_mul(const __m256 a, const __m256 b, const __m256 c)
    {
        return _mm256_add_ps(a, _mm256_mul_ps(b, c));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> set(const float a)
    {
        return _mm256_set1_ps(a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> load(const float *p)
    {
        return _mm256_loadu_ps(p);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, void> store(float *p, const __m256 a)
    {
        _mm256_storeu_ps(p, a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, __m256> zero()
    {
        return _mm256_setzero_ps();
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256>, float> horizontal_sum(const __m256 a)
    {
        // https://stackoverflow.com/questions/13219146/how-to-sum-m256-horizontally
        // a = ( a7, a6, a5, a4, a3, a2, a1, a0 )
        // hiQuad = ( a7, a6, a5, a4 )
        const __m128 hiQuad = _mm256_extractf128_ps(a, 1);
        // loQuad = ( a3, a2, a1, a0 )
        const __m128 loQuad = _mm256_castps256_ps128(a);
        // sumQuad = ( a3 + a7, a2 + a6, a1 + a5, a0 + a4 )
        const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);
        // loDual = ( -, -, a1 + a5, a0 + a4 )
        const __m128 loDual = sumQuad;
        // hiDual = ( -, -, a3 + a7, a2 + a6 )
        const __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);
        // sumDual = ( -, -, a1 + a3 + a5 + a7, a0 + a2 + a4 + a6 )
        const __m128 sumDual = _mm_add_ps(loDual, hiDual);
        // lo = ( -, -, -, a0 + a2 + a4 + a6 )
        const __m128 lo = sumDual;
        // hi = ( -, -, -, a1 + a3 + a5 + a7 )
        const __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);
        // sum = ( -, -, -, a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 )
        const __m128 sum = _mm_add_ss(lo, hi);
        return _mm_cvtss_f32(sum);
    }
#endif //__AVX__

#ifdef __AVX2__
    /*
     * AVX2 INT32 arithmetic
     */

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> add(const __m256i a, const __m256i b)
    {
        return _mm256_add_epi32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> sub(const __m256i a, const __m256i b)
    {
        return _mm256_sub_epi32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> max(const __m256i a, const __m256i b)
    {
        return _mm256_max_epi32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> min(const __m256i a, const __m256i b)
    {
        return _mm256_min_epi32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> mul(const __m256i a, const __m256i b)
    {
        // use 64-bit intermediate multiplication result for better accuracy

        // first half (4x 32-bit * 32-bit -> 4x 64-bit result)
        const auto r1 = _mm256_mul_epi32(a, b);

        // second half (4x 32-bit * 32-bit -> 4x 64-bit result)
        const auto a2 = _mm256_shuffle_epi32(a, _MM_SHUFFLE(0, 3, 0, 1));
        const auto b2 = _mm256_shuffle_epi32(b, _MM_SHUFFLE(0, 3, 0, 1));
        const auto r2 = _mm256_mul_epi32(a2, b2);

        // neither the 64-bit arithmetic right shift nor the integer division is available in AVX2 :-(
        alignas(alignment) int64_t tmp[8];

        _mm256_storeu_si256(reinterpret_cast<__m256i *>(tmp), r1);
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(tmp + 4), r2);

        for (size_t i = 0; i < 8; ++i)
        {
            tmp[i] /= (1 << INT_SHIFT);
        }

        // values in tmp[i] are small enough to cast to 32-bit now
        return _mm256_setr_epi32(static_cast<int32_t>(tmp[0]), static_cast<int32_t>(tmp[4]),
                                 static_cast<int32_t>(tmp[1]), static_cast<int32_t>(tmp[5]),
                                 static_cast<int32_t>(tmp[2]), static_cast<int32_t>(tmp[6]),
                                 static_cast<int32_t>(tmp[3]), static_cast<int32_t>(tmp[7]));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> add_mul(const __m256i a, const __m256i b, const __m256i c)
    {
        return add(a, mul(b, c));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> set(const int32_t a)
    {
        return _mm256_set1_epi32(a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> load(const int32_t *p)
    {
        return _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, void> store(int32_t *p, const __m256i a)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(p), a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, int32_t> horizontal_sum(const T a)
    {
        // a = ( a7, a6, a5, a4, a3, a2, a1, a0 )
        // hiQuad = ( a7, a6, a5, a4 )
        const __m128i hiQuad = _mm256_extractf128_si256(a, 1);
        // loQuad = ( a3, a2, a1, a0 )
        const __m128i loQuad = _mm256_castsi256_si128(a);
        // sumQuad = ( a3 + a7, a2 + a6, a1 + a5, a0 + a4 )
        const __m128i sumQuad = _mm_add_epi32(loQuad, hiQuad);
        // loDual = ( -, -, a1 + a5, a0 + a4 )
        const __m128i loDual = sumQuad;
        // hiDual = ( -, -, a3 + a7, a2 + a6 )
        const __m128i hiDual = _mm_shuffle_epi32(sumQuad, _MM_SHUFFLE(0, 0, 3, 2));
        // sumDual = ( -, -, a1 + a3 + a5 + a7, a0 + a2 + a4 + a6 )
        const __m128i sumDual = _mm_add_epi32(loDual, hiDual);
        // lo = ( -, -, -, a0 + a2 + a4 + a6 )
        const __m128i lo = sumDual;
        // hi = ( -, -, -, a1 + a3 + a5 + a7 )
        const __m128i hi = _mm_shuffle_epi32(sumDual, _MM_SHUFFLE(0, 0, 0, 1));
        // sum = ( -, -, -, a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 )
        const __m128i sum = _mm_add_epi32(lo, hi);
        return _mm_extract_epi32(sum, 0);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m256i>, __m256i> zero()
    {
        return _mm256_setzero_si256();
    }
#endif //__AVX2__

#ifdef __AVX512F__
    /*
     * AVX-512 float arithmetic
     */

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> add(const __m512 a, const __m512 b)
    {
        return _mm512_add_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> sub(const __m512 a, const __m512 b)
    {
        return _mm512_sub_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> max(const __m512 a, const __m512 b)
    {
        return _mm512_max_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> min(const __m512 a, const __m512 b)
    {
        return _mm512_min_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> mul(const __m512 a, const __m512 b)
    {
        return _mm512_mul_ps(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> add_mul(const __m512 a, const __m512 b, const __m512 c)
    {
        // AVX-512 FMA instruction _mm512_fmadd_ps is intentionally not used here because
        // the FMA instruction is more accurate and gives different results than MUL+ADD
        // instructions and we want exactly the same results as SSE/AVX versions.
        // However, it is not sufficient to replace the FMA by MUL+ADD because
        // the gcc compiler automatically fuses MUL+ADD to FMA by default.
        // Compilation option "-ffp-contract=off" must be used to disable it.
        // For more information see
        // https://stackoverflow.com/questions/43352510/difference-in-gcc-ffp-contract-options
        return _mm512_add_ps(a, _mm512_mul_ps(b, c));
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> set(const float a)
    {
        return _mm512_set1_ps(a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> load(const float *p)
    {
        return _mm512_loadu_ps(p);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, void> store(float *p, const __m512 a)
    {
        _mm512_storeu_ps(p, a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, __m512> zero()
    {
        return _mm512_setzero_ps();
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, __m512>, float> horizontal_sum(const __m512 a)
    {
        return _mm512_reduce_add_ps(a);
    }
#endif //__AVX512F__

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
    /*
     * NEON float arithmetic
     */
    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> add(const float32x4_t a, const float32x4_t b)
    {
        return vaddq_f32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> sub(const float32x4_t a, const float32x4_t b)
    {
        return vsubq_f32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> max(const float32x4_t a, const float32x4_t b)
    {
        return vmaxq_f32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> min(const float32x4_t a, const float32x4_t b)
    {
        return vminq_f32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> mul(const float32x4_t a, const float32x4_t b)
    {
        return vmulq_f32(a, b);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> add_mul(const float32x4_t a, const float32x4_t b, const float32x4_t c)
    {
        return vmlaq_f32(a, b, c);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> set(const float a)
    {
        return vdupq_n_f32(a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> load(const float *p)
    {
        return vld1q_f32(p);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, void> store(float * p, const float32x4_t a) {
        vst1q_f32(p, a);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float32x4_t> zero()
    {
        return vdupq_n_f32(0);
    }

    FORCE_INLINE_PRAGMA
    template <typename T = SimdT>
    static FORCE_INLINE std::enable_if_t<std::is_same_v<T, float32x4_t>, float> horizontal_sum(const float32x4_t a) {
        auto tmp = vadd_f32(vget_high_f32(a), vget_low_f32(a));
        return vget_lane_f32(vpadd_f32(tmp, tmp), 0);
    }
#endif // defined(__ARM_NEON__) || defined(__ARM_NEON)
};

#pragma GCC diagnostic pop // "-Wignored-attributes"

}  // namespace utils_micro::arithmetic

#endif  // UTILS_MICRO_SIMD_ARITHMETIC_H
