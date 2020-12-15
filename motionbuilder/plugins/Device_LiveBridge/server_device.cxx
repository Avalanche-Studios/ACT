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
/**	\file	server_device.cxx
*	
*	Function definitions of the CServerDeivce class.
*
*/

//--- Class declaration
#include "server_device.h"
#include "AnimLiveBridge/AnimLiveBridge.h"

//--- Device strings
#define SERVERDEVICE__CLASS		SERVERDEVICE__CLASSNAME
#define SERVERDEVICE__NAME		SERVERDEVICE__CLASSSTR
#define SERVERDEVICE__LABEL		"Bridge Server"
#define SERVERDEVICE__DESC		"Bridge Server Device"
#define SERVERDEVICE__PREFIX	"BridgeServer"

//--- FiLMBOX implementation and registration
FBDeviceImplementation	(SERVERDEVICE__CLASS	);
FBRegisterDevice		(SERVERDEVICE__NAME,
						SERVERDEVICE__CLASS,
						SERVERDEVICE__LABEL,
						SERVERDEVICE__DESC,
							FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/

void CServerDevice::ActionImportTarget(HIObject object, bool value)
{
	CServerDevice *pBase = FBCast<CServerDevice>(object);
	if (pBase && value)
	{
		pBase->DoImportTarget();
	}
}

void CServerDevice::ActionExportTarget(HIObject object, bool value)
{
	CServerDevice *pBase = FBCast<CServerDevice>(object);
	if (pBase && value)
	{
		pBase->DoExportTarget();
	}
}

void CServerDevice::ActionSaveRotationSetup(HIObject object, bool value)
{
	CServerDevice *pBase = FBCast<CServerDevice>(object);
	if (pBase && value)
	{
		pBase->DoSaveRotationSetup();
	}
}

bool CServerDevice::FBCreate()
{

	FBPropertyPublish(this, PairName, "Pair Name", nullptr, nullptr);
	PairName = SHARED_MAPPING_DEFNAME;

	FBPropertyPublish(this, SaveRotationSetup, "Save Rotation Setup", nullptr, ActionSaveRotationSetup);

	FBPropertyPublish(this, RotationIncludeDOF, "Rotation Include DOF", nullptr, nullptr);

	FBPropertyPublish(this, LookAtRoot, "LookAt Root", nullptr, nullptr);
	FBPropertyPublish(this, LookAtLeft, "LookAt Left", nullptr, nullptr);
	FBPropertyPublish(this, LookAtRight, "LookAt Right", nullptr, nullptr);
	FBPropertyPublish(this, ImportTarget, "Import Target", nullptr, ActionImportTarget);
	FBPropertyPublish(this, ExportTarget, "Export Target", nullptr, ActionExportTarget);

	RotationIncludeDOF = false;

	//
	LookAtRoot.SetSingleConnect(true);
	//LookAtRoot.SetFilter(FBModel::GetInternalClassId());
	LookAtLeft.SetSingleConnect(true);
	//LookAtLeft.SetFilter(FBModel::GetInternalClassId());
	LookAtRight.SetSingleConnect(true);
	//LookAtRight.SetFilter(FBModel::GetInternalClassId());

	//
	mLookAtRootNode = AnimationNodeInCreate(101, "LookAtRoot", ANIMATIONNODE_TYPE_LOCAL_TRANSLATION);
	mLookAtLeftNode = AnimationNodeInCreate(102, "LookAtLeft", ANIMATIONNODE_TYPE_LOCAL_TRANSLATION);
	mLookAtRightNode = AnimationNodeInCreate(103, "LookAtRight", ANIMATIONNODE_TYPE_LOCAL_TRANSLATION);

	//

	ChangeTemplateDefenition();

	mExportRate		= 30.0;
	mNeedSaveXML = 0;
	mUpdateInitTransform = false;

	mSystem.OnUIIdle.Add(this, (FBCallback)&CServerDevice::EventUIIdle);

	return true;
}

void CServerDevice::ChangeTemplateDefenition()
{
	using namespace NShared;
	// DONE: remove old definition and replace with a new one

	if (mDataChannels.GetNumberOfChannels() > 0)
	{
		mDataChannels.BeginChannelSetDefinition();
		mDataChannels.EndChannelSetDefinition();

		ModelTemplate.Children.Remove(mDataChannels.GetRootTemplate());
		mDataChannels.Free();
	}

	mDataChannels.Init(this, GetRootName(), true);
	ModelTemplate.Children.Add(mDataChannels.GetRootTemplate());

	const int numberOfJoints = GetSetJointsCount();

	mDataChannels.BeginChannelSetDefinition();
	for (int i = 0; i < numberOfJoints; i++)
	{
		const char* joint_name = GetTestJointName(i);
		mDataChannels.UseChannel(joint_name);
	}
	mDataChannels.EndChannelSetDefinition();
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void CServerDevice::FBDestroy()
{
	mSystem.OnUIIdle.Remove(this, (FBCallback)&CServerDevice::EventUIIdle);
}


/************************************************
 *	Device operation.
 ************************************************/
bool CServerDevice::DeviceOperation( kDeviceOperations pOperation )
{
	switch (pOperation)
	{
		case kOpInit:	return Init();
		case kOpStart:	return Start();
		case kOpStop:	return Stop();
		case kOpReset:	return Reset();
		case kOpDone:	return Done();
	}
	return FBDevice::DeviceOperation( pOperation );
}


/************************************************
 *	Initialization of device.
 ************************************************/
bool CServerDevice::Init()
{
	return true;
}


/************************************************
 *	Device is put online.
 ************************************************/
bool CServerDevice::Start()
{
	using namespace NShared;
	FBProgress	lProgress;
	lProgress.Caption	= "Setting up device";

	const int numberOfJoints = GetSetJointsCount();

	mHardware.SetNumberOfActiveModels(numberOfJoints);
	mHardware.SetNumberOfActiveProperties(0);

	// store model hashes
	for (int i = 0; i < numberOfJoints; ++i)
	{
		mHardware.SetModelName(i, GetTestJointName(i));
	}

	// Step 1: Open device
	lProgress.Text	= "Opening communication to device";
	Information		= "Opening device";
	if(!mHardware.Open(PairName))
	{
		Information = "Could not open device";
		return false;
	}

	// Step 2: Ask hardware to get channel information
	lProgress.Text	= "Device found, scanning for channel information...";
	Information		= "Retrieving channel information";
	if(!mHardware.GetSetupInfo())
	{
		Information = "Could not get channel information from device.";
		return false;
	}

	// Step 3: Create FB channel list and setup hierarchy
	

	// Step 3: Set the sampling rate
	lProgress.Text	= "Setting sampling rate";
	Information	= "Setting sampling rate";

	FBTime	lTime;
	lTime.SetSecondDouble( 1.0 / mExportRate );
	SamplingPeriod	= lTime;

	
	mNeedSaveXML = 1;
	
	return true;
}


/************************************************
 *	Device is stopped (offline).
 ************************************************/
bool CServerDevice::Stop()
{
	//
	FBProgress	lProgress;
	lProgress.Caption	= "Shutting down device";

	// Shutdown device
	lProgress.Text	= "Closing communication to device";
	Information		= "Closing device";
	if(! mHardware.Close() )
	{
		Information = "Could not close device";
	}

    return false;
}


/************************************************
 *	Removal of device.
 ************************************************/
bool CServerDevice::Done()
{
	return false;
}


/************************************************
 *	Reset of device.
 ************************************************/
bool CServerDevice::Reset()
{
    Stop();
    return Start();
}


/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool CServerDevice::AnimationNodeNotify(FBAnimationNode* pAnimationNode ,FBEvaluateInfo* pEvaluateInfo)
{
	
	FBTVector	Pos(0.0, 0.0, 0.0, 1.0);
	FBVector3d* pV = (FBVector3d*)&Pos;
	FBRVector	Rot;
	FBSVector	Scal(1.0, 1.0, 1.0);
	FBMatrix	GlobalNodeTransformation, GlobalReferenceTransformation;
	FBMatrix	LocalNodeTransformation;

	//
	//
	if (nullptr != mLookAtRootNode)
	{
		mLookAtRootNode->WriteData(mLookAtRootPos, pEvaluateInfo);
	}

	if (nullptr != mLookAtLeftNode)
	{
		mLookAtLeftNode->WriteData(mLookAtLeftPos, pEvaluateInfo);
	}

	if (nullptr != mLookAtRightNode)
	{
		mLookAtRightNode->WriteData(mLookAtRightPos, pEvaluateInfo);
	}

	//
	//

	if (mDataChannels.GetRootTemplate())
	{
		if (FBModel* pRootModel = mDataChannels.GetRootTemplate()->Model)
		{
			pRootModel->GetMatrix(GlobalReferenceTransformation, kModelTransformation, true, pEvaluateInfo);
		}
	}
	
	const int numberOfJoints = NShared::GetSetJointsCount();

	for (int i = 0; i < numberOfJoints; ++i)
	{
		
		if (true == mDataChannels.IsChannelUsed(i))
		{
			mDataChannels.ReadInputData(i, Pos, Rot, pEvaluateInfo);

			FBTRSToMatrix(GlobalNodeTransformation, Pos, Rot, Scal);
			FBGetLocalMatrix(LocalNodeTransformation, GlobalReferenceTransformation, GlobalNodeTransformation);

			FBMatrixToTranslation(Pos, LocalNodeTransformation);
			FBMatrixToRotation(Rot, LocalNodeTransformation);

			if (FBModel* pModel = mDataChannels.GetChannelModel(i))
			{
				pModel->GetVector(*pV, kModelTranslation, false, pEvaluateInfo);
				pModel->GetVector(Rot, kModelRotation, false, pEvaluateInfo);
			}

			mDataChannels.WriteData(i, Pos, Rot, pEvaluateInfo);
		}
	}
	
	return ParentClass::AnimationNodeNotify(pAnimationNode, pEvaluateInfo);
}


/************************************************
 *	Device Evaluation Notify.
 ************************************************/
void CServerDevice::ComputeFullRotationMatrix(FBMatrix& tm, FBEvaluateInfo* pEvaluateInfo, const int index)
{
	if (FBModel* pModel = mDataChannels.GetChannelModel(index))
	{
		FBMatrix pretm;
		pModel->GetMatrix(tm, kModelTransformation, false, pEvaluateInfo);

		FBTVector v;
		FBRVector r;
		FBSVector s;

		{
			// world scene pre rotation
			FBRotationToMatrix(pretm, FBRVector(0.0, 90.0, 0.0));
			FBMatrixMult(tm, pretm, tm);

			pModel->GetMatrix(tm, kModelTransformation, true, pEvaluateInfo);
		}

		// model pre rotation
		FBVector3d preRotation = pModel->PreRotation;
		FBRotationToMatrix(pretm, preRotation);
		
		if (FBModel* parent = pModel->Parent)
		{
			FBMatrix parent_tm;
			parent->GetMatrix(parent_tm, kModelTransformation, true, pEvaluateInfo);

			FBGetLocalMatrix(tm, parent_tm, tm);
		}
	}
}

bool CServerDevice::DeviceEvaluationNotify(kTransportMode pMode, FBEvaluateInfo* pEvaluateInfo)
{
	using namespace NShared;
	
	switch(pMode)
	{
		case kPreparePlay:
		case kPlayReady:
		case kPlayStop:
		case kJog:
		{
		}
		break;
		case kStop:
		case kPlay:
		{
			double lPos[3];
			double lRot[3];

			const int numberOfJoints = GetSetJointsCount();

			for (int i = 0; i < numberOfJoints; ++i)
			{
				mDataChannels.ReadOutputData(i, lPos, lRot, pEvaluateInfo);

				if (false) // JointSet == eJointSet_Body
				{
					if (FBModel* pModel = mDataChannels.GetChannelModel(i))
					{
						FBMatrix pretm, tm;
						pModel->GetMatrix(tm, kModelTransformation, false, pEvaluateInfo);

						FBTVector v;
						FBRVector r;
						FBSVector s;

						//if (i == 1)
						{
							// world scene pre rotation
							FBRotationToMatrix(pretm, FBRVector(0.0, 90.0, 0.0));
							FBMatrixMult(tm, pretm, tm);

							pModel->GetMatrix(tm, kModelTransformation, true, pEvaluateInfo);
						}

						// model pre rotation
						FBVector3d preRotation = pModel->PreRotation;
						FBRotationToMatrix(pretm, preRotation);
						//FBMatrixMult(tm, pretm, tm);

						FBMatrixToTRS(v, r, s, tm);

						mHardware.WritePos(i, v);
						mHardware.WriteRot(i, r);
						mHardware.WriteMatrix(i, tm);
					}
				}
				else
				{
					mHardware.WritePos(i, lPos);
					mHardware.WriteRot(i, lRot);

					if (RotationIncludeDOF)
					{
						FBMatrix tm;
						ComputeFullRotationMatrix(tm, pEvaluateInfo, i);
						mHardware.WriteMatrix(i, tm);
					}
				}
			}
			
			AckOneSampleReceived();
		}
		break;
	}

	return true;
}


/************************************************
 *	Real-Time Synchronous Device IO.
 ************************************************/
void CServerDevice::DeviceIONotify( kDeviceIOs  pAction,FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	FBTime lEvalTime(pDeviceNotifyInfo.GetLocalTime());
    switch (pAction)
	{
		// Output devices
		case kIOPlayModeWrite:
		case kIOStopModeWrite:
		{
			const bool is_playing = mPlayerControl.IsPlaying; //  pDeviceNotifyInfo.GetEvaluateInfo().IsStop();
			mHardware.GetTimelineSync()->SetLocalTime(lEvalTime.GetSecondDouble());
			mHardware.GetTimelineSync()->SetIsPlaying(is_playing);

			mHardware.GetTimelineSync()->SetLocalTimeline(lEvalTime.GetSecondDouble(), is_playing);

			//lEvalTime = pDeviceNotifyInfo.GetSystemTime();
			mHardware.SendDataPacket( lEvalTime );
			
			if (mHardware.GetTimelineSync()->IsRemoteTimeChanged())
			{
				FBTime remote_time;
				remote_time.SetSecondDouble(mHardware.GetTimelineSync()->GetRemoteTime());
				pDeviceNotifyInfo.SetLocalTime(remote_time);

				mHardware.SendDataPacket(lEvalTime);
				//mHardware.ResetTimeChange();
			}

			mLookAtRootPos = mHardware.GetLookAtRootPos();
			if (LookAtRoot.GetCount() > 0)
			{
				FBModel* pModel = (FBModel*)LookAtRoot.GetAt(0);
				pModel->Translation.SetCandidate(mLookAtRootPos, sizeof(double) * 3);
			}

			mLookAtLeftPos = mHardware.GetLookAtLeftPos();
			if (LookAtLeft.GetCount() > 0)
			{
				FBModel* pModel = (FBModel*)LookAtLeft.GetAt(0);
				pModel->Translation.SetCandidate(mLookAtLeftPos, sizeof(double) * 3);
			}

			mLookAtRightPos = mHardware.GetLookAtRightPos();
			if (LookAtRight.GetCount() > 0)
			{
				FBModel* pModel = (FBModel*)LookAtRight.GetAt(0);
				pModel->Translation.SetCandidate(mLookAtRightPos, sizeof(double) * 3);
			}
		}
		break;
		// Input devices
		case kIOStopModeRead:
		case kIOPlayModeRead:
		{
		}
		break;
	}
}


//--- FBX load/save tags
#define FBX_TAG_SECTION		SERVERDEVICE__CLASSSTR
#define FBX_TAG_SERIALPORT	"SerialPort"

/************************************************
 *	Store data in FBX.
 ************************************************/
bool CServerDevice::FbxStore(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
	mDataChannels.FbxStore(pFbxObject, pStoreWhat);
	return ParentClass::FbxStore(pFbxObject, pStoreWhat);
}


/************************************************
 *	Retrieve data from FBX.
 ************************************************/
bool CServerDevice::FbxRetrieve(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
	mDataChannels.FbxRetrieve(pFbxObject, pStoreWhat);
	return ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);
}

void CServerDevice::EventUIIdle(HISender pSender, HKEvent pEvent)
{
	FBTime curr_time(mSystem.LocalTime);

	if (mUpdateInitTransform)
	{
		mUpdateInitTransform = false;
		mDataChannels.RestoreDefaultTransforms();
	}

	if (mHardware.HasNewSync())
	{
		DoImportTarget();
	}

	if (Online && Live)
	{
		if (mNeedSaveXML > 0)
		{
			mNeedSaveXML += 1;

			if (30 == mNeedSaveXML)
			{
				FBString path(mSystem.UserConfigPath, "\\");
				path = path + PairName + ".xml";
				if (false == mDataChannels.SaveModelsInfoToXML(path))
				{
					FBMessageBox("Server Device", "Failed to save models info into xml", "Ok");
				}
				mNeedSaveXML = 0;
			}
		}

		double remote_time{ 0.0 };
		FBTime local_time = mSystem.LocalTime;
		const bool is_playing = mPlayerControl.IsPlaying;

		if (mHardware.GetTimelineSync()->CheckForARemoteTimeControl(remote_time, local_time.GetSecondDouble(), is_playing))
		{
			local_time.SetSecondDouble(remote_time);
			mPlayerControl.Goto(local_time);
		}
	}
}

bool CServerDevice::ModelTemplateBindNotify(FBModel* pModel, int pIndex, FBModelTemplate* pModelTemplate)
{
	mDataChannels.UpdateDefaultTransforms();
	mUpdateInitTransform = true;

	return ParentClass::ModelTemplateBindNotify(pModel, pIndex, pModelTemplate);
}

void CServerDevice::DoImportTarget()
{
	if (0 == LookAtRoot.GetCount() || nullptr == mDataChannels.GetRootTemplate()->Model
		|| 0 == LookAtLeft.GetCount() || 0 == LookAtRight.GetCount())
	{
		FBMessageBox("Server Device", "We have nothing to sync!", "Ok");
		return;
	}

	ImportTargetAnimation(mDataChannels.GetRootTemplate()->Model, (FBModel*)LookAtRoot.GetAt(0), (FBModel*)LookAtLeft.GetAt(0), (FBModel*)LookAtRight.GetAt(0));
}

void CServerDevice::DoExportTarget()
{
	if (0 == LookAtRoot.GetCount() || nullptr == mDataChannels.GetRootTemplate()->Model
		|| 0 == LookAtLeft.GetCount() || 0 == LookAtRight.GetCount())
	{
		FBMessageBox("Server Device", "We have nothing to sync!", "Ok");
		return;
	}

	FBModel* pModel = (FBModel*)LookAtRoot.GetAt(0);
	FBModel* pRootModel = pModel->Parent;

	ExportTargetAnimation(pRootModel, pModel, (FBModel*)LookAtLeft.GetAt(0), (FBModel*)LookAtRight.GetAt(0));

	mHardware.SyncSaved();
}

void CServerDevice::DoSaveRotationSetup()
{
	mNeedSaveXML = 1;
}

bool CServerDevice::PlugDataNotify(FBConnectionAction pAction, FBPlug* pThis, void* pData, void* pDataOld, int pDataSize)
{
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool CServerDevice::ImportJointSet(const char* filename)
{
	if (NShared::ImportJointSet(filename))
	{
		ChangeTemplateDefenition();
		return true;
	}
	return false;
}

void CServerDevice::ExportJointSet(const char* filename)
{
	NShared::ExportJointSet(filename);
}