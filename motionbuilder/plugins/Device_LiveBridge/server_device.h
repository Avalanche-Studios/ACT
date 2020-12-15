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
/**	\file	server_device.h
*	Declaration of an animation output device.
*
*/

//--- Class declaration
#include "server_hardware.h"
#include "shared.h"
#include <vector>

//--- Registration defines
#define SERVERDEVICE__CLASSNAME		CServerDevice
#define SERVERDEVICE__CLASSSTR		"CServerDevice"


////////////////////////////////////////////////////////////////////////////////
//! Server output device.
class CServerDevice : public FBDevice
{
	//--- FiLMBOX declaration
	FBDeviceDeclare(CServerDevice, FBDevice);

public:
	//--- FiLMBOX Construction/Destruction
	virtual bool FBCreate() override;		//!< FiLMBOX constructor.
	virtual void FBDestroy() override;		//!< FiLMBOX destructor.

	//--- Initialisation/Shutdown
	bool  Init();		//!< Initialize/create device.
	bool  Done();		//!< Remove device.
	bool  Reset();		//!< Reset device.
	bool  Stop();		//!< Stop device (offline).
	bool  Start();		//!< Start device (online).

	//--- The following will be called by the real-time engine.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo) override;	//!< Real-time evaluation for node.
	virtual void DeviceIONotify(kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo) override;	//!< Notification of/for Device IO.
	virtual bool DeviceEvaluationNotify(kTransportMode pMode, FBEvaluateInfo* pEvaluateInfo) override;	//!< Evaluation the device (write to hardware).
	virtual bool DeviceOperation(kDeviceOperations pOperation) override;	//!< Operate device.

	virtual bool ModelTemplateBindNotify(FBModel* pModel, int pIndex, FBModelTemplate* pModelTemplate) override;

	virtual bool PlugDataNotify(FBConnectionAction pAction, FBPlug* pThis, void* pData, void* pDataOld, int pDataSize) override;

	//--- FBX Load/Save.
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;	//!< Store in FBX file.
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;	//!< Retrieve from FBX file.

	//--- Attribute management.
	double	GetExportRate() { return mExportRate; }
	void	SetExportRate(double pRate) { mExportRate = pRate; }

	//
	void	EventUIIdle(HISender pSender, HKEvent pEvent);
	
	bool ImportJointSet(const char* filename);
	void ExportJointSet(const char* filename);

public:

	FBPropertyString		PairName;

	FBPropertyListObject	LookAtRoot;
	FBPropertyListObject	LookAtLeft;
	FBPropertyListObject	LookAtRight;
	
	FBPropertyAction		ImportTarget;
	FBPropertyAction		ExportTarget;

	FBPropertyBool			RotationIncludeDOF;	// do we want to send rotation including a pre rotation matrix

	FBPropertyAction		SaveRotationSetup;

	static void ActionImportTarget(HIObject object, bool value);
	static void ActionExportTarget(HIObject object, bool value);
	static void ActionSaveRotationSetup(HIObject object, bool value);

public:
	FBAnimationNode*		mRootNode;			//!< Root animation node.
	FBAnimationNode*		mLookAtRootNode;
	FBAnimationNode*		mLookAtLeftNode;
	FBAnimationNode*		mLookAtRightNode;

	CDataChannelManager		mDataChannels;

protected:
	FBSystem				mSystem;
	FBApplication			mApp;
	FBPlayerControl			mPlayerControl;
	double					mExportRate;
	CServerHardware			mHardware;			//!< Handle onto hardware.
	int						mNeedSaveXML;

	bool					mUpdateInitTransform;

	FBVector3d				mLookAtRootPos;
	FBVector3d				mLookAtLeftPos;
	FBVector3d				mLookAtRightPos;

	void		DoImportTarget();
	void		DoExportTarget();
	void		DoSaveRotationSetup();

	void		ChangeTemplateDefenition();

	void ComputeFullRotationMatrix(FBMatrix& tm, FBEvaluateInfo* pEvaluateInfo, const int index);
};
