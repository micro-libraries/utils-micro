/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_FIXED_SHAPE_MATRIX_H
#define UTILS_MICRO_MATRIX_FIXED_SHAPE_MATRIX_H
/*
Stored in contiguous memory with optional custom alignment.
*/
#include <memory>
#include <utils-micro/span.hpp>
#include <utils-micro/matrix/shape.hpp>
#include <utils-micro/assertion-handler.hpp>
#include <utils-micro/memory.hpp>
#include <utils-micro/matrix/coordinates.hpp>

namespace utils_micro::matrix {

template <typename T, TemplateHeightT HEIGHT, TemplateWidthT WIDTH, class Allocator = std::allocator<T>>
class FixedShapeMatrix;

template <typename T, TemplateHeightT HEIGHT, TemplateWidthT WIDTH>
class FixedShapeMatrixView {
  protected:
    const T * data;

    void validate_row_access(const CoordinateT r) const {
      error_handling::assertion(0 <= r && r < getHeight(), error_handling::AssertionType::OutOfBounds, "Row index does not lie in [0, height).");
    }

    void validate_bounds_access(const Row r, const Column c) const {
      error_handling::assertion(0 <= r && r < getHeight(), error_handling::AssertionType::OutOfBounds, "Row index does not lie in [0, height).");
      error_handling::assertion(0 <= c && c < getWidth(), error_handling::AssertionType::OutOfBounds, "Column index does not lie in [0, width).");
    }

    void validate_shape(const Shape shape) const {
      error_handling::assertion(shape.height == HEIGHT, error_handling::AssertionType::ShapeMismatch, "Height passed to constructor must match height given by type.");
      error_handling::assertion(shape.width == WIDTH, error_handling::AssertionType::ShapeMismatch, "Width passed to constructor must match width given by type.");
    }

  public:
    explicit FixedShapeMatrixView(span<const T, HEIGHT * WIDTH> data_) noexcept : data{data_.data()} {}

    explicit FixedShapeMatrixView(span<const T> data_) noexcept : data{data_.data()} {
      error_handling::assertion(data_.size() == getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
    }

    explicit FixedShapeMatrixView(const T * data_) noexcept : data{data_} {}

    // Redundant but provided for compatibility with variable shape matrix types.
    explicit FixedShapeMatrixView(const Shape shape, span<const T, HEIGHT * WIDTH> data_) noexcept : FixedShapeMatrixView{data_} {
      this->validate_shape(shape);
    }

    explicit FixedShapeMatrixView(const Shape shape, span<const T> data_) noexcept : FixedShapeMatrixView{data_} {
      this->validate_shape(shape);
      error_handling::assertion(data_.size() == getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
    }

    explicit FixedShapeMatrixView(const Shape shape, const T * data_) noexcept : FixedShapeMatrixView{data_} {
      this->validate_shape(shape);
    }

    static Shape getShape() { return {Height{getHeight()}, Width{getWidth()}}; }

    static constexpr TemplateHeightT getHeight() { return TemplateHeightT{HEIGHT}; }

    static constexpr TemplateWidthT getWidth() { return TemplateWidthT{WIDTH}; }

    static constexpr SizeT getSize() { return getHeight() * getWidth(); }

    span<const T, getSize()> getData() const { return {this->data, static_cast<std::size_t>(getSize())}; }

    template <CoordinateT R>
    span<const T, getWidth()> getRowData() const {
      static_assert(0 <= R && R < getHeight(), "Row index does not lie in [0, height).");
      return this->data + R * getWidth();
    }

    span<const T, getWidth()> getRowData(CoordinateT r) const {
      this->validate_row_access(r);
      return this->data + r * getWidth();
    }

  	template <TemplateRowT R, TemplateColumnT C>
    const T & get() const {
      static_assert(0 <= R && R < getHeight(), "Row index does not lie in [0, height).");
      static_assert(0 <= C && C < getWidth(), "Column index does not lie in [0, width).");
      return this->data[R * getWidth() + C];
    }

    const T & get(const Row r, const Column c) const {
      this->validate_bounds_access(r, c);
      return this->data[r * getWidth() + c];
    }

    template <class Allocator = std::allocator<T>>
    FixedShapeMatrix<T, HEIGHT, WIDTH, Allocator> clone() const {
      return this->data;
    }
};

template <typename T, TemplateHeightT HEIGHT, TemplateWidthT WIDTH>
class MutableFixedShapeMatrixView : public FixedShapeMatrixView<T, HEIGHT, WIDTH> {
    using View = FixedShapeMatrixView<T, HEIGHT, WIDTH>;

  public:
    explicit MutableFixedShapeMatrixView(span<T, HEIGHT * WIDTH> data_) noexcept : FixedShapeMatrixView<T, HEIGHT, WIDTH>{data_} {}

    explicit MutableFixedShapeMatrixView(span<T> data_) noexcept : FixedShapeMatrixView<T, HEIGHT, WIDTH>{data_} {
      error_handling::assertion(data_.size() == View::getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
    }

    explicit MutableFixedShapeMatrixView(T * data_) noexcept : FixedShapeMatrixView<T, HEIGHT, WIDTH>{data_} {}

    span<T, View::getSize()> getMutableData() const { return const_cast<T *>(this->data); }

    template <CoordinateT R>
    span<T, View::getWidth()> getMutableRowData() const {
      static_assert(0 <= R && R < View::getHeight(), "Row index does not lie in [0, height).");
      return this->data + R * View::getWidth();
    }

    span<T, View::getWidth()> getMutableRowData(const CoordinateT r) const {
      this->validate_row_access(r);
      return this->data + r * View::getWidth();
    }

  	template <TemplateRowT R, TemplateColumnT C>
    T & getMutable() const {
      static_assert(0 <= R && R < View::getHeight(), "Row index does not lie in [0, height).");
      static_assert(0 <= C && C < View::getWidth(), "Column index does not lie in [0, width).");
      return const_cast<T *>(this->data)[R * View::getWidth() + C];
  	}

    T & getMutable(Row r, Column c) const {
      this->validate_bounds_access(r, c);
  		return const_cast<T *>(this->data)[r * View::getWidth() + c];
  	}

    template <TemplateRowT R, TemplateColumnT C>
    void set(const T & value) {
      static_assert(0 <= R && R < View::getHeight(), "Row index does not lie in [0, height).");
      static_assert(0 <= C && C < View::getWidth(), "Column index does not lie in [0, width).");
      const_cast<T *>(this->data)[R * View::getWidth() + C] = value;
  	}

    void set(Row r, Column c, const T & value) {
      this->validate_bounds_access(r, c);
      const_cast<T *>(this->data)[r * View::getWidth() + c] = value;
    }

    template <TemplateRowT R, TemplateColumnT C>
    void set(T && value) {
      static_assert(0 <= R && R < View::getHeight(), "Row index does not lie in [0, height).");
      static_assert(0 <= C && C < View::getWidth(), "Column index does not lie in [0, width).");
      const_cast<T *>(this->data)[R * View::getWidth() + C] = std::move(value);
    }

    void set(Row r, Column c, T && value) {
      this->validate_bounds_access(r, c);
      const_cast<T *>(this->data)[r * View::getWidth() + c] = std::move(value);
    }
};

template <typename T, TemplateHeightT HEIGHT, TemplateWidthT WIDTH, class Allocator>
class FixedShapeMatrix : memory::ContainerBase<T, Allocator>, public MutableFixedShapeMatrixView<T, HEIGHT, WIDTH> {
  using View = FixedShapeMatrixView<T, HEIGHT, WIDTH>;

  T * data_storage;

  void allocate() {
    this->data_storage = this->allocate_unaligned_data(View::getSize());
    this->data = memory::align(this->allocator.getAlignment(), this->data_storage);
  }

  void deallocate() {
    this->deallocate_unaligned_data(this->data_storage, View::getSize());
  }

  void initialize_default() {
#if defined(__cpp_exceptions)
    try {
      std::uninitialized_default_construct_n(this->data, View::getSize());
    }
    catch (...) {
      this->deallocate();
      throw;
    }
#else
    std::uninitialized_default_construct_n(this->data, View::getSize());
#endif
  }

  void initialize_from_data(const T * data_) {
#if defined(__cpp_exceptions)
    try {
      std::uninitialized_copy_n(data_, View::getSize(), this->data);
    }
    catch (...) {
      this->deallocate();
      throw;
    }
#else
    std::uninitialized_copy_n(data_, View::getSize(), this->data);
#endif
  }

  static constexpr auto is_noexcept = std::is_nothrow_copy_constructible_v<T> && memory::ContainerBase<T, Allocator>::is_allocator_noexcept;

  public:
    FixedShapeMatrix() noexcept(is_noexcept) :
      memory::ContainerBase<T, Allocator>(), MutableFixedShapeMatrixView<T, HEIGHT, WIDTH>(nullptr) {
      this->allocate();
      this->initialize_default();
    }

    explicit FixedShapeMatrix(const T * data_) noexcept(is_noexcept) :
      memory::ContainerBase<T, Allocator>(), MutableFixedShapeMatrixView<T, HEIGHT, WIDTH>() {
      this->allocate();
      this->initialize_from_data(data_);
    }

    explicit FixedShapeMatrix(span<const T> data_) noexcept(is_noexcept) :
      memory::ContainerBase<T, Allocator>(), MutableFixedShapeMatrixView<T, HEIGHT, WIDTH>() {
      error_handling::assertion(
        data_.size() == View::getSize(),
        error_handling::AssertionType::SanityCheckFailed,
        "Matrix view is constructed from a span with different size."
      );
      this->allocate();
      this->initialize_from_data(data_.data());
    }

    explicit FixedShapeMatrix(span<const T, View::getSize()> data_) noexcept(is_noexcept) :
      memory::ContainerBase<T, Allocator>(), MutableFixedShapeMatrixView<T, HEIGHT, WIDTH>() {
      this->allocate();
      this->initialize_from_data(data_.data());
    }

    FixedShapeMatrix(const FixedShapeMatrix &) = delete;
    FixedShapeMatrix & operator=(const FixedShapeMatrix &) = delete;
    FixedShapeMatrix(FixedShapeMatrix &&) = default;
    FixedShapeMatrix & operator=(FixedShapeMatrix &&) = default;

    ~FixedShapeMatrix() {
      std::destroy_n(this->data, View::getSize());
      this->deallocate();
    }
};

}  // namespace utils_micro::matrix

#endif  // UTILS_MICRO_MATRIX_FIXED_SHAPE_MATRIX_H
