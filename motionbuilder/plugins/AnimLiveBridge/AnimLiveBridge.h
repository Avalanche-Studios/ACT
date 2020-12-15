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

// AnimLiveBridge.h

#include <vector>

#ifdef AnimLiveBridgeAPI_EXPORTS
#    define LIBRARY_API __declspec(dllexport)
#else
#    define LIBRARY_API __declspec(dllimport)
#endif



	/////////////////////////////////////////////////////////////////////////////
	//
	// maxmimum number of supported exchange models

	enum ELimits
	{
		NUMBER_OF_JOINTS			= 128,
		NUMBER_OF_PROPERTIES		= 32
	};

	enum EFlags
	{
		HINT_ROTATION_QUATERNION	= 1 << 0,
		HINT_ROTATION_EULERANGLES	= 1 << 1,
		HINT_TRANSFORM_LOCAL		= 1 << 2,
		HINT_TRANSFORM_MODEL		= 1 << 3
	};

	enum ECommand
	{
		ECommand_None,
		ECommand_Server_Request,
		ECommand_Client_Request,
		ECommand_Count
	};

	////////////////////////////////////////////////////////////////////////////
	// SSharedData
	struct SHeader
	{
		unsigned int	m_ServerTag;
		unsigned int	m_ClientTag;
		unsigned int	m_ModelsCount;
		unsigned int	m_PropsCount;
		unsigned int	m_ModelsOffset;
		unsigned int	m_PropsOffset;
	};

	struct SVector3
	{
		float m_X;
		float m_Y;
		float m_Z;
	};

	struct SVector4
	{
		float m_X;
		float m_Y;
		float m_Z;
		float m_W;
	};

	struct STransform
	{
		SVector3		m_Translation;
		SVector4		m_Rotation;		// quaternion or euler depends on a target
		SVector3		m_Scale;
	}; // 40

	struct SJointData
	{
		unsigned int		m_NameHash;				// model name hash
		unsigned int		m_ParentHash;			// parent name hash, 0 if no parent
		// flags can be used for different purposes (specify Quaternion/Euler rotation, Local/Model Space Transformation)
		unsigned int		m_Flags;

		STransform			m_Transform;
	}; // 52 bytes

	struct SJointDataArray
	{
		SJointData	m_Data[NUMBER_OF_JOINTS];
	};

	struct SPropertyData
	{
		unsigned int		m_NameHash;
		float				m_Value;
	};

	struct SPropertyDataArray
	{
		SPropertyData	m_Data[NUMBER_OF_PROPERTIES];
	};

	struct SPlayerInfo
	{
		double		m_SystemTime;
		double		m_LocalTime;
		int		m_IsPlaying;
		float		m_StartTime;
		float		m_StopTime;
		float		m_TimeChangedEvent;
	};

	

	struct SSharedModelData
	{
		SHeader			m_Header;

		unsigned int		m_ModelNameHash;		// we bind to a specified model
		unsigned int		m_ModelResourceHash;	// could be skeleton, geometry export resource

		ECommand		m_Command;

		SPlayerInfo		m_ServerPlayer;
		SPlayerInfo     m_ClientPlayer;

		// we are using 3 animated models to control eyes direction 
		float			m_LookAtRoot[4];
		float			m_LookAtLeft[4];
		float			m_LookAtRight[4];
		
		SJointDataArray		m_Joints;
		SPropertyDataArray	m_Properties;	//< could be wrinkle map or anything else
	};

	//////////////////////////////////////////////////////////////
	// STimelineSyncManager
	// LIBRARY_API
	struct STimelineSyncManager
	{
	public:
		//! a constructor
		STimelineSyncManager();

		// use method to set info from a local timeline
		void SetLocalTimeline(const double local_time, const bool is_playing);

		// we could read from server or client data values
		void ReadFromData(const bool is_server, SSharedModelData& data);

		// NOTE: call check local timeline before writeToData
		// write to server or client data values
		void WriteToData(const bool is_server, SSharedModelData& data);

		
		bool CheckForARemoteTimeControl(double& remote_time, const double local_time, const bool is_playing);

		void SetLocalTime(const double time);
		void SetOffsetTime(const double time);
		void SetIsPlaying(const bool value);

		const bool IsRemoteTimeChanged() const;
		const double GetRemoteTime() const;

	protected:

		bool		m_IsPlaying{ false };

		double		m_LocalTime{ 0.0 };
		double      m_OffsetTime{ 0.0 };				//< time in seconds

		bool		m_LocalTimeChanged{ false };
		double      m_LocalLastTime{ 0.0 };			//< time in seconds

		bool		m_RemoteTimeChanged{ false };
		double		m_RemoteLastTime{ 0.0 };			//< time in seconds
	};


	// log LIBRARY_API
	class  CLiveBridgeLogger
	{
	public:
		virtual void LogInfo(const char* info) { };
		virtual void LogWarning(const char* info) { };
		virtual void LogError(const char* info) { };
	};



extern "C"
{
	void SetModelJointData(SSharedModelData* model_data, const std::vector<SJointData>& data);
	void SetModelPropertyData(SSharedModelData* model_data, const std::vector<SPropertyData>& data);

	unsigned int HashPairName(const char* pair_name);

	// open server or client (session is storing local states and data that you can map and read/modify)
	unsigned int NewLiveSession();  //< return a live session unique index
	bool EraseLiveSession(unsigned int session_id);

	//NAnimationLiveBridge::
	// Get access to session data
	SSharedModelData*		MapModelData(unsigned int session_id);
	void					UnMapModelData(unsigned int session_id);
	STimelineSyncManager*	MapTimelineSync(unsigned int session_id);
	void					UnMapTimelineSync(unsigned int session_id);

	bool SetModelDataJoints(unsigned int session_id, const std::vector<SJointData>& data);
	bool SetModelDataProperties(unsigned int session_id, const std::vector<SPropertyData>& data);

	// NOTE! you should have administrative rights for a shared memory communication!
	int HardwareOpen(unsigned int session_id, const char* pair_name, const bool is_server);
	int HardwareClose(unsigned int session_id);

	// exchange local data with a shared buffer or send/receive packets
	int FlushData(unsigned int session_id, const bool auto_finish_event=true);

	// NOTE: use only if flush data is not doing auto finish event!
	bool SetFinishEvent(unsigned int session_id);

	// look at sync properties
	bool SetLookAtVectors(unsigned int session_id, const SVector4& lookat_root, const SVector4& lookat_left, const SVector4& lookat_right);
	bool GetLookAtVectors(unsigned int session_id, SVector4& lookat_root, SVector4& lookat_left, SVector4& lookat_right);

	void SetHasNewSync(unsigned int session_id, const bool value);
	bool GetHasNewSync(unsigned int session_id);
	bool GetAndResetHasNewSync(unsigned int session_id);
	void SetSyncSaved(unsigned int session_id, const bool value);
	bool GetSyncSaved(unsigned int session_id);

	// trace info
	void SetVerboseLevel(const int verbose_level);
	void SetLiveBridgeLogger(CLiveBridgeLogger* logger);
};
