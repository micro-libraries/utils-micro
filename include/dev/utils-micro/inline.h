/*
 * Copyright (c) 2026, Jakub Safin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
 * was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTILS_MICRO_INLINE_H
#define UTILS_MICRO_INLINE_H

#ifdef _MSC_VER
#define FORCE_INLINE_PRAGMA
#define FORCE_INLINE inline __forceinline
#elif defined(__IAR_SYSTEMS_ICC__) || defined(__IAR_SYSTEMS_ICC)
#define FORCE_INLINE_PRAGMA _Pragma("inline=forced")
#define FORCE_INLINE inline
#else
#define FORCE_INLINE_PRAGMA
#define FORCE_INLINE inline __attribute__((always_inline))
#endif

#endif //UTILS_MICRO_INLINE_H
