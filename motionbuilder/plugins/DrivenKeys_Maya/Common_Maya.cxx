
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

/**	\file	Common_Maya.cpp
*	Maya API specified implementation of driven keys graph IO
*/

#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MDagModifier.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItDag.h>

#include <vector>
#include <map>

#include "Common.h"
#include "tinyxml.h"

////////////////////////////////////////////////////////////////////////
// store nodes and it's data
extern std::map < std::string, SNodeDataBase*> nodes;
extern std::map<std::string, std::string>	edges;

////////////////////////////////////////////////////////////////////////

void OutputAnimCurve(MObject &obj, SCurveData* out_data)
{
	MFnAnimCurve fn(obj);

	uint32_t key_count = fn.numKeys();
	out_data->m_NumberOfKeys = key_count;

	// dont bother if its a pointless curve....
	if (key_count == 0) return;

	out_data->m_Keys = new SCurveKeyframe[key_count];

	// get all keyframe times & values
	for (uint32_t i = 0; i < key_count; i++)
	{
		double time_value = 0.0;

		if (fn.isUnitlessInput())
		{
			time_value = fn.unitlessInput(i);
		}
		else
		{
			const MTime time = fn.time(i);
			time_value = time.as(MTime::kSeconds);
		}

		const double value = fn.value(i);

		// now i'm not too sure about this, so I will
		// state something that may be untrue. When 
		// getting the tangents to the animation curves
		// you can pretty much ignore the x tangent 
		// values because time is generally a linear
		// value. (am i right here?) If I am wrong then
		// write the x values as well.
		float ix, iy, ox, oy;
		ix = iy = ox = oy = 0.0f;
		//fn.getTangent(i, ix, iy, true);
		//fn.getTangent(i, ox, oy, false);

		out_data->m_Keys[i].m_Time = time_value;
		out_data->m_Keys[i].m_Value = value;
		out_data->m_Keys[i].m_InTangent = iy;
		out_data->m_Keys[i].m_OutTangent = oy;
	}
}

void PrintConnections(MPlug& plug)
{
	// will hold the connections to this node
	MPlugArray plugs;

	cout << "plug " << plug.name().asChar() << endl;

	// get input plugs
	plug.connectedTo(plugs, true, false);

	cout << "\tinputs " << plugs.length() << endl;
	for (uint32_t i = 0; i < plugs.length(); ++i) {

		cout << "\t\t" << plugs[i].name().asChar() << endl;

		// loop through all connections. We are looking for
		// any attached animation curves
		for (uint32_t j = 0; j != plugs.length(); ++i)
		{
			// get a reference to the node 
			MObject connected = plugs[j].node();

			// if it's an animation curve output it
			if (connected.hasFn(MFn::kAnimCurve))
			{
				// check if curve is just animated one or a curve has input connection
				OutputAnimCurve(connected, nullptr);
			}

		}
	}

	// get output plugs
	plug.connectedTo(plugs, false, true);

	cout << "\toutputs " << plugs.length() << endl;
	for (uint32_t i = 0; i < plugs.length(); ++i) {

		cout << "\t\t" << plugs[i].name().asChar() << endl;
	}
}



bool ProcessObject(MObject& node)
{
	MStatus	stat;

	// attach the function set to the object
	MFnDependencyNode fn(node);

	const char* node_key = fn.name().asChar();

	auto iter = nodes.find(node_key);

	if (iter == end(nodes))
	{
		SNodeDataBase* node_data = nullptr;
		// if it's an animation curve output it
		if (node.hasFn(MFn::kAnimCurve))
		{
			SCurveData* curve_data = new SCurveData();
			// check if curve is just animated one or a curve has input connection
			OutputAnimCurve(node, curve_data);

			node_data = static_cast<SNodeDataBase*>(curve_data);
		}
		else if (node.hasFn(MFn::kUnitConversion))
		{
			SConversionData* data = new SConversionData();
			data->m_ConversionFactor = fn.findPlug("conversionFactor", true, &stat).asFloat();
			node_data = static_cast<SNodeDataBase*>(data);
		}
		else if (node.hasFn(MFn::kBlendWeighted))
		{
			SBlendWeightedData* data = new SBlendWeightedData();
			data->m_Weight = fn.findPlug("weight", true, &stat).asFloat();
			node_data = static_cast<SNodeDataBase*>(data);
		}
		else
		{
			node_data = new SNodeDataBase();
		}

		memset(node_data->m_Type, 0, sizeof(node_data->m_Type));
		strcpy_s(node_data->m_Type, node.apiTypeStr());

		nodes.emplace(node_key, node_data);

		//
		// will hold the connections to this node

		MPlugArray nodeplugs;

		// using the getConnections function we return a list
		// of attributes on THIS node that have connections. 
		fn.getConnections(nodeplugs);

		for (uint32_t i = 0; i < nodeplugs.length(); ++i)
		{
			// will hold the connections to this node
			MPlug node_plug{ nodeplugs[i] };
			MString plug_name{ node_plug.name(&stat) };
			const char* dst_key = plug_name.asChar();

			// get input plugs
			MPlugArray plugs;
			nodeplugs[i].connectedTo(plugs, true, false, &stat);
			
			// loop through all connections. We are looking for
			// any attached animation curves
			for (uint32_t j = 0; j != plugs.length(); ++j)
			{
				MPlug connected_plug{ plugs[j] };
				MString connected_name{ connected_plug.name(&stat) };
				const char* connected_key = connected_name.asChar();

				// store connection between nodes
				if (edges.find(dst_key) != end(edges))
				{
					printf("this key is already exist!");
				}
				else if (strcmp(dst_key, connected_key) != 0)
				{
					edges.emplace(dst_key, connected_key);
				}

				// get a reference to the node 
				MObject connected = plugs[j].node();

				// recursive dag check
				ProcessObject(connected);
			}
		}
	}

	return true;
}



void ReconstructDAG()
{
	// look into nodes and create missing nodes, recover connections
	std::vector<bool>		exist_flags(nodes.size(), false);
	std::map<std::string, MObject>	node_objects;

	//Loop through Maya scene and find meshes
	MStatus	status;
	MItDag	DagIterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
	if (!status)
	{
		return;
	}

	MDagPath	DagPath;
	for (; !DagIterator.isDone(); DagIterator.next())
	{
		status = DagIterator.getPath(DagPath);
		if (!status) continue;

		MObject maya_object(DagPath.node());

		MFnDependencyNode fn(maya_object);
		MString name = fn.name(&status);

		if (status)
		{
			auto iter = begin(nodes);
			auto flag_iter = begin(exist_flags);

			for (; iter != end(nodes); ++iter, ++flag_iter)
			{
				if (strcmp(name.asChar(), iter->first.c_str()) == 0)
				{
					node_objects[name.asChar()] = maya_object;
					*flag_iter = true;
				}
			}
		}
	}

	auto iter = begin(nodes);
	auto flag_iter = begin(exist_flags);

	MDGModifier	modifier;


	for (; iter != end(nodes); ++iter, ++flag_iter)
	{
		// create a missing node
		if (*flag_iter == false)
		{
			if (strcmp(iter->second->m_Type, "kAnimCurveUnitlessToDistance") == 0
				|| strcmp(iter->second->m_Type, "kAnimCurveUnitlessToAngular") == 0)
			{
				MFnAnimCurve fn_curve;
				fn_curve.setName(MString(iter->first.c_str()));

				MFnAnimCurve::AnimCurveType type = MFnAnimCurve::AnimCurveType::kAnimCurveUL;

				if (strcmp(iter->second->m_Type, "kAnimCurveUnitlessToAngular") == 0)
				{
					type = MFnAnimCurve::AnimCurveType::kAnimCurveUA;
				}
				node_objects[iter->first] = fn_curve.create(type);

				modifier.renameNode(node_objects[iter->first], MString(iter->first.c_str()));

				SCurveData* curve_data = static_cast<SCurveData*>(iter->second);
				int32_t count = curve_data->m_NumberOfKeys;

				for (int32_t j = 0; j < count; ++j)
				{
					fn_curve.addKey(curve_data->m_Keys[j].m_Time, curve_data->m_Keys[j].m_Value);
				}
			}
			else if (strcmp(iter->second->m_Type, "kUnitConversion") == 0)
			{
				node_objects[iter->first] = modifier.createNode("unitConversion", &status); // MObject::kNullObj
				modifier.renameNode(node_objects[iter->first], MString(iter->first.c_str()));
				if (!status)
				{
					MGlobal::displayError(status.errorString());
					printf("couldn't create a node!");
				}
				else
				{
					SConversionData* data = static_cast<SConversionData*>(iter->second);
					MFnDependencyNode fn(node_objects[iter->first]);
					fn.findPlug("conversionFactor", true).setFloat(static_cast<float>(data->m_ConversionFactor));
				}
			}
			else if (strcmp(iter->second->m_Type, "kBlendWeighted") == 0)
			{
				node_objects[iter->first] = modifier.createNode("blendWeighted", &status); // MObject::kNullObj
				modifier.renameNode(node_objects[iter->first], MString(iter->first.c_str()));
				if (!status)
				{
					MGlobal::displayError(status.errorString());
					printf("couldn't create a node!");
				}
				else
				{
					SBlendWeightedData* data = static_cast<SBlendWeightedData*>(iter->second);
					MFnDependencyNode fn(node_objects[iter->first]);
					fn.findPlug("weight", true).setFloat(static_cast<float>(data->m_Weight));
				}
			}
			else
			{
				node_objects[iter->first] = modifier.createNode(MString(iter->second->m_Type), &status); // MObject::kNullObj

				if (!status)
				{
					MGlobal::displayError(status.errorString());
					printf("couldn't create a node!");
				}
			}
		}
	}

	// recover connections

	auto edge_iter = begin(edges);

	for (; edge_iter != end(edges); ++edge_iter)
	{
		// DONE: extract object name and attribute name
		size_t src_delim = edge_iter->second.find('.');
		size_t dst_delim = edge_iter->first.find('.');

		if (src_delim != std::string::npos && dst_delim != std::string::npos
			&& edge_iter->first != edge_iter->second)
		{
			// extract attribute name and array index (check if attribute is an array plug)

			MString src_name(edge_iter->second.c_str(), static_cast<int32_t>(src_delim));
			MString src_attrib(edge_iter->second.c_str() + static_cast<int32_t>(src_delim) + 1);

			MString dst_name(edge_iter->first.c_str(), static_cast<int32_t>(dst_delim));

			std::string dst_attrib_full(edge_iter->first.c_str() + dst_delim + 1);

			MString dst_attrib;
			int32_t dst_array_index = -1;

			size_t array_delim = dst_attrib_full.find('[');
			if (array_delim != std::string::npos)
			{
				// we have a connection to an array attribute, let's extract index
				dst_attrib.set(dst_attrib_full.c_str(), static_cast<int32_t>(array_delim));
				dst_array_index = atoi(dst_attrib_full.c_str() + array_delim + 1);
			}
			else
			{
				dst_attrib.set(edge_iter->first.c_str() + dst_delim + 1);
			}

			//
			//

			auto src_iter = node_objects.find(src_name.asChar());
			auto dst_iter = node_objects.find(dst_name.asChar());

			if (src_iter != end(node_objects) && dst_iter != end(node_objects))
			{
				MObject src = src_iter->second;
				MObject dst = dst_iter->second;

				MFnDependencyNode fn_src(src);
				MFnDependencyNode fn_dst(dst);

				MPlug src_plug = fn_src.findPlug(src_attrib, true);
				MPlug dst_plug;
				if (dst_array_index >= 0)
				{
					dst_plug = fn_dst.findPlug(dst_attrib, true);
					if (static_cast<int32_t>(dst_plug.numElements()) < dst_array_index)
					{
						dst_plug.setNumElements(static_cast<uint32_t>(dst_array_index + 1));
					}
					dst_plug = dst_plug.elementByLogicalIndex(static_cast<uint32_t>(dst_array_index));
				}
				else
				{
					dst_plug = fn_dst.findPlug(dst_attrib, true);
				}

				modifier.connect(src_plug, dst_plug);
			}
		}

	}

	modifier.doIt();
}