
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

/**	\file	Common_MoBu.cpp
*	MotionBuilder specified realization of Shared Interface for the FBX extension plugins.
*/

#include "Common.h"
#include "tinyxml.h"

#include <vector>
#include <map>
#include <set>

// Open Reality SDK
#include <fbsdk/fbsdk.h>

//////////////////////////////////////////
// store nodes, edges and it's data
extern std::map < std::string, SNodeDataBase*> nodes;
extern std::map<std::string, std::string>	edges;

struct SHoldItem
{
	FBBox*	m_NumberToVector = nullptr;
	FBBox*	m_Owner = nullptr;
	FBConstraintRelation* m_Constraint = nullptr;
};

std::vector<SHoldItem>	items_to_check;

constexpr double box_offset_x = 325.0;
constexpr double box_offset_y = 200.0;

////////////////////////////////////////////////

FBAnimationNode* FindAnimationNode(FBAnimationNode* parent, const char* name)
{
	FBAnimationNode* result = nullptr;

	for (int i = 0; i < parent->Nodes.GetCount(); ++i)
	{
		if (strcmp(name, parent->Nodes[i]->Name) == 0)
		{
			result = parent->Nodes[i];
			break;
		}
	}

	return result;
}

bool ReconstructCurve(FBFCurve* curve, SCurveData* data)
{
	const bool is_angular = strcmp(data->m_Type, "kAnimCurveUnitlessToAngular") == 0;

	curve->EditClear();
	curve->EditBegin();

	FBTime time;

	for (uint32_t i = 0; i < data->m_NumberOfKeys; ++i)
	{
		SCurveKeyframe& key = data->m_Keys[i];

		time.SetSecondDouble(key.m_Time);
		int idx = curve->KeyAdd(time, (!is_angular) ? key.m_Value : (key.m_Value * 180.0 / 3.141516));

		curve->Keys[idx].Interpolation = kFBInterpolationCubic;
		curve->Keys[idx].TangentMode = kFBTangentModeClampProgressive;
		curve->Keys[idx].TangentClampMode = kFBTangentClampModeNone;
	}

	curve->EditEnd();

	curve->SetPreExtrapolationMode(kFCurveExtrapolationConst);
	curve->SetPostExtrapolationMode(kFCurveExtrapolationConst);

	return true;
}

// Check source node type
bool CheckSourceNodeType(const char* dst_key, const char* type)
{
	auto edge_iter = edges.find(dst_key);

	if (edge_iter != end(edges))
	{
		size_t src_delim = edge_iter->second.find('.');

		if (src_delim != std::string::npos)
		{
			std::string src_name(edge_iter->second.c_str(), src_delim);

			auto node_iter = nodes.find(src_name);

			if (node_iter != end(nodes))
			{
				if (SNodeDataBase* src_node = node_iter->second)
				{
					return (strcmp(src_node->m_Type, type) == 0);
				}
			}
		}
	}
	return false;
}

// dst_attrib - attribute (property) name
// relation_box - destination relation box
//
bool CreateOrUpdateSourceNode(const FBVector2d offset, const char* dst_key, const char* dst_attrib, const int attrib_index, FBModel* dst_model, FBBox* relation_box, FBConstraintRelation* constraint)
{
	// 1 - find a dst property to connect (or make some property animatable, to be able to connect)
	bool result = false;

	FBAnimationNode* attrib_anim_node = FindAnimationNode(relation_box->AnimationNodeInGet(), dst_attrib);

	// convert attrib name into motionbuilder property
	if (attrib_anim_node == nullptr)
	{
		// if dst_model is not nullptr, that mean we have a last dst model in a chain
		if (strcmp(dst_attrib, "translate") == 0)
		{
			// no need to make a Number To Vector
			attrib_anim_node = FindAnimationNode(relation_box->AnimationNodeInGet(), "Lcl Translation");
		}
		else if (strcmp(dst_attrib, "rotate") == 0)
		{
			// no need to make a Number To Vector
			attrib_anim_node = FindAnimationNode(relation_box->AnimationNodeInGet(), "Lcl Rotation");
		}
		else if (strcmp(dst_attrib, "rotateX") == 0 || strcmp(dst_attrib, "rotateY") == 0 || strcmp(dst_attrib, "rotateZ") == 0
			|| strcmp(dst_attrib, "translateX") == 0 || strcmp(dst_attrib, "translateY") == 0 || strcmp(dst_attrib, "translateZ") == 0)
		{
			// create Rotation to vector node or use existing and connect rotateZ there
			
			// need to make a Number To Vector
			FBBox* box_ntv = nullptr;

			FBAnimationNode* node_root = relation_box->AnimationNodeInGet();
			FBAnimationNode* dst_anim_node = FindAnimationNode(node_root, (strstr(dst_attrib, "translate") != nullptr) ? "Lcl Translation" : "Lcl Rotation");

			for (int j = 0; j < dst_anim_node->GetSrcCount(); ++j)
			{
				FBPlug* plug = dst_anim_node->GetSrc(j);
				FBBox* plug_owner = static_cast<FBBox*>(plug->GetOwner());
				FBString plug_name(plug_owner->Name);
				std::string str_plug_name(plug_name);

				if (str_plug_name.find("Number to Vector") != std::string::npos)
				{
					box_ntv = plug_owner;
				}
			}


			// if not exist, let create a new one and connect it
			if (box_ntv == nullptr)
			{
				box_ntv = constraint->CreateFunctionBox("Converters", "Number to Vector");
				constraint->SetBoxPosition(box_ntv, offset[0], offset[1]);
				offset[0] -= box_offset_x;

				FBAnimationNode* src_node = FindAnimationNode(box_ntv->AnimationNodeOutGet(), "Result");
				FBAnimationNode* dst_node = FindAnimationNode(node_root, (strstr(dst_attrib, "rotate") != nullptr) ? "Lcl Rotation" : "Lcl Translation");
				FBConnect(src_node, dst_node);

				// check later if all components are connected
				SHoldItem hold_item;
				hold_item.m_NumberToVector = box_ntv;
				hold_item.m_Owner = relation_box;
				hold_item.m_Constraint = constraint;
				items_to_check.push_back(hold_item);
			}

			// get number to vector dst node
			if (strcmp(dst_attrib, "rotateX") == 0 || strcmp(dst_attrib, "translateX") == 0)
			{
				attrib_anim_node = FindAnimationNode(box_ntv->AnimationNodeInGet(), "X");
			}
			else if (strcmp(dst_attrib, "rotateY") == 0 || strcmp(dst_attrib, "translateY") == 0)
			{
				attrib_anim_node = FindAnimationNode(box_ntv->AnimationNodeInGet(), "Y");
			}
			else if (strcmp(dst_attrib, "rotateZ") == 0 || strcmp(dst_attrib, "translateZ") == 0)
			{
				attrib_anim_node = FindAnimationNode(box_ntv->AnimationNodeInGet(), "Z");
			}
		}
	}


	if (attrib_anim_node != nullptr)
	{
		// find which node we need to create to connect with this attribute
		auto edge_iter = edges.find(dst_key);

		if (edge_iter != end(edges))
		{
			size_t src_delim = edge_iter->second.find('.');

			if (src_delim != std::string::npos)
			{
				std::string src_name(edge_iter->second.c_str(), src_delim);

				auto node_iter = nodes.find(src_name);

				if (node_iter != end(nodes))
				{
					if (SNodeDataBase* src_node = node_iter->second)
					{
						if (strcmp(src_node->m_Type, "kJoint") == 0)
						{
							FBModel* source = FBFindModelByLabelName(src_name.c_str());

							if (source)
							{
								FBBox* box = constraint->SetAsSource(source);
								((FBModelPlaceHolder*)box)->UseGlobalTransforms = false;

								std::string src_attrib(edge_iter->second.c_str() + src_delim + 1);

								if (strcmp(src_attrib.c_str(), "rotateX") == 0 || strcmp(src_attrib.c_str(), "rotateY") == 0 || strcmp(src_attrib.c_str(), "rotateZ") == 0)
								{
									// insert Number To Vector and connect to Lcl Rotation
									FBBox* box_vtn = constraint->CreateFunctionBox("Converters", "Vector to Number");
									constraint->SetBoxPosition(box_vtn, offset[0], offset[1]);
									constraint->SetBoxPosition(box, offset[0] - box_offset_x, offset[1]);

									FBAnimationNode* src_anim_node = FindAnimationNode(box->AnimationNodeOutGet(), "Lcl Rotation");
									FBAnimationNode* dst_anim_node = FindAnimationNode(box_vtn->AnimationNodeInGet(), "V");
									FBConnect(src_anim_node, dst_anim_node);

									int axis = 1;
									if (strcmp(src_attrib.c_str(), "rotateY") == 0)
									{
										axis = 2;
									}
									else if (strcmp(src_attrib.c_str(), "rotateZ") == 0)
									{
										axis = 3;
									}

									src_anim_node = FindAnimationNode(box_vtn->AnimationNodeOutGet(), (axis == 1) ? "X" : (axis == 2) ? "Y" : "Z");
									result = FBConnect(src_anim_node, attrib_anim_node);
								}
								else if (strcmp(src_attrib.c_str(), "translateX") == 0 || strcmp(src_attrib.c_str(), "translateY") == 0 || strcmp(src_attrib.c_str(), "translateZ") == 0)
								{
									// insert Number To Vector and connect to Lcl Translation
									FBBox* box_vtn = constraint->CreateFunctionBox("Converters", "Vector to Number");
									constraint->SetBoxPosition(box_vtn, offset[0], offset[1]);
									constraint->SetBoxPosition(box, offset[0] - box_offset_x, offset[1]);

									FBAnimationNode* src_anim_node = FindAnimationNode(box->AnimationNodeOutGet(), "Lcl Translation");
									FBAnimationNode* dst_anim_node = FindAnimationNode(box_vtn->AnimationNodeInGet(), "V");
									FBConnect(src_anim_node, dst_anim_node);

									int axis = 1;
									if (strcmp(src_attrib.c_str(), "translateY") == 0)
									{
										axis = 2;
									}
									else if (strcmp(src_attrib.c_str(), "translateZ") == 0)
									{
										axis = 3;
									}

									src_anim_node = FindAnimationNode(box_vtn->AnimationNodeOutGet(), (axis == 1) ? "X" : (axis == 2) ? "Y" : "Z");
									result = FBConnect(src_anim_node, attrib_anim_node);
								}
								else
								{
									constraint->SetBoxPosition(box, offset[0], offset[1]);

									// try to find source box from some animatable property
									if (FBAnimationNode* src_anim_node = FindAnimationNode(box->AnimationNodeOutGet(), src_attrib.c_str()))
									{
										result = FBConnect(src_anim_node, attrib_anim_node);
									}
								}
							}
						}
						else if (strcmp(src_node->m_Type, "kUnitConversion") == 0)
						{
							// function box
							FBBox* box = constraint->CreateFunctionBox("Converters", "Seconds to Time");
							constraint->SetBoxPosition(box, offset[0], offset[1]);

							result = FBConnect(FindAnimationNode(box->AnimationNodeOutGet(), "Result"), attrib_anim_node);

							std::string next_dst_key(src_name);
							next_dst_key = next_dst_key + ".input";

							FBVector2d new_offset(offset[0] - box_offset_x, offset[1]);
							CreateOrUpdateSourceNode(new_offset, next_dst_key.c_str(), "Seconds", 0, nullptr, box, constraint);
						}
						else if (strcmp(src_node->m_Type, "kAnimCurveUnitlessToDistance") == 0
							|| strcmp(src_node->m_Type, "kAnimCurveUnitlessToAngular") == 0)
						{
							// if there is no Second To Time connection ,then create FCurve Number (%) and connect Position %
							std::string next_dst_key(src_name);
							next_dst_key = next_dst_key + ".input";

							FBBox* curve_box = constraint->CreateFunctionBox("Other", "FCurve Number (Time)");
							constraint->SetBoxPosition(curve_box, offset[0], offset[1]);
							std::string position_label("Position");

							// function box
							FBAnimationNode* node = FindAnimationNode(curve_box->AnimationNodeOutGet(), "Value");
							result = FBConnect(node, attrib_anim_node);

							// reconstruct curve
							ReconstructCurve(node->FCurve, (SCurveData*)src_node);

							if (!CheckSourceNodeType(next_dst_key.c_str(), "kUnitConversion"))
							{
								// add manually unit conversion
								FBBox* box = constraint->CreateFunctionBox("Converters", "Seconds to Time");
								constraint->SetBoxPosition(box, offset[0] - box_offset_x, offset[1]);

								result = FBConnect(FindAnimationNode(box->AnimationNodeOutGet(), "Result"), FindAnimationNode(curve_box->AnimationNodeInGet(), "Position"));

								FBVector2d new_offset(offset[0] - box_offset_x, offset[1]);
								CreateOrUpdateSourceNode(new_offset, next_dst_key.c_str(), "Seconds", 0, nullptr, box, constraint);
							}
							else
							{
								FBVector2d new_offset(offset[0] - box_offset_x, offset[1]);
								CreateOrUpdateSourceNode(new_offset, next_dst_key.c_str(), position_label.c_str(), 0, nullptr, curve_box, constraint);
							}
						}
						else if (strcmp(src_node->m_Type, "kBlendWeighted") == 0)
						{
							FBBox* box = constraint->CreateFunctionBox("Number", "Sum 10 numbers");
							constraint->SetBoxPosition(box, offset[0], offset[1]);

							result = FBConnect(FindAnimationNode(box->AnimationNodeOutGet(), "Result"), attrib_anim_node);

							// check for all possible array connections

							std::string next_dst_key;

							next_dst_key = src_name + ".input[0]";
							FBVector2d new_offset(offset[0] - box_offset_x, offset[1] + box_offset_y);
							CreateOrUpdateSourceNode(new_offset, next_dst_key.c_str(), "a", 0, nullptr, box, constraint);

							//
							next_dst_key = src_name + ".input[1]";
							new_offset[1] -= box_offset_y;
							CreateOrUpdateSourceNode(new_offset, next_dst_key.c_str(), "b", 0, nullptr, box, constraint);
						}
					}
				}
			}
		}
	}

	return result;
}

// this function is API specified
void ReconstructDAG()
{
	// list all nodes - go through destination nodes and get model name / attribute (property) name

	// check if that nodes already have been constrainted with relation (create a new one if missing)
	// go node by node and check connection graph up to source, recover connection graph and intermediate nodes

	std::map<std::string, FBConstraintRelation*> m_constraints;
	FBConstraintRelation* default_constraint = nullptr;

	for (auto edge_iter = begin(edges); edge_iter != end(edges); ++edge_iter)
	{
		// DONE: extract object name and attribute name

		size_t dst_delim = edge_iter->first.find('.');

		if (dst_delim != std::string::npos)
		{
			// extract attribute name and array index (check if attribute is an array plug)

			std::string dst_name(edge_iter->first.c_str(), dst_delim);

			FBModel* model = FBFindModelByLabelName(dst_name.c_str());
			FBConstraintRelation* constr = nullptr;

			if (model != nullptr)
			{
				const int src_count = model->GetSrcCount();

				for (int i = 0; i < src_count; ++i)
				{
					FBPlug* plug = model->GetSrc(i);

					if (FBIS(plug, FBConstraintRelation))
					{
						constr = (FBConstraintRelation*)plug;
					}
				}

				if (constr == nullptr)
				{
					if (default_constraint == nullptr)
					{
						default_constraint = (FBConstraintRelation*)FBConstraintManager::TheOne().TypeCreateConstraint("Relation");
					}

					constr = default_constraint;
				}

				m_constraints.emplace(edge_iter->first, constr);
			}
		}
	}

	//
	FBVector2d offset(-box_offset_x, 0.0);

	for (auto iter = begin(m_constraints); iter != end(m_constraints); ++iter)
	{
		FBConstraintRelation* relation = iter->second;

		if (relation == nullptr)
		{
			continue;
		}

		size_t dst_delim = iter->first.find('.');

		if (dst_delim == std::string::npos)
		{
			// skip because of no attribute
			continue;
		}

		// extract attribute name and array index (check if attribute is an array plug)
		std::string dst_name(iter->first.c_str(), dst_delim);


		FBBox* dst_box = nullptr;

		for (int i = 0; i < relation->Boxes.GetCount(); ++i)
		{
			const char* name = relation->Boxes[i]->Name;
			if (strcmp(name, dst_name.c_str()) == 0)
			{
				dst_box = relation->Boxes[i];
				break;
			}
		}

		FBModel* model = FBFindModelByLabelName(dst_name.c_str());

		if (model != nullptr)
		{
			// constraint object with that 
			if (dst_box == nullptr)
			{
				dst_box = relation->ConstrainObject(model);
				relation->SetBoxPosition(dst_box, offset[0], offset[1]);

				((FBModelPlaceHolder*)dst_box)->UseGlobalTransforms = false;
			}
			// process all the chain
			if (dst_box != nullptr)
			{
				std::string dst_attrib_full(iter->first.c_str() + dst_delim + 1);

				std::string dst_attrib;
				int32_t dst_array_index = -1;

				size_t array_delim = dst_attrib_full.find('[');
				if (array_delim != std::string::npos)
				{
					// we have a connection to an array attribute, let's extract index
					dst_attrib.assign(dst_attrib_full.c_str(), array_delim);
					dst_array_index = atoi(dst_attrib_full.c_str() + array_delim + 1);
				}
				else
				{
					dst_attrib.assign(iter->first.c_str() + dst_delim + 1);
				}

				offset[0] -= box_offset_x;
				bool result = CreateOrUpdateSourceNode(offset, iter->first.c_str(), dst_attrib.c_str(), dst_array_index, model, dst_box, relation);

				if (result == false)
				{
					//dst_box->FBDelete();
				}
				else
				{
					offset[1] -= 2.0 * box_offset_y;
				}
			}
		}

	}

	// cleanup - remove empty boxes and connect empty channels in Number To Vector transform

	for (const SHoldItem& item : items_to_check)
	{
		FBBox* src_box = nullptr;
		FBBox* vtn_box = nullptr;
		int vtn_index = 0;

		FBConstraintRelation* relation = item.m_Constraint;

		FBAnimationNode* in_node = item.m_NumberToVector->AnimationNodeInGet();

		for (int i = 0; i < in_node->Nodes.GetCount(); ++i)
		{
			if (in_node->Nodes[i]->GetSrcCount() == 0)
			{
				if (src_box == nullptr)
				{
					src_box = relation->SetAsSource(((FBModelPlaceHolder*)item.m_Owner)->Model);
					((FBModelPlaceHolder*)src_box)->UseGlobalTransforms = false;

					int x, y;
					relation->GetBoxPosition(item.m_Owner, x, y);

					x -= 2 * box_offset_x;
					relation->SetBoxPosition(src_box, x, y);

					vtn_box = relation->CreateFunctionBox("Converters", "Vector to Number");

					x += box_offset_x;
					relation->SetBoxPosition(vtn_box, x, y);

					// !!! need to correctly specify source node translation, rotation, scale or custom ?!

					FBAnimationNode* src_node = FindAnimationNode(src_box->AnimationNodeOutGet(), "Lcl Rotation");
					FBAnimationNode* dst_node = FindAnimationNode(vtn_box->AnimationNodeInGet(), "V");
					FBConnect(src_node, dst_node);


				}

				FBAnimationNode* src_node = FindAnimationNode(vtn_box->AnimationNodeOutGet(), (i == 0) ? "X" : (i == 1) ? "Y" : "Z");
				FBConnect(src_node, in_node->Nodes[i]);
			}
			else
			{
				vtn_index = i;
			}
		}

		// connect pull vector
		if (vtn_box != nullptr)
		{
			FBBox* pull_box = relation->CreateFunctionBox("Number", "Pull Number");

			FBAnimationNode* src_node = FindAnimationNode(vtn_box->AnimationNodeOutGet(), (vtn_index == 0) ? "X" : (vtn_index == 1) ? "Y" : "Z");
			FBAnimationNode* dst_node = FindAnimationNode(pull_box->AnimationNodeInGet(), "a");
			FBConnect(src_node, dst_node);

			int x, y;
			relation->GetBoxPosition(vtn_box, x, y);
			x -= box_offset_x;
			relation->SetBoxPosition(pull_box, x, y);
		}
	}

	items_to_check.clear();

}