
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
#include "AnimLiveBridgeSession.h"

#include <windows.h>

#include <vector>
#include <string>

#define EXPORT_FUNCTION comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

// store all opened sessions

static std::vector<CAnimLiveBridgeSession*>		g_Sessions;
static int										g_VerboseLevel{ 1 };		// 1 - minumum log, 2 - full log info
static CLiveBridgeLogger*						g_Logger{ nullptr };

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

CAnimLiveBridgeSession* FindOpenSession(unsigned int session_id, const bool find_only_opened=true)
{
	if (session_id >= static_cast<unsigned int>(g_Sessions.size()))
		return nullptr;

	CAnimLiveBridgeSession* session = g_Sessions[session_id];

	// session was already free
	if (session == nullptr)
		return nullptr;

	if (find_only_opened && !session->IsOpen())
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

	size_t index = g_Sessions.size();

	for (size_t i = 0; i < g_Sessions.size(); ++i)
		if (g_Sessions[i] == nullptr)
		{
			index = i;
			break;
		}

	CAnimLiveBridgeSession* session = new CAnimLiveBridgeSession();

	if (g_VerboseLevel && g_Logger)
	{
		std::string info("NewLiveSession ");
		info += std::to_string(index);

		g_Logger->LogInfo(info.c_str());
	}

	if (index >= g_Sessions.size())
		g_Sessions.emplace_back(session);
	else
		g_Sessions[index] = session;

	return static_cast<unsigned int>(index);
}

bool FreeLiveSession(unsigned int session_id)
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
	CAnimLiveBridgeSession* session = *iter;

	if (session == nullptr)
		return true;

	if (session->IsOpen())
	{
		if (g_VerboseLevel && g_Logger)
		{
			std::string info("[EraseLiveSession] closing map file handle for session id ");
			info += std::to_string(session_id);

			g_Logger->LogInfo(info.c_str());
		}

		session->Close();
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
	
	*iter = nullptr;
	delete session;

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

	if (g_Sessions[session_id] == nullptr)
		return -1;

	CAnimLiveBridgeSession& session = *g_Sessions[session_id];

	if (!session.IsOpen())
	{
		return session.Open(pair_name, is_server);
	}

	return 0;
}



int HardwareClose(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id))
	{
		if (session->IsOpen())
		{
			if (g_VerboseLevel && g_Logger)
			{
				std::string info("[HardwareClose] closing map file handle for session id ");
				info += std::to_string(session_id);

				g_Logger->LogInfo(info.c_str());
			}

			session->Close();
		}
	}
	return 0;
}

SSharedModelData* MapModelData(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id))
	{
		return session->GetDataPtr();
	}
	return nullptr;
}

bool SetModelDataJoints(unsigned int session_id, const std::vector<SJointData>& data)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id))
	{
		const size_t joints_len = static_cast<size_t>(NUMBER_OF_JOINTS);
		const size_t len = (data.size() < joints_len) ? data.size() : joints_len;
		memcpy_s(session->GetDataPtr()->m_Joints.m_Data, sizeof(SJointData)*joints_len, data.data(), sizeof(SJointData) * len);
		return true;
	}
	return false;
}

bool SetModelDataProperties(unsigned int session_id, const std::vector<SPropertyData>& data)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id))
	{
		const size_t max_len = static_cast<size_t>(NUMBER_OF_JOINTS);
		const size_t len = (data.size() < max_len) ? data.size() : max_len;

		memcpy_s(session->GetDataPtr()->m_Properties.m_Data, sizeof(SPropertyData)*max_len, data.data(), sizeof(SPropertyData) * len);
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

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id))
	{
		return session->GetTimelinePtr();
	}
	return nullptr;
}

void UnMapTimelineSync(unsigned int session_id)
{
#pragma EXPORT_FUNCTION
	// TODO: if we think to make a thread safe data access
}

int HardwareCommit(unsigned int session_id, const bool auto_finish_event)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id))
	{
		return session->Commit(auto_finish_event);
	}
	
	return -1;
}

bool SetFinishEvent(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id))
	{
		session->ManualPostCommitFinish();
		return true;
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

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id, false))
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
	
	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id, false))
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

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id, false))
	{
		session->m_HasNewSync = value;
	}
}

bool GetHasNewSync(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id, false))
	{
		return session->m_HasNewSync;
	}
	return false;
}

bool GetAndResetHasNewSync(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id, false))
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

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id, false))
	{
		session->m_SyncSaved = value;
	}
}

bool GetSyncSaved(unsigned int session_id)
{
#pragma EXPORT_FUNCTION

	if (CAnimLiveBridgeSession* session = FindOpenSession(session_id, false))
	{
		return session->m_SyncSaved;
	}
	return false;
}

void SetLiveSessionPropertyInt(unsigned int session_id, unsigned int property_id, int value)
{
#pragma EXPORT_FUNCTION
}

void SetLiveSessionPropertyString(unsigned int session_id, unsigned int property_id, const char* value)
{
#pragma EXPORT_FUNCTION
}

int GetLiveSessionPropertyInt(unsigned int session_id, unsigned int property_id)
{
#pragma EXPORT_FUNCTION
	return 0;
}

const char* GetLiveSessionPropertyString(unsigned int session_id, unsigned int property_id)
{
#pragma EXPORT_FUNCTION
	return nullptr;
}

/////////////////////////////////////////////////////////////////
// STimelineSyncManager

STimelineSyncManager::STimelineSyncManager(bool is_server)
	: m_IsServer(is_server)
{
}

void STimelineSyncManager::Initialize(const bool is_server, SSharedModelData* data)
{
	m_IsServer = is_server;
	m_Data = data;
}

void STimelineSyncManager::SetLocalTimeline(const double local_time, const bool is_playing)
{
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
void STimelineSyncManager::ReadFromData(const bool is_server_data, SSharedModelData& data)
{
	SPlayerInfo& player_info = (is_server_data) ? data.m_ServerPlayer : data.m_ClientPlayer;

	const double secs = player_info.m_LocalTime + m_OffsetTime;
	
	if (player_info.m_TimeChangedEvent > 0.0f)
	{
		player_info.m_TimeChangedEvent = 0.0f;

		m_RemoteTimeChanged = true;
		m_RemoteLastTime = secs;
	}
}

// write to server or client data values
void STimelineSyncManager::WriteToData(const bool is_server_data, SSharedModelData& data)
{
	SPlayerInfo& playerInfo = (is_server_data) ? data.m_ServerPlayer : data.m_ClientPlayer;

	if (!m_RemoteTimeChanged && (m_LocalTimeChanged || m_IsPlaying))
	{
		m_LocalTimeChanged = false;
		m_LocalLastTime = m_LocalTime;

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