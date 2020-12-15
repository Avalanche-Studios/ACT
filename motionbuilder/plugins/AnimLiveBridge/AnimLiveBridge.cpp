
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

// AnimLiveBridge.cpp


#include "AnimLiveBridge.h"

#include <windows.h>

#include <vector>
#include <string>


#define EXPORT_FUNCTION comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

const char* SHARED_MAPPING_PREFIX = "Global\\";
const char* SHARED_MAPPING_DEFNAME = "AnimationBridgePair";

const char* EVENT_TOCLIENT = "_event_to_client";
const char* EVENT_FROMCLIENT = "_event_from_client";

const int NAME_SIZE = 64;
const int BUF_SIZE = sizeof(SSharedModelData);

//

struct SBridgeSession
{
	bool			m_IsServer{ true };
	bool			m_FileOpen{ false };				//!< Is file open?
	HANDLE			m_MapFile{ 0 };

	HANDLE			m_EventToClient{ 0 };
	HANDLE			m_EventFromClient{ 0 };

	char			m_PairName[NAME_SIZE]{ 0 };

	SSharedModelData		m_Data{ 0 };
	
	// Ownership for the timeline between server and client

	STimelineSyncManager	m_TimelineSync;

	// lookat sync properties

	SVector4	m_LookAtRootPos{ 0.0f, 0.0f, 0.0f, 0.0f };
	SVector4	m_LookAtLeftPos{ 0.0f, 0.0f, 0.0f, 0.0f };
	SVector4	m_LookAtRightPos{ 0.0f, 0.0f, 0.0f, 0.0f };

	bool m_HasNewSync{ false };
	bool m_SyncSaved{ false };

};

// store all opened sessions

static std::vector<SBridgeSession>		g_Sessions;
static int								g_VerboseLevel{ 1 };		// 1 - minumum log, 2 - full log info
static CLiveBridgeLogger*				g_Logger{ nullptr };

////////////////////////////////////////
//

void SetVerboseLevel(const int verbose_level)
{
#pragma EXPORT_FUNCTION

	g_VerboseLevel = verbose_level;
}

void SetLiveBridgeLogger(CLiveBridgeLogger* logger)
{
#pragma EXPORT_FUNCTION

	g_Logger = logger;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

SBridgeSession* FindOpenSession(unsigned int session_id, const bool find_only_opened=true)
{
	if (session_id >= static_cast<unsigned int>(g_Sessions.size()))
		return nullptr;

	SBridgeSession* session = &g_Sessions[session_id];

	if (find_only_opened && !session->m_FileOpen)
		return nullptr;

	return session;
}

unsigned int HashPairName(const char* pair_name)
{
#pragma EXPORT_FUNCTION

	unsigned int session_hash = static_cast<unsigned int>(std::hash<std::string>{}(pair_name));
	return session_hash;
}

unsigned int NewLiveSession()
{
#pragma EXPORT_FUNCTION

	SBridgeSession session;

	memset(&session, 0, sizeof(SBridgeSession));
	session.m_FileOpen = false;

	const unsigned int index = static_cast<unsigned int>(g_Sessions.size());
	g_Sessions.emplace_back(session);

	if (g_VerboseLevel && g_Logger)
	{
		std::string info("NewLiveSession ");
		info += std::to_string(index);

		g_Logger->LogInfo(info.c_str());
	}

	return index;
}

bool EraseLiveSession(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (session_id >= static_cast<unsigned int>(g_Sessions.size()))
	{
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[EraseLiveSession] session id is not found ");
			info += std::to_string(session_id);

			g_Logger->LogError(info.c_str());
		}
		return false;
	}
	
	auto iter = g_Sessions.begin() + session_id;

	if (iter->m_FileOpen)
	{
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[EraseLiveSession] closing map file handle for session id ");
			info += std::to_string(session_id);

			g_Logger->LogInfo(info.c_str());
		}

		CloseHandle(iter->m_MapFile);
		iter->m_FileOpen = false;
	}
	else
	{
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[EraseLiveSession] map file is already closed for session id ");
			info += std::to_string(session_id);

			g_Logger->LogWarning(info.c_str());
		}
	}
	

	// TODO: we can't erase element, because some ids could be used and erasing will move indices
	//g_Sessions.erase(iter);
	return true;
}

int HardwareOpen(unsigned int session_id, const char* pair_name, const bool is_server)
{
#pragma EXPORT_FUNCTION

	if (g_VerboseLevel && g_Logger)
	{
		std::string info("[HardwareOpen] HardwareOpen for ");
		info += std::to_string(session_id);
		info += " pair_name ";
		info += pair_name;
		info += (is_server) ? " ; is_server - true" : " ; is_server - false";
		
		g_Logger->LogInfo(info.c_str());
	}

	if (session_id >= static_cast<unsigned int>(g_Sessions.size()))
	{
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[HardwareOpen] session id is not found ");
			info += std::to_string(session_id);

			g_Logger->LogError(info.c_str());
		}

		return -1;
	}
	
	HardwareClose(session_id);

	SBridgeSession& session = g_Sessions[session_id];

	if (!session.m_FileOpen)
	{
		strcpy_s(session.m_PairName, sizeof(char) * NAME_SIZE, pair_name);

		std::string full_pair_name = SHARED_MAPPING_PREFIX;
		full_pair_name += session.m_PairName;

		std::string event_toclient_name = SHARED_MAPPING_PREFIX;
		event_toclient_name += session.m_PairName;
		event_toclient_name += EVENT_TOCLIENT;

		std::string event_fromclient_name = SHARED_MAPPING_PREFIX;
		event_fromclient_name += session.m_PairName;
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
		session.m_IsServer = is_server;

		if (is_server)
		{
			session.m_MapFile = CreateFileMapping(
				INVALID_HANDLE_VALUE,    // use paging file
				NULL,                    // default security
				PAGE_READWRITE,          // read/write access
				0,                       // maximum object size (high-order DWORD)
				BUF_SIZE,                // maximum object size (low-order DWORD)
				full_pair_name.c_str());                 // name of mapping object

			session.m_FileOpen = true;

			if (session.m_MapFile == NULL)
			{
				const int err = static_cast<int>(GetLastError());

				if (g_VerboseLevel && g_Logger)
				{
					std::string info("[HardwareOpen] Failed to CreateFileMapping, error - ");
					info += std::to_string(err);
					
					g_Logger->LogError(info.c_str());
				}

				session.m_FileOpen = false;
				return err;
			}

			//
			
			session.m_EventToClient = CreateEvent(nullptr, FALSE, FALSE, event_toclient_name.c_str());
			session.m_EventFromClient = CreateEvent(nullptr, FALSE, TRUE, event_fromclient_name.c_str());

			if (session.m_EventToClient == NULL || session.m_EventFromClient == NULL)
			{
				const int err = static_cast<int>(GetLastError());

				if (g_VerboseLevel && g_Logger)
				{
					std::string info("[HardwareOpen] Failed to CreateEvent, error - ");
					info += std::to_string(err);

					g_Logger->LogError(info.c_str());
				}

				session.m_FileOpen = false;
				return err;
			}

			char *buffer = static_cast<char*>(MapViewOfFile(session.m_MapFile,
				FILE_MAP_WRITE, // write permission
				0,
				0,
				BUF_SIZE));

			if (buffer == nullptr)
			{
				CloseHandle(session.m_MapFile);

				const int err = static_cast<int>(GetLastError());
				if (g_VerboseLevel && g_Logger)
				{
					std::string info("[HardwareOpen] Failed to MapViewOfFile, error - ");
					info += std::to_string(err);

					g_Logger->LogError(info.c_str());
				}

				session.m_FileOpen = false;
				return err;
			}

			memset(buffer, 0, BUF_SIZE);

			UnmapViewOfFile(buffer);

			if (SetEvent(session.m_EventToClient) == FALSE)
			{
				const int err = static_cast<int>(GetLastError());
				if (g_VerboseLevel && g_Logger)
				{
					std::string info("[HardwareOpen] Failed to SetEvent ToClient, error - ");
					info += std::to_string(err);

					g_Logger->LogError(info.c_str());
				}

				session.m_FileOpen = false;
				return err;
			}
		}
		else
		{
			// client
			session.m_MapFile = OpenFileMapping(
				FILE_MAP_ALL_ACCESS, // read/write access
				FALSE, // don't inherit the name
				full_pair_name.c_str()	// name of mapping object
			);

			session.m_FileOpen = true;

			if (session.m_MapFile == NULL)
			{
				const int err = static_cast<int>(GetLastError());
				if (g_VerboseLevel && g_Logger)
				{
					std::string info("[HardwareOpen] Failed to OpenFileMapping, error - ");
					info += std::to_string(err);

					g_Logger->LogError(info.c_str());
				}

				session.m_FileOpen = false;
				return err;
			}

			session.m_EventToClient = OpenEvent(EVENT_ALL_ACCESS, FALSE, event_toclient_name.c_str());
			session.m_EventFromClient = OpenEvent(EVENT_ALL_ACCESS, FALSE, event_fromclient_name.c_str());

			if (session.m_EventToClient == NULL || session.m_EventFromClient == NULL)
			{
				const int err = static_cast<int>(GetLastError());
				if (g_VerboseLevel && g_Logger)
				{
					std::string info("[HardwareOpen] Failed to OpenEvent, error - ");
					info += std::to_string(err);

					g_Logger->LogError(info.c_str());
				}

				session.m_FileOpen = false;
				return err;
			}

			//session.m_EventToClient = CreateEvent(NULL, false, false, event_toclient_name.c_str());
			//session.m_EventFromClient = CreateEvent(NULL, false, false, event_fromclient_name.c_str());
		}
	}

	return 0;
}



int HardwareClose(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id))
	{
		if (session->m_FileOpen)
		{
			if (g_VerboseLevel && g_Logger)
			{
				std::string info("[HardwareClose] closing map file handle for session id ");
				info += std::to_string(session_id);

				g_Logger->LogInfo(info.c_str());
			}

			CloseHandle(session->m_MapFile);
			session->m_FileOpen = false;
		}
	}
	return 0;
}

SSharedModelData* MapModelData(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id))
	{
		return &session->m_Data;
	}
	return nullptr;
}

bool SetModelDataJoints(unsigned int session_id, const std::vector<SJointData>& data)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id))
	{
		const size_t joints_len = static_cast<size_t>(NUMBER_OF_JOINTS);
		const size_t len = (data.size() < joints_len) ? data.size() : joints_len;
		memcpy_s(session->m_Data.m_Joints.m_Data, sizeof(SJointData)*joints_len, data.data(), sizeof(SJointData) * len);
		return true;
	}
	return false;
}

bool SetModelDataProperties(unsigned int session_id, const std::vector<SPropertyData>& data)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id))
	{
		const size_t max_len = static_cast<size_t>(NUMBER_OF_JOINTS);
		const size_t len = (data.size() < max_len) ? data.size() : max_len;

		memcpy_s(session->m_Data.m_Properties.m_Data, sizeof(SPropertyData)*max_len, data.data(), sizeof(SPropertyData) * len);
		return true;
	}
	return false;
}

void UnMapModelData(unsigned int session_id)
{
#pragma EXPORT_FUNCTION
	// TODO: if we think to make a thread safe data access
}

STimelineSyncManager* MapTimelineSync(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id))
	{
		return &session->m_TimelineSync;
	}
	return nullptr;
}

void UnMapTimelineSync(unsigned int session_id)
{
#pragma EXPORT_FUNCTION
	// TODO: if we think to make a thread safe data access
}

// server send data
bool SetServerFinishEvent(SBridgeSession &session)
{
	if (g_VerboseLevel > 1 && g_Logger)
	{
		std::string info("[SetServerFinishEvent] SetEvent eventToClient ");
		g_Logger->LogInfo(info.c_str());
	}
	if (SetEvent(session.m_EventToClient) == FALSE)
	{
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[SetServerFinishEvent] Failed to SetEvent eventToClient ");
			g_Logger->LogError(info.c_str());
		}
	}
	return true;
}

int FlushServerData(SBridgeSession &session, const bool auto_finish_event)
{
	if (!session.m_FileOpen)
		return -3;

	if (WaitForSingleObjectEx(session.m_EventFromClient, 0, FALSE) != WAIT_TIMEOUT)
	{
		char *buffer = static_cast<char*>(MapViewOfFile(session.m_MapFile,
			FILE_MAP_ALL_ACCESS, // write permission
			0,
			0,
			BUF_SIZE));
		
		if (buffer == nullptr)
			return -2;

		// read client time and sync with a server time
		SSharedModelData *client_data = reinterpret_cast<SSharedModelData*>(buffer);

		session.m_TimelineSync.ReadFromData(false, *client_data);
		session.m_TimelineSync.WriteToData(true, session.m_Data);

		for (int i = 0; i < 4; ++i)
		{
			session.m_Data.m_LookAtRoot[i] = client_data->m_LookAtRoot[i];
			session.m_Data.m_LookAtLeft[i] = client_data->m_LookAtLeft[i];
			session.m_Data.m_LookAtRight[i] = client_data->m_LookAtRight[i];
		}

		// check if we have sync event from client
		if (session.m_Data.m_LookAtRoot[3] == 1.0f)
		{
			session.m_HasNewSync = true;
			session.m_Data.m_LookAtRoot[3] = 0.0f;
		}
		else if (session.m_SyncSaved)
		{
			session.m_Data.m_LookAtRoot[3] = 2.0f;
		}
		session.m_SyncSaved = false;

		// write server data

		CopyMemory(static_cast<PVOID>(buffer), &session.m_Data, sizeof(char)*BUF_SIZE);

		UnmapViewOfFile(buffer);

		memcpy_s(&session.m_LookAtRootPos.m_X, sizeof(SVector4), session.m_Data.m_LookAtRoot, sizeof(float) * 4);
		memcpy_s(&session.m_LookAtLeftPos.m_X, sizeof(SVector4), session.m_Data.m_LookAtLeft, sizeof(float) * 4);
		memcpy_s(&session.m_LookAtRightPos.m_X, sizeof(SVector4), session.m_Data.m_LookAtRight, sizeof(float) * 4);

		//
		if (auto_finish_event)
		{
			SetServerFinishEvent(session);
		}
		return 0;
	}

	return -1;
}

// client poll data
bool SetClientFinishEvent(SBridgeSession &session)
{
	if (g_VerboseLevel > 1 && g_Logger)
	{
		std::string info("[SetClientFinishEvent] SetEvent eventFromClient ");
		g_Logger->LogInfo(info.c_str());
	}
	if (SetEvent(session.m_EventFromClient) == FALSE)
	{
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[SetClientFinishEvent] Failed to SetEvent eventFromClient ");
			g_Logger->LogInfo(info.c_str());
		}
	}
	return true;
}

int FlushClientData(SBridgeSession &session, const bool auto_finish_event)
{
	if (!session.m_FileOpen)
		return -3;

	if (WaitForSingleObjectEx(session.m_EventToClient, 0, false) != WAIT_TIMEOUT)
	{
		char *buffer = (char*)MapViewOfFile(session.m_MapFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			BUF_SIZE);

		if (buffer == nullptr)
			return -2;

		SSharedModelData* server_data = (SSharedModelData*)buffer;

		session.m_TimelineSync.ReadFromData(true, *server_data);
		session.m_TimelineSync.WriteToData(false, *server_data);

		// check if we have sync event from server
		if (server_data->m_LookAtRoot[3] == 2.0f)
		{
			session.m_HasNewSync = true;
		}

		// look at pos
		server_data->m_LookAtRoot[0] = session.m_LookAtRootPos.m_X;
		server_data->m_LookAtRoot[1] = session.m_LookAtRootPos.m_Y;
		server_data->m_LookAtRoot[2] = session.m_LookAtRootPos.m_Z;
		server_data->m_LookAtRoot[3] = (session.m_SyncSaved) ? 1.0f : 0.0f;

		server_data->m_LookAtLeft[0] = session.m_LookAtLeftPos.m_X;
		server_data->m_LookAtLeft[1] = session.m_LookAtLeftPos.m_Y;
		server_data->m_LookAtLeft[2] = session.m_LookAtLeftPos.m_Z;

		server_data->m_LookAtRight[0] = session.m_LookAtRightPos.m_X;
		server_data->m_LookAtRight[1] = session.m_LookAtRightPos.m_Y;
		server_data->m_LookAtRight[2] = session.m_LookAtRightPos.m_Z;

		session.m_SyncSaved = false;

		memcpy(&session.m_Data, server_data, sizeof(SSharedModelData));

		UnmapViewOfFile(buffer);

		//

		if (auto_finish_event)
		{
			SetClientFinishEvent(session);
		}
		
		return 0;
	}
	return -1;
}



int FlushData(unsigned int session_id, const bool auto_finish_event)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id))
	{
		if (session->m_IsServer)
		{
			return FlushServerData(*session, auto_finish_event);
		}

		return FlushClientData(*session, auto_finish_event);
	}

	return false;
}

bool SetFinishEvent(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id))
	{
		if (session->m_IsServer)
		{
			return SetServerFinishEvent(*session);
		}

		return SetClientFinishEvent(*session);
	}
	return false;
}

void SetModelJointData(SSharedModelData* model_data, const std::vector<SJointData>& data)
{
#pragma EXPORT_FUNCTION

	if (!model_data || data.empty())
		return;

	const size_t len = (data.size() < NUMBER_OF_JOINTS) ? data.size() : NUMBER_OF_JOINTS;
	memcpy(model_data->m_Joints.m_Data, data.data(), sizeof(SJointData) * len);
}

void SetModelPropertyData(SSharedModelData* model_data, const std::vector<SPropertyData>& data)
{
#pragma EXPORT_FUNCTION

	if (!model_data || data.empty())
		return;

	const size_t len = (data.size() < NUMBER_OF_PROPERTIES) ? data.size() : NUMBER_OF_PROPERTIES;
	memcpy(model_data->m_Properties.m_Data, data.data(), sizeof(SPropertyData) * len);
}

bool SetLookAtVectors(unsigned int session_id, const SVector4& lookat_root, const SVector4& lookat_left, const SVector4& lookat_right)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id, false))
	{
		session->m_LookAtRootPos = lookat_root;
		session->m_LookAtLeftPos = lookat_left;
		session->m_LookAtRightPos = lookat_right;
		return true;
	}
	return false;
}

bool GetLookAtVectors(unsigned int session_id, SVector4& lookat_root, SVector4& lookat_left, SVector4& lookat_right)
{
#pragma EXPORT_FUNCTION
	
	if (SBridgeSession* session = FindOpenSession(session_id, false))
	{
		lookat_root = session->m_LookAtRootPos;
		lookat_left = session->m_LookAtLeftPos;
		lookat_right = session->m_LookAtRightPos;
		return true;
	}
	return false;
}

void SetHasNewSync(unsigned int session_id, const bool value)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id, false))
	{
		session->m_HasNewSync = value;
	}
}

bool GetHasNewSync(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id, false))
	{
		return session->m_HasNewSync;
	}
	return false;
}

bool GetAndResetHasNewSync(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id, false))
	{
		const bool value{ session->m_HasNewSync };
		session->m_HasNewSync = false;
		return value;
	}
	return false;
}

void SetSyncSaved(unsigned int session_id, const bool value)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id, false))
	{
		session->m_SyncSaved = value;
	}
}

bool GetSyncSaved(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (SBridgeSession* session = FindOpenSession(session_id, false))
	{
		return session->m_SyncSaved;
	}
	return false;
}

/////////////////////////////////////////////////////////////////
// STimelineSyncManager

STimelineSyncManager::STimelineSyncManager()
{
}

void STimelineSyncManager::SetLocalTimeline(const double local_time, const bool is_playing)
{
	//FBTime localTime(mSystem.LocalTime);
	//mIsPlaying = mPlayerControl.IsPlaying;
	
	if (m_IsPlaying)
	{
		m_LocalTimeChanged = true;
		m_LocalLastTime = local_time;

		m_RemoteTimeChanged = false;
	}
	else if (local_time != m_LocalLastTime && false == m_RemoteTimeChanged)
	{
		m_LocalTimeChanged = true;
		m_LocalLastTime = local_time;
	}
}

// we could read from server or client data values
void STimelineSyncManager::ReadFromData(const bool is_server, SSharedModelData& data)
{
	SPlayerInfo& player_info = (is_server) ? data.m_ServerPlayer : data.m_ClientPlayer;

	//const double secs = playerInfo.m_LocalTime + mOffsetTime.GetSecondDouble();
	const double secs = player_info.m_LocalTime + m_OffsetTime;
	
	if (player_info.m_TimeChangedEvent > 0.0f)
	{
		player_info.m_TimeChangedEvent = 0.0f;

		m_RemoteTimeChanged = true;
		m_RemoteLastTime = secs;
	}
}

// write to server or client data values
void STimelineSyncManager::WriteToData(const bool is_server, SSharedModelData& data)
{
	SPlayerInfo& playerInfo = (is_server) ? data.m_ServerPlayer : data.m_ClientPlayer;

	//CheckLocalTimeline();

	if (!m_RemoteTimeChanged && (m_LocalTimeChanged || m_IsPlaying))
	{
		m_LocalTimeChanged = false;
		m_LocalLastTime = m_LocalTime; // mSystem.LocalTime;

		playerInfo.m_LocalTime = m_LocalLastTime - m_OffsetTime;
		playerInfo.m_TimeChangedEvent = 1.0f;
	}
	else
	{
		playerInfo.m_TimeChangedEvent = 0.0f;
	}
}

bool STimelineSyncManager::CheckForARemoteTimeControl(double& remote_time, const double local_time, const bool is_playing)
{
	if (m_RemoteTimeChanged)
	{
		m_RemoteTimeChanged = false;

		//
		SetLocalTime(local_time);
		SetIsPlaying(is_playing);

		if (m_RemoteLastTime != local_time && !m_IsPlaying)
		{
			remote_time = m_RemoteLastTime;

			//
			m_LocalTimeChanged = false;
			m_LocalLastTime = m_RemoteLastTime;

			return true;
		}
	}
	return false;
}

void STimelineSyncManager::SetLocalTime(const double time)
{
	m_LocalTime = time;
}

void STimelineSyncManager::SetOffsetTime(const double time)
{
	m_OffsetTime = time;
}

void STimelineSyncManager::SetIsPlaying(const bool value)
{
	m_IsPlaying = value;
}

const bool STimelineSyncManager::IsRemoteTimeChanged() const
{
	return m_RemoteTimeChanged;
}

const double STimelineSyncManager::GetRemoteTime() const
{
	return m_RemoteLastTime;
}