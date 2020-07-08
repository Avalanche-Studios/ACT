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

/**	\file	MayaDrivenKeysDataPlugin.h
*	declaration of extension plugin library entry points
*/

#include "mayaextension.h"

#include <maya/MDagPath.h>
#include <maya/MFnIkJoint.h>

#include <vector>
#include <map>

#include "Common.h"
#include "tinyxml.h"

///////////////////////////////////////////////////////////
//

extern std::map < std::string, SNodeDataBase*> nodes;
extern std::map<std::string, std::string>	edges;

constexpr bool g_PrintLog{ false };

extern bool ProcessObject(MObject& node);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Plugin declaration and initialization methods
class CMayaDrivenKeysDataPlugin : public FbxPlugin
{
	FBXSDK_PLUGIN_DECLARE(CMayaDrivenKeysDataPlugin);

protected:
	explicit CMayaDrivenKeysDataPlugin(const FbxPluginDef& pDefinition, FbxModule pFbxModule) : FbxPlugin(pDefinition, pFbxModule)
	{
	}

	// Implement kfbxmodules::FbxPlugin
	virtual bool SpecificInitialize()
	{
		//Register MyCustomObject class with the plug-in's manager
		gFbxManager = GetData().mSDKManager;
		gFbxManager->RegisterFbxClass(GRAPH_NODE_CLASS_NAME, FBX_TYPE(CDAGNode), FBX_TYPE(FbxObject), GRAPH_NODE_CLASS_TYPE, GRAPH_NODE_CLASS_SUBTYPE);
		gFbxManager->RegisterFbxClass(GRAPH_EDGE_CLASS_NAME, FBX_TYPE(CDAGEdge), FBX_TYPE(FbxObject), GRAPH_EDGE_CLASS_TYPE, GRAPH_EDGE_CLASS_SUBTYPE);
		return true;
	}

	virtual bool SpecificTerminate()
	{
		//Unregister MyCustomObject class with the plug-in's manager
		gFbxManager->UnregisterFbxClass(FBX_TYPE(CDAGNode));
		gFbxManager->UnregisterFbxClass(FBX_TYPE(CDAGEdge));
		return true;
	}
};

FBXSDK_PLUGIN_IMPLEMENT(CMayaDrivenKeysDataPlugin);

// FBX Interface
extern "C"
{
    //The DLL is owner of the plug-in
    static CMayaDrivenKeysDataPlugin* sPlugin = nullptr;

    //This function will be called when an application will request the plug-in
    FBXSDK_DLLEXPORT void FBXPluginRegistration(FbxPluginContainer& pContainer, FbxModule pFbxModule)
    {
        if( sPlugin == nullptr )
        {
            //Create the plug-in definition which contains the information about the plug-in
            FbxPluginDef sPluginDef;
            sPluginDef.mName = "MayaExtensionDAGPlugin";
            sPluginDef.mVersion = "1.0";

            //Create an instance of the plug-in.  The DLL has the ownership of the plug-in
            sPlugin = CMayaDrivenKeysDataPlugin::Create(sPluginDef, pFbxModule);

            //Register the plug-in
            pContainer.Register(*sPlugin);
        }
    }

	FBXSDK_MAYA_EXTENSION_DECLARE();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Maya Extension Export Callbacks
bool MayaExt_ExportHandled(MObject& pMayaObject)
{
	return false;
}

void MayaExt_ExportBegin(FbxScene* pFbxScene)
{
}

void MayaExt_ExportTranslated(FbxObject* pFbxObject, MObject& pMayaObject)
{
}


bool MayaExt_ExportProcess(FbxObject** pOutputFbxObject, MObject& pInputObject, FbxScene* pFbxScene)
{
    return false;
}

// DONE: extract processing graph connected to nodes
// store graph with objects and it's connections by populating classes for each plug and plug connection inbetween

void MayaExt_ExportEnd(FbxScene* pFbxScene)
{
	// DONE: check if have animation stack and layer
	const int32_t num_stacks = pFbxScene->GetSrcObjectCount(FbxCriteria::ObjectType(FbxAnimStack::ClassId));

	FbxAnimStack* anim_stack = nullptr;
	if (num_stacks != 0)
	{
		anim_stack = FbxCast<FbxAnimStack>(pFbxScene->GetSrcObject(FbxCriteria::ObjectType(FbxAnimStack::ClassId), 0));
	}
	else
	{
		// Create one animation stack
		anim_stack = FbxAnimStack::Create(pFbxScene, "Take 001");
	}

	const int32_t num_layers = anim_stack->GetMemberCount(FbxCriteria::ObjectType(FbxAnimLayer::ClassId));

	FbxAnimLayer* anim_layer = nullptr;
	if (num_layers != 0)
	{
		anim_layer = FbxCast<FbxAnimLayer>(anim_stack->GetMember(FbxCriteria::ObjectType(FbxAnimLayer::ClassId), 0));
	}
	else
	{
		// all animation stacks need, at least, one layer.
		anim_layer = FbxAnimLayer::Create(pFbxScene, "Base Layer");	// the AnimLayer object name is "Base Layer"
		anim_stack->AddMember(anim_layer);											// add the layer to the stack
	}


	//Since we didn't export our custom data in the pre-export callback, we'll have to do it now.
	//This is the simplest solution for simple plugin-of-plugin like this example.

	// Loop through dag nodes and check attributes.
	// driven keys is when one attribute is connected to another with animCurve

	//Loop through Maya scene and find joints
	MStatus	status;
	MItDag	DagIterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
	if (!status)
	{
		return;
	}

	MDagPath	DagPath;
	for( ; !DagIterator.isDone(); DagIterator.next() )
	{
		status = DagIterator.getPath(DagPath);
        if( !status) continue;

		MObject maya_object(DagPath.node());
		MFnIkJoint joint(maya_object, &status);
		
		if (status)
		{
			// we found a joint, lets store a connection graph
			ProcessObject(maya_object);
		}
	}

	// DONE: print xml with a result stored graph!
	if (g_PrintLog)
	{
		PrintData("D:\\maya_export.xml");
	}
	
	//Make sure our custom object class was correctly registred
	FbxClassId GraphNodeClassId = gFbxManager->FindClass(GRAPH_NODE_CLASS_NAME);
	FbxClassId GraphEdgeClassId = gFbxManager->FindClass(GRAPH_EDGE_CLASS_NAME);

	if (GraphNodeClassId.IsValid() && GraphEdgeClassId.IsValid())
	{
		constexpr int32_t version = { 1 };

		// store out DAG
		for (auto node : nodes)
		{
			//Create the custom object to hold the custom data
			CDAGNode* dag_node = CDAGNode::Create(pFbxScene, node.first.c_str());

			// store version
			FbxProperty	property = FbxProperty::Create(dag_node, FbxIntDT, "Version");
			if (property.IsValid())
			{
				property.Set(version);
			}

			// store node data
			if (node.second != nullptr)
			{
				node.second->SaveToNode(dag_node, anim_layer);
			}
		}

		// store edges
		for (auto edge : edges)
		{
			//Create the custom object to hold the custom data

			CDAGEdge* dag_node = CDAGEdge::Create(pFbxScene, edge.first.c_str());

			// store version
			FbxProperty	property = FbxProperty::Create(dag_node, FbxIntDT, "Version");
			if (property.IsValid()) property.Set(version);

			// store connection
			property = FbxProperty::Create(dag_node, FbxStringDT, "src");
			if (property.IsValid())
			{
				property.Set(FbxString(edge.second.c_str()));
			}

			property = FbxProperty::Create(dag_node, FbxStringDT, "dst");
			if (property.IsValid())
			{
				property.Set(FbxString(edge.first.c_str()));
			}
		}
	}

	FreeData();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Maya Extension Import Callbacks
bool MayaExt_ImportHandled(FbxObject* pFbxObject)
{
	return false;
}

void MayaExt_ImportBegin(FbxScene* pFbxScene)
{
}

bool MayaExt_ImportProcess(MObject& pOutputObject, FbxObject* pInputFbxObject, bool pIsAnInstance, bool pMerge)
{
    return false;
}
void MayaExt_ImportTranslated(FbxObject* pFbxObject, MObject& pMayaObject)
{
}

void MayaExt_ImportEnd(FbxScene* pFbxScene)
{
	// DONE: import and reconstruct DAG !
	
	if (ImportEnd(pFbxScene))
	{
		ReconstructDAG();
	}
	FreeData();
}


