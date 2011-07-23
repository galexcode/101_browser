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

#ifndef _GIF_h
#define _GIF_h

#include "Util/ReadResult.h"
#include "MiniStdlib/MTAx_cstdio.h"
#include "MiniStdlib/cstdint.h"
#include "MiniStdlib/cstdbool.h"
#include "MiniStdlib/declspec.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4214)
#endif
typedef struct
{
	uint16_t Logical_Screen_Width;
	uint16_t Logical_Screen_Height;
	uint8_t Size_Of_Global_Color_Table : 3;
	uint8_t Sort_Flag : 1;
	uint8_t Color_Resolution : 3;
	uint8_t Global_Color_Table_Flag : 1;
	uint8_t Background_Color_Index;
	uint8_t Pixel_Aspect_Ratio;
} Logical_Screen_Descriptor;

typedef struct
{
	uint8_t Image_Separator;
	uint16_t Image_Left_Position;
	uint16_t Image_Top_Position;
	uint16_t Image_Width;
	uint16_t Image_Height;
	uint8_t Size_of_Local_Color_Table : 3;
	uint8_t Reserved : 2;
	uint8_t Sort_Flag : 1;
	uint8_t Interlace_Flag : 1;
	uint8_t Local_Color_Table_Flag : 1;
} Image_Descriptor;

typedef struct
{
	uint8_t Block_Size;
	uint8_t Transparent_Color_Flag : 1;
	uint8_t User_Input_Flag : 1;
	uint8_t Disposal_Method : 3;
	uint8_t Reserved : 3;
	uint16_t Delay_Time;
	uint8_t Transparent_Color_Index;
} Graphic_Control_Extension;

#ifdef _WIN32
#pragma warning(pop)
#endif

typedef struct
{
	Logical_Screen_Descriptor logicalScreenDescriptor;
	uint8_t *globalColorTable;
} Logical_Screen;

typedef struct
{
	char Signature[3];
	char Version[3];
} Header;

typedef struct
{
	Header header;
	Logical_Screen logicalScreen;
} GIF_Data_Stream;

typedef struct
{
	uint8_t Block_Size;
	uint8_t *Data_Values;
} Data_SubBlock;

typedef struct
{
	uint8_t Block_Size;
	char Application_Identifier[8];
	char Application_Authentication_Code[3];
} Application_Extension;

typedef struct
{
	Image_Descriptor imageDescriptor;
	uint8_t *localColorTable;
	uint8_t *imageData;
} TableBased_Image;

#pragma pack(pop)

// Helper functions
size_t bytesOfColorTable(unsigned char in_sizeOfColorTable);

// Grammar words
DLLEXPORT ReadResult read_GIF_Data_Stream(FILE* in_gifFile, GIF_Data_Stream *in_pDataStream);
ReadResult read_Logical_Screen(FILE* in_gifFile, Logical_Screen *in_pLogicalScreen);
ReadResult read_Data(FILE* in_gifFile, uint8_t in_introducer, bool in_is89a);
/*
* Preconditions:
* PRE:GIF_h_126: (0x21 == in_separator) || (0x2C == in_separator)
* PRE:GIF_h_127: (0x21 == in_separator) => ((in_label == 0xF9) || (in_label == 0x01))
* PRE:GIF_h_128: (0x21 == in_separator) => we have a GIF 89a file
*/
ReadResult read_Graphic_Block(FILE* in_gifFile, uint8_t in_separator, uint8_t in_label);
ReadResult read_GraphicRendering_Block(FILE* in_gifFile, uint8_t in_separator);
ReadResult read_TableBased_Image(FILE* in_gifFile, TableBased_Image *in_pTableBasedImage);
/*
* Precondition:
* PRE:GIF_h_129: we have a GIF 89a file
*/
ReadResult read_SpecialPurpose_Block(FILE* in_gifFile, uint8_t in_label);

// Terminal symbols
ReadResult read_Header(FILE* in_gifFile, Header *in_pHeader, bool *out_pIs89a);
ReadResult read_Trailer(FILE* in_gifFile);
ReadResult read_Logical_Screen_Descriptor(FILE* in_gifFile);
#if 0
ReadResult read_Global_Color_Table(FILE* in_gifFile);
#endif
ReadResult read_Graphic_Control_Extension(FILE* in_gifFile);
#if 0
ReadResult read_Plain_Text_Extension(FILE* in_gifFile);
#endif
ReadResult read_Image_Descriptor(FILE* in_gifFile, Image_Descriptor* in_pImageDescriptor);
#if 0
ReadResult read_Local_Color_Table(FILE* in_gifFile);
#endif
ReadResult read_Image_Data(FILE* in_gifFile);
/*
* Precondition:
* PRE:GIF_h_148: we have a GIF 89a file
*/
ReadResult read_Application_Extension(FILE* in_gifFile);
/*
* Precondition:
* PRE:GIF_h_158: we have a GIF 89a file
*/
ReadResult read_Comment_Extension(FILE* in_gifFile);

ReadResult skipBlock(FILE* in_gifFile);

#ifdef __cplusplus
}
#endif

#endif
