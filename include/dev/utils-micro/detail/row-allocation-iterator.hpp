/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_DETAIL_ROW_ALLOCATION_ITERATOR_H
#define UTILS_MICRO_DETAIL_ROW_ALLOCATION_ITERATOR_H
#include <iterator>
#include <utils-micro/size.hpp>

namespace utils_micro::detail {

template <class ContainerBaseT>
class RowAllocationIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = int;
    using value_type = typename ContainerBaseT::AllocatorWithAlignment::value_type *;
    using pointer = value_type;
    using reference = value_type;

private:
    int index;
    SizeT size;
    ContainerBaseT & container_base;
    mutable value_type allocated_data;

    void allocate() const {
        if (!this->allocated_data) {
            this->allocated_data = this->container_base.allocate_unaligned_data(this->size);
        }
    }

public:
    RowAllocationIterator(int index_, SizeT size_, ContainerBaseT & container_base_) :
        index{index_}, size{size_}, container_base{container_base_}, allocated_data{nullptr} {}

    value_type operator*() const {
        this->allocate();
        return this->allocated_data;
    }

    pointer operator->() const {
        this->allocate();
        return this->allocated_data;
    }

    RowAllocationIterator & operator++() {
        this->allocated_data = nullptr;
        ++this->index;
        return *this;
    }

    RowAllocationIterator operator++(int) {
        RowAllocationIterator result(*this);
        ++result;
        return result;
    }

    bool operator==(const RowAllocationIterator& other) const {
        return this->index == other.index;
    }

    bool operator!=(const RowAllocationIterator& other) const {
        return this->index != other.index;
    }
};

}

#endif  // UTILS_MICRO_DETAIL_ROW_ALLOCATION_ITERATOR_H
