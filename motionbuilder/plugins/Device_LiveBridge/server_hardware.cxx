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
	mCounter		= 0;
	m_SessionId = NewLiveSession();
	m_TimelineSync = MapTimelineSync(m_SessionId);

	m_Data = new SSharedModelData();
}


/************************************************
 *	Destructor.
 ************************************************/
CServerHardware::~CServerHardware()
{
	EraseLiveSession(m_SessionId);

	if (m_Data)
	{
		delete m_Data;
		m_Data = nullptr;
	}
}


/************************************************
 *	Open device communications.
 ************************************************/
bool CServerHardware::Open(const char* pair_name)
{
	const int status = HardwareOpen(m_SessionId, pair_name, true);

	if (status != 0)
	{
		FBTrace("Could not create file mapping object (%d).\n", status);
		FBMessageBox("Server Hardware", "Please start MoBu with administrative rights", "Ok");
		return false;
	}
	
	m_TimelineSync = MapTimelineSync(m_SessionId);
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
	UnMapTimelineSync(m_SessionId);
	HardwareClose(m_SessionId);
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
	
	if (SSharedModelData* model_data = MapModelData(m_SessionId))
	{
		memcpy(model_data, m_Data, sizeof(SSharedModelData));
		UnMapModelData(m_SessionId);

		const int error_code = FlushData(m_SessionId, true);

		if (error_code != 0)
		{
			//FBTrace("Failed to Flush Server Data with error code %d and session id %d\n", error_code, m_SessionId);
			return false;
		}
		else
		{
			//FBTrace("Flush Server Data succeed for session %d\n", m_SessionId);
		}
		return true;
	}
	else
	{
		FBTrace("Failed to MapModelData for session %d\n", m_SessionId);
	}

	return false;
}


void CServerHardware::SetNumberOfActiveModels(const int count)
{
	m_Data->m_Header.m_ModelsCount = count;
}

void CServerHardware::SetModelName(const int index, const char* name)
{
	m_Data->m_Joints.m_Data[index].m_NameHash = static_cast<uint32_t>(std::hash<std::string>{}(name));
}
unsigned int CServerHardware::GetModelNameHash(const int index)
{
	return m_Data->m_Joints.m_Data[index].m_NameHash;
}

void CServerHardware::SetNumberOfActiveProperties(const int count)
{
	m_Data->m_Header.m_PropsCount = count;
}

/************************************************
 *	Write the new position values into the hardware abstraction.
 ************************************************/
void CServerHardware::WritePos( const int index, const double* pPos )
{
	float* dst_values = &m_Data->m_Joints.m_Data[index].m_Transform.m_Translation.m_X;

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
	float* dst_values = &m_Data->m_Joints.m_Data[index].m_Transform.m_Rotation.m_X;

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

	SJointData& info = m_Data->m_Joints.m_Data[index];
	
	float* translation = &info.m_Transform.m_Translation.m_X;
	float* rotation = &info.m_Transform.m_Rotation.m_X;

	for (int i = 0; i < 3; ++i)
	{
		translation[i] = static_cast<float>(t[i]);
		rotation[i] = static_cast<float>(q[i]);
	}

	rotation[3] = static_cast<float>(q[3]);
}

void CServerHardware::WriteProp(const int index, const double value)
{
	m_Data->m_Properties.m_Data[index].m_Value = static_cast<float>(value);
}

void CServerHardware::SyncSaved() 
{ 
	SetSyncSaved(m_SessionId, true);
}

bool CServerHardware::HasNewSync()
{
	return GetAndResetHasNewSync(m_SessionId);
}