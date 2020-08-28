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

/**	\file	server_hardware.cxx
*	Definition of server output hardware.
*/

//--- Class declaration
#include "server_hardware.h"
#include <tchar.h>
#include "shared.h"
#include <unordered_map>

#include <AnimLiveBridge/AnimLiveBridge.h>

/************************************************
 *	Constructor.
 ************************************************/
CServerHardware::CServerHardware()
{
	mMapFile = NULL;
	//mExportFile		= NULL;
	mCounter		= 0;
	mFileOpen		= false;
   
	mHasNewSync = false;
	mSyncSaved = false;

	mData = new NAnimationLiveBridge::SSharedModelData();
	memset(mData, 0, sizeof(NAnimationLiveBridge::SSharedModelData));
}


/************************************************
 *	Destructor.
 ************************************************/
CServerHardware::~CServerHardware()
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
bool CServerHardware::Open(const char* pair_name)
{
	mMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		sizeof(NAnimationLiveBridge::SSharedModelData),                // maximum object size (low-order DWORD)
		FBString(SHARED_MAPPING_PREFIX, pair_name));                 // name of mapping object

	mEventToClient = CreateEvent(NULL, false, false, FBString(pair_name, EVENT_TOCLIENT));
	mEventFromClient = CreateEvent(NULL, false, true, FBString(pair_name, EVENT_FROMCLIENT));

	mFileOpen = true;
	
	if (mMapFile == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d).\n"),
			GetLastError());

		mFileOpen = false;
		FBMessageBox("Server Hardware", "Please start MoBu with administrative rights", "Ok");
	}

	if (mFileOpen)
	{
		char *pBuf = (char*)MapViewOfFile(mMapFile,
			FILE_MAP_WRITE, // write permission
			0,
			0,
			sizeof(NAnimationLiveBridge::SSharedModelData));

		if (pBuf == nullptr)
		{
			CloseHandle(mMapFile);
			_tprintf(TEXT("Could not map a file (%d).\n"), GetLastError());

			mFileOpen = false;
			//return false;
		}
		else
		{
			memset(pBuf, 0, sizeof(NAnimationLiveBridge::SSharedModelData));
			UnmapViewOfFile(pBuf);
		}
	}
	
	if (!SetEvent(mEventToClient))
	{
		_tprintf(TEXT("Could not set event to a client (%d).\n"),
			GetLastError());

		FBMessageBox("Server Hardware", "Could not set event to a client", "Ok");
		//return false;
	}

	return true;
}


/************************************************
 *	Get device setup information.
 ************************************************/
bool CServerHardware::GetSetupInfo()
{
	return true;
}


/************************************************
 *	Close device communications.
 ************************************************/
bool CServerHardware::Close()
{
    if( mFileOpen )
    {
		CloseHandle(mMapFile);
	    mFileOpen = false;
    }
	return true;
}


/************************************************
 *	Fetch a data packet from the device.
 ************************************************/
bool CServerHardware::FetchDataPacket(FBTime &pTime)
{
	//Returns true if a new data packet is ready
	// TODO: insert time stamp from data packet
	pTime = mSystem.SystemTime;

	// TODO: Replace this bogus code with real NON-BLOCKING TCP, UDP or serial calls
	// to your data server.
	PollData();

	// As soon as enough bytes have been read to have data for all makers,
	// return immediately, even if another data packer is waiting
	// This function is nested in a while loop and will be called immediately
	// as soon as the current data frame is processed.

	// TODO: mCounter is a bogus variable there to simulate a time-reference
	if(mCounter%4)
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
bool CServerHardware::PollData()
{
	mCounter++;
	return true;
}


/************************************************
 *	Send a data packet over the network to move data.
 ************************************************/
bool CServerHardware::SendDataPacket(FBTime &pTime)
{
	// Send to hardware
	// High priority IO thread, must have minimal CPU usage.
	// Non-blocking IO, send data to hardware via comm port, network, etc.
	
	if (false == mFileOpen)
		return false;

	bool signalled = WaitForSingleObjectEx(mEventFromClient, 0, false) != WAIT_TIMEOUT;

	if (true == signalled)
	{
		char *pBuf = (char*)MapViewOfFile(mMapFile,
			FILE_MAP_ALL_ACCESS, // write permission
			0,
			0,
			sizeof(NAnimationLiveBridge::SSharedModelData));

		if (pBuf == nullptr)
		{
			return false;
		}

		// read a client time and sync with a server time
		NAnimationLiveBridge::SSharedModelData *pData = (NAnimationLiveBridge::SSharedModelData*)pBuf;
		
		mTimeChangeManager.ReadFromData(false, *pData);
		mTimeChangeManager.WriteToData(true, *mData);

		for (int i = 0; i < 4; ++i)
		{
			mData->m_LookAtRoot[i] = pData->m_LookAtRoot[i];
			mData->m_LookAtLeft[i] = pData->m_LookAtLeft[i];
			mData->m_LookAtRight[i] = pData->m_LookAtRight[i];
		}
			
		// check if we have sync event from a client
		if (mData->m_LookAtRoot[3] == 1.0f)
		{
			mHasNewSync = true;
			mData->m_LookAtRoot[3] = 0.0f;
		}
		else if (mSyncSaved)
		{
			mData->m_LookAtRoot[3] = 2.0f;
		}
		mSyncSaved = false;
	
		CopyMemory((PVOID)pBuf, mData, sizeof(char)*sizeof(NAnimationLiveBridge::SSharedModelData));

		UnmapViewOfFile(pBuf);

		//
		mLookAtRootPos = FBVector3d((double)mData->m_LookAtRoot[0], (double)mData->m_LookAtRoot[1], (double)mData->m_LookAtRoot[2]);
		mLookAtLeftPos = FBVector3d((double)mData->m_LookAtLeft[0], (double)mData->m_LookAtLeft[1], (double)mData->m_LookAtLeft[2]);
		mLookAtRightPos = FBVector3d((double)mData->m_LookAtRight[0], (double)mData->m_LookAtRight[1], (double)mData->m_LookAtRight[2]);

		//
		SetEvent(mEventToClient);
	}

	return true;
}


void CServerHardware::SetNumberOfActiveModels(const int count)
{
	mData->m_Header.m_ModelsCount = count;
}

void CServerHardware::SetModelName(const int index, const char* name)
{
	mData->m_Joints[index].m_NameHash = static_cast<uint32_t>(std::hash<std::string>{}(name));
}
unsigned int CServerHardware::GetModelNameHash(const int index)
{
	return mData->m_Joints[index].m_NameHash;
}

void CServerHardware::SetNumberOfActiveProperties(const int count)
{
	mData->m_Header.m_PropsCount = count;
}

/************************************************
 *	Write the new position values into the hardware abstraction.
 ************************************************/
void CServerHardware::WritePos( const int index, const double* pPos )
{
	float* dst_values = mData->m_Joints[index].m_Transform.m_Translation;

	dst_values[0] = static_cast<float>(pPos[0]);
	dst_values[1] = static_cast<float>(pPos[1]);
	dst_values[2] = static_cast<float>(pPos[2]);
}

/************************************************
 *	Write the new rotation values into the hardware abstraction.
 ************************************************/
void CServerHardware::WriteRot( const int index, const double* pRot )
{
	// combine with pre-rotation
	float* dst_values = mData->m_Joints[index].m_Transform.m_Rotation;

	dst_values[0] = static_cast<float>(pRot[0]);
	dst_values[1] = static_cast<float>(pRot[1]);
	dst_values[2] = static_cast<float>(pRot[2]);
}

void CServerHardware::WriteMatrix(const int index, const FBMatrix& tm)
{
	// combine with pre-rotation

	FBTVector t;
	FBQuaternion q;
	FBSVector s;

	FBMatrixToTQS(t, q, s, tm);

	NAnimationLiveBridge::SSharedModelData::SJointData& info = mData->m_Joints[index];
	
	for (int i = 0; i < 3; ++i)
	{
		info.m_Transform.m_Translation[i] = static_cast<float>(t[i]);
		info.m_Transform.m_Rotation[i] = static_cast<float>(q[i]);
	}

	info.m_Transform.m_Rotation[3] = static_cast<float>(q[3]);
}

void CServerHardware::WriteProp(const int index, const double value)
{
	mData->m_Properties[index].m_Value = static_cast<float>(value);
}
