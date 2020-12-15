/*
#
# Copyright(c) 2020 Avalanche Studios.All rights reserved.
# Licensed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE
#
*/
/**	\file	device_main.cxx
*	Library declarations.
*	Contains the basic routines to declare the DLL as a loadable
*	library for FiLMBOX.
*
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
#define _WINSOCKAPI_    // stops windows.h including winsock.h
	#include <windows.h>
#endif

#include <AnimLiveBridge/AnimLiveBridge.h>

//--- Library declaration
FBLibraryDeclare( devicebridge )
{
	FBLibraryRegister( CServerDevice		);
	FBLibraryRegister( CServerDeviceLayout	);

	FBLibraryRegister(CClientDevice);
	FBLibraryRegister(CClientDeviceLayout);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/

class CDeviceLiveBridgeLogger : public CLiveBridgeLogger
{
public:
	void LogInfo(const char* info) override
	{
		FBTrace("%s\n", info);
	}
	void LogWarning(const char* info) override
	{
		FBTrace("Warning! %s\n", info);
	}
	void LogError(const char* info) override
	{
		FBTrace("ERROR!! %s\n", info);
	}
};

static CDeviceLiveBridgeLogger		g_Logger;

bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	
{ 
	SetLiveBridgeLogger(&g_Logger);
	return true; 
}
bool FBLibrary::LibReady()	{ return true; }
bool FBLibrary::LibClose()	
{
	SetLiveBridgeLogger(nullptr);
	return true; 
}
bool FBLibrary::LibRelease(){ return true; }
