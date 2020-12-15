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
// shared.cpp
//
#include <windows.h>

#include "shared.h"
#include "tinyxml.h"

#include <AnimLiveBridge/AnimLiveBridge.h>

#include <algorithm>

#define DEVICE__PREFIX		"AnimBridge"

/////////////////////////
//

namespace NShared
{
	std::string					g_ReferenceName("");
	std::vector<std::string>	g_JointSet;

	const char* GetRootName()
	{
		return g_ReferenceName.c_str();
	}

	const int GetSetJointsCount()
	{
		return static_cast<int>(g_JointSet.size());
	}

	const char* GetTestJointName(const int index)
	{
		if (index >= 0 && index < static_cast<int>(g_JointSet.size()))
		{
			return g_JointSet[index].c_str();
		}
		return nullptr;
	}

	bool FindParentInList(FBModel* model, FBModelList* models)
	{

		if (model)
		{
			for (int i = 0; i < models->GetCount(); ++i)
			{
				if (model == models->GetAt(i))
				{
					return true;
				}
			}

			if (FindParentInList(model->Parent, models))
				return true;
		}

		return false;
	}

	bool ImportJointSet(const char* filename)
	{
		TiXmlDocument	doc;

		if (doc.LoadFile(filename) == false)
		{
			return false;
		}

		//
		g_ReferenceName = "";
		g_JointSet.clear();

		TiXmlNode *node = nullptr;
		TiXmlElement *head = nullptr;
		TiXmlElement *item = nullptr;

		node = doc.FirstChild("JointSet");
		if (node == nullptr)
		{
			return false;
		}

		head = node->ToElement();
		if (!head)
			return false;

		item = node->FirstChildElement("Model");
		int count = 0;
		head->Attribute("count", &count);
		if (const char* root_name = head->Attribute("root"))
		{
			g_ReferenceName = root_name;
		}

		if (count == 0 || g_ReferenceName.empty())
			return false;

		for (; nullptr != item; item = item->NextSiblingElement())
		{
			const char* model_name = item->Attribute("name");
			g_JointSet.push_back(model_name);
		}

		return true;
	}

	bool ExportJointSet(const char* filename)
	{
		TiXmlDocument doc;

		FBModelList		models;
		FBGetSelectedModels(models);

		const int count = models.GetCount();

		if (count == 0)
			return false;

		FBModel* root = nullptr;

		for (int i = 0; i < count; ++i)
		{
			FBModel* model = models[i];

			// try to find parent in a selection list

			if (!FindParentInList(model->Parent, &models))
			{
				root = model->Parent;
				break;
			}
		}

		TiXmlElement	head("JointSet");
		head.SetAttribute("count", count);
		head.SetAttribute("root", (root) ? root->Name : "_Empty_");
		head.SetAttribute("version", 1);

		for (int i = 0; i < count; ++i)
		{
			TiXmlElement item("Model");
			item.SetAttribute("name", models[i]->Name);

			head.InsertEndChild(item);
		}

		doc.InsertEndChild(head);
		doc.SaveFile(filename);

		if (doc.Error())
		{
			printf("%s", doc.ErrorDesc());
			return false;
		}
		return true;
	}

};


/////////////////////////////////////////////////////////////////
// STimeChangeManager

STimeChangeManager::STimeChangeManager()
{
	mLocalTimeChanged = false;
	mRemoteTimeChanged = false;
	mIsPlaying = false;
}

void STimeChangeManager::CheckLocalTimeline()
{
	FBTime localTime(mSystem.LocalTime);

	mIsPlaying = mPlayerControl.IsPlaying;

	if (mIsPlaying)
	{
		mLocalTimeChanged = true;
		mLocalLastTime = localTime;

		mRemoteTimeChanged = false;
	}
	else
		if (localTime != mLocalLastTime && false == mRemoteTimeChanged)
		{
			mLocalTimeChanged = true;
			mLocalLastTime = localTime;
		}
}

// we could read from server or client data values
void STimeChangeManager::ReadFromData(const bool server, SSharedModelData& data)
{
	SPlayerInfo& playerInfo = (server) ? data.m_ServerPlayer : data.m_ClientPlayer;

	const double secs = playerInfo.m_LocalTime + mOffsetTime.GetSecondDouble();
	FBTime packet_time;
	packet_time.SetSecondDouble(secs);

	if (playerInfo.m_TimeChangedEvent > 0.0f)
	{
		playerInfo.m_TimeChangedEvent = 0.0f;

		mRemoteTimeChanged = true;
		mRemoteLastTime = packet_time;
	}
}

// write to server or client data values
void STimeChangeManager::WriteToData(const bool server, SSharedModelData& data)
{
	SPlayerInfo& playerInfo = (server) ? data.m_ServerPlayer : data.m_ClientPlayer;

	CheckLocalTimeline();

	if (!mRemoteTimeChanged && (mLocalTimeChanged || mIsPlaying))
	{
		mLocalTimeChanged = false;
		mLocalLastTime = mSystem.LocalTime;

		playerInfo.m_LocalTime = mLocalLastTime.GetSecondDouble() - mOffsetTime.GetSecondDouble();
		playerInfo.m_TimeChangedEvent = 1.0f;
	}
	else
	{
		playerInfo.m_TimeChangedEvent = 0.0f;
	}
}

void STimeChangeManager::OnUIIdle()
{
	if (mRemoteTimeChanged)
	{
		mRemoteTimeChanged = false;

		//

		FBTime curr_time(mSystem.LocalTime);

		mIsPlaying = mPlayerControl.IsPlaying;

		if (mRemoteLastTime != curr_time && false == mIsPlaying)
		{
			mPlayerControl.Goto(mRemoteLastTime);

			//
			mLocalTimeChanged = false;
			mLocalLastTime = mRemoteLastTime;
		}
	}
}

void STimeChangeManager::SetOffsetTime(const FBTime& time) 
{ 
	mOffsetTime = time; 
}

void STimeChangeManager::SetIsPlaying(const bool value) 
{ 
	mIsPlaying = value; 
}

const bool STimeChangeManager::IsRemoteTimeChanged() const 
{ 
	return mRemoteTimeChanged; 
}

const FBTime& STimeChangeManager::GetRemoteTime() const 
{ 
	return mRemoteLastTime; 
}

////////////////////////////////////////////////////////////////
// CDataChannelManager

bool CDataChannelManager::SaveModelsInfoToXML(const char* file_name)
{
	TiXmlDocument doc;

	const int numberOfModels = GetNumberOfChannels();

	TiXmlElement	head("Header");
	head.SetAttribute("numberOfModels", numberOfModels);
	head.SetAttribute("version", 1);

	for (DataChannel* pChannel : mChannels)
	{
		if (false == pChannel->mIsUsed)
			continue;

		TiXmlElement modelItem("Model");
		modelItem.SetAttribute("name", pChannel->mName);
		
		if (FBModel* pModel = pChannel->mModelTemplate->Model)
		{
			// Rotation limits
			TiXmlElement rotItem("Rotation");
			rotItem.SetAttribute("order", int(pModel->RotationOrder));
			rotItem.SetAttribute("active", (pModel->RotationActive) ? 1 : 0);
			rotItem.SetAttribute("minX", (pModel->RotationMinX) ? 1 : 0);
			rotItem.SetAttribute("minY", (pModel->RotationMinY) ? 1 : 0);
			rotItem.SetAttribute("minZ", (pModel->RotationMinZ) ? 1 : 0);
			rotItem.SetAttribute("maxX", (pModel->RotationMaxX) ? 1 : 0);
			rotItem.SetAttribute("maxY", (pModel->RotationMaxY) ? 1 : 0);
			rotItem.SetAttribute("maxZ", (pModel->RotationMaxZ) ? 1 : 0);
			
			FBVector3d r = pModel->PreRotation;

			TiXmlElement preRotItem("PreRotation");
			preRotItem.SetDoubleAttribute("x", r[0]);
			preRotItem.SetDoubleAttribute("y", r[1]);
			preRotItem.SetDoubleAttribute("z", r[2]);
			rotItem.InsertEndChild(preRotItem);

			r = pModel->PostRotation;
			TiXmlElement postRotItem("PostRotation");
			postRotItem.SetDoubleAttribute("x", r[0]);
			postRotItem.SetDoubleAttribute("y", r[1]);
			postRotItem.SetDoubleAttribute("z", r[2]);
			rotItem.InsertEndChild(postRotItem);

			r = pModel->RotationMin;
			TiXmlElement minRotItem("RotationMin");
			minRotItem.SetDoubleAttribute("x", r[0]);
			minRotItem.SetDoubleAttribute("y", r[1]);
			minRotItem.SetDoubleAttribute("z", r[2]);
			rotItem.InsertEndChild(minRotItem);

			r = pModel->RotationMax;
			TiXmlElement maxRotItem("RotationMax");
			maxRotItem.SetDoubleAttribute("x", r[0]);
			maxRotItem.SetDoubleAttribute("y", r[1]);
			maxRotItem.SetDoubleAttribute("z", r[2]);
			rotItem.InsertEndChild(maxRotItem);

			modelItem.InsertEndChild(rotItem);

			//
			// Geometric Offset
			r = pModel->GeometricTranslation;
			TiXmlElement geomTranslation("GeometricTranslation");
			geomTranslation.SetDoubleAttribute("x", r[0]);
			geomTranslation.SetDoubleAttribute("y", r[1]);
			geomTranslation.SetDoubleAttribute("z", r[2]);
			modelItem.InsertEndChild(geomTranslation);

			r = pModel->GeometricRotation;
			TiXmlElement geomRot("GeometricRotation");
			geomRot.SetDoubleAttribute("x", r[0]);
			geomRot.SetDoubleAttribute("y", r[1]);
			geomRot.SetDoubleAttribute("z", r[2]);
			modelItem.InsertEndChild(geomRot);

			r = pModel->GeometricScaling;
			TiXmlElement geomScl("GeometricScaling");
			geomScl.SetDoubleAttribute("x", r[0]);
			geomScl.SetDoubleAttribute("y", r[1]);
			geomScl.SetDoubleAttribute("z", r[2]);
			modelItem.InsertEndChild(geomScl);
		}

		head.InsertEndChild(modelItem);
	}

	doc.InsertEndChild(head);
	doc.SaveFile(file_name);

	if (doc.Error())
	{
		printf("%s", doc.ErrorDesc());
		return false;
	}
	return true;
}

bool CDataChannelManager::LoadModelsInfoFromXML(const char* file_name)
{
	TiXmlDocument	doc;

	if (doc.LoadFile(file_name) == false)
	{
		return false;
	}

	TiXmlNode *node = nullptr;
	TiXmlElement *headElement = nullptr;
	TiXmlElement *modelElement = nullptr;
	
	node = doc.FirstChild("Header");
	if (node == nullptr)
	{
		return false;
	}

	headElement = node->ToElement();
	if (headElement)
	{
		modelElement = node->FirstChildElement("Model");
	}

	for (; nullptr != modelElement; modelElement = modelElement->NextSiblingElement())
	{
		const char* model_name = modelElement->Attribute("name");

		if (nullptr == model_name)
			continue;

		for (DataChannel* pChannel : mChannels)
		{
			if (0 == strcmp(model_name, pChannel->mName) && pChannel->mIsUsed)
			{
				FBModel* pModel = pChannel->mModelTemplate->Model;

				if (TiXmlElement *element = modelElement->FirstChildElement("Rotation"))
				{
					int ivalue = 0;
					// double dvalue = 0.0;

					if (nullptr != element->Attribute("active", &ivalue))
					{
						pModel->RotationActive = (ivalue > 0) ? true : false;
					}
					if (nullptr != element->Attribute("order", &ivalue))
					{
						pModel->RotationOrder = (FBModelRotationOrder)ivalue;
					}
					if (nullptr != element->Attribute("minX", &ivalue))
						pModel->RotationMinX = static_cast<bool>(ivalue);
					if (nullptr != element->Attribute("minY", &ivalue))
						pModel->RotationMinY = static_cast<bool>(ivalue);
					if (nullptr != element->Attribute("minZ", &ivalue))
						pModel->RotationMinZ = static_cast<bool>(ivalue);

					if (nullptr != element->Attribute("maxX", &ivalue))
						pModel->RotationMaxX = static_cast<bool>(ivalue);
					if (nullptr != element->Attribute("maxY", &ivalue))
						pModel->RotationMaxY = static_cast<bool>(ivalue);
					if (nullptr != element->Attribute("maxZ", &ivalue))
						pModel->RotationMaxZ = static_cast<bool>(ivalue);

					if (TiXmlElement* child = element->FirstChildElement("PreRotation"))
					{
						FBVector3d v;
						if (nullptr != child->Attribute("x", &v[0])
							&& nullptr != child->Attribute("y", &v[1])
							&& nullptr != child->Attribute("z", &v[2]))
						{
							pModel->PreRotation = v;
						}
					}

					if (TiXmlElement* child = element->FirstChildElement("PostRotation"))
					{
						FBVector3d v;
						if (nullptr != child->Attribute("x", &v[0])
							&& nullptr != child->Attribute("y", &v[1])
							&& nullptr != child->Attribute("z", &v[2]))
						{
							pModel->PostRotation = v;
						}
					}

					if (TiXmlElement* child = element->FirstChildElement("RotationMin"))
					{
						FBVector3d v;
						if (nullptr != child->Attribute("x", &v[0])
							&& nullptr != child->Attribute("y", &v[1])
							&& nullptr != child->Attribute("z", &v[2]))
						{
							pModel->RotationMin = v;
						}
					}

					if (TiXmlElement* child = element->FirstChildElement("RotationMax"))
					{
						FBVector3d v;
						if (nullptr != child->Attribute("x", &v[0])
							&& nullptr != child->Attribute("y", &v[1])
							&& nullptr != child->Attribute("z", &v[2]))
						{
							pModel->RotationMax = v;
						}
					}
				}

				if (TiXmlElement* element = modelElement->FirstChildElement("GeometricTranslation"))
				{
					FBVector3d v;
					if (nullptr != element->Attribute("x", &v[0])
						&& nullptr != element->Attribute("y", &v[1])
						&& nullptr != element->Attribute("z", &v[2]))
					{
						pModel->GeometricTranslation = v;
					}
				}

				if (TiXmlElement* element = modelElement->FirstChildElement("GeometricRotation"))
				{
					FBVector3d v;
					if (nullptr != element->Attribute("x", &v[0])
						&& nullptr != element->Attribute("y", &v[1])
						&& nullptr != element->Attribute("z", &v[2]))
					{
						pModel->GeometricRotation = v;
					}
				}

				if (TiXmlElement* element = modelElement->FirstChildElement("GeometricScaling"))
				{
					FBVector3d v;
					if (nullptr != element->Attribute("x", &v[0])
						&& nullptr != element->Attribute("y", &v[1])
						&& nullptr != element->Attribute("z", &v[2]))
					{
						pModel->GeometricScaling = v;
					}
				}
			}
		}
	}
	
	return true;
}

bool CDataChannelManager::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat & kAttributes)
	{
		if (GetRootTemplate())
		{
			if (FBModel* pRootModel = GetRootTemplate()->Model)
			{
				pFbxObject->FieldWriteBegin("RootTransform");

				for (int i = 0; i < 16; ++i)
				{
					pFbxObject->FieldWriteD(mRootDefaultMatrix[i]);
				}

				FBString model_name(pRootModel->LongName);
				pFbxObject->FieldWriteC(model_name);

				pFbxObject->FieldWriteEnd();

				//
				pFbxObject->FieldWriteBegin("Models");

				const int count = GetNumberOfChannels();
				pFbxObject->FieldWriteI(count);

				for (int i = 0; i < count; ++i)
				{
					pFbxObject->FieldWriteC(GetChannelName(i));

					if (FBModel* pModel = GetChannelModel(i))
					{
						pFbxObject->FieldWriteC(pModel->LongName);
					}
					else
					{
						pFbxObject->FieldWriteC("None");
					}

					//
					const double* pos = mChannels[i]->GetPosition();
					const double* rot = mChannels[i]->GetRotation();
					for (int j = 0; j < 3; ++j)
					{
						pFbxObject->FieldWriteD(pos[j]);
						pFbxObject->FieldWriteD(rot[j]);
					}
				}

				pFbxObject->FieldWriteEnd();
			}
		}
	}
	return true;
}

bool CDataChannelManager::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat & kAttributes)
	{
		if (pFbxObject->FieldReadBegin("RootTransform"))
		{
			for (int i = 0; i < 16; ++i)
			{
				mRootDefaultMatrix[i] = pFbxObject->FieldReadD();
			}

			mRetrieveRootModelName = pFbxObject->FieldReadC();

			pFbxObject->FieldReadEnd();
		}

		if (pFbxObject->FieldReadBegin("Models"))
		{
			const int count = pFbxObject->FieldReadI();

			for (int i = 0; i < count; ++i)
			{
				FBString channel_name = pFbxObject->FieldReadC();
				FBString model_longname = pFbxObject->FieldReadC();

				double pos[3], rot[3];

				for (int j = 0; j < 3; ++j)
				{
					pos[j] = pFbxObject->FieldReadD();
					rot[j] = pFbxObject->FieldReadD();
				}

				for (DataChannel* pChannel : mChannels)
				{
					if (pChannel->mName == channel_name)
					{
						pChannel->SetTransform(pos, rot);
						break;
					}
				}
			}

			pFbxObject->FieldReadEnd();
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////
// CDataChannelManager

CDataChannelManager::CDataChannelManager()
{
	mDevice = nullptr;
	mHierarchyIsDefined = false;
	mRootTemplate = nullptr;
	mUseReadNodes = false;
	mRetrieveRootModelName = "";
}

//! a destructor
CDataChannelManager::~CDataChannelManager()
{
	for (DataChannel* channel : mChannels)
	{
		channel->mIsUsed = false;
	}
	EndChannelSetDefinition();
}

void CDataChannelManager::Init(FBDevice* pdevice, const char* root_name, bool use_read_nodes)
{
	mDevice = pdevice;
	if (nullptr == mRootTemplate)
	{
		mRootTemplate = new FBModelTemplate(DEVICE__PREFIX, root_name, kFBModelTemplateSkeleton);
		mHierarchyIsDefined = false;
	}
	else
	{
		mRootTemplate->Name = root_name;
	}
	
	mUseReadNodes = use_read_nodes;
}

void CDataChannelManager::Free()
{
}

/************************************************
*	Define model template hierarchy.
************************************************/
void CDataChannelManager::DefineHierarchy()
{
	if (mChannels.size() == 0)
	{
		mHierarchyIsDefined = false;
		FBTrace("hierarchy is empty\n");
	}
		

	if (!mHierarchyIsDefined && mChannels.size() > 0)
	{
		for (DataChannel* pChannel : mChannels)
		{
			mRootTemplate->Children.Add(pChannel->mModelTemplate);
		}

		mHierarchyIsDefined = true;
	}
}


/************************************************
*	Start defining the channel set.
************************************************/
void CDataChannelManager::BeginChannelSetDefinition()
{
	// Enter edit mode: tag all channels as unused.
	for (DataChannel* pChannel : mChannels)
	{
		pChannel->mIsUsed = false;
	}
}


/************************************************
*	End the channel set definition.
************************************************/
void CDataChannelManager::EndChannelSetDefinition()
{
	// Exit edit mode:
	// All used channels: if already defined, don't touch, if new: create animation node and model template
	// All unused channels: delete animation nodes and associated model template
	int count_to_remove = 0;

	for (DataChannel* pChannel : mChannels)
	{
		if (pChannel->mIsUsed)
		{
			// Create new translation and rotation animation nodes if necessary
			if (!pChannel->mTAnimNode)
			{
				// We must use a unique name for our connector.
				FBString lName(pChannel->mName, " T");
				pChannel->mTAnimNode = mDevice->AnimationNodeOutCreate(0, lName, ANIMATIONNODE_TYPE_LOCAL_TRANSLATION);
			}

			if (!pChannel->mRAnimNode)
			{
				// We must use a unique name for our connector.
				FBString lName(pChannel->mName, " R");
				pChannel->mRAnimNode = mDevice->AnimationNodeOutCreate(0, lName, ANIMATIONNODE_TYPE_LOCAL_ROTATION);
			}
			if (mUseReadNodes && !pChannel->mTFBNode)
			{
				FBString lName(pChannel->mName, " T OUT");
				pChannel->mTFBNode = mDevice->AnimationNodeInCreate(0, lName, ANIMATIONNODE_TYPE_LOCAL_TRANSLATION);
			}
			
			if (mUseReadNodes && !pChannel->mRFBNode)
			{
				FBString lName(pChannel->mName, " R OUT");
				pChannel->mRFBNode = mDevice->AnimationNodeInCreate(0, lName, ANIMATIONNODE_TYPE_LOCAL_ROTATION);
			}

			// Create new model templates
			if (!pChannel->mModelTemplate)
			{
				pChannel->mModelTemplate = new FBModelTemplate(DEVICE__PREFIX, pChannel->mName, kFBModelTemplateSkeleton);

				// Bind model template to T and R animation nodes
				pChannel->mModelTemplate->Bindings.Add(pChannel->mTAnimNode);
				pChannel->mModelTemplate->Bindings.Add(pChannel->mRAnimNode);
			}
		}
		else //Unused channels: cleanup
		{
			count_to_remove += 1;

			// Unbind model templates from T and R animation nodes
			if (pChannel->mTAnimNode)
			{
				if (pChannel->mModelTemplate)
				{
					pChannel->mModelTemplate->Bindings.Remove(pChannel->mTAnimNode);
				}
			}

			if (pChannel->mRAnimNode)
			{
				if (pChannel->mModelTemplate)
				{
					pChannel->mModelTemplate->Bindings.Remove(pChannel->mRAnimNode);
				}
			}

			// Remove as child of root template
			if (mRootTemplate->Children.Find(pChannel->mModelTemplate) >= 0)
			{
				mRootTemplate->Children.Remove(pChannel->mModelTemplate);
			}

			if (pChannel->mModelTemplate)
			{
				for (int j = 0; j < pChannel->mModelTemplate->Children.GetCount(); j++)
				{
					pChannel->mModelTemplate->Children.RemoveAt(0);
				}
			}

			// Delete model template
			delete pChannel->mModelTemplate;
			pChannel->mModelTemplate = nullptr;

			// Destroy unused animation nodes
			if (pChannel->mTAnimNode)
			{
				mDevice->AnimationNodeDestroy(pChannel->mTAnimNode);
			}

			if (pChannel->mRAnimNode)
			{
				mDevice->AnimationNodeDestroy(pChannel->mRAnimNode);
			}

			if (pChannel->mTFBNode)
			{
				mDevice->AnimationNodeDestroy(pChannel->mTFBNode);
			}
			if (pChannel->mRFBNode)
			{
				mDevice->AnimationNodeDestroy(pChannel->mRFBNode);
			}

			pChannel->mTAnimNode = nullptr;
			pChannel->mRAnimNode = nullptr;
			pChannel->mTFBNode = nullptr;
			pChannel->mRFBNode = nullptr;
		}
	}

	//Delete entry in list for all unused channels
	if (count_to_remove == static_cast<int>(mChannels.size()))
	{
		for (DataChannel* channel : mChannels)
		{
			delete channel;
		}

		mChannels.clear();
	}
	else
	{
		mChannels.erase(
			std::remove_if(begin(mChannels), end(mChannels), [](DataChannel* channel)->bool
			{
				bool need_to_remove = (!channel->mIsUsed);
				if (need_to_remove)
				{
					delete channel;
				}
				return need_to_remove;
			}),
		end(mChannels));
	}
	
	//Make sure reference number still match
	for (size_t i = 0; i < mChannels.size(); ++i)
	{
		if (mChannels[i]->mTAnimNode)
		{
			mChannels[i]->mTAnimNode->Reference = static_cast<kReference>(i);
		}
		if (mChannels[i]->mRAnimNode)
		{
			mChannels[i]->mRAnimNode->Reference = static_cast<kReference>(i);
		}
		if (mChannels[i]->mTFBNode)
		{
			mChannels[i]->mTFBNode->Reference = static_cast<kReference>(i);
		}
		if (mChannels[i]->mRFBNode)
		{
			mChannels[i]->mRFBNode->Reference = static_cast<kReference>(i);
		}
	}

	//Define hierarchy if needed
	DefineHierarchy();
}

void CDataChannelManager::UpdateDefaultTransforms()
{
	if (mRetrieveRootModelName.GetLen() == 0)
	{
		if (mRootTemplate)
		{
			if (FBModel* pModel = mRootTemplate->Model)
			{
				pModel->GetMatrix(mRootDefaultMatrix, kModelTransformation, false);
			}
		}

		for (DataChannel* pChannel : mChannels)
		{
			pChannel->AssignDefaultTransform();
		}
	}
}

void CDataChannelManager::RestoreDefaultTransforms()
{
	if (mRootTemplate)
	{
		if (FBModel* pModel = mRootTemplate->Model)
		{
			pModel->SetMatrix(mRootDefaultMatrix, kModelTransformation, false);
		}
	}

	for (DataChannel* pChannel : mChannels)
	{
		pChannel->RestoreDefaultTransform();
	}

	mRetrieveRootModelName = "";
}

/************************************************
*	Use the channel pName.
************************************************/
int CDataChannelManager::UseChannel(const char *pName)
{
	char lName[256];
	strcpy_s(lName, sizeof(char)*256, pName);

	// FB cannot deal with spaces in names replace with '_'
	for (unsigned int j = 0; j < strlen(lName); j++)
	{
		if (lName[j] == ' ') lName[j] = '_';
	}

	for (size_t i = 0; i < mChannels.size(); ++i)
	{
		if (mChannels[i]->mName == lName)
		{
			mChannels[i]->mIsUsed = true;
			return (int)i;
		}
	}

	DataChannel *newChannel = new DataChannel;
	newChannel->mName = lName;
	newChannel->mIsUsed = true;
	mChannels.push_back(newChannel);

	return (int)(mChannels.size() - 1);
}

bool CDataChannelManager::ReadInputData(const int Index, double* Pos, double* Rot, FBEvaluateInfo* pEvaluateInfo)
{
	if (mChannels[Index]->mTFBNode && mChannels[Index]->mRFBNode)
	{
		mChannels[Index]->mTFBNode->ReadData(Pos, pEvaluateInfo);
		mChannels[Index]->mRFBNode->ReadData(Rot, pEvaluateInfo);
		return true;
	}
	return false;
}

bool CDataChannelManager::ReadOutputData(const int Index, double* Pos, double* Rot, FBEvaluateInfo* pEvaluateInfo)
{
	if (mChannels[Index]->mTAnimNode && mChannels[Index]->mRAnimNode)
	{
		mChannels[Index]->mTAnimNode->ReadData(Pos, pEvaluateInfo);
		mChannels[Index]->mRAnimNode->ReadData(Rot, pEvaluateInfo);
		return true;
	}
	return false;
}

bool CDataChannelManager::WriteData(const int Index, const double* Pos, const double* Rot, FBEvaluateInfo* pEvaluateInfo)
{
	if (mChannels[Index]->mTAnimNode && mChannels[Index]->mRAnimNode)
	{
		mChannels[Index]->mTAnimNode->WriteData((double*)Pos, pEvaluateInfo);
		mChannels[Index]->mRAnimNode->WriteData((double*)Rot, pEvaluateInfo);
		return true;
	}
	return false;
}

bool CDataChannelManager::RecordData(const int index, const double* pos, const double* rot, FBTime* time)
{
	DataChannel* pChannel = mChannels[index];

	if (pChannel->mIsUsed)
	{
		// Translation information.
		if (pChannel->mTAnimNode)
		{
			if (FBAnimationNode* Data = pChannel->mTAnimNode->GetAnimationToRecord())
			{
				if (nullptr == time)
				{
					Data->KeyAdd((double*)pos);
				}
				else
				{
					Data->KeyAdd(*time, (double*)pos);
				}

			}
		}

		//
		if (pChannel->mRAnimNode)
		{
			if (FBAnimationNode* Data = pChannel->mRAnimNode->GetAnimationToRecord())
			{
				if (nullptr == time)
				{
					Data->KeyAdd((double*)rot);
				}
				else
				{
					Data->KeyAdd(*time, (double*)rot);
				}

			}
		}
	}

	return true;
}

bool ExportTargetAnimation(FBModel* pFacialRoot, FBModel* pRoot, FBModel* pLeft, FBModel* pRight)
{
	if (nullptr == pFacialRoot || nullptr == pRoot || 0 == pLeft || 0 == pRight)
	{
		FBMessageBox("Client Device", "We have nothing to sync!", "Ok");
		return false;
	}

	bool lResult = false;

	FBSystem			lSystem;
	FBApplication		lApp;

	FBString full_name_root(LOOKAT_RIG_NS, ":");
	full_name_root = full_name_root + LOOKAT_CTRL_ROOT;

	FBString full_name_left(LOOKAT_RIG_NS, ":");
	full_name_left = full_name_left + LOOKAT_CTRL_LEFT;

	FBString full_name_right(LOOKAT_RIG_NS, ":");
	full_name_right = full_name_right + LOOKAT_CTRL_RIGHT;

	FBModel* obj_root = FBFindModelByLabelName(full_name_root);
	if (obj_root != nullptr)
		obj_root->LongName = FBString(full_name_root, "_temp_temp");
	FBModel* obj_left = FBFindModelByLabelName(full_name_left);
	if (obj_left != nullptr)
		obj_left->LongName = FBString(full_name_left, "_temp_temp");
	FBModel* obj_right = FBFindModelByLabelName(full_name_right);
	if (obj_right != nullptr)
		obj_right->LongName = FBString(full_name_right, "_temp_temp");

	FBModelNull*	tmpRoot = new FBModelNull(full_name_root);
	tmpRoot->Show = true;
	tmpRoot->Parent = pFacialRoot;

	FBModelNull*	tmpLeft = new FBModelNull(full_name_left);
	tmpLeft->Show = true;
	tmpLeft->Parent = tmpRoot;

	FBModelNull*	tmpRight = new FBModelNull(full_name_right);
	tmpRight->Show = true;
	tmpRight->Parent = tmpRoot;

	FBConstraint* constraint = FBConstraintManager::TheOne().TypeCreateConstraint("Parent/Child");
	
	if (constraint) // && constraintL && constraintR)
	{
		
		constraint->ReferenceAdd(0, tmpRoot);
		constraint->ReferenceAdd(1, pRoot);
		constraint->Lock = true;
		constraint->Active = true;

		FBTime lPeriod(0, 0, 0, 1);
		FBArrayTemplate<FBBox*>	objs;
		objs.Add(tmpRoot);

		FBTake* currTake = lSystem.CurrentTake;
		currTake->PlotTakeOnObjects(lPeriod, &objs);

		constraint->Active = false;

		tmpRoot->Scaling = pRoot->Scaling;

		FBFbxOptions	lOptions(false);
		lOptions.SaveSelectedModelsOnly = true;
		lOptions.SetAll(kFBElementActionDiscard, false);
		lOptions.Models = kFBElementActionSave;
		lOptions.ModelsAnimation = true;
		lOptions.UpdateRecentFiles = false;

		for (int i = 0, count = lOptions.GetTakeCount(); i < count; ++i)
		{
			if (0 == strcmp(currTake->LongName, lOptions.GetTakeName(i)))
			{
				lOptions.SetTakeSelect(i, true);
				lOptions.SetTakeDestinationName(i, "Take 001");
			}
			else
			{
				lOptions.SetTakeSelect(i, false);
			}
		}

		tmpRoot->Selected = true;
		tmpLeft->Selected = true;
		tmpRight->Selected = true;

		FBString filename(lSystem.TempPath, SHARED_SYNC_TEMP_FILE);
		lApp.FileSave(filename, &lOptions);

		lResult = true;

		constraint->FBDelete();
	}

	tmpRoot->FBDelete();
	tmpLeft->FBDelete();
	tmpRight->FBDelete();

	if (obj_root != nullptr)
		obj_root->LongName = full_name_root;
	if (obj_left != nullptr)
		obj_left->LongName = full_name_left;
	if (obj_right != nullptr)
		obj_right->LongName = full_name_right;

	return lResult;
}

bool IsFileExists(const char *filename) {

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(filename, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		FindClose(hFind);
		return true;
	}
}

bool ImportTargetAnimation(FBModel* pFacialRoot, FBModel* pRoot, FBModel* pLeft, FBModel* pRight)
{
	FBSystem		lSystem;
	FBApplication	lApp;

	// DONE: check if sync file is exist

	FBString filename(lSystem.TempPath, SHARED_SYNC_TEMP_FILE);

	if (false == IsFileExists(filename))
	{
		FBMessageBox("Server Device", "Sync file is not found!", "Ok");
		return false;
	}

	if (nullptr == pRoot || nullptr == pLeft || nullptr == pRight)
	{
		FBMessageBox("Server Device", "Couldnt locate a LookAtRoot object in the scene", "Ok");
		return false;
	}

	pRoot->Parent = pFacialRoot;

	FBString temp_root_name = pRoot->LongName;
	FBString temp_left_name = pLeft->LongName;
	FBString temp_right_name = pRight->LongName;

	FBString full_name_root(LOOKAT_RIG_NS, ":");
	full_name_root = full_name_root + LOOKAT_CTRL_ROOT;

	FBString full_name_left(LOOKAT_RIG_NS, ":");
	full_name_left = full_name_left + LOOKAT_CTRL_LEFT;

	FBString full_name_right(LOOKAT_RIG_NS, ":");
	full_name_right = full_name_right + LOOKAT_CTRL_RIGHT;

	pRoot->LongName = full_name_root;
	pLeft->LongName = full_name_left;
	pRight->LongName = full_name_right;

	// DONE: merge sync file with a current take and LookAtCtrl model
	FBFbxOptions	lOptions(true, filename);
	lOptions.SetAll(kFBElementActionDiscard, false);
	lOptions.Models = kFBElementActionDiscard;
	lOptions.ModelsAnimation = true;
	lOptions.UpdateRecentFiles = false;
	for (int i = 0, count = lOptions.GetTakeCount(); i < count; ++i)
	{
		lOptions.SetTakeSelect(i, false);
	}

	FBTake* currTake = lSystem.CurrentTake;

	lOptions.SetTakeSelect(0, true);
	lOptions.SetTakeDestinationName(0, currTake->LongName);

	bool lResult = lApp.FileMerge(filename, false, &lOptions);

	pRoot->LongName = temp_root_name;
	pLeft->LongName = temp_left_name;
	pRight->LongName = temp_right_name;

	return lResult;
}
