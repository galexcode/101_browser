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

#ifndef _SwfStructure_h
#define _SwfStructure_h

#include "BasicDataStructures/Stream/BasicReadStream.h"
#include <cstdlib>
#include <list>

struct SwfRECT
{
	unsigned char NBits; // NBits UB[5]	Bits in each rect value field
	// signed int is the size required for SB[UB[5]]
#pragma pack(push, 1)
	signed int Xmin; // Xmin SB[Nbits] x minimum position for rect
	signed int Xmax; // Xmax SB[Nbits] x maximum position for rect
	signed int Ymin; // Ymin SB[Nbits] y minimum position for rect
	signed int Ymax; // Ymax SB[Nbits] y maximum position for rect
#pragma pack(pop)

	inline SwfRECT() : Xmin(0), Xmax(0), Ymin(0), Ymax(0) { }
};

/*!
 * Reads a RECT.
 *
 * Return value: 
 * true on success
 * false on failure
 */
bool readSwfRECT(BasicReadStream* in_pStream, SwfRECT* in_pRect);

struct SwfHeader1
{
	char Signature[3];
	unsigned char Version;
	unsigned int FileLength;
};

struct SwfHeader2
{
	SwfRECT FrameSize;
#pragma pack(push, 1)
	unsigned short FrameRate;
	unsigned short FrameCount;
#pragma pack(pop)
};

struct SwfRECORDHEADER
{
	unsigned short TagType; // 10 Bits
	signed int Length;      // 6 bits or SI32

	SwfRECORDHEADER() : TagType(0), Length(0) { }
};

struct SwfTag
{
	SwfRECORDHEADER recordHeader;
	unsigned char* tagContent;

	inline SwfTag() : tagContent(NULL) { }
};

struct SwfFile
{
	SwfHeader1 swfHeader1;
	SwfHeader2 swfHeader2;
	std::list<SwfTag> tags;
};

#endif
