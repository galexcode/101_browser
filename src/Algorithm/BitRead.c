#include "Algorithm/BitRead.h"
#include <assert.h>

void initBitReadState(BitReadState *in_pBitReadState)
{
	in_pBitReadState->bitCountInBuffer = 0;
}

bool readBits(BitReadState *in_pBitReadState, void* in_pBuffer, size_t in_bitsCount, FILE* in_file)
{
	fpos_t lPosBackup;
	uint8_t *currentBufferElement = (uint8_t *) in_pBuffer;
	size_t currentBitIndex; // counter in the loop

	assert(in_bitsCount < SIZE_MAX);
	
	if (fgetpos(in_file, &lPosBackup) != 0)
		return false;

	for (currentBitIndex = 0; currentBitIndex < in_bitsCount; ++currentBitIndex)
	{
		uint8_t currentBit;
		uint8_t position;

		if (in_pBitReadState->bitCountInBuffer == 0)
		{
			if (fread(&in_pBitReadState->buffer, 1, 1, in_file) != 1)
			{
				fsetpos(in_file, &lPosBackup);
				return false;
			}

			in_pBitReadState->bitCountInBuffer = 8;
		}

		assert(in_pBitReadState->bitCountInBuffer > 0);
		assert(in_pBitReadState->bitCountInBuffer <= 8);

		if (currentBitIndex % 8 == 0)
		{
			*currentBufferElement = 0;
		}

		position = (8-in_pBitReadState->bitCountInBuffer) % 8;
		currentBit = (in_pBitReadState->buffer >> position) & 1;
		*currentBufferElement |= (currentBit<<(currentBitIndex % 8));
		in_pBitReadState->bitCountInBuffer--;

		if ((currentBitIndex+1) % 8 == 0)
		{
			currentBufferElement++;
		}
	}

	return true;
}
