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
/**	\file	client_hardware.cxx
*	Definition of a client hardware class.
*
*/

//--- Class declaration
#include "client_hardware.h"
#include <tchar.h>
#include "shared.h"

#include <AnimLiveBridge/AnimLiveBridge.h>

/************************************************
 *	Constructor.
 ************************************************/
CClientHardware::CClientHardware() 
	: mChannelCount( 0 )
	, mCounter( 0 )
{
	mFileOpen = false;
	mTimeChanged = false;
	
	mHasNewSync = false;
	mSyncSaved = false;

	mData = new NAnimationLiveBridge::SSharedModelData();
	memset(mData, 0, sizeof(NAnimationLiveBridge::SSharedModelData));

	for (int i = 0; i < MAX_CHANNEL; ++i)
	{
		for (int j = 0; j < DATA_TYPE_COUNT; ++j)
		{
			mChannelData[i][j] = 0.0;
		}
	}

	mDataReceived = false;
}


/************************************************
 *	Destructor.
 ************************************************/
CClientHardware::~CClientHardware()
{
	if (mData)
	{
		delete mData;
		mData = nullptr;
	}
}


/************************************************
 *	Open device communications.
 ************************************************/
bool CClientHardware::Open(const char* pair_name)
{
	mMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS, // read/write access
		FALSE, // don't inherit the name
		FBString(SHARED_MAPPING_PREFIX, pair_name)	// name of mapping object
		);

	mFileOpen = true;
	
	if (mMapFile == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d).\n"),
			GetLastError());
		
		mFileOpen = false;
		FBMessageBox("Client Hardware", "Please start MoBu with administrative rights", "Ok");
	}

	mEventToClient = CreateEvent(NULL, false, false, FBString(pair_name, EVENT_TOCLIENT));
	mEventFromClient = CreateEvent(NULL, false, false, FBString(pair_name, EVENT_FROMCLIENT));

	return true;
}


/************************************************
 *	Get device setup information.
 ************************************************/
bool CClientHardware::GetSetupInfo()
{
	using namespace NShared;
	
	mChannelCount	= GetSetJointsCount();

	for (int i = 0; i < mChannelCount; ++i)
	{
		mChannelName[i] = GetTestJointName(i);
	}

	return true;
}


/************************************************
 *	Close device communications.
 ************************************************/
bool CClientHardware::Close()
{
	if (mFileOpen)
	{
		CloseHandle(mMapFile);
		mFileOpen = false;
	}
	return true;
}


/************************************************
 *	Fetch a data packet from the device.
 ************************************************/
bool CClientHardware::FetchDataPacket(FBTime &pTime)
{
	//Returns true if a new data packet is ready
	// TODO: insert time stamp from data packet
	//pTime = mSystem.SystemTime;

	// TODO: Replace this bogus code with real NON-BLOCKING TCP, UDP or serial calls
	// to your data server.
	PollData();

	if (mTimeChanged)
	{
		pTime = mLocalTime;
	}

	// As soon as enough bytes have been read to have data for all makers,
	// return immediately, even if another data packer is waiting
	// This function is nested in a while loop and will be called immediately
	// as soon as the current data frame is processed.

	// TODO: mCounter is a bogus variable there to simulate a time-reference
	if(mCounter%2)
	{
		return true; // data for all elements of mChannelData[] has been found.
	}
	else
	{
		return false; // incomplete data packet has been read.
	}
}


/************************************************
 *	Poll device.
 *	Device should get itself to send more data.
 ************************************************/
bool CClientHardware::PollData()
{
	//Rotational data is in Euler angles in degrees
	//the order is: XYZ. In other words:
	//- rotation around the 'X' axis
	//- followed by a rotation around the 'Y' axis
	//- followed by a rotation around the 'Z' axis

	mCounter++;
	
	if (false == mFileOpen)
		return false;

	bool signalled = WaitForSingleObjectEx(mEventToClient, 0, false) != WAIT_TIMEOUT;

	if (true == signalled)
	{
		char *pBuf = (char*)MapViewOfFile(mMapFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			sizeof(NAnimationLiveBridge::SSharedModelData));

		if (pBuf == nullptr)
		{
			return false;
		}

		NAnimationLiveBridge::SSharedModelData* pData = (NAnimationLiveBridge::SSharedModelData*)pBuf;

		mTimeChangeManager.ReadFromData(true, *pData);
		mTimeChangeManager.WriteToData(false, *pData);

		// check if we have sync event from a server
		if (pData->m_LookAtRoot[3] == 2.0f)
		{
			mHasNewSync = true;
		}

		// look at pos
		pData->m_LookAtRoot[0] = (float)mLookAtRootPos[0];
		pData->m_LookAtRoot[1] = (float)mLookAtRootPos[1];
		pData->m_LookAtRoot[2] = (float)mLookAtRootPos[2];
		pData->m_LookAtRoot[3] = (mSyncSaved) ? 1.0f : 0.0f;
		
		pData->m_LookAtLeft[0] = (float)mLookAtLeftPos[0];
		pData->m_LookAtLeft[1] = (float)mLookAtLeftPos[1];
		pData->m_LookAtLeft[2] = (float)mLookAtLeftPos[2];
		
		pData->m_LookAtRight[0] = (float)mLookAtRightPos[0];
		pData->m_LookAtRight[1] = (float)mLookAtRightPos[1];
		pData->m_LookAtRight[2] = (float)mLookAtRightPos[2];

		mSyncSaved = false;

		memcpy(mData, pData, sizeof(NAnimationLiveBridge::SSharedModelData));

		UnmapViewOfFile(pBuf);

		//

		FBVector3d r;
		FBQuaternion q;

		const int count = mData->m_Header.m_ModelsCount;
		for (int i = 0; i < count; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				mChannelData[i][DATA_TX + j] = static_cast<double>(mData->m_Joints[i].m_Transform.m_Translation[j]);
				q[j] = static_cast<double>(mData->m_Joints[i].m_Transform.m_Rotation[j]);
			}
			q[3] = static_cast<double>(mData->m_Joints[i].m_Transform.m_Rotation[3]);
			FBQuaternionToRotation(r, q);

			memcpy(&mChannelData[i][DATA_RX], r, sizeof(double) * 3);
		}

		SetEvent(mEventFromClient);

		mDataReceived = true;
	}
	
	return true;
}

/************************************************
 *	Start data streaming from device.
 ************************************************/
bool CClientHardware::StartDataStream()
{
	return true;
}

/************************************************
 *	Stop data streaming from device.
 ************************************************/
bool CClientHardware::StopDataStream()
{
	return true;
}
