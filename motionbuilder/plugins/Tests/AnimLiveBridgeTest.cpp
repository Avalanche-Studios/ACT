
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

// Test communication between live bridge server and client
// Server is updating it's header tag and playback local time
// Client is getting the tag and local time and print it

#include "AnimLiveBridge.h"
#include <thread>

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(double percentage, int tag) {
	int val = (int)(percentage * 100);
	int lpad = (int)(percentage * PBWIDTH);
	int rpad = PBWIDTH - lpad;
	printf("\r%3d%% [%.*s%*s] - tag %d", val, lpad, PBSTR, rpad, "", tag);
	fflush(stdout);
}

class CLogger : public CLiveBridgeLogger
{
public:
	void LogInfo(const char* info) override
	{
		printf("[INFO] %s\n", info);
	}
	void LogWarning(const char* info) override
	{
		printf("[WARN] %s\n", info);
	}
	void LogError(const char* info) override
	{
		printf("[ERR] %s\n", info);
	}
};

///////////////////////////////////////////////////////////////////////////////
// simple communication test

void server_simple()
{
	const unsigned session_id = NewLiveSession();

	int result = HardwareOpen(session_id, "test_pair", true);
	_ASSERT(result == 0);

	for (int i = 0; i < 1000; ++i)
	{
		SSharedModelData* data = MapModelData(session_id);
		_ASSERT(data != nullptr);

		data->m_Header.m_ModelsCount = 1;
		data->m_Header.m_ServerTag = i;

		data->m_ServerPlayer.m_IsPlaying = true;
		data->m_ServerPlayer.m_LocalTime = 0.001 * static_cast<double>(i);

		printf("server access - client tag %d\n", data->m_Header.m_ClientTag);

		HardwareCommit(session_id, true);
	}

	// close server - define a specified tag

	while (true)
	{
		SSharedModelData* data = MapModelData(session_id);
		_ASSERT(data != nullptr);
		data->m_Header.m_ServerTag = UINT32_MAX;

		result = HardwareCommit(session_id, true);
		if (result == 0)
			break;
	}

	HardwareClose(session_id);
	FreeLiveSession(session_id);
}

void client_simple()
{
	const unsigned session_id = NewLiveSession();

	int result = -1;

	for (int attemps = 0; attemps < 10; ++attemps)
	{
		result = HardwareOpen(session_id, "test_pair", false);
		if (result == 0)
			break;

		constexpr std::chrono::milliseconds timespan(100);
		std::this_thread::sleep_for(timespan);
	}

	if (result != 0)
		return;

	int i = 0;
	while (true)
	{
		SSharedModelData* data = MapModelData(session_id);
		_ASSERT(data != nullptr);

		data->m_Header.m_ClientTag = i;
		++i;

		data->m_ClientPlayer.m_IsPlaying = true;
		data->m_ClientPlayer.m_LocalTime = static_cast<double>(i);

		printf("client access - server tag %d\n", data->m_Header.m_ServerTag);

		HardwareCommit(session_id, true);

		if (data->m_Header.m_ServerTag == UINT32_MAX)
			break;
	}

	HardwareClose(session_id);
	FreeLiveSession(session_id);
}

///////////////////////////////////////////////////////////////////////////////////
// timeline test

void server_timeline()
{
	const unsigned session_id = NewLiveSession();

	int result = HardwareOpen(session_id, "test_pair", true);
	_ASSERT(result == 0);

	double local_time{ 0.0 };
	double remote_time{ 0.0 };
	bool is_playing{ false };

	for (int i = 0; i < 1000; ++i)
	{
		SSharedModelData* data = MapModelData(session_id);
		_ASSERT(data != nullptr);

		STimelineSyncManager* timeline = MapTimelineSync(session_id);
		_ASSERT(timeline != nullptr);

		if (timeline->CheckForARemoteTimeControl(remote_time, local_time, is_playing))
		{
			// client take control over the server time
			local_time = remote_time;

			printProgress(local_time, i);
		}

		data->m_Header.m_ModelsCount = 1;
		data->m_Header.m_ServerTag = i;

		HardwareCommit(session_id, true);

		constexpr std::chrono::milliseconds timespan(5);
		std::this_thread::sleep_for(timespan);
	}
	
	// close server - define a specified tag

	while (true)
	{
		SSharedModelData* data = MapModelData(session_id);
		_ASSERT(data != nullptr);
		data->m_Header.m_ServerTag = UINT32_MAX;

		result = HardwareCommit(session_id, true);
		if (result == 0)
			break;
	}
	
	HardwareClose(session_id);
	FreeLiveSession(session_id);
}

void client_timeline()
{
	const unsigned session_id = NewLiveSession();

	int result = -1;

	for (int attemps = 0; attemps<10; ++attemps)
	{
		result = HardwareOpen(session_id, "test_pair", false);
		if (result == 0)
			break;
		
		constexpr std::chrono::milliseconds timespan(100);
		std::this_thread::sleep_for(timespan);
	}

	if (result != 0)
		return;

	double remote_time{ 0.0 };
	double local_time{ 0.0 };
	bool is_playing{ true };

	int i = 0;
	while(true)
	{
		SSharedModelData* data = MapModelData(session_id);
		_ASSERT(data != nullptr);

		STimelineSyncManager* timeline = MapTimelineSync(session_id);
		_ASSERT(timeline != nullptr);

		if (timeline->CheckForARemoteTimeControl(remote_time, local_time, is_playing))
		{
			local_time = remote_time;
		}
		else
		{
			local_time = 0.001 * static_cast<double>(data->m_Header.m_ServerTag);
		}

		timeline->SetLocalTime(local_time);
		timeline->SetIsPlaying(is_playing);

		timeline->SetLocalTimeline(local_time, is_playing);

		data->m_Header.m_ClientTag = i;
		++i;

		data->m_ClientPlayer.m_IsPlaying = true;
		data->m_ClientPlayer.m_LocalTime = static_cast<double>(i);

		HardwareCommit(session_id, true);

		if (data->m_Header.m_ServerTag == UINT32_MAX)
			break;
	}

	HardwareClose(session_id);
	FreeLiveSession(session_id);
}


int main()
{
	// Test 1 - communication and logger
	printf("=== Test 1 ===\n");
	{
		CLogger logger;

		SetVerboseLevel(1);
		SetLiveBridgeLogger(&logger);

		std::thread server_thread(server_simple);
		std::thread client_thread(client_simple);

		client_thread.join();
		server_thread.join();

		SetVerboseLevel(0);
		SetLiveBridgeLogger(nullptr);
	}
	
	// Test 2 - timeline
	printf("\n=== Test 2 ===\n");
	{
		std::thread server_thread(server_timeline);
		std::thread client_thread(client_timeline);

		client_thread.join();
		server_thread.join();
	}

	getchar();
	return 0;
}