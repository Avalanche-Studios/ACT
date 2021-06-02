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

#include <vector>

#ifdef SWIG
	#define LIBRARY_API
#else
	#ifdef AnimLiveBridgeAPI_EXPORTS
		#define LIBRARY_API __declspec(dllexport)
	#else
		#define LIBRARY_API __declspec(dllimport)
	#endif
#endif

// TODO: start using namespace and make it compatible with a swig setup
//namespace NAnimationLiveBridge
//{

	/////////////////////////////////////////////////////////////////////////////
	//
	// maxmimum number of supported exchange models

	enum class ECommunicationType
	{
		SharedMemory,
		NetworkTCP
	};

#ifdef SWIG
	constexpr ECommunicationType SharedMemory = ECommunicationType::SharedMemory;
	constexpr ECommunicationType NetworkTCP = ECommunicationType::NetworkTCP;
#endif

	enum ELiveSessionProperties
	{
		ELiveSessionProperty_CommunicationType,
		ELiveSessionProperty_SharedPairName,
		ELiveSessionProperty_IsServer,
		ELiveSessionProperty_NetworkAddress,
		ELiveSessionProperty_NetworkPort,
		ELiveSessionProperty_Count
	};

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
		int			m_IsPlaying;
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
	struct LIBRARY_API STimelineSyncManager
	{
	public:
		//! a constructor
		STimelineSyncManager(bool is_server=true);

		void Initialize(const bool is_server, SSharedModelData* data);

		// use method to set info from a local timeline
		void SetLocalTimeline(const double local_time, const bool is_playing);

		// we could read from server or client data values
		void ReadFromData(const bool is_server_data, SSharedModelData& data);

		// NOTE: call check local timeline before writeToData
		// write to server or client data values
		void WriteToData(const bool is_server_data, SSharedModelData& data);

		bool CheckForARemoteTimeControl(double& remote_time, const double local_time, const bool is_playing);

		void SetLocalTime(const double time);
		void SetOffsetTime(const double time);
		void SetIsPlaying(const bool value);

		const bool IsRemoteTimeChanged() const;
		const double GetRemoteTime() const;

	protected:
		SSharedModelData*	m_Data{ nullptr };

		bool		m_IsServer{ true };
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

//};

extern "C"
{
	void SetModelJointData(SSharedModelData* model_data, const std::vector<SJointData>& data);
	void SetModelPropertyData(SSharedModelData* model_data, const std::vector<SPropertyData>& data);

	//! utility function to generate a hash value for a string
	/*!
		Use this function to generate objects keys
		\param pair_name provide a string to generate a key out of it
		\return a generated 32 bit unique hash value
	*/
	unsigned int HashPairName(const char* pair_name);

	//! open server or client (session is storing local properties, states and data that you can map and read/modify)
	/*!
		Everything should start with starting a new session, assigning properties and opening hardware
		\return unique session id that you can use to get/set properties/data for the session 
	*/
	unsigned int NewLiveSession();

	//! close hardware and erase data associated with a session id
	/*!
		\param session_id specify a session id which you want to free
		\return true if operation is successful
	*/
	bool FreeLiveSession(unsigned int session_id);

	//! setup live session properties
	/*!
		Set a specified property value for a session
		\param session_id specify on which session you want to set a property
		\param property_id this is from ELiveSessionProperties enum values
		\param value this is an int value to set to a property
		\sa ELiveSessionProperties, SetLiveSessionPropertyString
	*/
	void SetLiveSessionPropertyInt(unsigned int session_id, unsigned int property_id, int value);
	void SetLiveSessionPropertyString(unsigned int session_id, unsigned int property_id, const char* value);

	int GetLiveSessionPropertyInt(unsigned int session_id, unsigned int property_id);
	const char* GetLiveSessionPropertyString(unsigned int session_id, unsigned int property_id);

	
	//! get access to a local session buffer
	/*!
		with the pointer you can modify buffer data and use FlushData to submit it to a global memory or send packets
		\param session_id specify on which session you want to set a property
		\return pointer to a local buffer or nullptr if session is not found
		\sa NewLiveSession, HardwareOpen, UnMapModelData
	*/
	SSharedModelData*		MapModelData(unsigned int session_id);

	//! finish access request to a local buffer
	/*!
		The function should be paired with MapModelData
		\param session_id specify on which session you want to set a property
		\sa NewLiveSession, HardwareOpen, MapModelData
	*/
	void					UnMapModelData(unsigned int session_id);

	//! get access to a local timerliner sync manager
	/*!
		use a manager to sync timeline playback and properties between communicated sessions
		\param session_id specify on which session you want to set a property
		\return pointer to a local timeline sync manager or nullptr if session is not found
		\sa NewLiveSession, HardwareOpen, UnMapTimelineSync
	*/
	STimelineSyncManager*	MapTimelineSync(unsigned int session_id);

	//! finish access request to a local timeline sync manager
	/*!
		should be paired with MapTimelineSync
		\param session_id specify on which session you want to set a property
		\sa NewLiveSession, HardwareOpen, MapTimelineSync
	*/
	void					UnMapTimelineSync(unsigned int session_id);


	//! assign local buffer joints data from a specified data array
	/*!
		alternative way to MapModelData for a data assignment
		\param session_id specify on which session you want to set a property
		\param data - vector of joint data
		\sa MapModelData
	*/
	bool SetModelDataJoints(unsigned int session_id, const std::vector<SJointData>& data);

	//! assign local buffer properties data from a specified data array
	/*!
		alternative way to MapModelData for a data assignment
		\param session_id specify on which session you want to set a property
		\param data - vector of properties data
		\sa MapModelData
	*/
	bool SetModelDataProperties(unsigned int session_id, const std::vector<SPropertyData>& data);

	//! starts a communication
	/*!
		NOTE! you should have administrative rights for a shared memory communication!
		Setup session properties before openning a hardware
		\param session_id specify on which session you want to set a property
		\param pair_name name of a global shared memory buffer or can be defined as network address
		\param is_server to define if we start a server or a client
		\return 0 if succeed, otherwiser returns a error code
		\sa NewLiveSession, HardwareClose
	*/
	int HardwareOpen(unsigned int session_id, const char* pair_name, const bool is_server);

	//! stops a communication
	/*!
		\param session_id specify on which session you want to set a property
		\return 0 if succeed, otherwiser returns a error code
		\sa NewLiveSession, HardwareOpen
	*/
	int HardwareClose(unsigned int session_id);

	//! submit a local buffer changes
	/*!
		updated shared memory with a local buffer data or send/receive packets via a network
		\param session_id specify on which session you want to set a property
		\param auto_finish_event do we want to automatically trigger that client is finished the update process and transfer a control
		\return 0 if succeed, otherwise returns a error code
		\sa SetFinishEvent
	*/
	int HardwareCommit(unsigned int session_id, const bool auto_finish_event=true);

	//! set an event to transfer a control
	/*!
		NOTE: use only if flush data is not doing auto finish event!
		\param session_id specify on which session you want to set a property
		\return true if succeed, otherwise returns false
		\sa FlushData
	*/
	bool SetFinishEvent(unsigned int session_id);

	// look at sync properties
	bool SetLookAtVectors(unsigned int session_id, const SVector4& lookat_root, const SVector4& lookat_left, const SVector4& lookat_right);
	bool GetLookAtVectors(unsigned int session_id, SVector4& lookat_root, SVector4& lookat_left, SVector4& lookat_right);

	void SetHasNewSync(unsigned int session_id, const bool value);
	bool GetHasNewSync(unsigned int session_id);
	bool GetAndResetHasNewSync(unsigned int session_id);
	void SetSyncSaved(unsigned int session_id, const bool value);
	bool GetSyncSaved(unsigned int session_id);

	//! set a verbose level, helps to control amount of log information you received
	/*!
		verbose level 0 is disabled, 1 is normal level, more then 1 is high level with a lot of additional log information included
		\param verbose_level specify a verbose level
	*/
	void SetVerboseLevel(const int verbose_level);

	//! set up a pointer to a callback
	/*!
		this callback will be used to call info, warning, error methods
		amount of calls are depend on verbose level
		\param logger pointer to a callback
	*/
	void SetLiveBridgeLogger(CLiveBridgeLogger* logger);
};
