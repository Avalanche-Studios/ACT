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

/**	\file	Common.h
*	Shared Interface for the FBX extension plugins.
*/

#include <stdint.h>
#include <fbxsdk.h>

#define GRAPH_NODE_CLASS_NAME		"CDAGNode"
#define GRAPH_NODE_CLASS_TYPE		"DAGNode"
#define GRAPH_NODE_CLASS_SUBTYPE	"DAGNode"

#define GRAPH_EDGE_CLASS_NAME		"CDAGEdge"
#define GRAPH_EDGE_CLASS_TYPE		"DAGEdge"
#define GRAPH_EDGE_CLASS_SUBTYPE	"DAGEdge"



// forward declaration
class TiXmlElement;


//
static FbxManager* gFbxManager = nullptr;

//! Graph Nodes and Edges classes which instances are stored in fbx

class CDAGNode : public FbxObject
{
	FBXSDK_OBJECT_DECLARE(CDAGNode, FbxObject);
};

class CDAGEdge : public FbxObject
{
	FBXSDK_OBJECT_DECLARE(CDAGEdge, FbxObject);
};

	
//////////////////////////////////////////////////////////////////////////////////////
// SNodeDataBase
struct SNodeDataBase
{
	char			m_Type[64];

	virtual ~SNodeDataBase();

	virtual bool SaveToXml(TiXmlElement* element);
	virtual bool LoadFromXml();
	virtual bool LoadFromNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer);
	virtual bool SaveToNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer);
};

///////////////////////////////////////////////////////////////////////////////////
// keyframe for a hermite spline
struct SCurveKeyframe
{
	double		m_Time;
	double		m_Value;
	float		m_InTangent;
	float		m_OutTangent;
};

//////////////////////////////////////////////////////////////////////////
// unitConversion
struct SConversionData : public SNodeDataBase
{
	double m_ConversionFactor;

	virtual ~SConversionData();

	virtual bool SaveToXml(TiXmlElement* element) override;
	virtual bool LoadFromNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer) override;
	virtual bool SaveToNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer) override;
};

//////////////////////////////////////////////////////////////////////////
// unitConversion
struct SBlendWeightedData : public SNodeDataBase
{
	double m_Weight = 1.0;

	virtual ~SBlendWeightedData();

	virtual bool SaveToXml(TiXmlElement* element) override;
	virtual bool LoadFromNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer) override;
	virtual bool SaveToNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer) override;
};

///////////////////////////////////////////////////////////
// animCurve
struct SCurveData : public SNodeDataBase
{
	uint32_t			m_NumberOfKeys;
	SCurveKeyframe*		m_Keys;

	virtual ~SCurveData();

	virtual bool SaveToXml(TiXmlElement* element) override;
	virtual bool LoadFromNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer) override;
	virtual bool SaveToNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer) override;
};




bool ImportEnd(FbxScene* pFbxScene);

bool PrintData(const char* file_name);

// this function is API specified (Maya, MoBu, etc.)
void ReconstructDAG();

void FreeData();