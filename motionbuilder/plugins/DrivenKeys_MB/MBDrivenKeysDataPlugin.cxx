
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

/**	\file	DrivenKeysDataPlugin.cpp
*	FBX Extension plugin to support driven keys relation
*/

#include "mbextension.h"

#include "Common.h"

#include <fbxsdk/fbxsdk_nsbegin.h>


/** Plugin declaration and initialization methods
*/
class CMBDrivenKeysDataPlugin : public FbxPlugin
{
    FBXSDK_PLUGIN_DECLARE(CMBDrivenKeysDataPlugin);

protected:
    explicit CMBDrivenKeysDataPlugin( const FbxPluginDef& pDefinition, FbxModule pLibHandle ) : FbxPlugin( pDefinition, pLibHandle )
    {
    }

    // Implement kfbxmodules::KFbxPlugin
    virtual bool SpecificInitialize()
    {
        // Register MyCustomObject class with the plug-in's manager
        gFbxManager = GetData().mSDKManager;
		gFbxManager->RegisterFbxClass(GRAPH_NODE_CLASS_NAME, FBX_TYPE(CDAGNode), FBX_TYPE(FbxObject), GRAPH_NODE_CLASS_TYPE, GRAPH_NODE_CLASS_SUBTYPE);
		gFbxManager->RegisterFbxClass(GRAPH_EDGE_CLASS_NAME, FBX_TYPE(CDAGEdge), FBX_TYPE(FbxObject), GRAPH_EDGE_CLASS_TYPE, GRAPH_EDGE_CLASS_SUBTYPE);

        return true;
    }

    virtual bool SpecificTerminate()
    {
        // Unregister MyCustomObject class with the plug-in's manager
		gFbxManager->UnregisterFbxClass(FBX_TYPE(CDAGNode));
		gFbxManager->UnregisterFbxClass(FBX_TYPE(CDAGEdge));
        return true;
    }
};

FBXSDK_PLUGIN_IMPLEMENT(CMBDrivenKeysDataPlugin);

/** FBX Interface
*/
extern "C"
{
    // The DLL is owner of the plug-in
    static CMBDrivenKeysDataPlugin* sPlugin = nullptr;

    // This function will be called when an application will request the plug-in
    EXPORT_DLL void FBXPluginRegistration( FbxPluginContainer& pContainer, FbxModule pLibHandle )
    {
        if ( sPlugin == nullptr )
        {
            // Create the plug-in definition which contains the information about the plug-in
            FbxPluginDef sPluginDef;
            sPluginDef.mName = "MoBuExtensionDAGPlugin";
            sPluginDef.mVersion = "1.0";

            // Create an instance of the plug-in.  The DLL has the ownership of the plug-in
            sPlugin = CMBDrivenKeysDataPlugin::Create( sPluginDef, pLibHandle );

            // Register the plug-in
            pContainer.Register( *sPlugin );
        }
    }

    FBX_MB_EXTENSION_DECLARE();
}

/** MB Extension Export Callbacks
*/
bool MBExt_ExportHandled( FBComponent* pFBComponent )
{
    return false;	
}

void MBExt_ExportBegin( FbxScene* pFbxScene )
{
}

bool MBExt_ExportProcess( FbxObject*& pOutputFbxObject, FBComponent* pInputObject, FbxScene* pFbxScene)
{
    return false;
}

void MBExt_ExportTranslated(FbxObject* pFbxObject, FBComponent* pFBComponent)
{
}

void MBExt_ExportEnd( FbxScene* pFbxScene )
{	
}

/** MB Extension Import Callbacks
*/
bool MBExt_ImportHandled( FbxObject* pFbxObject )
{
    return false;
}

void MBExt_ImportBegin( FbxScene* pFbxScene )
{
}

bool MBExt_ImportProcess( FBComponent*& pOutputObject, FbxObject* pInputFbxObject, bool pIsAnInstance, bool pMerge)
{
    return false;
}

void MBExt_ImportTranslated( FbxObject* pFbxObject, FBComponent* pFBComponent )
{
}

void MBExt_ImportEnd( FbxScene* pFbxScene )
{
	if (ImportEnd(pFbxScene))
	{
		ReconstructDAG();
	}
	FreeData();
}

#include <fbxsdk/fbxsdk_nsend.h>
