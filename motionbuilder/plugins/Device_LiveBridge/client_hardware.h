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
/**	\file	client_hardware.h
*	Declaration of a client animation hardware class.
*
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <Windows.h>

#include "shared.h"

//--- Elements in array of models.
#define JOINT_A		0
#define JOINT_B		1
#define JOINT_C		2

//--- Data codes.
enum {
	DATA_TX			= 0,
	DATA_TY			= 1,
	DATA_TZ			= 2,
	DATA_RX			= 3,
	DATA_RY			= 4,
	DATA_RZ			= 5,
	DATA_TYPE_COUNT = 6
};

//--- Number of channels.
#define MAX_CHANNEL 100

////////////////////////////////////////////////////////////////////////////////////////////////
//! Client hardware.
class CClientHardware
{
public:
	//! Constructor.
	CClientHardware();

	//! Destructor.
	~CClientHardware();

	//--- Opens and closes connection with data server. returns true if successful
	bool	Open(const char* pair_name);								//!< Open the connection.
	bool	Close();							//!< Close connection.

	//--- Hardware communication
	bool	FetchDataPacket	(FBTime &pTime);		//!< Fetch a data packet from the computer.
	bool	PollData		();						//!< Poll the device for a data packet.
	bool	GetSetupInfo	();						//!< Get the setup information.
	bool	StartDataStream	();						//!< Put the device in streaming mode.
	bool	StopDataStream	();						//!< Take the device out of streaming mode.

	//--- Channel & Channel data management
	char*	GetChannelName	(int pChannel)		{	return (char *)mChannelName[pChannel];	}
	int		GetChannelCount	()					{	return mChannelCount;					}
	double	GetDataTX		(int pChannel)		{	return mChannelData[pChannel][DATA_TX];	}
	double	GetDataTY		(int pChannel)		{	return mChannelData[pChannel][DATA_TY];	}
	double	GetDataTZ		(int pChannel)		{	return mChannelData[pChannel][DATA_TZ];	}
	double	GetDataRX		(int pChannel)		{	return mChannelData[pChannel][DATA_RX];	}
	double	GetDataRY		(int pChannel)		{	return mChannelData[pChannel][DATA_RY];	}
	double	GetDataRZ		(int pChannel)		{	return mChannelData[pChannel][DATA_RZ];	}

	const bool IsDataReceived() const { return mDataReceived; }

	void SetLookAtRoot(const FBVector3d& pos) { mLookAtRootPos = pos; }
	void SetLookAtLeft(const FBVector3d& pos) { mLookAtLeftPos = pos; }
	void SetLookAtRight(const FBVector3d& pos) { mLookAtRightPos = pos; }

	void SetSyncSaved() { mSyncSaved = true; }

	const bool HasNewSync() {
		bool value = mHasNewSync;
		mHasNewSync = false;
		return value;
	}

	//
	STimeChangeManager			mTimeChangeManager;

private:
	FBSystem			mSystem;									//!< System interface.
	FBPlayerControl		mPlayerControl;

	NAnimationLiveBridge::SSharedModelData*			mData;

	bool				mIsPlaying;
	bool				mTimeChanged;
	
	FBVector3d			mLookAtRootPos;
	FBVector3d			mLookAtLeftPos;
	FBVector3d			mLookAtRightPos;
	
	bool				mSyncSaved;
	bool				mHasNewSync;

	FBTime				mLocalTime;
	FBTime				mOffsetTime;

	bool			mDataReceived;
	FBString	mChannelName[MAX_CHANNEL];					//!< Channel name.
	double		mChannelData[MAX_CHANNEL][DATA_TYPE_COUNT];	//!< Channel data.

	int			mChannelCount;								//!< Channel count.
	long		mCounter;									//!< Time counter for hands.

	bool		mFileOpen;
	HANDLE		mMapFile;

	HANDLE		mEventToClient;
	HANDLE		mEventFromClient;
};
