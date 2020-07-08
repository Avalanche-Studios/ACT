
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

/**	\file	Common.cpp
*	Shared Interface for the FBX extension plugins.
*/


#include <fbxsdk.h>

#include "Common.h"
#include "tinyxml.h"

#include <vector>
#include <map>
#include <set>

FBXSDK_OBJECT_IMPLEMENT(CDAGNode);
FBXSDK_OBJECT_IMPLEMENT(CDAGEdge);

std::map < std::string, SNodeDataBase*> nodes;

// store edges and it's data
std::map<std::string, std::string>	edges;

////////////////////////////////////////////////////////////////////////////////////////////////
// SNodeDataBase

SNodeDataBase::~SNodeDataBase()
{
}

bool SNodeDataBase::SaveToXml(TiXmlElement* element)
{
	element->SetAttribute("type", m_Type);

	return true;
}
bool SNodeDataBase::LoadFromXml()
{
	return true;
}
bool SNodeDataBase::LoadFromNode(CDAGNode* /*dag_node*/, FbxAnimLayer* /*anim_layer*/)
{
	return true;
}
bool SNodeDataBase::SaveToNode(CDAGNode* dag_node, FbxAnimLayer* /*anim_layer*/)
{
	FbxProperty	property;

	property = FbxProperty::Create(dag_node, FbxStringDT, "Type");
	if (property.IsValid())
	{
		property.Set(FbxString(m_Type));
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// unitConversion

SConversionData::~SConversionData()
{
}

bool SConversionData::SaveToXml(TiXmlElement* element)
{
	SNodeDataBase::SaveToXml(element);

	element->SetAttribute("subtype", "conversion");
	element->SetDoubleAttribute("factor", m_ConversionFactor);

	return true;
}
bool SConversionData::LoadFromNode(CDAGNode* dag_node, FbxAnimLayer* /*anim_layer*/)
{
	FbxProperty prop = dag_node->FindProperty("Factor");

	if (prop.IsValid())
	{
		m_ConversionFactor = static_cast<double>(prop.Get<float>());
	}

	return true;
}
bool SConversionData::SaveToNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer)
{
	SNodeDataBase::SaveToNode(dag_node, anim_layer);

	FbxProperty	property;

	property = FbxProperty::Create(dag_node, FbxStringDT, "SubType");
	if (property.IsValid())
	{
		property.Set(FbxString("conversion"));
	}

	property = FbxProperty::Create(dag_node, FbxFloatDT, "Factor");
	if (property.IsValid())
	{
		property.Set(static_cast<float>(m_ConversionFactor));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
// unitConversion

SBlendWeightedData::~SBlendWeightedData()
{
}

bool SBlendWeightedData::SaveToXml(TiXmlElement* element)
{
	SNodeDataBase::SaveToXml(element);

	element->SetAttribute("subtype", "blendWeighted");
	element->SetDoubleAttribute("weight", m_Weight);

	return true;
}
bool SBlendWeightedData::LoadFromNode(CDAGNode* dag_node, FbxAnimLayer* /*anim_layer*/)
{
	FbxProperty prop = dag_node->FindProperty("Weight");

	if (prop.IsValid())
	{
		m_Weight = static_cast<double>(prop.Get<float>());
	}

	return true;
}
bool SBlendWeightedData::SaveToNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer)
{
	SNodeDataBase::SaveToNode(dag_node, anim_layer);

	FbxProperty	property;

	property = FbxProperty::Create(dag_node, FbxStringDT, "SubType");
	if (property.IsValid())
	{
		property.Set(FbxString("blendWeighted"));
	}

	property = FbxProperty::Create(dag_node, FbxFloatDT, "Weight");
	if (property.IsValid())
	{
		property.Set(static_cast<float>(m_Weight));
	}
	return true;
}

///////////////////////////////////////////////////////////
// animCurve

SCurveData::~SCurveData()
{
	if (m_Keys)
	{
		delete[] m_Keys;
		m_Keys = nullptr;
	}
	m_NumberOfKeys = 0;
}

bool SCurveData::SaveToXml(TiXmlElement* element)
{
	SNodeDataBase::SaveToXml(element);

	element->SetAttribute("subtype", "animCurve");
	element->SetAttribute("count", m_NumberOfKeys);

	for (uint32_t i = 0; i < m_NumberOfKeys; ++i)
	{
		SCurveKeyframe& key = m_Keys[i];

		TiXmlElement key_elem("Key");
		key_elem.SetDoubleAttribute("time", key.m_Time);
		key_elem.SetDoubleAttribute("value", key.m_Value);
		key_elem.SetDoubleAttribute("in_tangent", static_cast<double>(key.m_InTangent));
		key_elem.SetDoubleAttribute("out_tangent", static_cast<double>(key.m_OutTangent));

		element->InsertEndChild(key_elem);
	}

	return true;
}
bool SCurveData::LoadFromNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer)
{
	FbxProperty count_prop, curve_prop;

	count_prop = dag_node->FindProperty("Count");
	curve_prop = dag_node->FindProperty("Curve");

	if (count_prop.IsValid() && curve_prop.IsValid())
	{
		int32_t count = count_prop.Get<int32_t>();
		FbxAnimCurve* curve = curve_prop.GetCurve(anim_layer);

		if (curve)
		{
			m_NumberOfKeys = count;
			m_Keys = new SCurveKeyframe[count];

			for (int32_t j = 0; j < count; ++j)
			{
				FbxAnimCurveKey key = curve->KeyGet(j);

				m_Keys[j].m_Time = key.GetTime().GetSecondDouble();
				m_Keys[j].m_Value = key.GetValue();
			}
		}
	}

	return true;
}
bool SCurveData::SaveToNode(CDAGNode* dag_node, FbxAnimLayer* anim_layer)
{
	SNodeDataBase::SaveToNode(dag_node, anim_layer);

	FbxProperty	property;

	property = FbxProperty::Create(dag_node, FbxStringDT, "SubType");
	if (property.IsValid())
	{
		property.Set(FbxString("animCurve"));
	}

	property = FbxProperty::Create(dag_node, FbxIntDT, "Count");
	if (property.IsValid())
	{
		property.Set(m_NumberOfKeys);
	}

	property = FbxProperty::Create(dag_node, FbxDoubleDT, "Curve");
	property.ModifyFlag(FbxPropertyFlags::eAnimatable, true);
	if (property.IsValid())
	{
		FbxAnimCurveNode* curve_node = property.GetCurveNode(anim_layer, true);
		if (curve_node)
		{
			FbxAnimCurve* curve = curve_node->GetCurve(0U);
			if (curve == nullptr)
			{
				curve = curve_node->CreateCurve(curve_node->GetName());
			}

			curve->KeyModifyBegin();
			curve->ResizeKeyBuffer(m_NumberOfKeys);

			FbxTime time;
			int32_t key_index = 0;

			for (uint32_t i = 0; i < m_NumberOfKeys; ++i)
			{
				SCurveKeyframe& key = m_Keys[i];

				time.SetSecondDouble(key.m_Time);

				FbxAnimCurveKey curve_key(time, static_cast<float>(key.m_Value));
				curve_key.SetInterpolation(FbxAnimCurveDef::eInterpolationCubic);

				curve->KeySet(key_index, curve_key);
				++key_index;
			}

			curve->KeyModifyEnd();
		}
	}

	return true;
}

bool PrintData(const char* file_name)
{
	// DONE: save data to a test xml file

	TiXmlDocument doc;

	TiXmlElement	head("Header");
	head.SetAttribute("version", 1);
	doc.InsertEndChild(head);

	TiXmlElement	nodes_root("Nodes");
	nodes_root.SetAttribute("count", static_cast<int32_t>(nodes.size()));
	for (auto elem : nodes)
	{
		TiXmlElement node_data("Node");
		node_data.SetAttribute("name", elem.first.c_str());

		if (elem.second != nullptr)
		{
			elem.second->SaveToXml(&node_data);
		}

		nodes_root.InsertEndChild(node_data);
	}
	doc.InsertEndChild(nodes_root);

	TiXmlElement	edges_root("Edges");
	edges_root.SetAttribute("count", static_cast<int32_t>(edges.size()));
	for (auto elem : edges)
	{
		TiXmlElement edge_data("Edge");
		edge_data.SetAttribute("dst", elem.first.c_str());
		edge_data.SetAttribute("src", elem.second.c_str());

		edges_root.InsertEndChild(edge_data);
	}
	doc.InsertEndChild(edges_root);

	doc.SaveFile(file_name);

	if (doc.Error())
	{
		printf(doc.ErrorDesc());
		return false;
	}
	return true;
}

bool ImportEnd(FbxScene* pFbxScene)
{
	const int32_t num_stacks = pFbxScene->GetSrcObjectCount(FbxCriteria::ObjectType(FbxAnimStack::ClassId));

	if (num_stacks == 0)
		return false;

	FbxAnimStack* anim_stack = nullptr;
	anim_stack = FbxCast<FbxAnimStack>(pFbxScene->GetSrcObject(FbxCriteria::ObjectType(FbxAnimStack::ClassId), 0));

	if (anim_stack == nullptr)
		return false;

	const int32_t num_layers = anim_stack->GetMemberCount(FbxCriteria::ObjectType(FbxAnimLayer::ClassId));

	if (num_layers == 0)
		return false;

	FbxAnimLayer* anim_layer = nullptr;
	anim_layer = FbxCast<FbxAnimLayer>(anim_stack->GetMember(FbxCriteria::ObjectType(FbxAnimLayer::ClassId), 0));

	if (anim_layer == nullptr)
		return false;

	// Make sure our custom object classes were correctly registered
	FbxClassId GraphNodeClassId = gFbxManager->FindClass(GRAPH_NODE_CLASS_NAME);
	FbxClassId GraphEdgeClassId = gFbxManager->FindClass(GRAPH_EDGE_CLASS_NAME);

	if (GraphNodeClassId.IsValid() && GraphEdgeClassId.IsValid())
	{
		//Search through the FBX scene to find our custom objects
		int32_t count = pFbxScene->GetSrcObjectCount<CDAGNode>();

		for (int32_t iter = 0; iter < count; ++iter)
		{
			CDAGNode* dag_node = pFbxScene->GetSrcObject<CDAGNode>(iter);
			if (dag_node)
			{
				const char* node_name = dag_node->GetName();
				SNodeDataBase* node_data = nullptr;

				FbxProperty property = dag_node->FindProperty("SubType");
				if (property.IsValid())
				{
					if (strcmp(property.Get<FbxString>().Buffer(), "animCurve") == 0)
					{
						SCurveData* curve_data = new SCurveData();
						node_data = curve_data;

						curve_data->LoadFromNode(dag_node, anim_layer);
					}
					else if (strcmp(property.Get<FbxString>().Buffer(), "conversion") == 0)
					{
						SConversionData* data = new SConversionData();
						node_data = data;

						data->LoadFromNode(dag_node, anim_layer);
					}
					else if (strcmp(property.Get<FbxString>().Buffer(), "blendWeighted") == 0)
					{
						SBlendWeightedData* data = new SBlendWeightedData();
						node_data = data;

						data->LoadFromNode(dag_node, anim_layer);
					}
				}

				if (node_data == nullptr)
				{
					node_data = new SNodeDataBase();
				}

				memset(node_data->m_Type, 0, sizeof(node_data->m_Type));

				property = dag_node->FindProperty("Type");
				if (property.IsValid())
				{
					strcpy_s(node_data->m_Type, property.Get<FbxString>().Buffer());
				}

				nodes.emplace(node_name, node_data);
			}
		}

		count = pFbxScene->GetSrcObjectCount<CDAGEdge>();

		for (int32_t iter = 0; iter < count; ++iter)
		{
			CDAGEdge* dag_edge = pFbxScene->GetSrcObject<CDAGEdge>(iter);
			if (dag_edge)
			{
				FbxProperty src_prop = dag_edge->FindProperty("src");
				FbxProperty dst_prop = dag_edge->FindProperty("dst");

				if (src_prop.IsValid() && dst_prop.IsValid())
				{
					FbxString src(src_prop.Get<FbxString>());
					FbxString dst(dst_prop.Get<FbxString>());

					edges.emplace(dst.Buffer(), src.Buffer());
				}
			}
		}
	}

	return true;
}

void FreeData()
{
	for (auto item : nodes)
	{
		if (SNodeDataBase* ptr = item.second)
		{
			delete ptr;
		}
	}

	nodes.clear();
	edges.clear();
}