
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

#include "AnimLiveBridgeSharedMemory.h"
#include <string>

const char* SHARED_MAPPING_PREFIX = "Global\\";
const char* SHARED_MAPPING_DEFNAME = "AnimationBridgePair";

const char* EVENT_TOCLIENT = "_event_to_client";
const char* EVENT_FROMCLIENT = "_event_from_client";


const int BUF_SIZE = sizeof(SSharedModelData);

extern int										g_VerboseLevel{ 1 };		// 1 - minumum log, 2 - full log info
extern CLiveBridgeLogger*						g_Logger{ nullptr };

//////////////////////////////////////////////////////////////////////////////////
//

int CAnimLiveBridgeSharedMemory::Open(const char* pair_name, const bool is_server)
{
	if (IsOpen())
		return 1;

	strcpy_s(m_PairName, sizeof(char) * NAME_SIZE, pair_name);

	std::string full_pair_name = SHARED_MAPPING_PREFIX;
	full_pair_name += m_PairName;

	std::string event_toclient_name = SHARED_MAPPING_PREFIX;
	event_toclient_name += m_PairName;
	event_toclient_name += EVENT_TOCLIENT;

	std::string event_fromclient_name = SHARED_MAPPING_PREFIX;
	event_fromclient_name += m_PairName;
	event_fromclient_name += EVENT_FROMCLIENT;


	if (g_VerboseLevel && g_Logger)
	{
		std::string info("[HardwareOpen] Open with names ");
		info += " file mapping - ";
		info += full_pair_name;
		info += "; to_client_event - ";
		info += event_toclient_name;
		info += "; from_client_event - ";
		info += event_fromclient_name;

		g_Logger->LogInfo(info.c_str());
	}

	//
	m_IsServer = is_server;
	return (is_server) ? OpenServer(full_pair_name.c_str(), event_toclient_name.c_str(), event_fromclient_name.c_str()) 
		: OpenClient(full_pair_name.c_str(), event_toclient_name.c_str(), event_fromclient_name.c_str());
}

int CAnimLiveBridgeSharedMemory::OpenServer(const char* full_pair_name, const char* event_toclient_name, const char* event_fromclient_name)
{
	m_MapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		full_pair_name);                 // name of mapping object

	m_FileOpen = true;

	if (m_MapFile == NULL)
	{
		const int err = static_cast<int>(GetLastError());

		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[HardwareOpen] Failed to CreateFileMapping, error - ");
			info += std::to_string(err);

			g_Logger->LogError(info.c_str());
		}

		m_FileOpen = false;
		return err;
	}

	//

	m_EventToClient = CreateEvent(nullptr, FALSE, FALSE, event_toclient_name);
	m_EventFromClient = CreateEvent(nullptr, FALSE, TRUE, event_fromclient_name);

	if (m_EventToClient == NULL || m_EventFromClient == NULL)
	{
		const int err = static_cast<int>(GetLastError());

		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[HardwareOpen] Failed to CreateEvent, error - ");
			info += std::to_string(err);

			g_Logger->LogError(info.c_str());
		}

		m_FileOpen = false;
		return err;
	}

	char *buffer = static_cast<char*>(MapViewOfFile(m_MapFile,
		FILE_MAP_WRITE, // write permission
		0,
		0,
		BUF_SIZE));

	if (buffer == nullptr)
	{
		CloseHandle(m_MapFile);

		const int err = static_cast<int>(GetLastError());
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[HardwareOpen] Failed to MapViewOfFile, error - ");
			info += std::to_string(err);

			g_Logger->LogError(info.c_str());
		}

		m_FileOpen = false;
		return err;
	}

	memset(buffer, 0, BUF_SIZE);

	UnmapViewOfFile(buffer);

	if (SetEvent(m_EventToClient) == FALSE)
	{
		const int err = static_cast<int>(GetLastError());
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[HardwareOpen] Failed to SetEvent ToClient, error - ");
			info += std::to_string(err);

			g_Logger->LogError(info.c_str());
		}

		m_FileOpen = false;
		return err;
	}
	return 0;
}

int CAnimLiveBridgeSharedMemory::OpenClient(const char* full_pair_name, const char* event_toclient_name, const char* event_fromclient_name)
{
	// client
	m_MapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS, // read/write access
		FALSE, // don't inherit the name
		full_pair_name	// name of mapping object
	);

	m_FileOpen = true;

	if (m_MapFile == NULL)
	{
		const int err = static_cast<int>(GetLastError());
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[HardwareOpen] Failed to OpenFileMapping, error - ");
			info += std::to_string(err);

			g_Logger->LogError(info.c_str());
		}

		m_FileOpen = false;
		return err;
	}

	m_EventToClient = OpenEvent(EVENT_ALL_ACCESS, FALSE, event_toclient_name);
	m_EventFromClient = OpenEvent(EVENT_ALL_ACCESS, FALSE, event_fromclient_name);

	if (m_EventToClient == NULL || m_EventFromClient == NULL)
	{
		const int err = static_cast<int>(GetLastError());
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[HardwareOpen] Failed to OpenEvent, error - ");
			info += std::to_string(err);

			g_Logger->LogError(info.c_str());
		}

		m_FileOpen = false;
		return err;
	}

	return 0;
}

int CAnimLiveBridgeSharedMemory::Commit(const bool auto_finish_event)
{
	return (m_IsServer) ? CommitServer(auto_finish_event) : CommitClient(auto_finish_event);
}

int CAnimLiveBridgeSharedMemory::CommitServer(const bool auto_finish_event)
{
	if (!IsOpen())
		return -3;

	if (WaitForSingleObjectEx(m_EventFromClient, 0, FALSE) != WAIT_TIMEOUT)
	{
		char *buffer = static_cast<char*>(MapViewOfFile(m_MapFile,
			FILE_MAP_ALL_ACCESS, // write permission
			0,
			0,
			BUF_SIZE));

		if (buffer == nullptr)
			return -2;

		// read client time and sync with a server time
		SSharedModelData* shared_data = reinterpret_cast<SSharedModelData*>(buffer);
		SSharedModelData* local_data = GetSessionPtr()->GetDataPtr();

		GetSessionPtr()->GetTimelinePtr()->ReadFromData(false, *shared_data);
		GetSessionPtr()->GetTimelinePtr()->WriteToData(true, *local_data);

		for (int i = 0; i < 4; ++i)
		{
			local_data->m_LookAtRoot[i] = shared_data->m_LookAtRoot[i];
			local_data->m_LookAtLeft[i] = shared_data->m_LookAtLeft[i];
			local_data->m_LookAtRight[i] = shared_data->m_LookAtRight[i];
		}

		// check if we have sync event from client
		if (local_data->m_LookAtRoot[3] == 1.0f)
		{
			GetSessionPtr()->m_HasNewSync = true;
			local_data->m_LookAtRoot[3] = 0.0f;
		}
		else if (GetSessionPtr()->m_SyncSaved)
		{
			local_data->m_LookAtRoot[3] = 2.0f;
		}
		GetSessionPtr()->m_SyncSaved = false;

		local_data->m_Header.m_ClientTag = shared_data->m_Header.m_ClientTag;

		// write server data

		CopyMemory(static_cast<PVOID>(buffer), GetSessionPtr()->GetDataPtr(), sizeof(char)*BUF_SIZE);

		UnmapViewOfFile(buffer);

		memcpy_s(&GetSessionPtr()->m_LookAtRootPos.m_X, sizeof(SVector4), local_data->m_LookAtRoot, sizeof(float) * 4);
		memcpy_s(&GetSessionPtr()->m_LookAtLeftPos.m_X, sizeof(SVector4), local_data->m_LookAtLeft, sizeof(float) * 4);
		memcpy_s(&GetSessionPtr()->m_LookAtRightPos.m_X, sizeof(SVector4), local_data->m_LookAtRight, sizeof(float) * 4);
		
		//
		if (auto_finish_event)
		{
			SetServerFinishEvent();
		}
		return 0;
	}

	return -1;
}

int CAnimLiveBridgeSharedMemory::CommitClient(const bool auto_finish_event)
{
	if (!IsOpen())
		return -3;

	if (WaitForSingleObjectEx(m_EventToClient, 0, false) != WAIT_TIMEOUT)
	{
		char *buffer = (char*)MapViewOfFile(m_MapFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			BUF_SIZE);

		if (buffer == nullptr)
			return -2;

		SSharedModelData* shared_data = reinterpret_cast<SSharedModelData*>(buffer);
		SSharedModelData* local_data = GetSessionPtr()->GetDataPtr();

		if (STimelineSyncManager* timeline = GetSessionPtr()->GetTimelinePtr())
		{
			timeline->ReadFromData(true, *shared_data);
			timeline->WriteToData(false, *shared_data);
		}
		
		// check if we have sync event from master
		if (shared_data->m_LookAtRoot[3] == 2.0f)
		{
			GetSessionPtr()->m_HasNewSync = true;
		}

		shared_data->m_Header.m_ClientTag = local_data->m_Header.m_ClientTag;

		// look at pos
		shared_data->m_LookAtRoot[0] = GetSessionPtr()->m_LookAtRootPos.m_X;
		shared_data->m_LookAtRoot[1] = GetSessionPtr()->m_LookAtRootPos.m_Y;
		shared_data->m_LookAtRoot[2] = GetSessionPtr()->m_LookAtRootPos.m_Z;
		shared_data->m_LookAtRoot[3] = (GetSessionPtr()->m_SyncSaved) ? 1.0f : 0.0f;

		shared_data->m_LookAtLeft[0] = GetSessionPtr()->m_LookAtLeftPos.m_X;
		shared_data->m_LookAtLeft[1] = GetSessionPtr()->m_LookAtLeftPos.m_Y;
		shared_data->m_LookAtLeft[2] = GetSessionPtr()->m_LookAtLeftPos.m_Z;

		shared_data->m_LookAtRight[0] = GetSessionPtr()->m_LookAtRightPos.m_X;
		shared_data->m_LookAtRight[1] = GetSessionPtr()->m_LookAtRightPos.m_Y;
		shared_data->m_LookAtRight[2] = GetSessionPtr()->m_LookAtRightPos.m_Z;

		GetSessionPtr()->m_SyncSaved = false;

		memcpy(local_data, shared_data, sizeof(SSharedModelData));

		UnmapViewOfFile(buffer);

		//

		if (auto_finish_event)
		{
			SetClientFinishEvent();
		}

		return 0;
	}
	return -1;
}

int CAnimLiveBridgeSharedMemory::ManualPostCommitFinish()
{
	return (m_IsServer) ? SetServerFinishEvent() : SetClientFinishEvent();
}

int CAnimLiveBridgeSharedMemory::SetServerFinishEvent()
{
	if (g_VerboseLevel > 1 && g_Logger)
	{
		g_Logger->LogInfo("[SetServerFinishEvent] SetEvent eventToClient");
	}
	if (SetEvent(m_EventToClient) == FALSE)
	{
		if (g_VerboseLevel && g_Logger)
		{
			
			g_Logger->LogError("[SetServerFinishEvent] Failed to SetEvent eventToClient ");
		}
	}
	return true;
}

int CAnimLiveBridgeSharedMemory::SetClientFinishEvent()
{
	if (g_VerboseLevel > 1 && g_Logger)
	{
		g_Logger->LogInfo("[SetClientFinishEvent] SetEvent eventFromClient ");
	}
	if (SetEvent(m_EventFromClient) == FALSE)
	{
		if (g_VerboseLevel && g_Logger)
		{
			g_Logger->LogInfo("[SetClientFinishEvent] Failed to SetEvent eventFromClient ");
		}
	}
	return true;
}

int CAnimLiveBridgeSharedMemory::Close()
{
	if (m_MapFile)
	{
		CloseHandle(m_MapFile);
		m_MapFile = 0;
	}
	m_FileOpen = false;
	return 0;
}