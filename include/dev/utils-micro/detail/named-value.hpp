/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_DETAIL_NAMED_VALUE_H
#define UTILS_MICRO_DETAIL_NAMED_VALUE_H
#include <type_traits>

namespace utils_micro::detail {

template <class /*Tag*/, typename ValueT>
class NamedValue {
public:
    ValueT value;

    explicit NamedValue(ValueT value_) : value(value_) {}

    // NOLINTNEXTLINE(CppNonExplicitConversionOperator)
    operator ValueT() const { return this->value; }

    bool operator==(const NamedValue & other) const { return this->value == other.value; }
    bool operator!=(const NamedValue & other) const { return this->value != other.value; }
    bool operator<(const NamedValue & other) const { return this->value < other.value; }
    bool operator>(const NamedValue & other) const { return this->value > other.value; }
    bool operator<=(const NamedValue & other) const { return this->value <= other.value; }
    bool operator>=(const NamedValue & other) const { return this->value >= other.value; }

    // bool operator==(const ValueT & other) const { return this->value == other; }
    // bool operator!=(const ValueT & other) const { return this->value != other; }
    // bool operator<(const ValueT & other) const { return this->value < other; }
    // bool operator>(const ValueT & other) const { return this->value > other; }
    // bool operator<=(const ValueT & other) const { return this->value <= other; }
    // bool operator>=(const ValueT & other) const { return this->value >= other; }
    //
    // friend bool operator==(const ValueT & lhs, const NamedValue & rhs) { return lhs == rhs.value; }
    // friend bool operator!=(const ValueT & lhs, const NamedValue & rhs) { return lhs != rhs.value; }
    // friend bool operator<(const ValueT & lhs, const NamedValue & rhs) { return lhs < rhs.value; }
    // friend bool operator>(const ValueT & lhs, const NamedValue & rhs) { return lhs > rhs.value; }
    // friend bool operator<=(const ValueT & lhs, const NamedValue & rhs) { return lhs <= rhs.value; }
    // friend bool operator>=(const ValueT & lhs, const NamedValue & rhs) { return lhs >= rhs.value; }
};

template <class LTag, class RTag, typename ValueT>
std::enable_if_t<!std::is_same_v<LTag, RTag>, bool>
operator==(const NamedValue<LTag, ValueT> &, const NamedValue<RTag, ValueT> &) = delete;

template <class LTag, class RTag, typename ValueT>
std::enable_if_t<!std::is_same_v<LTag, RTag>, bool>
operator!=(const NamedValue<LTag, ValueT> &, const NamedValue<RTag, ValueT> &) = delete;

template <class LTag, class RTag, typename ValueT>
std::enable_if_t<!std::is_same_v<LTag, RTag>, bool>
operator<(const NamedValue<LTag, ValueT> &, const NamedValue<RTag, ValueT> &) = delete;

template <class LTag, class RTag, typename ValueT>
std::enable_if_t<!std::is_same_v<LTag, RTag>, bool>
operator>(const NamedValue<LTag, ValueT> &, const NamedValue<RTag, ValueT> &) = delete;

template <class LTag, class RTag, typename ValueT>
std::enable_if_t<!std::is_same_v<LTag, RTag>, bool>
operator<=(const NamedValue<LTag, ValueT> &, const NamedValue<RTag, ValueT> &) = delete;

template <class LTag, class RTag, typename ValueT>
std::enable_if_t<!std::is_same_v<LTag, RTag>, bool>
operator>=(const NamedValue<LTag, ValueT> &, const NamedValue<RTag, ValueT> &) = delete;

}  // namespace utils_micro::detail

#endif  // UTILS_MICRO_DETAIL_NAMED_VALUE_H
