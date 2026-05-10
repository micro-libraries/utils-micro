/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_SPAN_H
#define UTILS_MICRO_SPAN_H

#if __cplusplus >= 202002L
#include <span>
#else
#include "detail/nonstd/span.tpp"
#endif

namespace utils_micro {

#if __cplusplus >= 202002L
template<class T, std::size_t Extent = std::dynamic_extent>
using span = std::span<T, Extent>;
#else
template<class T, std::size_t Extent = nonstd::dynamic_extent>
using span = nonstd::span<T, Extent>;
#endif

template <class T, size_t SIZE>
inline span<const T> arrayToSpan2D(const std::array<std::array<T, SIZE>, SIZE> & a)
{
    if (SIZE == 0) { return span<const T>(); }
    return span<const T>(a[0].data(), SIZE * SIZE);
}

template <class T, size_t SIZE>
inline span<T> arrayToSpan2D(std::array<std::array<T, SIZE>, SIZE> & a)
{
    if (SIZE == 0) { return span<T>(); }
    return span<T>(a[0].data(), SIZE * SIZE);
}

}

#endif  // UTILS_MICRO_SPAN_H
