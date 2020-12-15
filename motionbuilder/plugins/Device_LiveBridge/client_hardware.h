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
	char*	GetChannelName	(int pChannel)		{	return (char *)m_ChannelName[pChannel];	}
	int		GetChannelCount	()					{	return m_ChannelCount;					}
	double	GetDataTX		(int pChannel)		{	return m_ChannelData[pChannel][DATA_TX];	}
	double	GetDataTY		(int pChannel)		{	return m_ChannelData[pChannel][DATA_TY];	}
	double	GetDataTZ		(int pChannel)		{	return m_ChannelData[pChannel][DATA_TZ];	}
	double	GetDataRX		(int pChannel)		{	return m_ChannelData[pChannel][DATA_RX];	}
	double	GetDataRY		(int pChannel)		{	return m_ChannelData[pChannel][DATA_RY];	}
	double	GetDataRZ		(int pChannel)		{	return m_ChannelData[pChannel][DATA_RZ];	}

	const bool IsDataReceived() const { return m_DataReceived; }

	void SetLookAtRoot(const FBVector3d& pos) { m_LookAtRootPos = pos; }
	void SetLookAtLeft(const FBVector3d& pos) { m_LookAtLeftPos = pos; }
	void SetLookAtRight(const FBVector3d& pos) { m_LookAtRightPos = pos; }

	void		SyncSaved();
	bool		HasNewSync();

	//
	STimelineSyncManager*	GetTimelineSync() { return m_TimelineSync; }

private:
	FBSystem			m_System;									//!< System interface.
	FBPlayerControl		m_PlayerControl;

	bool				m_IsPlaying{ false };
	bool				m_TimeChanged{ false };
	
	FBVector3d			m_LookAtRootPos;
	FBVector3d			m_LookAtLeftPos;
	FBVector3d			m_LookAtRightPos;
	
	FBTime				m_LocalTime;
	FBTime				m_OffsetTime;

	bool			m_DataReceived;
	FBString		m_ChannelName[MAX_CHANNEL];					//!< Channel name.
	unsigned int	m_ChannelNameHash[MAX_CHANNEL];
	double			m_ChannelData[MAX_CHANNEL][DATA_TYPE_COUNT];	//!< Channel data.

	int				m_ChannelCount;								//!< Channel count.
	long			m_Counter;									//!< Time counter for hands.

	unsigned int				m_SessionId{ 0 };
	STimelineSyncManager*		m_TimelineSync{ nullptr };
};
