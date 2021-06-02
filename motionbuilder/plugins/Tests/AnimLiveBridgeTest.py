
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

# This is a AnimLiveBridge Test

import AnimLiveBridge

# global session id and connection pair name

global g_ServerId
g_ServerId = 0

global g_ClientId
g_ClientId = 0

global g_PairName
g_PairName = 'AnimationBridgePair'


def StartServer():

	global g_ServerId
	g_ServerId = AnimLiveBridge.NewLiveSession()
	error_code = AnimLiveBridge.HardwareOpen(g_ServerId, g_PairName, True)
	
	if error_code != 0:
		print("Please restart maya with administrative rights!, error " + str(error_code))
		return False
	
	connector1 = AnimLiveBridge.MapModelData(g_ServerId)
	
	if not connector1:
		return False

	connector1.m_Header.m_ModelsCount = 0
	connector1.m_Header.m_PropsCount = 0    
	result = AnimLiveBridge.FlushData(g_ServerId, True)
	print ('Server flush data - {}'.format(result))
	return True


def StopServer():

	connector1 = AnimLiveBridge.MapModelData(g_ServerId)
	if connector1:    
		connector1.m_Header.m_ModelsCount = 0
		connector1.m_Header.m_PropsCount = 0

		AnimLiveBridge.FreeLiveSession(g_ServerId)


global g_LocalFrame
g_LocalFrame = 0.0

def UpdateServer():

	global g_LocalFrame
	g_LocalFrame += 1.0

	connector1 = AnimLiveBridge.MapModelData(g_ServerId)
	
	if not connector1:
		return False

	connector1.m_Header.m_ModelsCount = 0
	connector1.m_Header.m_PropsCount = 0    
	
	conversion = 1.0 / 30.0
	connector1.m_ServerPlayer.m_LocalTime = conversion * g_LocalFrame
	connector1.m_ServerPlayer.m_StartTime = 1.0
	connector1.m_ServerPlayer.m_StopTime = 100.0

	result = AnimLiveBridge.FlushData(g_ServerId, True)
	#print result


def StartClient():
	global g_ClientId
	g_ClientId = AnimLiveBridge.NewLiveSession()
	error_code = AnimLiveBridge.HardwareOpen(g_ClientId, g_PairName, False)
	
	if error_code != 0:
		print("Please restart maya with administrative rights!, error " + str(error_code))
		return False
	
	connector1 = AnimLiveBridge.MapModelData(g_ClientId)
	
	if not connector1:
		return False

	print connector1.m_Header.m_ModelsCount
	print connector1.m_Header.m_PropsCount
	result = AnimLiveBridge.FlushData(g_ClientId, True)
	print ('Client flush data - {}'.format(result))
	return True


def StopClient():
	connector1 = AnimLiveBridge.MapModelData(g_ClientId)
	if connector1:    
		connector1.m_Header.m_ModelsCount = 0
		connector1.m_Header.m_PropsCount = 0

		AnimLiveBridge.FreeLiveSession(g_ClientId)


def UpdateClient():
	connector1 = AnimLiveBridge.MapModelData(g_ClientId)
	
	if not connector1:
		return False
	
	print ('Server player time - {}'.format(connector1.m_ServerPlayer.m_LocalTime))
	result = AnimLiveBridge.FlushData(g_ClientId, True)
	

StartServer()
StartClient()

for i in xrange(100):
	UpdateServer()
	UpdateClient()

StopServer()
StopClient()