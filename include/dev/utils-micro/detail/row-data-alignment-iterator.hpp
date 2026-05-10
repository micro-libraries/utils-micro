/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_DETAIL_ROW_DATA_ALIGNMENT_ITERATOR_H
#define UTILS_MICRO_DETAIL_ROW_DATA_ALIGNMENT_ITERATOR_H
#include <iterator>
#include <utils-micro/memory.hpp>
#include <utils-micro/detail/row-allocation-iterator.hpp>

namespace utils_micro::detail {

template <class Allocator, class DataStorageIterator>
class RowDataAlignmentIterator {
    Allocator & allocator;
    DataStorageIterator data_storage_iterator;

public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = int;
    using value_type = typename DataStorageIterator::value_type;
    using pointer = value_type;
    using reference = value_type;

private:
    value_type get_allocated_data(value_type data) const {
        return static_cast<value_type>(memory::align(this->allocator.getAlignment(), static_cast<void *>(data)));
    }

public:
    RowDataAlignmentIterator(Allocator & allocator_, DataStorageIterator && data_storage_iterator_) :
        allocator{allocator_}, data_storage_iterator{data_storage_iterator_} {}

    value_type operator*() const {
        return this->get_allocated_data(*this->data_storage_iterator);
    }

    pointer operator->() const {
        return this->get_allocated_data(*this->data_storage_iterator);
    }

    RowDataAlignmentIterator & operator++() {
        ++this->data_storage_iterator;
        return *this;
    }

    RowDataAlignmentIterator operator++(int) {
        RowAllocationIterator result(*this);
        ++result;
        return result;
    }

    bool operator==(const RowDataAlignmentIterator& other) const {
        return this->data_storage_iterator == other.data_storage_iterator;
    }

    bool operator!=(const RowDataAlignmentIterator& other) const {
        return this->data_storage_iterator != other.data_storage_iterator;
    }
};

}  // namespace utils_micro::detail

#endif  // UTILS_MICRO_DETAIL_ROW_DATA_ALIGNMENT_ITERATOR_H
