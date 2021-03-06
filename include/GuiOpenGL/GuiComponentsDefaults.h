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

#ifndef _GuiComponentsDefaults_h
#define _GuiComponentsDefaults_h

#include "GraphicsDataStructures/Color.h"

namespace Gui
{
	namespace Components
	{
		namespace Defaults
		{
			const float cColorByteToFloat = 1.0f/255.0f;

			const Color4<float> cColorWhite = {1.0f, 1.0f, 1.0f, 1.0f}; // (255, 255, 255)
			const Color4<float> cColorRed = {1.0f, 0.0f, 0.0f, 1.0f}; // (255, 255, 255)

			const Color4<float> cWindowBackgroundColor = {
				236*cColorByteToFloat, 
				233*cColorByteToFloat, 
				216*cColorByteToFloat, 1.0f
			};

			const Color4<float> cWindowTitleBarColor = {
				0*cColorByteToFloat,
				84*cColorByteToFloat,
				227*cColorByteToFloat, 1.0f
			};

			const Color4<float> cCheckBoxTopLeftBackgroundColor = {
				220*cColorByteToFloat, 
				220*cColorByteToFloat, 
				215*cColorByteToFloat, 1.0f
			};
			const Color4<float> cCheckBoxTopRightBackgroundColor = {
				241*cColorByteToFloat, 
				241*cColorByteToFloat, 
				239*cColorByteToFloat, 1.0f
			};
			const Color4<float> cCheckBoxBottomLeftBackgroundColor = cCheckBoxTopRightBackgroundColor;
			const Color4<float> cCheckBoxBottomRightBackgroundColor = cColorWhite;
			const Color4<float> cCheckBoxBorderColor = {
				28*cColorByteToFloat, 
				81*cColorByteToFloat, 
				128*cColorByteToFloat, 1.0f
			};
			const Color4<float> cCheckBoxHookColor = {
				33*cColorByteToFloat, 
				161*cColorByteToFloat, 
				33*cColorByteToFloat, 1.0f
			};


			const Color4<float> cTextBoxBackgroundColor = cColorWhite;
			const Color4<float> cTextBoxBorderColor = {
				127*cColorByteToFloat, 
				157*cColorByteToFloat, 
				185*cColorByteToFloat, 1.0f
			};


			// The width and height of the checkbox (without border)
			const unsigned int cCheckBoxWidth = 11;
			const unsigned int cCheckBoxHeight = 11;
			const unsigned int cCheckBoxBorderSize = 1;
			const unsigned int cCheckBoxTextDistance = 7;

			// all the size is 13 - and it gets clipped to this size
			const unsigned int cRadioButtonWidth = 11;
			const unsigned int cRadioButtonHeight = 11;
			const unsigned int cRadioButtonTextDistance = 7;

			// The height of the textbox (without border)
			const unsigned int cTextBoxHeight = 18;
			const unsigned int cTextBoxBorderSize = 1;
		}
	}
}

#endif
