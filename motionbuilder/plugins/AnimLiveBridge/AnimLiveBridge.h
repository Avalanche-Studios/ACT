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

#include <cstdint>

#ifdef LIBRARY_EXPORTS
#    define LIBRARY_API __declspec(dllexport)
#else
#    define LIBRARY_API __declspec(dllimport)
#endif

namespace NAnimationLiveBridge
{

	/////////////////////////////////////////////////////////////////////////////
	//
	// maximum number of supported exchange models

	enum ELimits
	{
		NUMBER_OF_JOINTS			= 128,
		NUMBER_OF_PROPERTIES		= 32,
	};

	enum EFlags
	{
		HINT_ROTATION_QUATERNION	= 1 << 0,
		HINT_ROTATION_EULERANGLES	= 1 << 1,
		HINT_TRANSFORM_LOCAL		= 1 << 2,
		HINT_TRANSFORM_MODEL		= 1 << 3
	};

	enum class ECommand : unsigned int
	{
		None,
		Server_Request,
		Client_Request,
		Count
	};

	////////////////////////////////////////////////////////////////////////////
	// SSharedData

	struct SSharedModelData
	{
		struct SHeader
		{
			uint32_t	m_ServerTag;
			uint32_t	m_ClientTag;
			uint32_t	m_ModelsCount;
			uint32_t	m_PropsCount;
			uint32_t	m_ModelsOffset;
			uint32_t	m_PropsOffset;
		};

		struct STransform
		{
			float			m_Translation[3];
			float			m_Rotation[4];			// quaternion or euler depends on a target
			float			m_Scale[3];
		}; // 40

		struct SJointData
		{
			uint32_t		m_NameHash;				// model name hash
			uint32_t		m_ParentHash;			// parent name hash, 0 if no parent
			// flags can be used for different purposes (specify Quaternion/Euler rotation, Local/Model Space Transformation)
			uint32_t		m_Flags;

			STransform		m_Transform;
		}; // 52 bytes

		struct SPropertyData
		{
			uint32_t		m_NameHash;
			float			m_Value;
		};

		struct SPlayerInfo
		{
			double		m_SystemTime;
			double		m_LocalTime;
			int32_t		m_IsPlaying;
			float		m_StartTime;
			float		m_StopTime;
			float		m_TimeChangedEvent;
		};

		SHeader			m_Header;

		uint32_t		m_ModelNameHash;		// we bind to a specified model
		uint32_t		m_ModelResourceHash;	// could be skeleton, geometry export resource

		ECommand		m_Command;

		SPlayerInfo		m_ServerPlayer;
		SPlayerInfo     m_ClientPlayer;

		// we are using 3 animated models to control eyes direction 

		float			m_LookAtRoot[4];
		float			m_LookAtLeft[4];
		float			m_LookAtRight[4];

		SJointData		m_Joints[NUMBER_OF_JOINTS];
		// could be wrinkle map or anything else
		SPropertyData	m_Properties[NUMBER_OF_PROPERTIES];

	};
};

extern "C"
{
	uint32_t __stdcall HashPairName(const char* pair_name);

	// open server or client
	// NOTE! you should have administrative rights!
	int32_t __stdcall HardwareOpen(uint32_t pair_hash, const bool server);
	int32_t __stdcall HardwareClose(uint32_t pair_hash);

	NAnimationLiveBridge::SSharedModelData* __stdcall MapModelData(uint32_t pair_hash);
	
	int32_t __stdcall ReadData(uint32_t pair_hash);
	int32_t __stdcall WriteData(uint32_t pair_hash);
}
