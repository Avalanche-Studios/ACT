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
/**	\file	server_hardware.h
*	Declaration of a server output hardware.
*
*/

//--- Class declarations
#include <fbsdk/fbsdk.h>
#include "shared.h"

////////////////////////////////////////////////////////////////////////////////////////
//! Server hardware.
class CServerHardware
{
public:
	//! Constructor.
	CServerHardware();

	//! Destructor.
	~CServerHardware();

	//--- Opens and closes connection with data server. returns true if successful
	bool	Open(const char* pair_name);	//!< Open the device.
	bool	Close();	//!< Close the device.

	/**	Channel information.
	*	After the connection has been established with the Open call
	*	This call will retreive all channel information (type, name, etc...)
	*/
	bool	GetSetupInfo();

	// Non blocking read of data stream which fills mChannel[] with a new data packet.
	// Returns true if a new data frame is ready
	bool	FetchDataPacket	(FBTime &pTime);		//!< Fetch a data packet.
	bool	SendDataPacket	(FBTime &pTime);			//!< Send a data packet.
	bool	PollData		();

	// Write an information
	void	SetNumberOfActiveModels(const int count);
	void	SetModelName(const int index, const char* name);
	unsigned int GetModelNameHash(const int index);
	void	WritePos( const int index, const double* pPos );
	void	WriteRot( const int index, const double* pRot );
	void	WriteMatrix(const int index, const FBMatrix& tm);
	void	SetNumberOfActiveProperties(const int count);
	void	WriteProp(const int index, const double value);
	
	//
	
	const FBVector3d&	GetLookAtRootPos() { return mLookAtRootPos; }
	const FBVector3d&	GetLookAtLeftPos() { return mLookAtLeftPos; }
	const FBVector3d&	GetLookAtRightPos() { return mLookAtRightPos; }
	
	void		SyncSaved();
	bool		HasNewSync();
	
	//
	STimelineSyncManager*	GetTimelineSync() { return m_TimelineSync; }

private:
	FBSystem	mSystem;				//!< System interface.
	long		mCounter;				//!< Time counter for hands.
	
	FBVector3d	mLookAtRootPos;
	FBVector3d	mLookAtLeftPos;
	FBVector3d	mLookAtRightPos;
	
	unsigned int				m_SessionId{ 0 };
	SSharedModelData*			m_Data;
	STimelineSyncManager*		m_TimelineSync{ nullptr };
};
