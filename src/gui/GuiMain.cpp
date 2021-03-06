/*
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

#include <windows.h>
#include <string>
#include <cassert>
#include "MiniStdlib/MTAx_cstdio.h"
#include <tchar.h>
#include "GuiOpenGL/GuiComponentsDefaults.h"
#include "GuiOpenGL/GuiOpenGLState.h"
#include "gui/MultiMouse.h"
#include "gui/Cursor.h"
#include "BasicDataStructures/Error/ErrorHandling.h"
#include "MiniStdlib/safe_free.h"
#include "Template/Memory/SafeMemoryManagement.h"

struct Window
{
	HINSTANCE	hInstance;		// Holds The Instance Of The Application
	std::wstring windowClassName;
	HWND		hWnd;		    // Holds Our Window Handle
	HDC			hDC;		    // Private GDI Device Context
	HGLRC		hGLRC;		    // Permanent Rendering Context
	ArrayBlock<Gui::Mouse::RawMouse> rawMice;
	UINT width;
	UINT height;

	Window(HINSTANCE in_hInstance, UINT in_width, UINT in_height) 
		: hInstance(in_hInstance), hWnd(0), hDC(0), hGLRC(0),
		width(in_width), height(in_height)
	{ }
};

void toggleMultiMouse(bool *multipleMice, HWND hWnd, ArrayBlock<Gui::Mouse::RawMouse>* pRawMice);

bool runProgram = true;
FILE* logFile = NULL;
Gui::Cursor cursor;
bool gMultipleMice = false;

void handleDraw(Window* window)
{
	drawGui(gMultipleMice ? &window->rawMice : NULL);
	SwapBuffers(window->hDC);
	ValidateRect(window->hWnd, NULL);
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window* window = (Window*) GetWindowLongPtr(hWnd, GWL_USERDATA);

	fprintf(logFile, "%u\treceived %x\n", GetTickCount(), uMsg);
	fflush(logFile);

	switch (uMsg)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);
			window = (Window*)(creation->lpCreateParams);
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR) window);
		}
		return 0;
	case WM_CLOSE:
		runProgram = false;
		return 0;
	case WM_SIZE:							// Size Action Has Taken Place
		switch (wParam)						// Evaluate Size Action
		{
			case SIZE_MINIMIZED:
				return 0;
			case SIZE_RESTORED:
			case SIZE_MAXIMIZED:
				window->width = LOWORD(lParam);
				window->height = HIWORD(lParam);

				for (size_t currentMouseIndex = 0; currentMouseIndex < window->rawMice.count(); 
					currentMouseIndex++)
				{
					if (window->rawMice.data()[currentMouseIndex].x >= window->width)
						window->rawMice.data()[currentMouseIndex].x = window->width - 1;
					if (window->rawMice.data()[currentMouseIndex].y >= window->height)
						window->rawMice.data()[currentMouseIndex].y = window->height - 1;
				}

				ReshapeGL(window->width, window->height);

				/*
				 * A WM_PAINT will be called automatically - so there is no need for
				 * InvalidateRect(window->hWnd, NULL, FALSE);
				 */
				return 0;
		}
		break;
	case WM_PAINT:
		handleDraw(window);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_F12)
		{
			toggleMultiMouse(&gMultipleMice, window->hWnd, &window->rawMice);
			InvalidateRect(window->hWnd, NULL, FALSE);
		}
		return 0;

	case WM_INPUT:
		{
			UINT cbSize;
			if (GetRawInputData((HRAWINPUT) lParam, RID_INPUT, 
				NULL, &cbSize, sizeof(RAWINPUTHEADER)) != 0)
			{
				showErrorMessageAndExit(L"GetRawInputData() count");
			}

			RAWINPUT *pRawInput = static_cast<RAWINPUT *>(malloc(cbSize));

			if (pRawInput == NULL)
			{
				showErrorMessageAndExit(L"malloc");
				/* 
				 * Of course the program flow ends here - but the code analyser
				 * of Visual Studio doesn't get it. :-(
				 */
				return 0;
			}

			if (GetRawInputData((HRAWINPUT) lParam, RID_INPUT, 
				pRawInput, &cbSize, sizeof(RAWINPUTHEADER)) != cbSize)
			{
				safe_free(&pRawInput);
				showErrorMessageAndExit(L"GetRawInputData() get");
			}

			for (size_t currentMouseIndex = 0; currentMouseIndex < window->rawMice.count();
				currentMouseIndex++)
			{
				if (window->rawMice.data()[currentMouseIndex].deviceHandle == 
					pRawInput->header.hDevice)
				{
					if (pRawInput->data.mouse.lLastX < 0 && 
						((ULONG) -pRawInput->data.mouse.lLastX) > window->rawMice.data()[currentMouseIndex].x)
						window->rawMice.data()[currentMouseIndex].x = 0;
					else
						window->rawMice.data()[currentMouseIndex].x += pRawInput->data.mouse.lLastX;

					if (pRawInput->data.mouse.lLastY < 0 && 
						((ULONG) -pRawInput->data.mouse.lLastY) > window->rawMice.data()[currentMouseIndex].y)
						window->rawMice.data()[currentMouseIndex].y = 0;
					else
						window->rawMice.data()[currentMouseIndex].y += pRawInput->data.mouse.lLastY;

					if (window->rawMice.data()[currentMouseIndex].x >= window->width)
						window->rawMice.data()[currentMouseIndex].x = window->width - 1;
					if (window->rawMice.data()[currentMouseIndex].y >= window->height)
						window->rawMice.data()[currentMouseIndex].y = window->height - 1;

					InvalidateRect(window->hWnd, NULL, FALSE);
					break;
				}
			}

			safe_free(&pRawInput);
			/*
			 * According to 
			 * http://msdn.microsoft.com/en-us/library/ms645590(VS.85).aspx
			 * "The application must call DefWindowProc so the system can perform the cleanup."
			 */
			DefWindowProc(hWnd, uMsg, wParam, lParam);

			return 0;
		}
	}
	
	fprintf(logFile, "Did not handle\n");
	fflush(logFile);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool registerWindowClass(const Window& in_window)
{
	WNDCLASSEX windowClass;												// Window Class
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));						// Make Sure Memory Is Cleared
	windowClass.cbSize			= sizeof(WNDCLASSEX);					// Size Of The windowClass Structure
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc		= (WNDPROC)(WndProc);					// WindowProc Handles Messages
	windowClass.hInstance		= in_window.hInstance;					// Set The Instance
	windowClass.hbrBackground	= NULL;									// Class Background Brush Color
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName	= in_window.windowClassName.c_str();	// Sets The Applications Classname
	if (!RegisterClassEx (&windowClass))								// Did Registering The Class Fail?
		return false;
	return true;														// Return True (Success)
}

bool unregisterWindowClass(const Window& in_window)
{
	if (!UnregisterClass(in_window.windowClassName.c_str(), in_window.hInstance))
		return false;
	return true;
}

void destroyWindow(Window* in_window);

bool createWindow(Window* in_window, std::wstring in_titleText, 
				  BYTE in_colorBits, BYTE in_depthBits)
{
	int			PixelFormat;											// Holds The Results After Searching For A Match
	// Previously we had dwExStyle = WS_EX_APPWINDOW;
	DWORD		dwExStyle = 0;											// Window Extended Style
	/*
	* Q: What does this style attribute mean?
	* A: WS_OVERLAPPEDWINDOW: If you use the WS_OVERLAPPED style, the window has a 
	*    title bar and border. If you use the WS_OVERLAPPEDWINDOW style, the 
	*    window has a title bar, sizing border, window menu, and minimize and 
	*    maximize buttons.
	*    (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms632599.aspx)
	*/
	DWORD		dwStyle = WS_OVERLAPPEDWINDOW;
	RECT		windowRect;												// Grabs Rectangle Upper Left / Lower Right Values
	windowRect.left=(long) 0;											// Set Left Value To 0
	windowRect.right=(long) in_window->width;							// Set Right Value To Requested Width
	windowRect.top=(long) 0;											// Set Top Value To 0
	windowRect.bottom=(long) in_window->height;							// Set Bottom Value To Requested Height

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);			// Adjust Window To True Requested Size

	in_window->hWnd = CreateWindowEx (
		dwExStyle,                                                      // Extended Style
		in_window->windowClassName.c_str(),                             // Class Name
		in_titleText.c_str(),                                           // Window Title
		dwStyle,                                                        // Window Style
		// We don't use windowRect.left and windowRect.top here
		// since this moves the window corner to places with negative screen
		// coordinates
		0,                                                              // Window X Position
		0,                                                              // Window Y Position
		windowRect.right - windowRect.left,                             // Window Width
		windowRect.bottom - windowRect.top,                             // Window Height
		HWND_DESKTOP,                                                   // Desktop Is Window's Parent
		0,                                                              // No Menu
		in_window->hInstance,                                           // Pass The Window Instance
		in_window);

	if (!in_window->hWnd)
		// Note that destroyWindow(in_window); is not necessary here
		return false;

	in_window->hDC = GetDC (in_window->hWnd);							// Grab A Device Context For This Window
	if (!in_window->hDC)												// Did We Get A Device Context?
	{
		destroyWindow(in_window);										// Destroy the window
		return false;													// Return False
	}

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		in_colorBits,													// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		0,																// No Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		in_depthBits,													// Z-Buffer (Depth Buffer) Bits
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	PixelFormat = ChoosePixelFormat (in_window->hDC, &pfd);				// Find A Compatible Pixel Format
	if (!PixelFormat)													// Did We Find A Compatible Format?
	{
		destroyWindow(in_window);										// Destroy the window
		return false;													// Return False
	}

	if (!SetPixelFormat (in_window->hDC, PixelFormat, &pfd))
	{
		destroyWindow(in_window);										// Destroy the window
		return false;													// Return False
	}

	in_window->hGLRC = wglCreateContext (in_window->hDC);				// Try To Get A Rendering Context

	if (!in_window->hGLRC)												// Did We Get A Rendering Context?
	{
		destroyWindow(in_window);										// Destroy the window
		return false;													// Return False
	}

	// Make The Rendering Context Our Current Rendering Context
	if (!wglMakeCurrent (in_window->hDC, in_window->hGLRC))
	{
		destroyWindow(in_window);										// Destroy the window
		return false;													// Return False
	}

	ReshapeGL(in_window->width, in_window->height);

	return true;
}

void showWindow(Window* in_window, int nCmdShow)
{
	ShowWindow(in_window->hWnd, nCmdShow);								// Make The Window Visible
}

void destroyWindow(Window* in_window)
{
	if (in_window->hWnd)												// Does The Window Have A Handle?
	{	
		if (in_window->hDC)												// Does The Window Have A Device Context?
		{
			wglMakeCurrent(in_window->hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (in_window->hGLRC)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext(in_window->hGLRC);						// Release The Rendering Context
				in_window->hGLRC = 0;									// Zero The Rendering Context

			}
			ReleaseDC (in_window->hWnd, in_window->hDC);				// Release The Device Context
			in_window->hDC = 0;											// Zero The Device Context
		}
		DestroyWindow (in_window->hWnd);								// Destroy The Window
		in_window->hWnd = 0;											// Zero The Window Handle
	}
}

/*!
 * toggles whether multiple mice or a single mouse is used
 * 
 * Parameters: 
 * multipleMice: set to *true* if at the moment only a single
 *               mouse is shown and you want to show multiple mice
 *               set to *false* if at the moment multiple mice are 
 *               shown and you only want to show a single one
 *
 * Note: it is important not to call the function when already the 
 *       desired state is reached
 */
void toggleMultiMouse(bool *in_pMultipleMice, HWND hWnd, ArrayBlock<Gui::Mouse::RawMouse>* pRawMice)
{
	if (in_pMultipleMice != NULL)
	{
		*in_pMultipleMice = !*in_pMultipleMice;

		if (*in_pMultipleMice)
		{
			Gui::Mouse::registerRawMice(hWnd);
			ArrayBlock<Gui::Mouse::RawMouse> block = Gui::Mouse::getRawMouseArray();
			Gui::Mouse::syncRawMousePositions(pRawMice, &block);
			Gui::Mouse::destroyRawMouseArray(pRawMice);
			*pRawMice = block;
			ShowCursor(FALSE);
		}
		else
		{
			Gui::Mouse::unregisterRawMice();
			ShowCursor(TRUE);
		}
	}
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE,	// hPrevInstance,	// Previous Instance
					LPSTR,		// lpCmdLine,		// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	logFile = MTAx_fopen("log.txt", "w+");

	if (logFile == NULL)
	{
		showErrorMessageAndExit(L"Could not open log file");
	}

	Window window(hInstance, 640, 480);
	window.windowClassName = L"101_window_class";

	if (!registerWindowClass(window))
	{
		showErrorMessageAndExit(L"RegisterClassEx failed!");
	}

	// for the 2nd time (and so on) we replace nCmdShow by SW_NORMAL (for example)
	if (!createWindow(&window, L"101 browser", 32, 32))
	{
		destroyWindow(&window);
		showErrorMessageAndExit(L"Could not create window!");
	}

	initializeOpenGLGuiState();

	if (!Gui::createCursor(&cursor))
	{
		// TODO Add some cleanup code and show error message
		exit(1);
	}

	createOpenGLTexture(&cursor.andMap);
	createOpenGLTexture(&cursor.xorColorMap);

	freeTextureMemory(&cursor.andMap);
	freeTextureMemory(&cursor.xorColorMap);

	window.rawMice = ArrayBlock<Gui::Mouse::RawMouse>();

	showWindow(&window, nCmdShow); // Alternative: use SW_NORMAL instead of nCmdShow

	while (runProgram)
	{
		MSG msg; // Windows Message Structure

		if (GetMessage(&msg, window.hWnd, 0, 0)!=0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	destroyWindow(&window);

	destroyRawMouseArray(&window.rawMice);

	if (!unregisterWindowClass(window))
	{
		showErrorMessageAndExit(L"UnregisterClass failed!");
	}

	return 0;
}
