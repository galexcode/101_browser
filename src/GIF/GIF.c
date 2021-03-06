﻿/*
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

#include "GIF/GIF.h"
#include "GIF/LZW.h"
#include "IO/BitRead.h"
#include "SetjmpUtil/ConditionalLongjmp.h"
#include "MiniStdlib/safe_free.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define EXTENSION_INTRODUCER  0x21
#define IMAGE_SEPARATOR       0x2C
#define GIF_TRAILER           0x3B

// Extensions
#define PLAIN_TEXT_LABEL      0x01
#define GRAPHIC_CONTROL_LABEL 0xF9
#define COMMENT_LABEL         0xFE
#define APPLICATION_LABEL     0xFF

uint16_t colorsOfColorTable(uint8_t in_sizeOfColorTable)
{
	return 1<<(in_sizeOfColorTable+1);
}

uint16_t bytesOfColorTable(uint8_t in_sizeOfColorTable)
{
	return 3*colorsOfColorTable(in_sizeOfColorTable);
}

ReadResult read_GIF_Data_Stream(void *in_pStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	GIF_Data_Stream *in_pDataStream, void (*in_pfErrorHandler)(void *))
{
	bool is89a;

	SetjmpStreamState setjmpReadStreamState;
	jmp_buf jmpBuf;
	ByteStreamInterface setjmpReadStreamInterface;
	int result;
	uint8_t lIntroducer;

	setjmpStreamInit(&setjmpReadStreamState, &jmpBuf, ReadResultPrematureEndOfStream, 
		in_pStreamState, in_byteStreamReadInterface);
	setjmpReadStreamInterface = getSetjmpStreamByteStreamInterface(&setjmpReadStreamState);
	
	in_pDataStream->logicalScreen.globalColorTable = NULL;

	if ((result = setjmp(jmpBuf)) != 0)
	{
		if (in_pDataStream->logicalScreen.globalColorTable)
		{
			safe_free(&in_pDataStream->logicalScreen.globalColorTable);
		}

		return (ReadResult) result;
	}

	read_Header(&setjmpReadStreamState, setjmpReadStreamInterface, 
		&is89a, in_pfErrorHandler);

	read_Logical_Screen(&setjmpReadStreamState, setjmpReadStreamInterface, 
		&in_pDataStream->logicalScreen, is89a, in_pfErrorHandler);

	(*setjmpReadStreamInterface.mpfRead)(&setjmpReadStreamState, &lIntroducer, sizeof(lIntroducer));

	while (GIF_TRAILER != lIntroducer)
	{
		read_Data(&setjmpReadStreamState, setjmpReadStreamInterface, 
			lIntroducer, is89a, &in_pDataStream->logicalScreen, in_pfErrorHandler);
		(*setjmpReadStreamInterface.mpfRead)(&setjmpReadStreamState, &lIntroducer, sizeof(lIntroducer));
	}

	if (in_pDataStream->logicalScreen.globalColorTable)
	{
		safe_free(&in_pDataStream->logicalScreen.globalColorTable);
	}

	return ReadResultOK;
}

void read_Header(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	bool *out_pIs89a, void (*in_pfErrorHandler)(void *))
{
	Header header;

	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, &header, sizeof(header));

	longjmpIf(strncmp(header.Signature, "GIF", sizeof(header.Signature)) != 0, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, ReadResultInvalidData, 
		in_pfErrorHandler, "read_Header: invalid Signature");

	if (strncmp(header.Version, "87a", sizeof(header.Version)) == 0)
	{
		*out_pIs89a = false;
	}
	else if (strncmp(header.Version, "89a", sizeof(header.Version)) == 0)
	{
		*out_pIs89a = true;
	}
	else
	{
		longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, "read_Header: invalid Version");
	}
}

void read_SpecialPurpose_Block(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	uint8_t in_label, void (*in_pfErrorHandler)(void *))
{
	switch (in_label)
	{
	case COMMENT_LABEL:
		/*
		* Because of PRE:GIF_h_129 the precondition PRE:GIF_h_158
		* is satisfied.
		*/
		read_Comment_Extension(in_out_pSetjmpStreamState, in_byteStreamReadInterface);
		return;
	case APPLICATION_LABEL:
		/*
		* Because of PRE:GIF_h_129 the precondition PRE:GIF_h_148
		* is satisfied.
		*/
		read_Application_Extension(in_out_pSetjmpStreamState, 
			in_byteStreamReadInterface, in_pfErrorHandler);
		return;
	default:
		longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, 
			"read_SpecialPurpose_Block: expecting Application Extension or Comment Extension");
	}
}

void read_Logical_Screen(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	Logical_Screen *in_pLogicalScreen, bool in_is89a, 
	void (*in_pfErrorHandler)(void *))
{
	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&in_pLogicalScreen->logicalScreenDescriptor, sizeof(Logical_Screen_Descriptor));

	if (!in_is89a)
	{
		if (in_pLogicalScreen->logicalScreenDescriptor.Sort_Flag)
		{
			longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
				ReadResultInvalidData, in_pfErrorHandler, 
				"read_Logical_Screen: in GIF 87a the (GIF 89a) Sort Flag must be set to 0");
		}

		if (in_pLogicalScreen->logicalScreenDescriptor.Pixel_Aspect_Ratio)
		{
			longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
				ReadResultInvalidData, in_pfErrorHandler, 
				"read_Logical_Screen: in GIF 87a the (GIF 89a) Pixel Aspect Ratio must be set to 0");
		}
	}

	if (in_pLogicalScreen->logicalScreenDescriptor.Global_Color_Table_Flag)
	{
		size_t bytesOfGlobalColorTable = bytesOfColorTable(in_pLogicalScreen->logicalScreenDescriptor.Size_Of_Global_Color_Table);
		jmp_buf freeMemoryJmpBuf;
		int result;

		if (in_pLogicalScreen->logicalScreenDescriptor.Background_Color_Index 
			>= colorsOfColorTable(in_pLogicalScreen->logicalScreenDescriptor.Size_Of_Global_Color_Table))
		{
			longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
				ReadResultInvalidData, in_pfErrorHandler, 
				"read_Logical_Screen: Background Color Index is >= # colors in Global Color table");
		}

		in_pLogicalScreen->globalColorTable = (Rgb8Color*) longjmpMalloc(
			in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultAllocationFailure, bytesOfGlobalColorTable);

		if ((result = XCHG_AND_SETJMP(*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			freeMemoryJmpBuf)) != 0)
		{
			safe_free(&in_pLogicalScreen->globalColorTable);
			xchgAndLongjmp(freeMemoryJmpBuf, 
				*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, result);
		}

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			in_pLogicalScreen->globalColorTable, bytesOfGlobalColorTable);

		xchgJmpBuf(freeMemoryJmpBuf, 
			*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer);
	}
	else
	{
		in_pLogicalScreen->globalColorTable = NULL;
	}
}

void read_Data(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	uint8_t in_introducer, bool in_is89a, 
	const Logical_Screen *in_cpLogicalScreen, 
	void (*in_pfErrorHandler)(void *))
{
	uint8_t lLabel;

	if (EXTENSION_INTRODUCER == in_introducer)
	{
		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			&lLabel, sizeof(lLabel));

		/*
		* We have the following situation:
		*
		* ┌───────────────────────────────────────────────────────┬─────────┬───────────────────────┐
		* │Extension Introducer                                   │ GIF 87a │ GIF 89a               │
		* ├───────────────────────────────────────────────────────┼─────────┼───────────────────────┤
		* │0xFE (COMMENT_LABEL) or 0xFF (APPLICATION_LABEL)       │         │ Special Purpose Block │
		* │0x01 (PLAIN_TEXT_LABEL) or 0xF9 (GRAPHIC_CONTROL_LABEL)│  skip   │     Graphic Block     │
		* │other                                                  │         │         skip          │
		* └───────────────────────────────────────────────────────┴─────────┴───────────────────────┘
		*/
		// CND:GIF_c_153
		if (in_is89a)
		{
			if (COMMENT_LABEL == lLabel || APPLICATION_LABEL == lLabel)
			{
				/*
				* Because of CND:GIF_c_153 the precondition
				* PRE:GIF_h_129 is satisfied.
				*/
				read_SpecialPurpose_Block(in_out_pSetjmpStreamState, 
					in_byteStreamReadInterface, lLabel, in_pfErrorHandler);
				return;
			}
			else if (PLAIN_TEXT_LABEL == lLabel || GRAPHIC_CONTROL_LABEL == lLabel)
			{
				read_Graphic_Block(in_out_pSetjmpStreamState, 
					in_byteStreamReadInterface, in_introducer, lLabel, 
					in_is89a, in_cpLogicalScreen, in_pfErrorHandler);
				return;
			}
			else
			{
				longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
					ReadResultInvalidData, in_pfErrorHandler, 
					"read_Data: the only accepted Extension Blocks are Graphic Control Extension and Plain Text Extension");
			}
		}
		else
		{
			skipBlock(in_out_pSetjmpStreamState, in_byteStreamReadInterface);
			return;
		}
	}
	else if (IMAGE_SEPARATOR == in_introducer)
	{
		read_Graphic_Block(in_out_pSetjmpStreamState, 
			in_byteStreamReadInterface, in_introducer, 0, 
			in_is89a, in_cpLogicalScreen, in_pfErrorHandler);
		return;
	}
	else
	{
		longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, 
			"read_Data: expecting Image Descriptor or Extension Introducer");
	}
}

void read_Graphic_Block(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	uint8_t in_separator, uint8_t in_label, bool in_is89a, 
	const Logical_Screen *in_cpLogicalScreen, 
	void (*in_pfErrorHandler)(void *))
{
	if (EXTENSION_INTRODUCER == in_separator && GRAPHIC_CONTROL_LABEL == in_label) 
	{
		// Precondition: we have a GIF 89a file
		read_Graphic_Control_Extension(in_out_pSetjmpStreamState, 
			in_byteStreamReadInterface, in_pfErrorHandler);

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			&in_separator, sizeof(in_separator));

		if (EXTENSION_INTRODUCER == in_separator)
		{
			(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
				&in_label, sizeof(in_label));
		}
	}

	read_GraphicRendering_Block(in_out_pSetjmpStreamState, 
		in_byteStreamReadInterface, in_separator, in_label, 
		in_is89a, in_cpLogicalScreen, in_pfErrorHandler);
}

void read_GraphicRendering_Block(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	uint8_t in_separator, uint8_t in_label, bool in_is89a, 
	const Logical_Screen *in_cpLogicalScreen, 
	void (*in_pfErrorHandler)(void *))
{
	if (IMAGE_SEPARATOR == in_separator)
	{
		TableBased_Image tableBasedImage;

		read_TableBased_Image(in_out_pSetjmpStreamState, 
			in_byteStreamReadInterface, &tableBasedImage, 
			in_is89a, in_cpLogicalScreen, in_pfErrorHandler);
	}
	// Plain Text Extension
	else if (EXTENSION_INTRODUCER == in_separator)
	{
		if (PLAIN_TEXT_LABEL != in_label)
		{
			longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
				ReadResultInvalidData, in_pfErrorHandler, 
				"read_GraphicRendering_Block: the only allowed extension is Plain Text Extension");
		}

		read_Plain_Text_Extension(in_out_pSetjmpStreamState, in_byteStreamReadInterface);
	}
	else
	{
		longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, 
			"read_GraphicRendering_Block: expecting Image Descriptor or Plain Text Extension");
	}
}

void read_TableBased_Image(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	TableBased_Image *in_pTableBasedImage, bool in_is89a, 
	const Logical_Screen *in_cpLogicalScreen, 
	void (*in_pfErrorHandler)(void *))
{
	// Initialising these 2 variables is not necessary, but does not hurt...
	Rgb8Color *lpColorTable = NULL;
	uint16_t lColorTableSize = 0;

	jmp_buf freeMemoryJmpBuf;
	
	in_pTableBasedImage->imageDescriptor.Image_Separator = IMAGE_SEPARATOR;

	read_Image_Descriptor(in_out_pSetjmpStreamState, in_byteStreamReadInterface, 
		&in_pTableBasedImage->imageDescriptor, in_is89a, 
		&in_cpLogicalScreen->logicalScreenDescriptor, in_pfErrorHandler);

	// Set localColorTable into a defined state (even if we do a longjmp)
	in_pTableBasedImage->localColorTable = NULL;
	// Not really necessary at the moment...
	in_pTableBasedImage->imageData = NULL;

	/*
	* If the Local Color Table Flag is set, read local color table before
	* reading Image Data.
	*/
	if (in_pTableBasedImage->imageDescriptor.Local_Color_Table_Flag)
	{
		size_t bytesOfLocalColorTableCount = bytesOfColorTable(in_pTableBasedImage->imageDescriptor.Size_of_Local_Color_Table);

		int result;

		in_pTableBasedImage->localColorTable = (Rgb8Color*) longjmpMalloc(
			in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultAllocationFailure, bytesOfLocalColorTableCount);

		if ((result = XCHG_AND_SETJMP(*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		freeMemoryJmpBuf)) != 0)
		{
			safe_free(&in_pTableBasedImage->localColorTable);
			xchgAndLongjmp(freeMemoryJmpBuf, *in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, result);
		}

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			in_pTableBasedImage->localColorTable, bytesOfLocalColorTableCount);

		lpColorTable = in_pTableBasedImage->localColorTable;
		lColorTableSize = colorsOfColorTable(in_pTableBasedImage->imageDescriptor.Size_of_Local_Color_Table);
	}
	else if (in_cpLogicalScreen->logicalScreenDescriptor.Global_Color_Table_Flag)
	{
		lpColorTable = in_cpLogicalScreen->globalColorTable;
		lColorTableSize = colorsOfColorTable(in_cpLogicalScreen->logicalScreenDescriptor.Size_Of_Global_Color_Table);
	}
	else
	{
		longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, 
			"read_TableBased_Image: neither Global Color Table nor Local Color Table is defined");
	}

	read_Image_Data(in_out_pSetjmpStreamState, in_byteStreamReadInterface, 
		&in_pTableBasedImage->imageDescriptor, lpColorTable, lColorTableSize, 
		/*
		* GIF 89a specification section "18. Logical Screen Descriptor."
		*
		* iii) Global Color Table Flag - Flag indicating the presence of a
		* Global Color Table; if the flag is set, the Global Color Table will
		* immediately follow the Logical Screen Descriptor. This flag also
		* selects the interpretation of the Background Color Index; if the
		* flag is set, the value of the Background Color Index field should
		* be used as the table index of the background color. (This field is
		* the most significant bit of the byte.)
		*/
		in_cpLogicalScreen->logicalScreenDescriptor.Global_Color_Table_Flag, 
		in_cpLogicalScreen->logicalScreenDescriptor.Background_Color_Index, 
		in_pfErrorHandler);

	if (in_pTableBasedImage->imageDescriptor.Local_Color_Table_Flag)
	{
		safe_free(&in_pTableBasedImage->localColorTable);
		xchgJmpBuf(freeMemoryJmpBuf, *in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer);
	}
}

void read_Graphic_Control_Extension(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, void (*in_pfErrorHandler)(void *))
{
	// Precondition: we have a GIF 89a file

	Graphic_Control_Extension graphicControlExtension;
	uint8_t terminator;

	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState,
		&graphicControlExtension, sizeof(graphicControlExtension));

	longjmpIf(graphicControlExtension.Block_Size != 4, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, in_pfErrorHandler, "read_Graphic_Control_Extension: invalid Block Size");

	longjmpIf(graphicControlExtension.Reserved != 0, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, in_pfErrorHandler, "read_Graphic_Control_Extension: reserved bits not zero");

	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState,
		&terminator, sizeof(terminator));

	longjmpIf(terminator != 0x00, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, in_pfErrorHandler, "read_Graphic_Control_Extension: no terminator");
}

void read_Plain_Text_Extension(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface)
{
	longjmp(*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultNotImplemented);
}

void read_Image_Descriptor(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	Image_Descriptor* out_pImageDescriptor, bool in_is89a, 
	const Logical_Screen_Descriptor *in_cpLogicalScreenDescriptor, 
	void (*in_pfErrorHandler)(void *))
{
	// sizeof(*in_pImageDescriptor)-1 since we have already read the first byte
	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&out_pImageDescriptor->Image_Left_Position, sizeof(*out_pImageDescriptor)-1);

	if (!in_is89a)
	{
		longjmpIf(out_pImageDescriptor->Sort_Flag, 
			in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, ReadResultInvalidData, 
			in_pfErrorHandler, "read_Image_Descriptor: in GIF 87a the (GIF 89a) Sort Flag must be set to 0");
	}

	longjmpIf(out_pImageDescriptor->Reserved != 0, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, ReadResultInvalidData, 
		in_pfErrorHandler, "read_Image_Descriptor: reserved bits not null");

	longjmpIf(
		out_pImageDescriptor->Image_Left_Position >= in_cpLogicalScreenDescriptor->Logical_Screen_Width, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, in_pfErrorHandler, 
		"read_Image_Descriptor: Image Left Position >= Logical Screen Width");

	assert(out_pImageDescriptor->Image_Left_Position < in_cpLogicalScreenDescriptor->Logical_Screen_Width);
	longjmpIf(
		out_pImageDescriptor->Image_Width > 
		in_cpLogicalScreenDescriptor->Logical_Screen_Width - out_pImageDescriptor->Image_Left_Position, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, in_pfErrorHandler, 
		"read_Image_Descriptor: Image Left Position + Image Width > Logical Screen Width");

	longjmpIf(
		out_pImageDescriptor->Image_Top_Position >= in_cpLogicalScreenDescriptor->Logical_Screen_Height, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, in_pfErrorHandler, 
		"read_Image_Descriptor: Image Top Position >= Logical Screen Height");

	assert(out_pImageDescriptor->Image_Top_Position < in_cpLogicalScreenDescriptor->Logical_Screen_Height);
	longjmpIf(
		out_pImageDescriptor->Image_Height > 
		in_cpLogicalScreenDescriptor->Logical_Screen_Height - out_pImageDescriptor->Image_Top_Position, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, in_pfErrorHandler, 
		"read_Image_Descriptor: Image Top Position + Image Height > Logical Screen Height");
}

typedef struct
{
	SetjmpStreamState *pSetjmpStreamState;
	ByteStreamInterface setjmpStreamInterface;
	uint8_t Block_Size_Bytes;
	uint8_t Read_Bytes;
	bool endOfStream;
} Image_Data_StreamState;

void init_Image_Data_StreamState(Image_Data_StreamState *in_pStreamState, 
	SetjmpStreamState *in_pSetjmpStreamState, ByteStreamInterface in_setjmpStreamInterface)
{
	in_pStreamState->pSetjmpStreamState = in_pSetjmpStreamState;
	in_pStreamState->setjmpStreamInterface = in_setjmpStreamInterface;
	in_pStreamState->Block_Size_Bytes = 0;
	in_pStreamState->Read_Bytes = 0;
	in_pStreamState->endOfStream = false;
}

size_t read_Image_Data_byteStreamInterface(void *in_out_pByteStreamState, void *out_pBuffer, size_t in_count)
{
	Image_Data_StreamState *pStreamState = (Image_Data_StreamState*) in_out_pByteStreamState;

	// A limitation of this function is that we can read only one byte at a time
	assert(in_count == 1);

	if (in_count != 1)
		return 0;

	if (pStreamState->endOfStream)
		return 0;

	/*
	* If we read the current block completely, we read the size of the next block.
	* Note that after initializing this condition satisfied.
	*/
	if (pStreamState->Read_Bytes == pStreamState->Block_Size_Bytes)
	{
		(*pStreamState->setjmpStreamInterface.mpfRead)(pStreamState->pSetjmpStreamState, 
			&pStreamState->Block_Size_Bytes, sizeof(pStreamState->Block_Size_Bytes));

		// If the next block has a size of 0, the end is reached
		if (pStreamState->Block_Size_Bytes == 0)
		{
			pStreamState->endOfStream = true;
			return 0;
		}

		pStreamState->Read_Bytes = 0;
	}

	(*pStreamState->setjmpStreamInterface.mpfRead)(pStreamState->pSetjmpStreamState, 
		out_pBuffer, 1);

	pStreamState->Read_Bytes++;

	return 1;
}

void read_Image_Data(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, 
	const Image_Descriptor *in_cpImageDescriptor, 
	const Rgb8Color *in_pColorTable, 
	uint16_t in_colorTableSize, 
	bool in_hasBackgroundColor, 
	uint8_t in_backgroundColorIndex, void (*in_pfErrorHandler)(void *))
{
	uint8_t LZW_Minimum_Code_Size;
	BitReadState bitReadState;
	ByteStreamInterface bitReadInterface;

	Image_Data_StreamState streamState;
	LZW_Decoder *pLZW_Decoder = NULL;
	jmp_buf freeMemoryJmpBuf;
	int result;

	uint32_t pixelsWritten = 0;
	uint32_t pixelsOfImageCount = ((uint32_t) in_cpImageDescriptor->Image_Width) * 
		((uint32_t) in_cpImageDescriptor->Image_Height);

	assert(in_pColorTable != NULL);

	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&LZW_Minimum_Code_Size, sizeof(LZW_Minimum_Code_Size));

	if (LZW_Minimum_Code_Size < 2 || LZW_Minimum_Code_Size > 8)
	{
		longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, 
			"read_Image_Data: invalid LZW minimum code size");
	}

	// Initialize bitReadInterface
	memset(&bitReadInterface, 0, sizeof(bitReadInterface));
	bitReadInterface.mpfRead = read_Image_Data_byteStreamInterface;

	initBitReadState(&bitReadState, &streamState, bitReadInterface);
	init_Image_Data_StreamState(&streamState, in_out_pSetjmpStreamState, in_byteStreamReadInterface);

	if ((result = XCHG_AND_SETJMP(*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		freeMemoryJmpBuf)) != 0)
	{
		if (pLZW_Decoder)
			safe_free(&pLZW_Decoder);
		
		xchgAndLongjmp(freeMemoryJmpBuf, *in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, result);
	}

	pLZW_Decoder = (LZW_Decoder *) longjmpMalloc(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultAllocationFailure, sizeof(LZW_Decoder));

	initLZW_Decoder(pLZW_Decoder, LZW_Minimum_Code_Size);
	
	while (1)
	{
		/*
		* readCount is simply for storing how many elements (bits/bytes) we have
		* read (to compare with the count that we want(ed) to read).
		*/
		size_t readCount;
		LZW_DecoderAction decoderAction;
		uint16_t currentCodeword;
		
		/*
		 * Q: Why is it necessary to set currentCodeWord to 0?
		 * A: Since the code word can have 8 or less bits, the higher byte would
		 *    not be initialized correctly otherwise.
		 *
		 *    To understand this, imagine that we wouldn't set it to 0 (i. e. the
		 *    upper byte has some arbitrary value) and read 8 or less bits.
		 *    readBitsLittleEndian will set the correct value for the lower byte (since this
		 *    is what it is supposed to do). But it will not change the value
		 *    of the upper byte, meaning we get a wrong code word.
		 */
		currentCodeword = 0;

		readCount = readBitsLittleEndian(&bitReadState, &currentCodeword, 
			LZW_Decoder_getCurrentCodeWordBitCount(pLZW_Decoder));

		/*
		* Q: Why can't we assert that readCount == currentCodeWordBitCount, 
		*    because otherwise we'd do a longjmp?
		*
		* A: If the Image Data is terminated, but we read more bits than 
		*    available, no longjmp is triggered.
		*    TODO: Insert reason
		*/
		if (readCount != LZW_Decoder_getCurrentCodeWordBitCount(pLZW_Decoder))
		{
			longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer,
				ReadResultPrematureEndOfStream, in_pfErrorHandler, 
				"read_Image_Data: readCount != currentCodeWordBitCount");
		}

		decoderAction = LZW_Decoder_handleCodeword(pLZW_Decoder, currentCodeword, 
			in_colorTableSize);

		if (LZW_DecoderAction_DoNothing == decoderAction)
			continue;
		else if (LZW_DecoderAction_Terminate == decoderAction)
			break;
		else if (LZW_DecoderAction_Error_CodewordGeqCurrentTableIndex == decoderAction)
		{
			longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
				ReadResultInvalidData, in_pfErrorHandler, 
				"read_Image_Data: currentCodeWord >= currentTableIndex");
		}
		else if (LZW_DecoderAction_Error_CodewordGeqColorTableSize == decoderAction)
		{
			longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
				ReadResultInvalidData, in_pfErrorHandler, 
				"read_Image_Data: current pixel index is out of the bounds of the currently active color table");
		}
		else
		{
			uint8_t currentPaletteIndex;

			assert(LZW_DecoderAction_DataAvailable == decoderAction);

			while (LZW_Decoder_popPaletteIndex(pLZW_Decoder, &currentPaletteIndex))
			{
				Rgba8Color rgbaColor;
				
				// TODO: Get color of palette index and write pixel

				// CND:GIF_500
				if (pixelsWritten == pixelsOfImageCount)
				{
					longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
						ReadResultInvalidData, in_pfErrorHandler, 
						"read_Image_Data: more pixels than defined in Image Descriptor");
				}

#if 0
				printf("%u ", currentPaletteIndex);
#endif

				// Follows from CND:LZW_90
				assert(currentPaletteIndex < in_colorTableSize);

				/*
				* Otherwise write pixel.
				* TODO: write pixel
				*/
				rgbaColor.rgb = in_pColorTable[currentPaletteIndex];
				rgbaColor.a = (in_hasBackgroundColor && 
					currentPaletteIndex == in_backgroundColorIndex) ? 0 : 255;

				pixelsWritten++;
			}
		}
	}

	/*
	* To quote from GIF 89a specification:
	* "An End of Information code is defined that explicitly indicates the end of
	" the image data stream. LZW processing terminates when this code is encountered.
	* It must be the last code output by the encoder for an image [!]. The value of this
	* code is <Clear code>+1."
	*/
	if (streamState.Read_Bytes != streamState.Block_Size_Bytes)
	{
		longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, ReadResultInvalidData, 
			in_pfErrorHandler, "read_Image_Data: data after End of Information code");
	}

	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&streamState.Block_Size_Bytes, sizeof(streamState.Block_Size_Bytes));

	// If there is no terminator block return failure
	longjmpIf(0 != streamState.Block_Size_Bytes, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, ReadResultInvalidData, 
		in_pfErrorHandler, "read_Image_Data: no terminator block");

	/*
	* Q: Why does this assertion hold?
	* A: the case pixelsWritten > pixelsOfImageCount is caught at CND:GIF_500.
	*/
	assert(pixelsWritten <= pixelsOfImageCount);

	if (pixelsWritten < pixelsOfImageCount)
	{
		longjmpWithHandler(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, 
			"read_Image_Data: less pixels than defined in Image Descriptor");
	}

#if 0
	printf("Pixels written: %u\n", pixelsWritten);
#endif

	safe_free(&pLZW_Decoder);
	xchgJmpBuf(freeMemoryJmpBuf, *in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer);
}

void read_Application_Extension(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface, void (*in_pfErrorHandler)(void *))
{
	// Precondition: we have a GIF 89a file
	Application_Extension applExt;

	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&applExt, sizeof(applExt));

	longjmpIf(applExt.Block_Size != 11, in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, in_pfErrorHandler, "read_Application_Extension: Block Size has to be 11");

	if (strncmp(applExt.Application_Identifier, "NETSCAPE", 8) == 0 && 
		strncmp(applExt.Application_Authentication_Code, "2.0", 3) == 0)
	{
		uint8_t blockSize;
		uint8_t blockData[3];

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			&blockSize, sizeof(blockSize));

		longjmpIf(blockSize != 3, in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, 
			"read_Application_Extension: Block Size of NETSCAPE 2.0 Application Extension has to be 3");

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			blockData, sizeof(blockData));

		// TODO: Interprete read data

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			&blockSize, sizeof(blockSize));

		longjmpIf(blockSize != 0, in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultInvalidData, in_pfErrorHandler, 
			"read_Application_Extension: expecting terminator block after NETSCAPE 2.0 Application Extension");
	}
	else if (strncmp(applExt.Application_Identifier, "XMP Data", 8) == 0 && 
		strncmp(applExt.Application_Authentication_Code, "XMP", 3) == 0)
	{
		// TODO: Implement properly
		skipBlock(in_out_pSetjmpStreamState, in_byteStreamReadInterface);
	}
	else
	{
		longjmp(*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, ReadResultNotImplemented);
	}
}

void read_Comment_Extension(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface)
{
	Data_SubBlock subBlock;

	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&subBlock.Block_Size, sizeof(subBlock.Block_Size));

	while (subBlock.Block_Size != 0)
	{
		int result;
		jmp_buf freeMemoryJmpBuf;

		subBlock.Data_Values = (uint8_t*) longjmpMalloc(
			in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			ReadResultAllocationFailure, subBlock.Block_Size);

		if ((result = XCHG_AND_SETJMP(*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
			freeMemoryJmpBuf)) != 0)
		{
			safe_free(&subBlock.Data_Values);
			xchgAndLongjmp(freeMemoryJmpBuf, 
				*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, result);
		}

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			subBlock.Data_Values, subBlock.Block_Size);

		// TODO: Interprete read block

		safe_free(&subBlock.Data_Values);
		xchgJmpBuf(freeMemoryJmpBuf, 
			*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer);

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			&subBlock.Block_Size, sizeof(subBlock.Block_Size));
	}
}

void skipBlock(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_byteStreamReadInterface)
{
	uint8_t Block_Size;

	(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&Block_Size, sizeof(Block_Size));

	while (Block_Size != 0)
	{
		size_t idx;
		uint8_t buffer[1];

		for (idx = 0; idx < Block_Size; idx++)
		{
			(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
				buffer, sizeof(buffer));
		}

		(*in_byteStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			&Block_Size, sizeof(Block_Size));
	}
}
