/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_TRAITS_HPP
#define UTILS_MICRO_MATRIX_TRAITS_HPP
#include <type_traits>
#include <utils-micro/matrix/fixed-shape-matrix.hpp>
#include <utils-micro/matrix/fixed-width-matrix.hpp>
#include <utils-micro/matrix/flat-matrix.hpp>
#include <utils-micro/matrix/nested-matrix.hpp>

namespace utils_micro::matrix {

template <class>
struct Traits : std::false_type {};

template <typename T, TemplateHeightT HEIGHT, TemplateWidthT WIDTH>
struct Traits<FixedShapeMatrixView<T, HEIGHT, WIDTH>> {
    using View = FixedShapeMatrixView<T, HEIGHT, WIDTH>;
    using MutableView = void;
};

template <typename T, TemplateHeightT HEIGHT, TemplateWidthT WIDTH>
struct Traits<MutableFixedShapeMatrixView<T, HEIGHT, WIDTH>> {
    using View = FixedShapeMatrixView<T, HEIGHT, WIDTH>;
    using MutableView = MutableFixedShapeMatrixView<T, HEIGHT, WIDTH>;
};

template <typename T, TemplateHeightT HEIGHT, SizeT WIDTH, class Allocator>
struct Traits<FixedShapeMatrix<T, HEIGHT, WIDTH, Allocator>> {
    using View = FixedShapeMatrixView<T, HEIGHT, WIDTH>;
    using MutableView = MutableFixedShapeMatrixView<T, HEIGHT, WIDTH>;
};

template <typename T, TemplateWidthT WIDTH>
struct Traits<FixedWidthMatrixView<T, WIDTH>> {
    using View = FixedWidthMatrixView<T, WIDTH>;
    using MutableView = void;
};

template <typename T, TemplateWidthT WIDTH>
struct Traits<MutableFixedWidthMatrixView<T, WIDTH>> {
    using View = FixedWidthMatrixView<T, WIDTH>;
    using MutableView = MutableFixedWidthMatrixView<T, WIDTH>;
};

template <typename T, TemplateWidthT WIDTH, class Allocator>
struct Traits<FixedWidthMatrix<T, WIDTH, Allocator>> {
    using View = FixedWidthMatrixView<T, WIDTH>;
    using MutableView = MutableFixedWidthMatrixView<T, WIDTH>;
};

template <typename T>
struct Traits<FlatMatrixView<T>> {
    using View = FlatMatrixView<T>;
    using MutableView = void;
};

template <typename T>
struct Traits<MutableFlatMatrixView<T>> {
    using View = FlatMatrixView<T>;
    using MutableView = MutableFlatMatrixView<T>;
};

template <typename T, class Allocator>
struct Traits<FlatMatrix<T, Allocator>> {
    using View = FlatMatrixView<T>;
    using MutableView = MutableFlatMatrixView<T>;
};

template <typename T, template <typename...> class Container>
struct Traits<NestedMatrixView<T, Container>> {
    using View = NestedMatrixView<T, Container>;
    using MutableView = void;
};

template <typename T, template <typename...> class Container>
struct Traits<MutableNestedMatrixView<T, Container>> {
    using View = NestedMatrixView<T, Container>;
    using MutableView = MutableNestedMatrixView<T, Container>;
};

template <typename T, template <typename...> class Container, class Allocator>
struct Traits<NestedMatrix<T, Container, Allocator>> {
    using View = NestedMatrixView<T, Container>;
    using MutableView = MutableNestedMatrixView<T, Container>;
};

}

#endif  // UTILS_MICRO_MATRIX_TRAITS_HPP
