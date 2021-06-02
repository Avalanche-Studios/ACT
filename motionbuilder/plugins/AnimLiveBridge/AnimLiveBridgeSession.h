
#pragma once

/*
#
# Copyright(c) 2021 Avalanche Studios.All rights reserved.
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

#include "AnimLiveBridge.h"

const int NAME_SIZE = 64;

// forward
class CAnimLiveBridgeSession;

//
class CAnimLiveBridgeHardware
{
public:
	//! a constructor
	CAnimLiveBridgeHardware(CAnimLiveBridgeSession* session)
		: m_Session(session)
	{}

	virtual int TypeId() const = 0;
	virtual const char* TypeStr() const = 0;

	virtual int Open(const char* pair_name, const bool is_server) { return -1; }
	virtual int Close() { return -1; }

	virtual bool IsOpen() const { return false; }

	// collect input before local data is being processed
	virtual int Prepare() { return -1; }
	// sync data (send, share, etc.)
	virtual int Commit(const bool auto_finish_event) { return - 1; }
	virtual int ManualPostCommitFinish() { return -1; }

	CAnimLiveBridgeSession* GetSessionPtr() { return m_Session; }

protected:
	bool			m_IsServer{ true };
	char			m_PairName[NAME_SIZE]{ 0 };

	CAnimLiveBridgeSession*	m_Session{ nullptr };
};

////////////////////////////////////////////////////////////////
// CAnimLiveBridgeSession
class CAnimLiveBridgeSession
{
public:

	CAnimLiveBridgeSession();
	~CAnimLiveBridgeSession();

	int Open(const char* pair_name, const bool is_server);
	int Close();

	bool IsOpen() const;

	SSharedModelData*		GetDataPtr() { return &m_Data; }
	STimelineSyncManager*	GetTimelinePtr() { return &m_TimelineSync; }

	int Commit(const bool auto_finish_event);
	int ManualPostCommitFinish();

public:
	// TODO: need to be refactor
	// lookat sync properties

	SVector4	m_LookAtRootPos{ 0.0f, 0.0f, 0.0f, 0.0f };
	SVector4	m_LookAtLeftPos{ 0.0f, 0.0f, 0.0f, 0.0f };
	SVector4	m_LookAtRightPos{ 0.0f, 0.0f, 0.0f, 0.0f };

	bool m_HasNewSync{ false };
	bool m_SyncSaved{ false };

protected:

	SSharedModelData				m_Data{ 0 };

	// Ownership for the timeline between server and client

	STimelineSyncManager			m_TimelineSync;

	CAnimLiveBridgeHardware*		m_Hardware{ nullptr };
};