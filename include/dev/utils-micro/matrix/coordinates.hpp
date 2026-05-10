/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_COORDINATES_H
#define UTILS_MICRO_MATRIX_COORDINATES_H
#include <utils-micro/detail/named-value.hpp>
#include <utils-micro/matrix/shape.hpp>

namespace utils_micro::matrix {

// Used to prevent passing r, c swapped by accident.
using CoordinateT = int;

namespace detail {

    template <class Tag>
    class NamedCoordinate : public utils_micro::detail::NamedValue<Tag, CoordinateT> {
    public:
        using utils_micro::detail::NamedValue<Tag, CoordinateT>::NamedValue;
    };

    struct RowTag {};
    struct ColumnTag {};

}

using Row = detail::NamedCoordinate<detail::RowTag>;
using Column = detail::NamedCoordinate<detail::ColumnTag>;

#if __cplusplus >= 202002L
using TemplateRowT = Row;
#else
using TemplateRowT = CoordinateT;
#endif

#if __cplusplus >= 202002L
using TemplateColumnT = Column;
#else
using TemplateColumnT = CoordinateT;
#endif

inline bool operator<(const Height & height, const Row & row) {
    return static_cast<SizeT>(height) < static_cast<CoordinateT>(row);
}

inline bool operator<=(const Height & height, const Row & row) {
    return static_cast<SizeT>(height) <= static_cast<CoordinateT>(row);
}

inline bool operator<(const Row & row, const Height & height) {
    return static_cast<CoordinateT>(row) < static_cast<SizeT>(height);
}

inline bool operator<=(const Row & row, const Height & height) {
    return static_cast<CoordinateT>(row) <= static_cast<SizeT>(height);
}

inline bool operator<(const Width & width, const Column & column) {
    return static_cast<SizeT>(width) < static_cast<CoordinateT>(column);
}

inline bool operator<=(const Width & width, const Column & column) {
    return static_cast<SizeT>(width) <= static_cast<CoordinateT>(column);
}

inline bool operator<(const Column & column, const Width & width) {
    return static_cast<CoordinateT>(column) < static_cast<SizeT>(width);
}

inline bool operator<=(const Column & column, const Width & width) {
    return static_cast<CoordinateT>(column) <= static_cast<SizeT>(width);
}

struct Coordinates {
    Row r;
    Column c;

    bool operator==(const Coordinates & other) const { return this->r == other.r && this->c == other.c; }
    bool operator!=(const Coordinates & other) const { return this->r != other.r || this->c != other.c; }
};

}

#endif  // UTILS_MICRO_MATRIX_COORDINATES_H
