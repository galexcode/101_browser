/*
* Copyright 2011 Wolfgang Keller
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

#ifndef _MTAx_IO_BidirectionalStream_h
#define _MTAx_IO_BidirectionalStream_h

#include "Coroutine/Coroutine.h"
#include "IO/CoroutineStream.h"
#include "MiniStdlib/cstdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	// This state is before the halfstream started reading or writing
	BidirectionalHalfStreamActionNoAction,
	BidirectionalHalfStreamActionReading,
	BidirectionalHalfStreamActionWriting,
	BidirectionalHalfStreamActionTerminated
} BidirectionalHalfStreamAction;

typedef struct
{
	CoroutineDescriptor *mpCoroutineDescriptor;
	const uint8_t * volatile mpCurrentBuffer;
	volatile size_t mCurrentBufferSize;
	volatile BidirectionalHalfStreamAction mAction;
} BidirectionalHalfStreamState;

typedef struct
{
	// this has to be the first member of the struct because it will be casted
	CoroutineStreamFunctions mFunctions;
	
	BidirectionalHalfStreamState mHalfStreamStates[2];
	uint8_t mCurrentSide;
} BidirectionalStreamState;

#ifdef _WIN32
__declspec(dllexport)
#endif
void initBidirectionalStream(BidirectionalStreamState out_pBidirectionalStreamState,
CoroutineDescriptor *out_pThisCoroutine,
CoroutineDescriptor *out_pOtherCoroutine,
void (*in_pOtherCoroutineStartup)(BidirectionalStreamState*, void*),
void *in_pUserData
);

#ifdef _WIN32
__declspec(dllexport)
#endif
bool bidirectionalStreamIsReadingPossible(void *in_pBidirectionalStreamState);

#ifdef _WIN32
__declspec(dllexport)
#endif
bool bidirectionalStreamIsWritingPossible(void *in_pBidirectionalStreamState);

#ifdef _WIN32
__declspec(dllexport)
#endif
size_t bidirectionalStreamRead(void *in_out_pBidirectionalStreamState, void *out_pBuffer, size_t in_count);

#ifdef _WIN32
__declspec(dllexport)
#endif
size_t bidirectionalStreamWrite(void *in_out_pBidirectionalStreamState, const void *in_pBuffer, size_t in_count);

#ifdef __cplusplus
}
#endif

#endif
