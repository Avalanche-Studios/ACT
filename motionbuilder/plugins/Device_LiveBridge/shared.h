
#pragma once

/*
#
# Copyright(c) 2021 Avalanche Studios.All rights reserved.
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

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <vector>


#define SHARED_MAPPING_DEFNAME	"AnimationBridgePair"

#define SHARED_SYNC_TEMP_FILE	"\\bridge_syncTarget.fbx"

#define LOOKAT_RIG_NS			"LookAtNS"
#define LOOKAT_CTRL_ROOT		"LookAtRoot"
#define LOOKAT_CTRL_LEFT		"LookAtCtrlLeft"
#define LOOKAT_CTRL_RIGHT		"LookAtCtrlRight"

// TODO:
//namespace NAnimationLiveBridge
//{
	struct SSharedModelData;
	struct STimelineSyncManager;
//};


///////////////////////////////////////////////////////////////
////! Data channel class.
class DataChannel
{
public:
	//! Constructor.
	DataChannel()
	{
		Clear(true);
	}
	DataChannel(const char *name)
		: mName(name)
	{
		Clear(false);
	}

	//! Destructor.
	~DataChannel()
	{
	}

	const bool IsAssigned() {
		return (nullptr != mTAnimNode && nullptr != mRAnimNode && nullptr != mTFBNode && nullptr != mRFBNode && nullptr != mModelTemplate);
	}
	void Clear(bool clear_name = false) {
		mTAnimNode = nullptr;
		mRAnimNode = nullptr;
		mTFBNode = nullptr;
		mRFBNode = nullptr;
		mModelTemplate = nullptr;

		if (clear_name)
		{
			mName = "NoName";
		}
		mIsUsed = false;
	}

public:
	FBString			mName;				//!< Name of marker as displayed in the spreadsheet.
	FBAnimationNode*	mTAnimNode;			//!< Position animation node.
	FBAnimationNode*	mRAnimNode;			//!< Rotation animation node.
	FBAnimationNode*		mTFBNode;		//!< input animation node (translation).
	FBAnimationNode*		mRFBNode;		//!< input animation node (rotation).
	FBModelTemplate*	mModelTemplate;		//!< Marker model template driven by the data channel.
	bool				mIsUsed;			//!< Set to true to confirm presence of this sensor/marker.

	void AssignDefaultTransform()
	{
		if (mModelTemplate)
		{
			if (FBModel* pModel = mModelTemplate->Model)
			{
				FBVector3d v;
				pModel->GetVector(v, kModelTranslation, false);
				memcpy(mPos, v, sizeof(double) * 3);
				pModel->GetVector(v, kModelRotation, false);
				memcpy(mRot, v, sizeof(double) * 3);
			}
		}
	}

	void RestoreDefaultTransform()
	{
		if (mModelTemplate)
		{
			if (FBModel* pModel = mModelTemplate->Model)
			{
				pModel->SetVector(FBVector3d(mPos), kModelTranslation, false);
				pModel->SetVector(FBVector3d(mRot), kModelRotation, false);
			}
		}
	}

	const double* GetPosition() const { return mPos; }
	const double* GetRotation() const { return mRot; }

	void SetTransform(const double* pos, const double* rot)
	{
		memcpy(mPos, pos, sizeof(double) * 3);
		memcpy(mRot, rot, sizeof(double) * 3);
	}

private:
	double			mPos[3];				//!< Position array.
	double			mRot[3];				//!< Rotation array.
};

////////////////////////////////////////////////////////////////////////////
// CDataChannelManager

class CDataChannelManager
{
public:
	//! a constructor
	CDataChannelManager();

	//! a destructor
	~CDataChannelManager();

	// should we create out animation nodes for template models (Server device)
	void Init(FBDevice* pdevice, const char* root_name, bool use_read_nodes);

	void Free();

	//--- Marker set management.
	void	DefineHierarchy();				//!< Define model template hierarchy.
	void	BeginChannelSetDefinition();				//!< Begin defining channel set.
	void	EndChannelSetDefinition();			//!< End channel set definition.
	int		UseChannel(const char *pName);	//!< Flag a channel for use.

	void	UpdateDefaultTransforms();
	void    RestoreDefaultTransforms();

	FBModelTemplate*		GetRootTemplate() { return mRootTemplate; }

	void SetHierarchyIsDefined(const bool value) { mHierarchyIsDefined = value; }

	bool ReadInputData(const int index, double* pos, double* rot, FBEvaluateInfo* pInfo);
	bool ReadOutputData(const int index, double* pos, double* rot, FBEvaluateInfo* pInfo);
	// write to output nodes
	bool WriteData(const int index, const double* pos, const double* rot, FBEvaluateInfo* pInfo);
	bool RecordData(const int index, const double* pos, const double* rot, FBTime* time=nullptr);

	const int GetNumberOfChannels() const { return (int)mChannels.size(); }
	const char* GetChannelName(const int index) const { return mChannels[index]->mName; }
	const bool IsChannelUsed(const int index) const { 
		if (index < 0 || index >= (int)mChannels.size())
			return false;

		return mChannels[index]->mIsUsed; 
	}
	FBModel* GetChannelModel(const int index) const {
		return mChannels[index]->mModelTemplate->Model;
	}

	void GetChannelDefault(const int index, double* pos, double* rot) {
		const double* def_pos = mChannels[index]->GetPosition();
		const double* def_rot = mChannels[index]->GetRotation();

		memcpy(pos, def_pos, sizeof(double) * 3);
		memcpy(rot, def_rot, sizeof(double) * 3);
	}

	//
	// store / retrieve information about pre/post rotation, etc. for models

	bool SaveModelsInfoToXML(const char* file_name);
	bool LoadModelsInfoFromXML(const char* file_name);

	//
	//--- Load/Save initial pose\

	bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);		//!< Store configuration in FBX.
	bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);		//!< Retrieve configuration from FBX.


protected:
	FBDevice*						mDevice;

	FBModelTemplate*				mRootTemplate;		//!< Root model binding.
	std::vector<DataChannel*>		mChannels;

	// default root transform
	FBMatrix				mRootDefaultMatrix;
	FBString				mRetrieveRootModelName;

	bool					mUseReadNodes;
	bool					mHierarchyIsDefined;		//!< True if the hierarchy is already defined
};


//////////////////////////////////////////////////////////////
// STimeChangeManager

struct STimeChangeManager
{
public:
	//! a constructor
	STimeChangeManager();

	void CheckLocalTimeline();

	// we could read from server or client data values
	void ReadFromData(const bool server, SSharedModelData& data);

	// write to server or client data values
	void WriteToData(const bool server, SSharedModelData& data);

	void OnUIIdle();

	void SetOffsetTime(const FBTime& time);
	void SetIsPlaying(const bool value);

	const bool IsRemoteTimeChanged() const;
	const FBTime& GetRemoteTime() const;

protected:

	FBSystem			mSystem;
	FBPlayerControl		mPlayerControl;

	bool		mIsPlaying;

	FBTime      mOffsetTime;

	bool		mLocalTimeChanged;
	FBTime      mLocalLastTime;

	bool		mRemoteTimeChanged;
	FBTime      mRemoteLastTime;
};

//
bool ExportTargetAnimation(FBModel* pFacialRoot, FBModel* pRoot, FBModel* pLeft, FBModel* pRight);
bool ImportTargetAnimation(FBModel* pFacialRoot, FBModel* pRoot, FBModel* pLeft, FBModel* pRight);

// test joint set

namespace NShared
{
	const char* GetRootName();
	const int GetSetJointsCount();
	const char* GetTestJointName(const int index);

	bool ImportJointSet(const char* filename);
	bool ExportJointSet(const char* filename);

	bool StoreJointSet(FBFbxObject* pFbxObject);
	bool RetrieveJointSet(FBFbxObject* pFbxObject);
};

