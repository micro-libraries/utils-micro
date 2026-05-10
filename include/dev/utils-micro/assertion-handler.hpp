/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_ASSERT_H
#define UTILS_MICRO_ASSERT_H
#include <iostream>
#include <ostream>
#include <string_view>

namespace utils_micro::error_handling {

/* An assertion is an internal consistency check that can't be resolved by library caller. This handler allows customizing
 * if assertions need to be performed and behavior in case an assertion fails, e.g. program termination, trap or calling a specific user-defined error handler.
 * All assertions are *handled* in the same way but may be classified as one of a few standard *types* and described with a *message*.
 * It also provides tracing as far as selected compilation flags (e.g. for stack frame) support it.
 *
 * Its main purpose is addressing weaknesses in the C++ assert functionality: oddities of macros, lack of tracing and customizability.
 *
 * All checks of input correctness in micro dev libraries (header-only) are assertions. This is for performance reasons
 * and since the dev libraries aren't intended to interact with a user, just to provide low-level functionality for more comprehensive operations.
 *
 * This header defines an example handler that's no-op if `UTILS_MICRO_ASSERT` isn't defined; otherwise if the predicate
 * is false, it logs the message & assertion type & trace and terminates with error code 1.
 *
 * In order to use a handler (this default or custom), explicitly instantiate the function `utils_micro::error_handling::assert<HandlerT>`.
 */

enum class AssertionType {
    OutOfBounds,
    SanityCheckFailed,
    ShapeMismatch,
    OtherPreconditionFailure,
};

class AssertionHandler
{
public:
    // TODO traced assert
    void assertion(bool condition, AssertionType type, std::string_view message = "") {
#ifdef UTILS_MICRO_ASSERT
        if (!condition)
        {
            std::cerr << "Assertion failed: " << message << std::endl;
            std::exit(1);
        }
#endif
    }

};

void assertion(bool condition, AssertionType type, std::string_view message = "") {
    static AssertionHandler assertion_handler;
    assertion_handler.assertion(condition, type, message);
}

};

#endif  // UTILS_MICRO_ASSERT_H
