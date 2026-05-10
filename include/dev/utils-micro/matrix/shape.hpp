/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_SHAPE_H
#define UTILS_MICRO_MATRIX_SHAPE_H
#include <utils-micro/size.hpp>
#include <utils-micro/detail/named-value.hpp>

namespace utils_micro::matrix {

namespace detail {

    template <class Tag>
    class NamedShapeComponent : public utils_micro::detail::NamedValue<Tag, SizeT> {
    public:
        using utils_micro::detail::NamedValue<Tag, SizeT>::NamedValue;
    };

    struct WidthTag {};
    struct HeightTag {};

}

using Height = detail::NamedShapeComponent<detail::HeightTag>;
using Width = detail::NamedShapeComponent<detail::WidthTag>;

struct Shape {
    Height height;
    Width width;

    bool operator==(const Shape & other) const { return this->height == other.height && this->width == other.width; }
    bool operator!=(const Shape & other) const { return this->height != other.height || this->width != other.width; }
};

#if __cplusplus >= 202002L
using TemplateHeightT = Height;
#else
using TemplateHeightT = SizeT;
#endif

#if __cplusplus >= 202002L
using TemplateWidthT = Width;
#else
using TemplateWidthT = SizeT;
#endif

template <class LhsMatrixT, class RhsMatrixT>
bool equalShape(const LhsMatrixT & lhs, const RhsMatrixT & rhs) {
    return (lhs.getHeight() == rhs.getHeight()) && (lhs.getWidth() == rhs.getWidth());
}

}

#endif //UTILS_MICRO_MATRIX_SHAPE_H
