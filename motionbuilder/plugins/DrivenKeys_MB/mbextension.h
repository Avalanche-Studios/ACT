
#pragma once

//
// Copyright(c) 2020 Avalanche Studios.All rights reserved.
// Licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE
//
//--------------------------------------------------------------------------------------

/**	\file	mbextension.h
*	declaration of extension plugin library entry points
*/

#if defined(_WIN32) || defined(_WIN64)
	#define EXPORT_DLL __declspec(dllexport)
#else
	#define EXPORT_DLL
#endif

#include <fbxsdk.h>

// Open Reality SDK
#include <fbsdk/fbsdk.h>

#include <fbxsdk/fbxsdk_nsbegin.h>

#define FBX_MB_EXTENSION_DECLARE()\
	EXPORT_DLL bool MBExt_IsExtension(){return true;}\
	\
	EXPORT_DLL bool MBExt_ExportHandled( FBComponent* pFBComponent );\
	EXPORT_DLL void MBExt_ExportBegin( FbxScene* pFbxScene );\
	EXPORT_DLL bool MBExt_ExportProcess( FbxObject*& output, FBComponent* inputObject, FbxScene* pFbxScene);\
	EXPORT_DLL void MBExt_ExportTranslated( FbxObject* pFbxObject, FBComponent* pFBComponent );\
	EXPORT_DLL void MBExt_ExportEnd( FbxScene* pFbxScene );\
	\
	EXPORT_DLL bool MBExt_ImportHandled( FbxObject* pFbxObject );\
	EXPORT_DLL void MBExt_ImportBegin( FbxScene* pFbxScene );\
	EXPORT_DLL bool MBExt_ImportProcess( FBComponent*& pOutputObject, FbxObject* pInputFbxObject, bool pIsAnInstance, bool pMerge);\
	EXPORT_DLL void MBExt_ImportTranslated( FbxObject* pFbxObject, FBComponent* pFBComponent );\
	EXPORT_DLL void MBExt_ImportEnd( FbxScene* pFbxScene );\

#include <fbxsdk/fbxsdk_nsend.h>
