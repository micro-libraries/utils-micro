/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_NESTED_MATRIX_H
#define UTILS_MICRO_MATRIX_NESTED_MATRIX_H
#include <memory>
#include <optional>
#include <vector>
#include <utils-micro/span.hpp>
#include <utils-micro/matrix/shape.hpp>
#include <utils-micro/matrix/coordinates.hpp>
#include <utils-micro/assertion-handler.hpp>
#include <utils-micro/detail/row-allocation-iterator.hpp>
#include <utils-micro/detail/row-data-alignment-iterator.hpp>
#include <utils-micro/detail/row-data-view-iterator.hpp>

namespace utils_micro::matrix {

template <typename T, template <typename...> class Container = std::vector, class Allocator = std::allocator<T>>
class NestedMatrix;

template <typename T, template <typename...> class Container = std::vector>
class NestedMatrixView {
    // immutable view

protected:
    Shape shape;
    Container<const T *> data;

    void validate_row_access(CoordinateT r) const {
        error_handling::assertion(0 <= r && r < this->getHeight(), error_handling::AssertionType::OutOfBounds, "Row index does not lie in [0, height).");
    }

    void validate_bounds_access(Row r, Column c) const {
        error_handling::assertion(0 <= r.value && r.value < this->getHeight(), error_handling::AssertionType::OutOfBounds, "Row index does not lie in [0, height).");
        error_handling::assertion(0 <= c.value && c.value < this->getWidth(), error_handling::AssertionType::OutOfBounds, "Column index does not lie in [0, width).");
    }

    // Container<const T *> & getData() const { return this->data; }

    explicit NestedMatrixView(const Shape shape_) : shape{shape_} {}

public:
    template <typename Iterator>
    NestedMatrixView(const Shape shape_, Iterator data_rows_begin) : shape{shape_} {
        this->data = Container<const T*>(
            utils_micro::detail::RowDataViewIterator<Iterator>{0, this->getHeight(), data_rows_begin},
            utils_micro::detail::RowDataViewIterator<Iterator>{this->getHeight(), this->getHeight()}
        );
    }

    Shape getShape() const { return this->shape; }
    Height getHeight() const { return Height{this->shape.height}; }
    Width getWidth() const { return Width{this->shape.width}; }
    SizeT getSize() const { return this->getHeight() * this->getWidth(); }

	template <CoordinateT R>
    span<const T> getRowData() const {
        this->validate_row_access(R);
        return {this->data[R], this->getWidth()};
    }

    span<const T> getRowData(CoordinateT r) const {
        this->validate_row_access(r);
        return {this->data[r], this->getWidth()};
    }

	template <CoordinateT R, CoordinateT C>
    const T & get() const {
	    this->validate_bounds_access(Row{R}, Column{C});
	    return this->data[R][C];
	}

    const T & get(Row r, Column c) const {
        this->validate_bounds_access(r, c);
        return this->data[r.value][c.value];
    }

    template <class Allocator = std::allocator<T>>
    NestedMatrix<T, Container, Allocator> clone() const {
	    return {this->getShape(), std::begin(this->data)};
	}
};

template <typename T, template <typename...> class Container = std::vector>
class MutableNestedMatrixView : public NestedMatrixView<T, Container> {
protected:
    explicit MutableNestedMatrixView(const Shape shape_) : NestedMatrixView<T, Container>{shape_} {}

public:
    template <typename Iterator>
    MutableNestedMatrixView(Shape shape_, Iterator data_rows_begin) : NestedMatrixView<T, Container>{shape_, data_rows_begin} {}

	template <CoordinateT R>
    span<T> getMutableRowData() const {
        this->validate_row_access(R);
        return {const_cast<T *>(this->data[R]), this->getWidth()};
    }

    span<T> getMutableRowData(CoordinateT r) const {
        this->validate_row_access(r);
        return {const_cast<T *>(this->data[r]), static_cast<std::size_t>(this->getWidth())};
    }

	template <CoordinateT R, CoordinateT C>
    T & getMutable() const {
        this->validate_bounds_access(Row{R}, Column{C});
        return const_cast<T *>(this->data[R])[C];
    }

    T & getMutable(Row r, Column c) const {
        this->validate_bounds_access(r, c);
        return const_cast<T *>(this->data[r])[c];
    }

	template <CoordinateT R, CoordinateT C>
    void set(const T & value) {
        this->validate_bounds_access(Row{R}, Column{C});
        const_cast<T *>(this->data[R])[C] = value;
    }

    void set(Row r, Column c, const T & value) {
        this->validate_bounds_access(r, c);
        const_cast<T *>(this->data[r])[c] = value;
    }

	template <CoordinateT R, CoordinateT C>
    void set(T && value) {
        this->validate_bounds_access(Row{R}, Column{C});
        const_cast<T *>(this->data[R])[C] = std::move(value);
    }

    void set(Row r, Column c, T && value) {
        this->validate_bounds_access(r, c);
        const_cast<T *>(this->data[r])[c] = std::move(value);
    }
};

template <typename T, template <typename...> class Container, class Allocator>
class NestedMatrix : memory::ContainerBase<T, Allocator>, public MutableNestedMatrixView<T, Container> {
    Container<T *> data_storage;  // e.g. std::vector

    using AllocatorWithAlignment = typename memory::ContainerBase<T, Allocator>::AllocatorWithAlignment;

     void allocate() {
         this->data_storage = Container<T *>(
             utils_micro::detail::RowAllocationIterator<memory::ContainerBase<T, Allocator>>{0, this->getWidth(), *this},
             utils_micro::detail::RowAllocationIterator<memory::ContainerBase<T, Allocator>>{this->getHeight(), this->getWidth(), *this}
         );
         using DataStorageIterator = typename Container<T *>::const_iterator;
         this->data = Container<const T *>(
             utils_micro::detail::RowDataAlignmentIterator<AllocatorWithAlignment, DataStorageIterator>{this->allocator, this->data_storage.begin()},
             utils_micro::detail::RowDataAlignmentIterator<AllocatorWithAlignment, DataStorageIterator>{this->allocator, this->data_storage.end()}
         );
     }

     void deallocate() {
         for (CoordinateT r = 0; r != this->getHeight(); ++r) {
             this->deallocate_unaligned_data(this->data_storage[r], this->getWidth());
         }
     }

    void initialize_default() {
        for (CoordinateT r = 0; r != this->getHeight(); ++r) {
#if defined(__cpp_exceptions)
            try {
                std::uninitialized_default_construct_n(const_cast<T *>(this->data[r]), static_cast<std::size_t>(this->getWidth()));
            }
            catch (...) {
                this->deallocate();
                throw;
            }
#else
            std::uninitialized_default_construct_n(const_cast<T *>(this->data[r]), static_cast<std::size_t>(this->getWidth()));
#endif
        }
    }

    template <typename Iterator>
    void initialize_from_data(Iterator data_rows) {
         auto w = static_cast<std::size_t>(this->getWidth());
        for (CoordinateT r = 0; r != this->getHeight(); ++r, ++data_rows) {
            const T * data_row = *data_rows;
#if defined(__cpp_exceptions)
            try {
                std::uninitialized_copy_n(data_row, w, this->getMutableRowData(r).data());
            }
            catch (...) {
                this->deallocate();
                throw;
            }
#else
            std::uninitialized_copy_n(data_row, w, this->getMutableRowData(r).data());
#endif
        }
    }

    constexpr static auto is_noexcept =
        std::is_nothrow_copy_constructible_v<T> &&
        std::is_nothrow_default_constructible_v<Container<T *>> &&
        memory::ContainerBase<T, Allocator>::is_allocator_noexcept;

public:
    template <typename NewT>
    using MatrixOf = NestedMatrix<NewT, Container, typename std::allocator_traits<Allocator>::template rebind_alloc<NewT>>;

    explicit NestedMatrix(Shape shape_) :
            memory::ContainerBase<T, Allocator>(),
            MutableNestedMatrixView<T, Container>(shape_) {
        this->allocate();
        this->initialize_default();
    }

    template <typename Iterator>
    NestedMatrix(Shape shape_, Iterator data_rows_begin) noexcept(is_noexcept) :
            memory::ContainerBase<T, Allocator>{},
            MutableNestedMatrixView<T, Container>{shape_} {
        this->allocate();
        this->initialize_from_data(data_rows_begin);
    }

    NestedMatrix(const NestedMatrix &) = delete;
    NestedMatrix & operator=(const NestedMatrix &) = delete;
    NestedMatrix(NestedMatrix &&) = default;
    NestedMatrix & operator=(NestedMatrix &&) = default;

    ~NestedMatrix() {
        for (CoordinateT r = 0; r != this->getHeight(); ++r) {
            std::destroy_n(this->data[r], this->getWidth());
        }
        this->deallocate();
    }
};

}

#endif
