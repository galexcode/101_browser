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

#include "TestSuite/Tests.h"
#include "TestSuite/TestSuite.h"
#include "GIF/GIF.h"

void testGIF()
{
	// We want to be sure...
	test(sizeof(Logical_Screen_Descriptor) == 7);
	test(sizeof(Graphic_Control_Extension) == 5);
	test(sizeof(Image_Descriptor) == 10);

	{
		char* filenames[] = {
			"testfiles/gif/wikipedia/GifSampleSmall.gif",
			"testfiles/gif/wikipedia/GifSample.gif",
			/*
			* Provides test case for Application Extension block
			*/
			"testfiles/gif/wikipedia/Newtons_cradle_animation_book_2.gif",
			/*
			* Provides test case for Application Extension block
			*/
			"testfiles/gif/wikipedia/Rotating_earth_(large).gif",
			/*
			* Provides test case for Application Extension block
			*/
			"testfiles/gif/wikipedia/Rotating_earth_(small).gif",
			/*
			* Provides test case for Comment Extension block
			*/
			"testfiles/gif/wikipedia/Sunflower_as_gif_small.gif",

			"testfiles/gif/a_image.gif",
			"testfiles/gif/tc217.gif",

			"testfiles/gif/fileformat.info/GMARBLES.GIF",
			"testfiles/gif/fileformat.info/MARBLES.GIF",
			"testfiles/gif/fileformat.info/WFPC01.GIF",
			"testfiles/gif/fileformat.info/WFPC02.GIF",
			"testfiles/gif/fileformat.info/WFPC03.GIF",
			"testfiles/gif/fileformat.info/WFPC04.GIF",
			"testfiles/gif/fileformat.info/WFPC05.GIF",
			"testfiles/gif/fileformat.info/WFPC06.GIF"
		};

		char* filenames2[] = {
			/*
			* Some block beginning with 0x00.
			*/
			//"testfiles/imagetestsuite/gif/0646caeb9b9161c777f117007921a687.gif", // -> ReadResultInvalidData
			/*
			* Invalid since the block size of the Graphic Control Extension
			* block is 0xEE instead of 0x04.
			*/
			//"testfiles/imagetestsuite/gif/243d9798466d64aba0acaa41f980bea6.gif", // -> ReadResultInvalidData
			/*
			* Invalid since minimum code size = 0x48 (instead of between 2 and 8)
			*/
			//"testfiles/imagetestsuite/gif/2b5bc31d84703bfb9f371925f0e3e57d.gif", // -> ReadResultInvalidData
			//"testfiles/imagetestsuite/gif/55abb3cc464305dd554171c3d44cb61f.gif", // -> ReadResultPrematureEndOfStream
			/*
			* Invalid since minimum code size = 0x48 (instead of between 2 and 8)
			*/
			//"testfiles/imagetestsuite/gif/5f09a896c191db3fa7ea6bdd5ebe9485.gif", // -> ReadResultInvalidData
			//"testfiles/imagetestsuite/gif/6d939393058de0579fca1bbf10ecff25.gif", // -> ReadResultPrematureEndOfStream
			/*
			* Invalid since the block size of the Graphic Control Extension
			* block is 0xEE instead of 0x04.
			*/
			//"testfiles/imagetestsuite/gif/7092f253998c1b6b869707ad7ae92854.gif", // -> ReadResultInvalidData
			//"testfiles/imagetestsuite/gif/9f8f6046eaf9ffa2d9c5d6db05c5f881.gif", // -> ReadResultPrematureEndOfStream
			//"testfiles/imagetestsuite/gif/adaf0da1764aafb7039440dbe098569b.gif", // -> ReadResultPrematureEndOfStream
			/*
			* Invalid since there is a code word currentCodeWord where (currentCodeWord >= currentTableIndex)
			*/
			//"testfiles/imagetestsuite/gif/adf6f850b13dff73ebb22862c6ab028b.gif", // -> ReadResultInvalidData
			//"testfiles/imagetestsuite/gif/bc7af0616c4ae99144c8600e7b39beea.gif", // -> ReadResultNotImplemented (!!!)
			/*
			* Invalid since minimum code size = 0x0 (instead of between 2 and 8)
			*/
			//"testfiles/imagetestsuite/gif/ce774930ac70449f38a18789c70095b8.gif", // -> ReadResultInvalidData
			/*
			* Signature 0x00 0x49 0x46 instead of 0x47 0x49 0x46 (if we correct 
			* it, we still have an invalid file since
			* minimum code size = 0x48 (instead of between 2 and 8))
			*/
			//"testfiles/imagetestsuite/gif/d5a0175c07418852152ef33a886a5029.gif", // -> ReadResultInvalidData
			//"testfiles/imagetestsuite/gif/e34116d68f49c7852b362ec72a636df5.gif", // -> ReadResultPrematureEndOfStream
			"testfiles/imagetestsuite/gif/e6aa0c45a13dd7fc94f7b5451bd89bf4.gif", // -> ReadResultOK
			//"testfiles/imagetestsuite/gif/ea754e040929b7f9c157efc88c4d0eaf.gif", // -> ReadResultPrematureEndOfStream
			//"testfiles/imagetestsuite/gif/ee6d1133f9264dc6467990e53d0bf104.gif", // -> ReadResultInvalidVersion (!!!) -> this is a bug
			//"testfiles/imagetestsuite/gif/f617c7af7f36296a37ddb419b828099c.gif", // -> ReadResultNotImplemented (!!!)
			"testfiles/imagetestsuite/gif/f88b6907ee086c4c8ac4b8c395748c49.gif", // -> ReadResultOK
			"testfiles/imagetestsuite/gif/fc3e2b992c559055267e26dc23e484c0.gif"  // -> ReadResultOK
		};

		FILE* imgFile;
		size_t filenameIdx;

		for (filenameIdx = 0; filenameIdx < sizeof(filenames)/sizeof(char*); filenameIdx++)
		{
			GIF_Data_Stream dataStream;
			ReadResult readResult;

			imgFile = MTAx_fopen(filenames[filenameIdx], "rb");

			test(imgFile != NULL);

			if (imgFile != NULL)
			{
				readResult = read_GIF_Data_Stream(imgFile, &dataStream);
				test(ReadResultOK == readResult);

				fclose(imgFile);
			}
		}

		for (filenameIdx = 0; filenameIdx < sizeof(filenames2)/sizeof(char*); filenameIdx++)
		{
			GIF_Data_Stream dataStream;
			ReadResult readResult;

			imgFile = MTAx_fopen(filenames2[filenameIdx], "rb");

			test(imgFile != NULL);

			if (imgFile != NULL)
			{
				readResult = read_GIF_Data_Stream(imgFile, &dataStream);
				test(ReadResultOK == readResult);

				fclose(imgFile);
			}
		}
	}
}
