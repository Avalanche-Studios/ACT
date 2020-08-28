
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

# AnimLiveBridge.py
# this is a represent of AnimLiveBridge.h SSharedData


import ctypes

NUMBER_OF_JOINTS = 128
NUMBER_OF_PROPERTIES = 32

HINT_ROTATION_QUATERNION = 1
HINT_ROTATION_EULERANGLES = 2
HINT_TRANSFORM_LOCAL = 4
HINT_TRANSFORM_MODEL = 8

class SHeader(ctypes.Structure):
	_fields_ = [("m_ServerTag", ctypes.c_uint),
				("m_ClientTag", ctypes.c_uint),
				("m_ModelsCount", ctypes.c_uint),
				("m_PropsCount", ctypes.c_uint),
				("m_ModelsOffset", ctypes.c_uint),
				("m_PropsOffset", ctypes.c_uint)]

class SPlayerInfo(ctypes.Structure):
    _fields_ = [("m_SystemTime", ctypes.c_double),
                ("m_LocalTime", ctypes.c_double),
                ("m_IsPlaying", ctypes.c_int),
                ("m_StartTime", ctypes.c_float),
                ("m_StopTime", ctypes.c_float),
                ("m_TimeChangedEvent", ctypes.c_float)]

class STransform(ctypes.Structure):
    _fields_ = [("m_Translation", (ctypes.c_float * 3)),
                ("m_Rotation", (ctypes.c_float * 4)),
                ("m_Scale", (ctypes.c_float * 3))]
        
class SJointData(ctypes.Structure):
    _fields_ = [("m_NameHash", ctypes.c_uint),
                ("m_ParentHash", ctypes.c_uint),
                ("m_Flags", ctypes.c_uint),
                ("m_Transform", STransform),
                ]
   
class SPropertyData(ctypes.Structure):
    _fields_ = [("m_NameHash", ctypes.c_uint),
                ("m_Value", ctypes.c_float),
                ]
   
class SSharedData(ctypes.Structure):
    _fields_ = [("m_Header", SHeader),
                ("m_ModelNameHash", ctypes.c_uint),
                ("m_ModelResourceHash", ctypes.c_uint),
                ("m_Command", ctypes.c_uint),
                ("m_ServerPlayer", SPlayerInfo),
                ("m_ClientPlayer", SPlayerInfo),
                ("m_LookAtRoot", ctypes.c_float * 4),
                ("m_LookAtLeft", ctypes.c_float * 4),
                ("m_LookAtRight", ctypes.c_float * 4),
                ("m_Joints", SJointData * NUMBER_OF_JOINTS),
                ("m_Properties", SPropertyData * NUMBER_OF_PROPERTIES)
                ]