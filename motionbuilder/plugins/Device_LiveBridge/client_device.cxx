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
/**	\file	client_device.cxx
*	Definition of a client input device class.
*
*/

//--- Class declaration
#include "client_device.h"
#include "shared.h"
#include "AnimLiveBridge/AnimLiveBridge.h"
#include <algorithm>

//--- Registration defines
#define CLIENTDEVICE__CLASS		CLIENTDEVICE__CLASSNAME
#define CLIENTDEVICE__NAME		CLIENTDEVICE__CLASSSTR
#define CLIENTDEVICE__LABEL		"Bridge Client"
#define CLIENTDEVICE__DESC		"Bridge Client Device"
#define CLIENTDEVICE__PREFIX		"BridgeClient"

//--- FiLMBOX implementation and registration
FBDeviceImplementation	(CLIENTDEVICE__CLASS	);
FBRegisterDevice		(CLIENTDEVICE__NAME,
							CLIENTDEVICE__CLASS,
							CLIENTDEVICE__LABEL,
							CLIENTDEVICE__DESC,
							FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/

void CClientDevice::ActionImportTarget(HIObject object, bool value)
{
	CClientDevice *pBase = FBCast<CClientDevice>(object);
	if (pBase && value)
	{
		pBase->DoImportTarget();
	}
}

void CClientDevice::ActionExportTarget(HIObject object, bool value)
{
	CClientDevice *pBase = FBCast<CClientDevice>(object);
	if (pBase && value)
	{
		pBase->DoExportTarget();
	}
}

void CClientDevice::ActionLoadRotationSetup(HIObject object, bool value)
{
	CClientDevice *pBase = FBCast<CClientDevice>(object);
	if (pBase && value)
	{
		pBase->DoLoadRotationSetup();
	}
}

bool CClientDevice::FBCreate()
{
	FBPropertyPublish(this, PairName, "Pair Name", nullptr, nullptr);
	PairName = SHARED_MAPPING_DEFNAME;
	FBPropertyPublish(this, SyncTimeWithStoryClip, "Sync Time With Story Clip", nullptr, nullptr);
	SyncTimeWithStoryClip = true;
	FBPropertyPublish(this, StoryClipIndex, "Story Clip Index", nullptr, nullptr);
	StoryClipIndex = 0;
	StoryClipIndex.SetMinMax(0.0, 10.0, true, true);

	FBPropertyPublish(this, LoadRotationSetup, "Load Rotation Setup", nullptr, ActionLoadRotationSetup);

	FBPropertyPublish(this, LookAtRoot, "LookAt Root Object", nullptr, nullptr);
	FBPropertyPublish(this, LookAtLeft, "LookAt Left Object", nullptr, nullptr);
	FBPropertyPublish(this, LookAtRight, "LookAt Right Object", nullptr, nullptr);

	FBPropertyPublish(this, ImportTarget, "Import Target", nullptr, ActionImportTarget);
	FBPropertyPublish(this, ExportTarget, "Export Target", nullptr, ActionExportTarget);

	//
	LookAtRoot.SetSingleConnect(true);
	LookAtRoot.SetFilter(FBModel::GetInternalClassId());
	LookAtLeft.SetSingleConnect(true);
	LookAtLeft.SetFilter(FBModel::GetInternalClassId());
	LookAtRight.SetSingleConnect(true);
	LookAtRight.SetFilter(FBModel::GetInternalClassId());

	// Add model templates
	ChangeTemplateDefenition();

	// Device sampling information
	SamplingMode = kFBSoftwareTimestamp;

	mNeedLoadXML = 0;
	mUpdateInitTransform = false;
	mSyncSaved = false;

	mSystem.OnUIIdle.Add(this, (FBCallback)&CClientDevice::EventUIIdle);
	return true;
}

void CClientDevice::ChangeTemplateDefenition()
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
void CClientDevice::FBDestroy()
{
	mSystem.OnUIIdle.Remove(this, (FBCallback)&CClientDevice::EventUIIdle);
}


/************************************************
 *	Device operation.
 ************************************************/
bool CClientDevice::DeviceOperation( kDeviceOperations pOperation )
{
	// Must return the online/offline status of device.
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
bool CClientDevice::Init()
{
	return true;
}


/************************************************
 *	Removal of device.
 ************************************************/
bool CClientDevice::Done()
{
	return false;
}


/************************************************
 *	Device is stopped (offline).
 ************************************************/
bool CClientDevice::Stop()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Shutting down device";

	// Step 1: Stop data from streaming.
	lProgress.Text		= "Stopping data stream";
	Information			= "Stopping data stream";
	if(! mHardware.StopDataStream() )
	{
		Information = "Could not stop data stream.";
	}

	// Step 2: Close down device
	lProgress.Text		= "Closing device communication";
	Information			= "Closing device communication";
	if(! mHardware.Close() )
	{
		Information = "Could not close device";
	}

    return false;
}


/************************************************
 *	Device is started (online).
 ************************************************/
bool CClientDevice::Start()
{
	FBProgress	Progress;

	Progress.Caption	= "Setting up device";

	// Step 1: Open device
	if(! mHardware.Open(PairName) )
	{
		Information = "Could not open device";
		return false;
	}

	// Step 2: Ask hardware to get channel information
	Progress.Text	= "Device found, scanning for channel information...";
	Information		= "Retrieving channel information";
	if(!mHardware.GetSetupInfo())
	{
		Information = "Could not get channel information from device.";
		return false;
	}

	// Step 3: Create FB channel list and setup hierarchy
	

	// Step 4: Start data stream
	if(! mHardware.StartDataStream() )
	{
		Information = "Could not start data stream.";
		return false;
	}

	mNeedLoadXML = 1;
	
    return true; // if true the device is online
}


/************************************************
 *	Reset of device.
 ************************************************/
bool CClientDevice::Reset()
{
    Stop();
    return Start();
}

/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool CClientDevice::AnimationNodeNotify(FBAnimationNode* pAnimationNode ,FBEvaluateInfo* pEvaluateInfo)
{
	double	Pos[3];
	double	Rot[3];

	int Index = pAnimationNode->Reference;

	if (mHardware.IsDataReceived())
	{
		Pos[0] = GetDataTX(Index);
		Pos[1] = GetDataTY(Index);
		Pos[2] = GetDataTZ(Index);
		Rot[0] = GetDataRX(Index);
		Rot[1] = GetDataRY(Index);
		Rot[2] = GetDataRZ(Index);

		mDataChannels.WriteData(Index, Pos, Rot, pEvaluateInfo);

		return true;
	}
	else
	{
		mDataChannels.GetChannelDefault(Index, Pos, Rot);
		mDataChannels.WriteData(Index, Pos, Rot, pEvaluateInfo);

		return true;
	}
	
	return false;
}


/************************************************
 *	Real-Time Synchronous Device IO.
 ************************************************/
void CClientDevice::DeviceIONotify( kDeviceIOs pAction,FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	FBTime lEvalTime(pDeviceNotifyInfo.GetLocalTime());
    switch (pAction)
	{
		case kIOPlayModeWrite:
		case kIOStopModeWrite:
		{
			// Output devices
		}
		break;

		case kIOStopModeRead:
		case kIOPlayModeRead:
		{
			//
			if (SyncTimeWithStoryClip)
			{
				FBTime time{ GetTimeOffsetFromStoryClip() };
				mHardware.GetTimelineSync()->SetOffsetTime(time.GetSecondDouble());
			}
			else
			{
				mHardware.GetTimelineSync()->SetOffsetTime(0.0);
			}

			const bool is_playing = mPlayerControl.IsPlaying; // pDeviceNotifyInfo.GetEvaluateInfo().IsStop();
			
			FBTime local_time{ mSystem.LocalTime };
			mHardware.GetTimelineSync()->SetLocalTime(local_time.GetSecondDouble());
			mHardware.GetTimelineSync()->SetIsPlaying(is_playing);
			
			mHardware.GetTimelineSync()->SetLocalTimeline(local_time.GetSecondDouble(), is_playing);

			if (LookAtRoot.GetCount() > 0 && mDataChannels.GetRootTemplate()->Model
				&& LookAtLeft.GetCount() > 0 && LookAtRight.GetCount() > 0)
			{
				FBMatrix targetTM;
				FBModel* pModel = (FBModel*) LookAtRoot.GetAt(0);
				pModel->GetMatrix(targetTM);

				FBMatrix rootTM;
				FBModel* pRootModel = mDataChannels.GetRootTemplate()->Model;
				pRootModel->GetMatrix(rootTM);

				FBMatrix localTM;
				FBGetLocalMatrix(localTM, rootTM, targetTM);

				// DONE: write to shared mem
				mHardware.SetLookAtRoot(FBVector3d(&localTM[12]));

				//
				// write left / right markers

				rootTM = targetTM;

				pModel = (FBModel*)LookAtLeft.GetAt(0);
				pModel->GetMatrix(targetTM);

				FBGetLocalMatrix(localTM, rootTM, targetTM);
				mHardware.SetLookAtLeft(FBVector3d(&localTM[12]));
				
				//
				pModel = (FBModel*)LookAtRight.GetAt(0);
				pModel->GetMatrix(targetTM);

				FBGetLocalMatrix(localTM, rootTM, targetTM);
				mHardware.SetLookAtRight(FBVector3d(&localTM[12]));

				if (mSyncSaved)
				{
					mHardware.SyncSaved();
					mSyncSaved = false;
				}
			}

			// Input devices
			while(mHardware.FetchDataPacket(lEvalTime))
			{
				if (mHardware.GetTimelineSync()->IsRemoteTimeChanged())
				{
					pDeviceNotifyInfo.SetLocalTime(mHardware.GetTimelineSync()->GetRemoteTime());
				}

				DeviceRecordFrame	(lEvalTime,pDeviceNotifyInfo);
				AckOneSampleReceived();
			}
		}
		break;
	}
}

FBTime	CClientDevice::GetTimeOffsetFromStoryClip()
{
	FBTime offsetTime(0);

	if (FBModelTemplate* root_template = mDataChannels.GetRootTemplate())
	{
		FBModel* pModel = root_template->Model;
		if (nullptr != pModel)
		{
			if (FBAnimationNode* pNode = pModel->Translation.GetAnimationNode())
			{
				const int src_count = pNode->GetSrcCount();

				for (int i = 0; i < src_count; ++i)
				{
					FBPlug *pPlug = pNode->GetSrc(i)->GetOwner();
					
					if (FBIS(pPlug, FBStoryTrack))
					{
						int clip_index = StoryClipIndex.AsInt();

						FBStoryTrack *pTrack = (FBStoryTrack*)pPlug;
						if (pTrack->Clips.GetCount() > 0)
						{
							if (clip_index < 0) clip_index = 0;
							if (clip_index >= pTrack->Clips.GetCount())
								clip_index = pTrack->Clips.GetCount() - 1;

							offsetTime = pTrack->Clips[clip_index]->Start;
						}
					}
				}

			}
		}
	}
	
	return offsetTime;
}

/************************************************
 *	Record a frame of the device (recording).
 ************************************************/
void CClientDevice::DeviceRecordFrame(FBTime &pTime,FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	//pTime = mSystem.LocalTime;

	double	Pos[3];
	double	Rot[3];

	for (int i = 0; i < GetChannelCount(); ++i)
	{
		Pos[0] = GetDataTX(i);
		Pos[1] = GetDataTY(i);
		Pos[2] = GetDataTZ(i);
		Rot[0] = GetDataRX(i);
		Rot[1] = GetDataRY(i);
		Rot[2] = GetDataRZ(i);

		if (mPlayerControl.GetTransportMode() == kFBTransportPlay)
		{
			switch (SamplingMode)
			{
			case kFBHardwareTimestamp:
			case kFBSoftwareTimestamp:
				mDataChannels.RecordData(i, Pos, Rot, &pTime);
				break;

			case kFBHardwareFrequency:
			case kFBAutoFrequency:
				mDataChannels.RecordData(i, Pos, Rot, nullptr);
				break;
			}
		}
	}
}


//--- FBX load/save tags
#define FBX_CHANNEL_TAG		"Channels"
#define FBX_VERSION_TAG		"Version"

/************************************************
 *	Store data in FBX.
 ************************************************/
bool CClientDevice::FbxStore(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
	mDataChannels.FbxStore(pFbxObject, pStoreWhat);
	return ParentClass::FbxStore(pFbxObject, pStoreWhat);
}

/************************************************
 *	Retrieve data from FBX.
 ************************************************/
bool CClientDevice::FbxRetrieve(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
	mDataChannels.FbxRetrieve(pFbxObject, pStoreWhat);
	return ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);
}

void CClientDevice::EventUIIdle(HISender pSender, HKEvent pEvent)
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
		if (mNeedLoadXML > 0)
		{
			mNeedLoadXML += 1;

			if (10 == mNeedLoadXML)
			{
				FBString path(mSystem.UserConfigPath, "\\");
				path = path + PairName + ".xml";
				if (false == mDataChannels.LoadModelsInfoFromXML(path))
				{
					FBMessageBox("client device", "Failed to load model info xml", "Ok");
				}

				mNeedLoadXML = 0;
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

bool CClientDevice::ModelTemplateBindNotify(FBModel* pModel, int pIndex, FBModelTemplate* pModelTemplate)
{
	mDataChannels.UpdateDefaultTransforms();
	mUpdateInitTransform = true;

	return ParentClass::ModelTemplateBindNotify(pModel, pIndex, pModelTemplate);
}

void CClientDevice::DoImportTarget()
{
	if (0 == LookAtRoot.GetCount() || nullptr == mDataChannels.GetRootTemplate()->Model
		|| 0 == LookAtLeft.GetCount() || 0 == LookAtRight.GetCount())
	{
		FBMessageBox("Client Device", "We have nothing to sync!", "Ok");
		return;
	}

	ImportTargetAnimation(mDataChannels.GetRootTemplate()->Model, (FBModel*)LookAtRoot.GetAt(0), (FBModel*)LookAtLeft.GetAt(0), (FBModel*)LookAtRight.GetAt(0));
}

void CClientDevice::DoExportTarget()
{
	if (0 == LookAtRoot.GetCount() || nullptr == mDataChannels.GetRootTemplate()->Model
		|| 0 == LookAtLeft.GetCount() || 0 == LookAtRight.GetCount())
	{
		FBMessageBox("Client Device", "We have nothing to sync!", "Ok");
		return;
	}
	
	FBModel* pModel = (FBModel*)LookAtRoot.GetAt(0);
	FBModel* pRootModel = mDataChannels.GetRootTemplate()->Model;

	ExportTargetAnimation(pRootModel, pModel, (FBModel*)LookAtLeft.GetAt(0), (FBModel*)LookAtRight.GetAt(0));

	mHardware.SyncSaved();

}

void CClientDevice::DoLoadRotationSetup()
{
	mNeedLoadXML = 1;
}

bool CClientDevice::ImportJointSet(const char* filename)
{
	if (NShared::ImportJointSet(filename))
	{
		ChangeTemplateDefenition();
		return true;
	}
	return false;
}

void CClientDevice::ExportJointSet(const char* filename)
{
	NShared::ExportJointSet(filename);
}