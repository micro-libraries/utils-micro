/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_MATRIX_H
#define UTILS_MICRO_MATRIX_H
#include <utils-micro/matrix/fixed-shape-matrix.hpp>
#include <utils-micro/matrix/fixed-width-matrix.hpp>
#include <utils-micro/matrix/flat-matrix.hpp>
#include <utils-micro/matrix/nested-matrix.hpp>
#include <utils-micro/matrix/shape.hpp>
#include <utils-micro/matrix/traits.hpp>
#include <utils-micro/matrix/operations/abs.hpp>
#include <utils-micro/matrix/operations/phase.hpp>

/*
Matrix data storage and view.

Each row of a matrix is stored in contiguous memory. Beyond that, the exact form of data storage is specific
to each matrix class, as is the extent to which its shape is known at compile time:
- FixedShapeMatrix: flat, height and width given as template parameters
- FixedWidthMatrix: flat, width given as a template parameter
- FlatMatrix: flat, shape is only known in runtime
- NestedMatrix: rows are independently allocated and stored in a user-defined container, shape is only known in runtime

The allocation strategy, including the alignment of each row, is determined by a user-defined allocator.
Note that for a flat matrix, alignment must divide the size of a row.

TODO: allocator type, might be just implicit where passing std-style allocator converts to it

All matrix types provide element-wise getters and setters. Where appropriate, getters for spans of raw data
are also provided. For lifetime safety, note that all data getters return referencing types.

# Ownership model

Each Matrix type has a View (immutable by default) and a MutableView. These also serve as wrapper types over
any data that isn't owned as/by a Matrix, but temporarily treated as one and owned externally. Therefore,
constructing a View or MutableView describes in-place construction and subsequent usage.

Constructors of a Matrix correspond to default-initializing an empty Matrix with given shape,
or taking ownership of (i.e. moving) existing data.

To copy-construct a Matrix from existing data, construct a View and clone() it.

When a Matrix is used in a function that doesn't affect data ownership or allocations, it should be passed
as a const View & or a MutableView & to benefit from automatic decay since views are used as base classes.
*/

#endif //UTILS_MICRO_MATRIX_H
