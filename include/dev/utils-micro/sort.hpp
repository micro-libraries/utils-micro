/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_SORT_H
#define UTILS_MICRO_SORT_H
#include <utils-micro/size.hpp>

/************************************************************************/
/* Copyright (C) 1991, 1992, 1996, 1997, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Douglas C. Schmidt (schmidt@ics.uci.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Usage:
 * first, define the following:
 *  QSORT_TYPE - type of array elements
 *  QSORT_BASE - pointer to array
 *  QSORT_NELT - number of elements in the array (must not be 0)
 *  QSORT_LT - QSORT_LT(a,b) should return true if *a < *b
 * and second, just #include this file into the place you want it.
 * Some C code will be inserted into that place, to sort array defined
 * by QSORT_TYPE, QSORT_BASE, QSORT_NELT and comparison routine QSORT_LT.
 */

namespace utils_micro::sort {

template <typename T>
class QuickSort {
    /* Swap two items pointed to by A and B using temporary buffer t. */
    static void swap(char * a, char * b, SizeT size) {
        auto * A = reinterpret_cast<unsigned int *>(a);
        auto * B = reinterpret_cast<unsigned int *>(b);
        for (SizeT i = 0; i != size / 4; i++, A++, B++) {
            unsigned int tmp = *A;
            *A = *B;
            *B = tmp;
        }
    }

    /* Discontinue quicksort algorithm when partition gets below this size.
    This particular magic number was chosen to work best on a Sun 4/260. */
    static constexpr unsigned MAX_THRESH = 4;

    /* Stack node declarations used to store unfulfilled partition obligations (inlined in QSORT).
    typedef struct {
    TYPE *_lo, *_hi;
    } qsort_stack_node;
    */

    /* The next 4 #defines implement a very fast in-line stack abstraction. */
    /* The stack needs log (total_elements) entries (we could even subtract
    log(MAX_THRESH)).  Since total_elements has type unsigned, we get as
    upper bound for log (total_elements):
    bits per byte (CHAR_BIT) * sizeof(unsigned).  */
    static constexpr unsigned STACK_SIZE = 8 * sizeof(unsigned);

    struct Range {
        SizeT _hi, _lo;
    };

    void push(Range *& top, SizeT low, SizeT high) {
        top = {.lo = low, .hi = high};
        ++top;
    }

    void pop(SizeT & low, SizeT & high, Range *& top) {
        --top;
        low = top->_lo;
        high = top->_hi;
    }

    /* Order size using quicksort.  This implementation incorporates
    four optimizations discussed in Sedgewick:

    1. Non-recursive, using an explicit stack of pointer that store the
      next array partition to sort.  To save time, this maximum amount
      of space required to store an array of SIZE_MAX is allocated on the
      stack.  Assuming a 32-bit (64 bit) integer for size_t, this needs
      only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
      Pretty cheap, actually.

    2. Chose the pivot element using a median-of-three decision tree.
      This reduces the probability of selecting a bad pivot value and
      eliminates certain extraneous comparisons.

    3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
      insertion sort to order the MAX_THRESH items within each partition.
      This is a big win, since insertion sort is faster for small, mostly
      sorted array segments.

    4. The larger of the two sub-partitions is always pushed onto the
      stack first, with the algorithm then concentrating on the
      smaller partition.  This *guarantees* no more than log (total_elems)
      stack size is needed (actually O(1) in this case)!  */

public:
    void sort(T * const base, const SizeT elems, const SizeT size, int (*lt)(const char *, const char *)) {
        if (elems <= 0) return;
        if (elems > MAX_THRESH) {
            SizeT lo = 0, hi = elems - 1;

            Range * _stack = new Range[STACK_SIZE];
            Range * top = _stack + 1;

            while (_stack != top) {
                /* Select median value from among LO, MID, and HI. Rearrange
                   LO and HI so the three values are sorted. This lowers the
                   probability of picking a pathological pivot value and
                   skips a comparison for both the LEFT_PTR and RIGHT_PTR in
                   the while loops. */

                SizeT mid = (hi + lo) >> 1;

                auto * lo_ptr = base + lo;
                auto * hi_ptr = base + hi;
                auto * mid_ptr = base + mid;

                if (lt(mid_ptr, lo_ptr)) { this->swap(mid_ptr, lo_ptr, size); }
                if (lt(hi_ptr, mid_ptr)) {
                    this->swap(mid_ptr, hi_ptr, size);
                    if (lt(mid_ptr, lo_ptr)) { this->swap(mid_ptr, lo_ptr, size); }
                }

                SizeT left = lo + 1;
                SizeT right = hi - 1;

                /* Here's the famous ``collapse the walls'' section of quicksort.
                   Gotta like those tight inner loops!  They are the main reason
                   that this algorithm runs much faster than others. */
                do {
                    mid_ptr = base + mid;
                    while (lt(base + left, mid_ptr)) { ++left; }

                    while (lt(mid_ptr, base + right)) { --right; }

                    if (left < right) {
                        this->swap(base + left, base + right, size);
                        if (mid == left) { mid = right; }
                        else if (mid == right) { mid = left; }
                        ++left, --right;
                    }
                    else if (left == right) {
                        ++left, --right;
                        break;
                    }
                }
                while (left <= right);

                /* Set up pointers for next iteration.  First determine whether
                   left and right partitions are below the threshold size.  If so,
                   ignore one or both.  Otherwise, push the larger partition's
                   bounds on the stack and continue sorting the smaller one. */

                if (right - lo <= MAX_THRESH) {
                    if (hi - left <= MAX_THRESH) {
                        // Ignore both small partitions.
                        _qsort_pop(lo, hi, top);
                    }
                    else {
                        // Ignore small left partition.
                        lo = left;
                    }
                }
                else if (hi - left <= MAX_THRESH) {
                    // Ignore small right partition.
                    hi = right;
                }
                else if (right - lo > hi - left) {
                    // Push larger left partition indices.
                    this->push(top, lo, right);
                    lo = left;
                }
                else {
                    // Push larger right partition indices.
                    this->push(top, left, hi);
                    hi = right;
                }
            }

            delete[] _stack;
        }

        /* Once the BASE array is partially sorted by quicksort the rest
           is completely sorted using insertion sort, since this is efficient
           for partitions below MAX_THRESH size. BASE points to the
           beginning of the array to sort, and END_PTR points at the very
           last element in the array (*not* one beyond it!). */

        {
            auto * const _end = base + (elems - 1);
            auto * _tmp_ptr = base;

            auto * _thresh = base + MAX_THRESH;
            if (MAX_THRESH > elems - 1) { _thresh = _end; }

            // Find the smallest element in first threshold and place it at the array's beginning. This is
            // the smallest array element, and the operation speeds up insertion sort's inner loop.

            for (auto * _run = _tmp_ptr + 1; _run <= _thresh; ++_run) {
                if (lt(_run, _tmp_ptr)) { _tmp_ptr = _run; }
            }

            if (_tmp_ptr != base) { this->swap(_tmp_ptr, base, size); }

            // Insertion sort, running from left-hand-side up to right-hand-side.

            SizeT run = 0;
            while (++run < elems) {
                _tmp_ptr = base + (run - 1);
                auto * _run = base + run;
                while (lt(_run, _tmp_ptr)) { --_tmp_ptr; }

                ++_tmp_ptr;
                auto * lo = _run;
                while (lo != _tmp_ptr) {
                    auto * hi = lo;
                    --lo;
                    this->swap(lo, hi, size);
                }
            }
        }
    }

    // "nth element"
    void partialSort(char * const base, const SizeT elems, const SizeT /*mid*/, const SizeT size,
                     int (*lt)(const char *, const char *)) {
        this->sort(base, elems, size, lt);
    }
};

}  // namespace utils_micro::sort

#endif  // UTILS_MICRO_SORT_H
