/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MEMORY_H
#define UTILS_MICRO_MEMORY_H
#include <algorithm>
#include <cstdint>
#include <memory>

namespace utils_micro::memory {

// variant of std::align that doesn't redundantly modify pointer argument
inline void * align(std::size_t alignment, std::size_t size, void * ptr, std::size_t & space)
{
    const auto intptr = reinterpret_cast<std::uintptr_t>(ptr);
    const auto aligned = (intptr - 1u + alignment) & -alignment;
    const auto diff = aligned - intptr;
    if (size + diff > space) { return nullptr; }
    space -= diff;
    return reinterpret_cast<void *>(aligned);
}

// short version if it's guaranteed that space is sufficient
inline void * align(std::size_t alignment, void * ptr)
{
    const auto intptr = reinterpret_cast<std::uintptr_t>(ptr);
    const auto aligned = (intptr - 1u + alignment) & -alignment;
    return reinterpret_cast<void *>(aligned);
}

// allocator that provides a large enough chunk of memory that align() is applicable

// todo remove T as an explicit type?
template <typename T, class BaseAllocator, std::size_t MIN_ALIGNMENT = 0>
class AllocatorWithAlignmentAdapter {
    static_assert((MIN_ALIGNMENT & (MIN_ALIGNMENT - 1)) == 0, "Alignment must be a power of two");

    template <typename, class, std::size_t>
    friend class AllocatorWithAlignment;

    constexpr static std::size_t getExtraSize() {
        return (getAlignment() - 1 + sizeof(void *)) / sizeof(T) + 1;
    }

    BaseAllocator base_allocator;

  public:
    using value_type = T;

    template <typename U>
    struct rebind {
        using other = AllocatorWithAlignmentAdapter<U, typename std::allocator_traits<BaseAllocator>::template rebind_alloc<U>, MIN_ALIGNMENT>;
    };

    constexpr static std::size_t getAlignment() {
        return std::max(MIN_ALIGNMENT, alignof(T));
    }

    AllocatorWithAlignmentAdapter() noexcept = default;

    template <typename U>
    explicit AllocatorWithAlignmentAdapter(const AllocatorWithAlignmentAdapter<U, typename std::allocator_traits<BaseAllocator>::template rebind_alloc<U>, MIN_ALIGNMENT> & other) noexcept
            : base_allocator(other.base_allocator) {}

    T * allocate(std::size_t n) {
        if (n == 0) { return nullptr; }
        T * mem = std::allocator_traits<BaseAllocator>::allocate(this->base_allocator, n + this->getExtraSize());
        return mem;
    }

    void deallocate(T * ptr, std::size_t n) noexcept {
        if (n == 0) { return; }
        std::allocator_traits<BaseAllocator>::deallocate(this->base_allocator, ptr, n + this->getExtraSize());
    }

    template <typename U>
    friend bool operator==(const AllocatorWithAlignmentAdapter & a, const AllocatorWithAlignmentAdapter<U, typename std::allocator_traits<BaseAllocator>::template rebind_alloc<U>, MIN_ALIGNMENT> & other) noexcept {
        return a.base_allocator == other.base_allocator;
    }

    template <typename U>
    friend bool operator!=(const AllocatorWithAlignmentAdapter & a, const AllocatorWithAlignmentAdapter<U, typename std::allocator_traits<BaseAllocator>::template rebind_alloc<U>, MIN_ALIGNMENT> & other) noexcept {
        return !(a == other);
    }

    using propagate_on_container_copy_assignment = typename std::allocator_traits<BaseAllocator>::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename std::allocator_traits<BaseAllocator>::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename std::allocator_traits<BaseAllocator>::propagate_on_container_swap;
    using is_always_equal = typename std::allocator_traits<BaseAllocator>::is_always_equal;
};

template <class Allocator>
inline constexpr bool is_allocator_noexcept = noexcept(std::allocator_traits<Allocator>::allocate(std::declval<Allocator&>(), 1));

template <class Allocator>
struct AllocatorTraits {
    template <typename, typename = void>
    struct allocator_has_alignment : std::false_type {};

    template <typename T>
    struct allocator_has_alignment<T, std::void_t<decltype(std::declval<T>().getAlignment())>> : std::true_type {};

    using AllocatorWithAlignment = std::conditional_t<allocator_has_alignment<Allocator>::value, Allocator, AllocatorWithAlignmentAdapter<typename Allocator::value_type, Allocator>>;

    static constexpr bool is_noexcept = memory::is_allocator_noexcept<AllocatorWithAlignment>;
};

template <typename T, class Allocator>
class ContainerBase {
public:
    using AllocatorWithAlignment = typename std::allocator_traits<typename AllocatorTraits<Allocator>::AllocatorWithAlignment>::template rebind_alloc<T>;

protected:
    AllocatorWithAlignment allocator;

public:
    static constexpr bool is_allocator_noexcept = AllocatorTraits<AllocatorWithAlignment>::is_noexcept;

    T * allocate_unaligned_data(int size) {
        return std::allocator_traits<AllocatorWithAlignment>::allocate(this->allocator, size);
    }

    void deallocate_unaligned_data(T * data, int size) {
        std::allocator_traits<AllocatorWithAlignment>::deallocate(this->allocator, data, size);
    }
};

}  // namespace utils_micro::memory

#endif  // UTILS_MICRO_MEMORY_H
