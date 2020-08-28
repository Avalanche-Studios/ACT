
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

#include <AnimLiveBridge/AnimLiveBridge.h>

#include <windows.h>

#include <map>
#include <string>

using namespace NAnimationLiveBridge;


#define EXPORT_FUNCTION comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

const char* SHARED_MAPPING_PREFIX = "Global\\";
const char* SHARED_MAPPING_DEFNAME = "AnimationBridgePair";

const char* EVENT_TOCLIENT = "_event_to_client";
const char* EVENT_FROMCLIENT = "_event_from_client";

const int32_t NAME_SIZE = 64;
const int32_t BUF_SIZE = sizeof(SSharedModelData);

//

struct SBridgeSession
{
	bool			m_FileOpen;				//!< Is file open?
	HANDLE			m_MapFile;

	HANDLE			m_EventToClient;
	HANDLE			m_EventFromClient;

	char			m_PairName[NAME_SIZE];

	SSharedModelData		m_Data;
};

// store all opened sessions
std::map<uint32_t, SBridgeSession>	g_Sessions;


uint32_t __stdcall HashPairName(const char* pair_name)
{
#pragma EXPORT_FUNCTION

	uint32_t session_hash = static_cast<uint32_t>(std::hash<std::string>{}(pair_name));
	return session_hash;
}


int32_t __stdcall HardwareOpen(const char* pair_name, const bool server)
{
#pragma EXPORT_FUNCTION

	const uint32_t pair_hash = static_cast<uint32_t>(std::hash<std::string>{}(pair_name));
	std::map<uint32_t, SBridgeSession>::iterator iter = g_Sessions.find(pair_hash);

	// don't need to open already opened session
	if (iter != end(g_Sessions))
	{
		if (iter->second.m_FileOpen)
		{
			return 0;
		}
	}

	if (iter == end(g_Sessions))
	{
		SBridgeSession session;

		memset(&session, 0, sizeof(SBridgeSession));
		strcpy_s(session.m_PairName, NAME_SIZE * sizeof(char), pair_name);

		g_Sessions.insert(std::pair<uint32_t, SBridgeSession>(pair_hash, session));
		// this is back compatibility with VS 2012
		iter = g_Sessions.find(pair_hash);
	}

	if (iter == end(g_Sessions))
	{
		return 1;
	}

	SBridgeSession &session = iter->second;

	if (session.m_FileOpen == false)
	{
		std::string pair_name = SHARED_MAPPING_PREFIX;
		pair_name += session.m_PairName;

		session.m_MapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			BUF_SIZE,                // maximum object size (low-order DWORD)
			pair_name.c_str());                 // name of mapping object

		std::string event_toclient_name = session.m_PairName;
		event_toclient_name += EVENT_TOCLIENT;

		std::string event_fromclient_name = session.m_PairName;
		event_fromclient_name += EVENT_FROMCLIENT;

		session.m_EventToClient = CreateEvent(nullptr, false, false, event_toclient_name.c_str());
		session.m_EventFromClient = CreateEvent(nullptr, false, true, event_fromclient_name.c_str());

		session.m_FileOpen = true;

		if (session.m_MapFile == nullptr)
		{
			const int err = static_cast<int>(GetLastError());

			session.m_FileOpen = false;
			return err;
		}

		if (session.m_FileOpen)
		{
			char *buffer = static_cast<char*>(MapViewOfFile(session.m_MapFile,
				FILE_MAP_WRITE, // write permission
				0,
				0,
				BUF_SIZE));

			if (buffer == nullptr)
			{
				CloseHandle(session.m_MapFile);

				const int err = static_cast<int>(GetLastError());

				session.m_FileOpen = false;
				return err;
			}

			memset(buffer, 0, BUF_SIZE);

			UnmapViewOfFile(buffer);
		}

		if (!SetEvent(session.m_EventToClient))
		{
			const int err = static_cast<int>(GetLastError());

			session.m_FileOpen = false;
			return err;
		}
	}

	return 0;
}

int32_t __stdcall HardwareClose(const int handle_id)
{
#pragma EXPORT_FUNCTION

	std::map<unsigned, SBridgeSession>::iterator iter = g_Sessions.find(handle_id);

	if (iter != end(g_Sessions))
	{
		SBridgeSession &session = iter->second;

		if (session.m_FileOpen)
		{
			CloseHandle(session.m_MapFile);
		}

		g_Sessions.erase(handle_id);
	}

	return 0;
}

SSharedModelData* __stdcall MapModelData(uint32_t pair_hash)
{
#pragma EXPORT_FUNCTION

	auto iter = g_Sessions.find(pair_hash);
	if (iter == end(g_Sessions))
	{
		return nullptr;
	}

	return &iter->second.m_Data;
}

int32_t __stdcall ReadData(uint32_t pair_hash)
{
#pragma EXPORT_FUNCTION

	return 0;
}

int32_t __stdcall WriteData(uint32_t pair_hash)
{
#pragma EXPORT_FUNCTION

	auto iter = g_Sessions.find(pair_hash);
	if (iter == end(g_Sessions))
	{
		return 1;
	}

	SBridgeSession &session = iter->second;

	if (session.m_FileOpen == false)
	{
		return 1;
	}

	bool signalled = WaitForSingleObjectEx(session.m_EventFromClient, 0, false) != WAIT_TIMEOUT;

	if (signalled == true)
	{
		char *buffer = static_cast<char*>(MapViewOfFile(session.m_MapFile,
			FILE_MAP_ALL_ACCESS, // write permission
			0,
			0,
			BUF_SIZE));

		if (buffer == nullptr)
		{
			return 1;
		}

		// write server data

		CopyMemory(static_cast<PVOID>(buffer), &session.m_Data, sizeof(char)*BUF_SIZE);

		UnmapViewOfFile(buffer);

		//
		SetEvent(session.m_EventToClient);
	}

	return 0;
}