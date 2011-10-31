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

#include "JpegDecoder/JpegSegmentsRead.h"
#include "MiniStdlib/memory.h"  // for ENDIANNESS_CONVERT_SIMPLE
#include "MiniStdlib/safe_free.h"
#include "SetjmpUtil/ConditionalLongjmp.h"
#include <assert.h>

void readFrameHeader(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_setjmpStreamReadInterface, 
	FrameHeader* in_pFrameHeader, uint8_t in_SOF_n)
{
	jmp_buf allocFailureJmpBuf;
	int result;
	uint8_t idx;
	
	in_pFrameHeader->SOF_n = in_SOF_n;
	
	(*in_setjmpStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&in_pFrameHeader->Lf, 
		offsetof(FrameHeader, pFrameComponentSpecificationParameters) - 
		offsetof(FrameHeader, Lf));

	ENDIANNESS_CONVERT_SIMPLE(in_pFrameHeader->Lf);
	ENDIANNESS_CONVERT_SIMPLE(in_pFrameHeader->Y);
	ENDIANNESS_CONVERT_SIMPLE(in_pFrameHeader->X);

	longjmpIf(in_pFrameHeader->Lf != 8 + 3*in_pFrameHeader->Nf, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, printHandler, 
		"readFrameHeader: Lf != 8 + 3*Nf");

	in_pFrameHeader->pFrameComponentSpecificationParameters = 
		(FrameComponentSpecificationParameter *)
		longjmpMalloc(in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultAllocationFailure, 
		in_pFrameHeader->Nf * sizeof(FrameComponentSpecificationParameter));

	if ((result = setjmp(allocFailureJmpBuf)) != 0)
	{
		safe_free(&in_pFrameHeader->pFrameComponentSpecificationParameters);
		longjmp(*in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, result);
	}

	(*in_setjmpStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		in_pFrameHeader->pFrameComponentSpecificationParameters, 
		in_pFrameHeader->Nf * sizeof(FrameComponentSpecificationParameter));

	printf("Lf = %u\tP = %u\tY = %u\tX=%u\tNf = %u\n", 
		in_pFrameHeader->Lf, in_pFrameHeader->P, in_pFrameHeader->Y, 
		in_pFrameHeader->X, in_pFrameHeader->Nf);

	for (idx = 0; idx < in_pFrameHeader->Nf; idx++)
	{
		printf("C_%u = %u\tH_%u = %u\tV_%u = %u\tTq_%u = %u\n", 
			idx+1, in_pFrameHeader->pFrameComponentSpecificationParameters[idx].C, 
			idx+1, in_pFrameHeader->pFrameComponentSpecificationParameters[idx].H, 
			idx+1, in_pFrameHeader->pFrameComponentSpecificationParameters[idx].V, 
			idx+1, in_pFrameHeader->pFrameComponentSpecificationParameters[idx].Tq);
	}
}

void readScanHeader(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_setjmpStreamReadInterface, 
	ScanHeader* in_pScanHeader)
{
	SetjmpState invalidDataSetjmpState;
	SetjmpState allocationFailureSetjmpState;

	uint8_t i;

	(*in_setjmpStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&in_pScanHeader->Ls, sizeof(in_pScanHeader->Ls)+sizeof(in_pScanHeader->Ns));

	ENDIANNESS_CONVERT_SIMPLE(in_pScanHeader->Ls);

	setjmpStateInit(&invalidDataSetjmpState, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, printHandler);

	setjmpStateLongjmpIf(&invalidDataSetjmpState, 
		in_pScanHeader->Ls != 6+2*in_pScanHeader->Ns, 
		"readScanHeader: Ls must be 6+2*Ns");
	// See table B.3
	// CND:JpegSegmentsRead_c_70
	setjmpStateLongjmpIf(&invalidDataSetjmpState, 
		in_pScanHeader->Ns == 0 || in_pScanHeader->Ns>4, 
		"readScanHeader: invalid value of Ns");

	printf("Ls = %u\tNs = %u\n", in_pScanHeader->Ls, in_pScanHeader->Ns);

	setjmpStateInit(&allocationFailureSetjmpState, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultAllocationFailure, NULL);

	// follows from CND:JpegSegmentsRead_c_70
	assert(in_pScanHeader->Ns <= 4);

	// Is initialized above

	for (i=0; i<in_pScanHeader->Ns; i++)
	{
		(*in_setjmpStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
			in_pScanHeader->componentSpecificationParameters+i, 
			sizeof(ScanComponentSpecificationParameter));
		
		printf("Cs%u = %2X\t", i+1, in_pScanHeader->componentSpecificationParameters[i].Cs);
		printf("Td%u = %1X\t", i+1, in_pScanHeader->componentSpecificationParameters[i].Td);
		printf("Ta%u = %1X\n", i+1, in_pScanHeader->componentSpecificationParameters[i].Ta);
	}

	(*in_setjmpStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&in_pScanHeader->Ss, sizeof(in_pScanHeader->Ss));

	printf("Ss = %2X\tSe = %2X\tAh = %1X\tAl = %1X\n", 
		in_pScanHeader->Ss, in_pScanHeader->Se, in_pScanHeader->Ah, in_pScanHeader->Al);
}

void readRestartInterval(SetjmpStreamState *in_out_pSetjmpStreamState, 
	ByteStreamInterface in_setjmpStreamReadInterface, 
	RestartIntervalState* in_pRestartIntervalState)
{
	SetjmpState invalidDataSetjmpState;
	
	(*in_setjmpStreamReadInterface.mpfRead)(in_out_pSetjmpStreamState, 
		&in_pRestartIntervalState->restartInterval, sizeof(in_pRestartIntervalState->restartInterval));

	in_pRestartIntervalState->isRestartIntervalInitialized = true;

	ENDIANNESS_CONVERT_SIMPLE(in_pRestartIntervalState->restartInterval.Lr);
	ENDIANNESS_CONVERT_SIMPLE(in_pRestartIntervalState->restartInterval.Ri);

	printf("Lr = %u\tRi = %u\n", 
		in_pRestartIntervalState->restartInterval.Lr, 
		in_pRestartIntervalState->restartInterval.Ri);

	setjmpStateInit(&invalidDataSetjmpState, 
		in_out_pSetjmpStreamState->setjmpState.mpJmpBuffer, 
		ReadResultInvalidData, printHandler);

	setjmpStateLongjmpIf(&invalidDataSetjmpState, 
		in_pRestartIntervalState->restartInterval.Lr != 4, 
		"Expected size 4 of DRI segment");
}
