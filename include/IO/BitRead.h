/*
 * Copyright 2008-2011 Wolfgang Keller
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

#ifndef _BitRead_h
#define _BitRead_h

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

#include "MiniStdlib/cstdint.h"
#include "MiniStdlib/cstdbool.h"
#include "MiniStdlib/cstddef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	/*
	 * This field contains a value from 0 to 7 and tells how many
	 * bits are stored in the buffer. Temporarily it can also take
	 * the value 8.
	 */
	uint8_t bitCountInBuffer;
	/*
	 * Up to 7 bits (temporarily 8) are buffered here. This variable
	 * acts as a kind of binary shift register.
	 */
	uint8_t buffer;
} BitReadState;

#ifdef _WIN32
__declspec(dllexport)
#endif
void initBitReadState(BitReadState *in_pBitReadState);

/*!
 * Reads in_bitsCount bits from the file.
 * 
 * Return value:
 * true if successful
 * false if not
 */
#ifdef _WIN32
__declspec(dllexport)
#endif
bool readBits(BitReadState *in_pBitReadState, void* in_pReaderState,  bool (*in_pReadByte)(void*, uint8_t*), 
void* in_pBuffer, size_t in_bitsCount);

#ifdef __cplusplus
}
#endif

#endif