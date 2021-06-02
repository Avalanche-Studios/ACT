
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

#include <windows.h>
#include "AnimLiveBridgeSession.h"

///////////////////////////////////////////////////////////
// CAnimLiveBridgeSharedMemory

class CAnimLiveBridgeSharedMemory : public CAnimLiveBridgeHardware
{
public:
	//! a constructor
	CAnimLiveBridgeSharedMemory(CAnimLiveBridgeSession* session)
		: CAnimLiveBridgeHardware(session)
	{}

	int TypeId() const override { return 1; }
	const char* TypeStr() const override { return "SharedMemory"; }

	int Open(const char* pair_name, const bool is_server) override;
	int Close() override;

	virtual bool IsOpen() const { return m_FileOpen; }

	int Commit(const bool auto_finish_event) override;
	int ManualPostCommitFinish() override;

protected:

	bool			m_FileOpen{ false };				//!< Is file open?
	HANDLE			m_MapFile{ 0 };

	HANDLE			m_EventToClient{ 0 };
	HANDLE			m_EventFromClient{ 0 };

	int OpenServer(const char* full_pair_name, const char* event_toclient_name, const char* event_fromclient_name);
	int OpenClient(const char* full_pair_name, const char* event_toclient_name, const char* event_fromclient_name);

	int CommitServer(const bool auto_finish_event);
	int CommitClient(const bool auto_finish_event);

	int SetServerFinishEvent();
	int SetClientFinishEvent();
};