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
	: m_ChannelCount( 0 )
	, m_Counter( 0 )
{
	m_TimeChanged = false;
	
	for (int i = 0; i < MAX_CHANNEL; ++i)
	{
		for (int j = 0; j < DATA_TYPE_COUNT; ++j)
		{
			m_ChannelData[i][j] = 0.0;
		}
		m_ChannelNameHash[i] = 0;
	}

	m_SessionId = NewLiveSession();
	m_DataReceived = false;
}


/************************************************
 *	Destructor.
 ************************************************/
CClientHardware::~CClientHardware()
{
	EraseLiveSession(m_SessionId);
}


/************************************************
 *	Open device communications.
 ************************************************/
bool CClientHardware::Open(const char* pair_name)
{
	const int status = HardwareOpen(m_SessionId, pair_name, false);

	if (status != 0)
	{
		FBTrace("[Client Hardware] Could not create file mapping object (%d).\n", status);
		FBMessageBox("Client Hardware", "Please start MoBu with administrative rights", "Ok");
		return false;
	}
	m_TimelineSync = MapTimelineSync(m_SessionId);

	return true;
}


/************************************************
 *	Get device setup information.
 ************************************************/
bool CClientHardware::GetSetupInfo()
{
	using namespace NShared;
	
	m_ChannelCount	= GetSetJointsCount();

	for (int i = 0; i < m_ChannelCount; ++i)
	{
		const char* name{ GetTestJointName(i) };
		m_ChannelName[i] = name;
		m_ChannelNameHash[i] = HashPairName(name);
	}

	return true;
}


/************************************************
 *	Close device communications.
 ************************************************/
bool CClientHardware::Close()
{
	UnMapTimelineSync(m_SessionId);
	HardwareClose(m_SessionId);
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
	if (!PollData())
		return false;

	if (m_TimeChanged)
	{
		pTime = m_LocalTime;
	}

	// As soon as enough bytes have been read to have data for all makers,
	// return immediately, even if another data packer is waiting
	// This function is nested in a while loop and will be called immediately
	// as soon as the current data frame is processed.

	// TODO: mCounter is a bogus variable there to simulate a time-reference
	if(m_Counter%2)
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

	m_Counter++;
	
	const int error_code = FlushData(m_SessionId, false);
	if (error_code == 0)
	{
		FBVector3d r;
		FBQuaternion q;

		SSharedModelData* data = MapModelData(m_SessionId);

		const int count = data->m_Header.m_ModelsCount;
		for (int i = 0; i < count; ++i)
		{
			// find a channel
			for (int j = 0; j < m_ChannelCount; ++j)
			{
				if (m_ChannelNameHash[j] == data->m_Joints.m_Data[i].m_NameHash)
				{
					const float* joint_translation{ &data->m_Joints.m_Data[i].m_Transform.m_Translation.m_X };
					const float* joint_rotation{ &data->m_Joints.m_Data[i].m_Transform.m_Rotation.m_X };

					for (int k = 0; k < 3; ++k)
					{
						m_ChannelData[j][DATA_TX + k] = static_cast<double>(joint_translation[k]);
						q[k] = static_cast<double>(joint_rotation[k]);
					}
					q[3] = static_cast<double>(joint_rotation[3]);
					FBQuaternionToRotation(r, q);

					memcpy(&m_ChannelData[j][DATA_RX], r, sizeof(double) * 3);

					break;
				}
			}
		}

		SetFinishEvent(m_SessionId);

		m_DataReceived = true;
	}
	else
	{
		//FBTrace("Failed to flush on client side, error code %d and session id %d\n", error_code, m_SessionId);
		return false;
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


void CClientHardware::SyncSaved() 
{ 
	SetSyncSaved(m_SessionId, true);
}

bool CClientHardware::HasNewSync() 
{
	return GetAndResetHasNewSync(m_SessionId);
}