/*
 * Copyright 2008-2012 Wolfgang Keller
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _2_5_5_h
#define _2_5_5_h

/*
 * 2.5.5 Dates and times
 * (previously 2.4.5 Dates and times
 * Draft Standard - 4th of August 2010)
 */

#include "MiniStdlib/cstdint.h"
#include "MiniStdlib/cstdbool.h"
#include "MiniStdlib/declspec.h"
#include "BigNumber/BigInteger.h"
#include "Unicode/Unicode.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * For 1 <= month <= 12 this function returns the number of days in the month
 * if month is out of this interval, 0 is returned.
 */
DLLEXPORT uint8_t numberOfDaysInMonthOfYear(uint8_t month, UnsignedBigInteger year);

DLLEXPORT bool isDigit(UnicodeCodePoint in_p);

#ifdef __cplusplus
}
#endif

#endif
