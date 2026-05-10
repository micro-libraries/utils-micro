/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_FLAT_MATRIX_H
#define UTILS_MICRO_MATRIX_FLAT_MATRIX_H
/*
Matrix data storage and view. Stored in contiguous memory with optional custom alignment.
*/
#include <memory>
#include <utils-micro/span.hpp>
#include <utils-micro/matrix/shape.hpp>
#include <utils-micro/assertion-handler.hpp>

namespace utils_micro::matrix {

template <typename T, class Allocator = std::allocator<T>>
class FlatMatrix;

template <typename T>
class FlatMatrixView {
protected:
    Shape shape;
    const T * data; // size = width * height

    void validate_row_access(CoordinateT r) const {
        error_handling::assertion(0 <= r && r < this->getHeight(), error_handling::AssertionType::OutOfBounds, "Row index does not lie in [0, height).");
    }

    void validate_bounds_access(Row r, Column c) const {
        error_handling::assertion(0 <= r && r < this->getHeight(), error_handling::AssertionType::OutOfBounds, "Row index does not lie in [0, height).");
        error_handling::assertion(0 <= c && c < this->getWidth(), error_handling::AssertionType::OutOfBounds, "Column index does not lie in [0, width).");
    }

public:
    FlatMatrixView(const Shape shape_, const T * data_) noexcept : shape{shape_}, data{data_} {}

    FlatMatrixView(const Shape shape_, span<const T> data_) noexcept : shape{shape_}, data{data_.data()} {
        error_handling::assertion(data_.size() == this->getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
    }

    Shape getShape() const { return this->shape; }
    Height getHeight() const { return Height{this->shape.height}; }
    Width getWidth() const { return Width{this->shape.width}; }
    SizeT getSize() const { return this->getHeight() * this->getWidth(); }

    span<const T> getData() const { return {this->data, static_cast<std::size_t>(this->getSize())}; }

    template <CoordinateT R>
    span<const T> getRowData() const {
        this->validate_row_access(R);
        return {this->data + R * this->getWidth(), this->getWidth()};
    }

    span<const T> getRowData(CoordinateT r) const {
        this->validate_row_access(r);
        return {this->data + r * this->getWidth(), this->getWidth()};
    }

    template <TemplateRowT R, TemplateColumnT C>
    const T & get() const {
        this->validate_bounds_access(Row{R}, Column{C});
        return this->data[R * this->getWidth() + C];
    }

    const T & get(Row r, Column c) const {
        this->validate_bounds_access(r, c);
        return this->data[r * this->getWidth() + c];
    }

    template <class Allocator = std::allocator<T>>
    FlatMatrix<T, Allocator> clone() const {
        return {this->shape, this->data};
    }
};

template <typename T>
class MutableFlatMatrixView : public FlatMatrixView<T> {
  public:
    MutableFlatMatrixView(Shape shape_, T * data_) noexcept : FlatMatrixView<T>{shape_, data_} {}

    MutableFlatMatrixView(Shape shape_, span<T> data_) noexcept : FlatMatrixView<T>{shape_, data_} {
        error_handling::assertion(data_.size() == this->getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
    }

    span<T> getMutableData() const { return {const_cast<T *>(this->data), this->getSize()}; }

    template <CoordinateT R>
    span<T> getMutableRowData() const {
        this->validate_row_access(R);
        return {this->data + R * this->getWidth(), this->getWidth()};
    }

    span<T> getMutableRowData(CoordinateT r) const {
        this->validate_row_access(r);
        return {this->data + r * this->getWidth(), this->getWidth()};
    }

    template <TemplateRowT R, TemplateColumnT C>
    T & getMutable() const {
        this->validate_bounds_access(R, C);
        return const_cast<T *>(this->data)[R * this->getWidth() + C];
    }

    T & getMutable(Row r, Column c) const {
        this->validate_bounds_access(r, c);
        return const_cast<T *>(this->data)[r * this->getWidth() + c];
    }

    template <TemplateRowT R, TemplateColumnT C>
    void set(const T & value) {
        this->validate_bounds_access(R, C);
        const_cast<T *>(this->data)[R * this->getWidth() + C] = value;
    }

    void set(Row r, Column c, const T & value) {
        this->validate_bounds_access(r, c);
        const_cast<T *>(this->data)[r * this->getWidth() + c] = value;
    }

    template <TemplateRowT R, TemplateColumnT C>
    void set(T && value) {
        this->validate_bounds_access(R, C);
        const_cast<T *>(this->data)[R * this->getWidth() + C] = std::move(value);
    }

    void set(Row r, Column c, T && value) {
        this->validate_bounds_access(r, c);
        const_cast<T *>(this->data)[r * this->getWidth() + c] = std::move(value);
    }
};

template <typename T, class Allocator>
class FlatMatrix : protected memory::ContainerBase<T, Allocator>, public MutableFlatMatrixView<T> {
    T * data_storage;

    void allocate() {
        this->data_storage = this->allocate_unaligned_data(this->getSize());
        this->data = static_cast<T *>(memory::align(this->allocator.getAlignment(), this->data_storage));
    }

    void deallocate() {
        this->deallocate_unaligned_data(this->data_storage, this->getSize());
    }

    void initialize_default() {
#if defined(__cpp_exceptions)
        try {
            std::uninitialized_default_construct_n(this->data, this->getSize());
        }
        catch (...) {
            this->deallocate();
            throw;
        }
#else
        std::uninitialized_default_construct_n(this->data, this->getSize());
#endif
    }

    void initialize_from_data(const T * data_) {
#if defined(__cpp_exceptions)
        try {
            std::uninitialized_copy_n(data_, this->getSize(), const_cast<T *>(this->data));
        }
        catch (...) {
            this->deallocate();
            throw;
        }
#else
        std::uninitialized_copy_n(data_, this->getSize(), this->data);
#endif
    }

    static constexpr auto is_noexcept = std::is_nothrow_copy_constructible_v<T> && memory::ContainerBase<T, Allocator>::is_allocator_noexcept;

  public:
    explicit FlatMatrix(Shape shape_) : memory::ContainerBase<T, Allocator>(), MutableFlatMatrixView<T>(shape_, nullptr) {
        this->allocate();
        this->initialize_default();
    }

    FlatMatrix(Shape shape_, const T * data_) noexcept(is_noexcept) : memory::ContainerBase<T, Allocator>(), MutableFlatMatrixView<T>(shape_, nullptr) {
        this->allocate();
        this->initialize_from_data(data_);
    }

    FlatMatrix(Shape shape_, span<const T> data_) noexcept(is_noexcept) : memory::ContainerBase<T, Allocator>(), MutableFlatMatrixView<T>(shape_, nullptr) {
        error_handling::assertion(data_.size() == this->getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
        this->allocate();
        this->initialize_from_data(data_.data());
    }

    FlatMatrix(const FlatMatrix &) = delete;
    FlatMatrix & operator=(const FlatMatrix &) = delete;
    FlatMatrix(FlatMatrix &&) = default;
    FlatMatrix & operator=(FlatMatrix &&) = default;

    ~FlatMatrix() {
        std::destroy_n(this->data, this->getSize());
        this->deallocate();
    }
};

}

#endif  // UTILS_MICRO_MATRIX_FLAT_MATRIX_H
