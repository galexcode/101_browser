/*
* Copyright 2012 Wolfgang Keller
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

#include "Unicode/Parser.h"
#include "MiniStdlib/MTAx_cstdlib.h" // for the conversation functions for endianness
#include <assert.h>

ReadResult parse_UTF32(
	ByteStreamReadInterface_v3 in_readInterface, 
	void *in_pReadState,
	ByteStreamWriteInterface_v3 in_writeInterface,
	void *in_pWriteState, 
	bool in_bigEndian)
{
	UnicodeCodePoint currentCodePoint;
	size_t rwCount;
	extern const UnicodeCodePoint cReplacementCharacter;

	assert(in_readInterface.mpfRead != NULL);
	assert(in_writeInterface.mpfWrite != NULL);
	
	if (in_readInterface.commonByteStreamInterface.mpfIsTerminated(in_pReadState))
		goto terminate;

	while (1)
	{
		rwCount = in_readInterface.mpfRead(in_pReadState, &currentCodePoint, 4);

		assert(rwCount <= 4);

		if (0 == rwCount)
		{
			assert(in_readInterface.commonByteStreamInterface.mpfIsTerminated(in_pReadState));

			goto terminate;
		}
		else if (4 != rwCount)
		{
			assert(rwCount > 0);
			assert(rwCount < 4);
			assert(in_readInterface.commonByteStreamInterface.mpfIsTerminated(in_pReadState));

			currentCodePoint = cReplacementCharacter;
			goto write_terminal_character;
		}

		if (in_bigEndian)
			currentCodePoint = _byteswap_ulong(currentCodePoint);

		if ((currentCodePoint >= 0xD800 && currentCodePoint <= 0xDFFF) ||
			currentCodePoint >= 0x110000)
			currentCodePoint = cReplacementCharacter;

		if (in_readInterface.commonByteStreamInterface.mpfIsTerminated(in_pReadState))
			goto write_terminal_character;

		rwCount = in_writeInterface.mpfWrite(in_pWriteState, &currentCodePoint, 
			sizeof(UnicodeCodePoint));

		if (rwCount != sizeof(UnicodeCodePoint))
			return ReadResultWriteError;
	}

write_terminal_character:
	rwCount = in_writeInterface.mpfWrite(in_pWriteState, &currentCodePoint, 
		sizeof(UnicodeCodePoint));

	if (sizeof(UnicodeCodePoint) != rwCount)
		return ReadResultWriteError;
	else
	{
		in_writeInterface.commonByteStreamInterface.mpfTerminate(in_pWriteState);
		return ReadResultOK;
	}

terminate:
	in_writeInterface.commonByteStreamInterface.mpfTerminate(in_pWriteState);
	return ReadResultOK;
}
