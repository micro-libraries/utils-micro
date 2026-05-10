/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_FIXED_WIDTH_MATRIX_H
#define UTILS_MICRO_MATRIX_FIXED_WIDTH_MATRIX_H
/*
Matrix data storage and view. Stored in contiguous memory with optional custom alignment.
*/
#include <memory>
#include <utils-micro/matrix/shape.hpp>
#include <utils-micro/span.hpp>
#include <utils-micro/assertion-handler.hpp>
#include <utils-micro/memory.hpp>

namespace utils_micro::matrix {

template <typename T, TemplateWidthT WIDTH, class Allocator = std::allocator<T>>
class FixedWidthMatrix;

template <typename T, TemplateWidthT WIDTH>
class FixedWidthMatrixView
{
protected:
	SizeT height;
	const T * data;

	void validate_row_access(const CoordinateT r) const {
		error_handling::assertion(0 <= r && r < this->getHeight(), error_handling::AssertionType::OutOfBounds, "Row index does not lie in [0, height).");
	}

	void validate_bounds_access(const Row r, const Column c) const {
		error_handling::assertion(0 <= r && r < this->getHeight(), error_handling::AssertionType::OutOfBounds, "Row index does not lie in [0, height).");
		error_handling::assertion(0 <= c && c < getWidth(), error_handling::AssertionType::OutOfBounds, "Column index does not lie in [0, width).");
	}

	void validate_shape(const Shape shape) const {
		error_handling::assertion(shape.width == WIDTH, error_handling::AssertionType::ShapeMismatch, "Width passed to constructor must match width given by type.");
	}

public:
	FixedWidthMatrixView(const SizeT height_, const T * data_) noexcept : height{height_}, data{data_} {}

	FixedWidthMatrixView(const SizeT height_, span<const T> data_) noexcept : height{height_}, data{data_.data()} {
		error_handling::assertion(data_.size() == this->getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
	}

    // Redundant but provided for compatibility with variable shape matrix types.
	FixedWidthMatrixView(const Shape shape, const T * data_) noexcept : height{shape.height}, data{data_} {
		this->validate_shape(shape);
	}

	FixedWidthMatrixView(const Shape shape, span<const T> data_) noexcept : height{shape.height}, data{data_.data()} {
		this->validate_shape(shape);
		error_handling::assertion(data_.size() == this->getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
	}

	Shape getShape() const { return {Height{this->getHeight()}, Width{getWidth()}}; }

	Height getHeight() const { return Height{this->height}; }

	static constexpr TemplateWidthT getWidth() { return TemplateWidthT{WIDTH}; }

	SizeT getSize() const { return this->getHeight() * getWidth(); }

    span<const T> getData() const { return {this->data, static_cast<std::size_t>(this->getSize())}; }

	template <CoordinateT R>
	span<const T> getRowData() const { return {this->data + R * getWidth(), getWidth()}; }

    span<const T> getRowData(const CoordinateT r) const { return {this->data + r * getWidth(), getWidth()}; }

	template <TemplateRowT R, TemplateColumnT C>
    const T & get() const { return this->data[R * getWidth() + C]; }

    const T & get(const Row r, const Column c) const {
		this->validate_bounds_access(r, c);
		return this->data[r * getWidth() + c];
	}

    template <class Allocator = std::allocator<T>>
    FixedWidthMatrix<T, WIDTH, Allocator> clone() const
			noexcept(std::is_nothrow_copy_constructible_v<T> && memory::is_allocator_noexcept<Allocator>) {
		return {this->getHeight(), this->data};
	}
};

template <typename T, TemplateWidthT WIDTH>
class MutableFixedWidthMatrixView : public FixedWidthMatrixView<T, WIDTH>
{
	using View = FixedWidthMatrixView<T, WIDTH>;

public:
	MutableFixedWidthMatrixView(SizeT height_, const T * data_) noexcept : FixedWidthMatrixView<T, WIDTH>{height_, data_} {}

	MutableFixedWidthMatrixView(SizeT height_, span<const T> data_) noexcept : FixedWidthMatrixView<T, WIDTH>{height_, data_} {
		error_handling::assertion(
			data_.size() == this->getSize(),
			error_handling::AssertionType::SanityCheckFailed,
			"Matrix view is constructed from a span with different size."
		);
	}

	span<T> getMutableData() const { return {const_cast<T *>(this->data), this->getSize()}; }

	template <CoordinateT R>
    span<T> getMutableRowData() const {
		this->validate_row_access(R);
		return {this->data + R * View::getWidth(), View::getWidth()};
	}

    span<T> getMutableRowData(CoordinateT r) const {
		this->validate_row_access(r);
		return {this->data + r * View::getWidth(), View::getWidth()};
	}

	template <TemplateRowT R, TemplateColumnT C>
    T & getMutable() const {
		this->validate_row_access(R);
		static_assert(0 <= C && C < View::getWidth());
		return const_cast<T *>(this->data)[R * View::getWidth() + C];
	}

    T & getMutable(Row r, Column c) const {
		this->validate_bounds_access(r, c);
		return const_cast<T *>(this->data)[r * View::getWidth() + c];
	}

	template <TemplateRowT R, TemplateColumnT C>
    void set(const T & value) {
		this->validate_row_access(R);
		static_assert(0 <= C && C < View::getWidth(), "Column index does not lie in [0, width).");
		const_cast<T *>(this->data)[R * View::getWidth() + C] = value;
	}

    void set(Row r, Column c, const T & value) {
		this->validate_bounds_access(r, c);
		const_cast<T *>(this->data)[r * View::getWidth() + c] = value;
	}

	template <TemplateRowT R, TemplateColumnT C>
    void set(T && value) {
		this->validate_row_access(R);
		static_assert(0 <= C && C < View::getWidth(), "Column index does not lie in [0, width).");
	    const_cast<T *>(this->data)[R * View::getWidth() + C] = std::move(value);
	}

    void set(Row r, Column c, T && value) {
		this->validate_bounds_access(r, c);
		const_cast<T *>(this->data)[r * View::getWidth() + c] = std::move(value);
	}
};

template <typename T, TemplateWidthT WIDTH, class Allocator>
class FixedWidthMatrix : memory::ContainerBase<T, Allocator>, public MutableFixedWidthMatrixView<T, WIDTH> {
	using View = FixedWidthMatrixView<T, WIDTH>;

	T * data_storage;

	void allocate() {
		this->data_storage = this->allocate_unaligned_data(this->getSize());
		this->data = memory::align(this->allocator.getAlignment(), this->data_storage);
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
			std::uninitialized_copy_n(data_, this->getSize(), this->data);
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
    explicit FixedWidthMatrix(SizeT height_) noexcept(is_noexcept) :
			memory::ContainerBase<T, Allocator>(), MutableFixedWidthMatrixView<T, WIDTH>(height_, nullptr) {
	    this->allocate();
    	this->initialize_default();
    };

	FixedWidthMatrix(SizeT height_, const T * data_) noexcept(is_noexcept) :
			memory::ContainerBase<T, Allocator>(), MutableFixedWidthMatrixView<T, WIDTH>(height_, nullptr) {
    	this->allocate();
		this->initialize_from_data(data_);
	}

    FixedWidthMatrix(SizeT height_, span<const T> data_) noexcept(is_noexcept) :
		    memory::ContainerBase<T, Allocator>(), MutableFixedWidthMatrixView<T, WIDTH>(height_, nullptr) {
	    error_handling::assertion(data_.size() == this->getSize(), error_handling::AssertionType::SanityCheckFailed, "Matrix view is constructed from a span with different size.");
    	this->allocate();
		this->initialize_from_data(data_.data());
	}

	FixedWidthMatrix(const FixedWidthMatrix &) = delete;
	FixedWidthMatrix & operator=(const FixedWidthMatrix &) = delete;
	FixedWidthMatrix(FixedWidthMatrix &&) = default;
	FixedWidthMatrix & operator=(FixedWidthMatrix &&) = default;

	~FixedWidthMatrix() {
		std::destroy_n(this->data, this->getSize());
		this->deallocate();
	}
};

}

#endif  // UTILS_MICRO_MATRIX_FIXED_WIDTH_MATRIX_H
