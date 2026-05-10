/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_DETAIL_ROW_DATA_VIEW_ITERATOR_H
#define UTILS_MICRO_DETAIL_ROW_DATA_VIEW_ITERATOR_H
#include <iterator>
#include <optional>
#include <utils-micro/size.hpp>

namespace utils_micro::detail {

// To avoid mixing up height and width, matrix constructors take a Shape object. Construction from a pair of iterators
// is thus redundant. However, STL containers do not support construction from size and an input iterator.
// The following iterator is a workaround for such a construction.
template <typename InputIterator>
class RowDataViewIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = int;
    using value_type = typename std::iterator_traits<InputIterator>::value_type;
    using pointer = value_type;
    using reference = value_type;

private:
    int index;
    SizeT size;
    std::optional<InputIterator> input_iterator;

public:
    RowDataViewIterator(int index_, SizeT size_, InputIterator input_iterator_) :
        index{index_}, size{size_}, input_iterator{input_iterator_} {
    }

    RowDataViewIterator(int index_, SizeT size_) :
        index{index_}, size{size_}, input_iterator{std::nullopt} {
    }

    value_type operator*() const {
        return *(*this->input_iterator);
    }

    pointer operator->() const {
        return *(*this->input_iterator);
    }

    RowDataViewIterator & operator++() {
        ++this->index;
        ++(*this->input_iterator);
        return *this;
    }

    RowDataViewIterator operator++(int) {
        RowDataViewIterator result(*this);
        ++result;
        return result;
    }

    bool operator==(const RowDataViewIterator& other) const {
        return this->index == other.index;
    }

    bool operator!=(const RowDataViewIterator& other) const {
        return this->index != other.index;
    }
};

}

#endif  // UTILS_MICRO_DETAIL_ROW_DATA_VIEW_ITERATOR_H

