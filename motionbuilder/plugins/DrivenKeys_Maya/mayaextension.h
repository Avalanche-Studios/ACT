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

/**	\file	mayaextension.h
*	declaration of extension plugin library entry points
*/
#pragma once

#include <fbxsdk.h>

//Include Maya SDK headers
#include <maya/MGlobal.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MItDependencyGraph.h>

class FbxMayaNode;

//Define the FBX Maya Extension interface
#define FBXSDK_MAYA_EXTENSION_DECLARE()\
	FBXSDK_DLLEXPORT bool MayaExt_IsExtension(){return true;}\
	\
	FBXSDK_DLLEXPORT bool MayaExt_ExportHandled(MObject& pMayaObject);\
	FBXSDK_DLLEXPORT void MayaExt_ExportBegin(FbxScene* pFbxScene);\
    FBXSDK_DLLEXPORT bool MayaExt_ExportProcess(FbxObject** pOutputFbxObject, MObject& pInputObject, FbxScene* pFbxScene);\
	FBXSDK_DLLEXPORT void MayaExt_ExportTranslated(FbxObject* pFbxObject, MObject& pMayaObject);\
	FBXSDK_DLLEXPORT void MayaExt_ExportEnd(FbxScene* pFbxScene);\
	\
	FBXSDK_DLLEXPORT bool MayaExt_ImportHandled(FbxObject* pFbxObject);\
	FBXSDK_DLLEXPORT void MayaExt_ImportBegin(FbxScene* pFbxScene);\
    FBXSDK_DLLEXPORT bool MayaExt_ImportProcess(MObject& pOutputObject, FbxObject* pInputFbxObject, bool pIsAnInstance, bool pMerge);\
	FBXSDK_DLLEXPORT void MayaExt_ImportTranslated(FbxObject* pFbxObject, MObject& pMayaObject);\
	FBXSDK_DLLEXPORT void MayaExt_ImportEnd(FbxScene* pFbxScene);\

