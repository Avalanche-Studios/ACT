#pragma once
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
/**	\file	client_device.h
*	Declaration of a CClientDevice class.
*
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "client_hardware.h"
#include <vector>

//--- Registration defines
#define CLIENTDEVICE__CLASSNAME		CClientDevice
#define CLIENTDEVICE__CLASSSTR		"CClientDevice"


///////////////////////////////////////////////////////////////////////////////////////////
//! Client input device.
class CClientDevice : public FBDevice
{
	//--- FiLMBOX declaration
	FBDeviceDeclare(CClientDevice, FBDevice);

public:
	//--- FiLMBOX Construction/Destruction
	virtual bool FBCreate() override;		//!< FiLMBOX Constructor.
	virtual void FBDestroy() override;		//!< FiLMBOX Destructor.

	//--- Initialisation/Shutdown
	bool  Init();					//!< Initialization routine.
	bool  Done();					//!< Device removal.
	bool  Reset();					//!< Reset function.
	bool  Stop();					//!< Device online routine.
	bool  Start();					//!< Device offline routine.

	//--- Real-Time Engine callbacks
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo) override;		//!< Real-time evaluation function.
	virtual void DeviceIONotify(kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo) override;		//!< Hardware I/O notification.

	//--- Device operation
	virtual bool DeviceOperation(kDeviceOperations pOperation) override;		//!< Operate device.

	//--- Load/Save.
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;		//!< Store configuration in FBX.
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;		//!< Retrieve configuration from FBX.

	virtual bool ModelTemplateBindNotify(FBModel* pModel, int pIndex, FBModelTemplate* pModelTemplate) override;


	//--- Recording of frame information
	void	DeviceRecordFrame(FBTime &pTime, FBDeviceNotifyInfo &pDeviceNotifyInfo);

	//--- Get data from hardware.
	double GetDataTX(int pChannelIndex) { return mHardware.GetDataTX(pChannelIndex); }
	double GetDataTY(int pChannelIndex) { return mHardware.GetDataTY(pChannelIndex); }
	double GetDataTZ(int pChannelIndex) { return mHardware.GetDataTZ(pChannelIndex); }
	double GetDataRX(int pChannelIndex) { return mHardware.GetDataRX(pChannelIndex); }
	double GetDataRY(int pChannelIndex) { return mHardware.GetDataRY(pChannelIndex); }
	double GetDataRZ(int pChannelIndex) { return mHardware.GetDataRZ(pChannelIndex); }

	//--- Channel list manipulation.
	int		GetChannelCount() { return mHardware.GetChannelCount(); }
	char *	GetChannelName(int pMarkerIndex) { return mHardware.GetChannelName(pMarkerIndex); }

	//
	void	EventUIIdle(HISender pSender, HKEvent pEvent);

	bool ImportJointSet(const char* filename);
	void ExportJointSet(const char* filename);

public:

	FBPropertyString		PairName;
	FBPropertyBool			SyncTimeWithStoryClip;		// get local time from story clip instead of timeline
	FBPropertyInt			StoryClipIndex;				// index of a clip on a story track to take start time from

	FBPropertyListObject	LookAtRoot;
	FBPropertyListObject	LookAtLeft;
	FBPropertyListObject	LookAtRight;

	FBPropertyAction		ImportTarget;
	FBPropertyAction		ExportTarget;

	FBPropertyAction		LoadRotationSetup;

	static void ActionImportTarget(HIObject object, bool value);
	static void ActionExportTarget(HIObject object, bool value);
	static void ActionLoadRotationSetup(HIObject object, bool value);

protected:
	CClientHardware			mHardware;					//!< Handle onto hardware.
	FBPlayerControl			mPlayerControl;				//!< To get play mode for recording.	
	FBSystem				mSystem;
	FBApplication			mApp;

	CDataChannelManager		mDataChannels;

	int						mNeedLoadXML{ 0 };

	bool					mUpdateInitTransform{ false };
	bool					mSyncSaved{ false };

	FBTime	GetTimeOffsetFromStoryClip();

	void DoImportTarget();
	void DoExportTarget();
	void DoLoadRotationSetup();

	void		ChangeTemplateDefenition();
};
