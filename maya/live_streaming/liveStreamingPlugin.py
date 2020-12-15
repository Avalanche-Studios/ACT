#
# Copyright (c) 2020 Avalanche Studios. All rights reserved.
# Licensed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE
#
#--------------------------------------------------------------------------------------
# File: liveStreamingPlugin.py
#
# Live Bridge Server plugin.
#  Streaming for selected joints and it's "float_" attributes
# Use StartLive(), StopLive() commands
#
#-------------------------------------------------------------------------------------

import os
import mmap
import struct

import AnimLiveBridge

import maya.OpenMaya as OpenMaya
import maya.OpenMayaAnim as anim
import maya.OpenMayaMPx as OpenMayaMPx
import maya.cmds as cmds

# global session id and connection pair name

global g_SessionId
g_SessionId = 0

global g_PairName
g_PairName = 'AnimationBridgePair'

# maya objects and properties

global g_Joints
g_Joints = []

global g_JointProps
g_JointProps = [] # model float properties

# storage for updated values

global g_SharedData
g_SharedData = AnimLiveBridge.SSharedModelData()

global g_JointData
g_JointData = AnimLiveBridge.SJointDataVector()

global g_JointPropData
g_JointPropData = AnimLiveBridge.SPropertyDataVector()

# misc maya global data

global g_Verbose
g_Verbose = True

global g_HUDInfo
g_HUDInfo = ""

global g_Callback
g_Callback = None

   
def track_models():
    
    global g_JointData
    
    q = OpenMaya.MQuaternion()
    q2 = OpenMaya.MQuaternion()
    scale_factor = 1.0
    
    otr = AnimLiveBridge.SVector3()
    oq = AnimLiveBridge.SVector4()
    
    # update joint transform

    if len(g_Joints) != g_JointData.size():
        g_JointData.resize(len(g_Joints))
    
    for index, obj in enumerate(g_Joints):

        xformFn = OpenMaya.MFnTransform(obj)
        
        v = xformFn.translation(OpenMaya.MSpace.kTransform)
        xformFn.getRotation(q, OpenMaya.MSpace.kTransform)
        
        if xformFn.object().hasFn(OpenMaya.MFn.kJoint):
            
            joint = anim.MFnIkJoint(obj)
            joint.getOrientation(q2)
            
            q = q * q2
        
        (otr.m_X, otr.m_Y, otr.m_Z) = (scale_factor * v[0], scale_factor * v[1], scale_factor * v[2])
        (oq.m_X, oq.m_Y, oq.m_Z, oq.m_W) = (q[0], q[1], q[2], q[3])
        
        g_JointData[index].m_Transform.m_Translation = otr
        g_JointData[index].m_Transform.m_Rotation = oq

    AnimLiveBridge.SetModelDataJoints(g_SessionId, g_JointData)    
    shared_data = AnimLiveBridge.MapModelData(g_SessionId)    
    
    # update joint properties

    if len(g_JointProps) != g_JointPropData.size():
        g_JointPropData.resize(len(g_JointProps))

    for i, plug in enumerate(g_JointProps):
        
        value = plug.asFloat()
        g_JointPropData[i].m_Value = 0.01 * value
        
    conversion = 1.0 / 30.0
    shared_data.m_ServerPlayer.m_LocalTime = conversion * cmds.currentTime(q = True)
    shared_data.m_ServerPlayer.m_StartTime = conversion * cmds.playbackOptions(query=True, min=True)
    shared_data.m_ServerPlayer.m_StopTime = conversion * cmds.playbackOptions(query=True, max=True)
    # TODO: send playback update trigger and is playing state
    
    AnimLiveBridge.SetModelDataProperties(g_SessionId, g_JointPropData)

    result = AnimLiveBridge.FlushData(g_SessionId, True)


def getShortName(dagPath):
    return dagPath.partialPathName().split('|')[-1]

        
def StartWithSelected(models):
    
    global g_MapFunc
    global g_Joints
    global g_JointData
    global g_JointProps
    
    g_Joints = []
    g_JointProps = []

    error_code = AnimLiveBridge.HardwareOpen(g_SessionId, g_PairName, True)
    
    if error_code != 0:
        OpenMaya.MGlobal.displayError("Please restart maya with administrative rights!, error " + str(error_code))
        return False
    
    connector1 = AnimLiveBridge.MapModelData(g_SessionId)
    
    if not connector1:
        return False

    connector1.m_Header.m_ModelsCount = 0
    connector1.m_Header.m_PropsCount = 0    
    
    conversion = 1.0 / 30.0
    connector1.m_ServerPlayer.m_LocalTime = conversion * cmds.currentTime(q = True)
    connector1.m_ServerPlayer.m_StartTime = conversion * cmds.playbackOptions(query=True, min=True)
    connector1.m_ServerPlayer.m_StopTime = conversion * cmds.playbackOptions(query=True, max=True)
    
    count = models.length() # connector1.m_Header.m_ModelsCount
    if count >= AnimLiveBridge.NUMBER_OF_JOINTS:
        count = AnimLiveBridge.NUMBER_OF_JOINTS-1
    
    for i in xrange(count):
        obj = OpenMaya.MObject()
        models.getDependNode(i, obj)
        
        if obj.hasFn( OpenMaya.MFn.kDagNode ):

            xformFn = OpenMaya.MFnTransform(obj)
            if xformFn.object().hasFn(OpenMaya.MFn.kJoint):
                g_Joints.append(obj)
                
            if len(g_JointProps) < AnimLiveBridge.NUMBER_OF_PROPERTIES:
                
                m_objFn = OpenMaya.MFnDependencyNode(obj)
                for j in xrange( m_objFn.attributeCount() ):
                    m_attr = m_objFn.attribute(j)
                    m_fnAttr = OpenMaya.MFnAttribute(m_attr)
                    attrib_name = m_fnAttr.name()
                    
                    if attrib_name.startswith('float_'):
                        if g_Verbose:
                            print ("found an attribute - " + attrib_name)
                        
                        plug = m_objFn.findPlug(m_attr, False)
                        g_JointProps.append((attrib_name, plug))

    if len(g_Joints) == 0:
        OpenMaya.MGlobal.displayError('No joints were selected for live streaming')
        return False
                        
    connector1.m_Header.m_ModelsCount = len(g_Joints)
    connector1.m_Header.m_PropsCount = len(g_JointProps)
    
    # fill name hashes
    g_JointData.resize(len(g_Joints))
    for i in xrange(len(g_Joints)):
        dagPath = OpenMaya.MDagPath()
        
        OpenMaya.MDagPath.getAPathTo(g_Joints[i], dagPath)
        
        model_name = getShortName(dagPath)
        objectSplit = model_name.split(':')
        
        g_JointData[i].m_NameHash = AnimLiveBridge.HashPairName(str(objectSplit[-1]))
    
    g_JointPropData.resize(len(g_JointProps))
    for i in xrange(len(g_JointProps)):
        g_JointPropData[i].m_NameHash = AnimLiveBridge.HashPairName(str(g_JointData[i][0]))
        g_JointPropData[i].m_Value = g_JointData[i][1].asFloat()

    #
    track_models()
    return True

 
def timer_callback(self, *args, **kw):
    track_models()
 

def hudTopLeft(*args):
    return "Live Streaming " + g_HUDInfo
 

class StartLive(OpenMayaMPx.MPxCommand):
    def __init__(self):
        OpenMayaMPx.MPxCommand.__init__(self)

    def doIt(self, argList):
        
        models = OpenMaya.MSelectionList()
        OpenMaya.MGlobal.getActiveSelectionList(models)
        
        if models.length() == 0:
            OpenMaya.MGlobal.displayError('Please select objects to stream')
            return
        
        global g_SessionId
        g_SessionId = AnimLiveBridge.NewLiveSession()
        
        if not StartWithSelected(models):
            AnimLiveBridge.EraseLiveSession(g_SessionId)
            g_SessionId = 0
            return
        
        global g_Callback
        
        if g_Callback is not None:
            OpenMaya.MMessage.removeCallback(g_Callback)
            g_Callback = None

        g_Callback = OpenMaya.MTimerMessage.addTimerCallback(0.03, timer_callback)
        OpenMaya.MGlobal.displayInfo('Live streaming is started!')
        
        global g_HUDInfo
        
        numberOfModels = len(g_Joints)
        numberOfProperties = len(g_JointProps)
        
        g_HUDInfo = str(numberOfModels) + " models and " + str(numberOfProperties) + " properties"
        cmds.headsUpDisplay('HUDLIVESTREAM',s=1,b=1,dataFontSize='large', command=hudTopLeft)


class StopLive(OpenMayaMPx.MPxCommand):
    def __init__(self):
        OpenMayaMPx.MPxCommand.__init__(self)

    def doIt(self, argList):

        global g_Callback

        connector1 = AnimLiveBridge.MapModelData(g_SessionId)
        if connector1:    
            connector1.m_Header.m_ModelsCount = 0
            connector1.m_Header.m_PropsCount = 0

        AnimLiveBridge.EraseLiveSession(g_SessionId)

        if g_Callback is not None:
            OpenMaya.MMessage.removeCallback(g_Callback)
            g_Callback = None

        OpenMaya.MGlobal.displayInfo('Live streaming is stoped!')
        cmds.headsUpDisplay( 'HUDLIVESTREAM', rem=True )
 

def creatorStartLive():
    return OpenMayaMPx.asMPxPtr( StartLive() )


def creatorStopLive():
    return OpenMayaMPx.asMPxPtr( StopLive() )
 

def initializePlugin(obj):
    
    plugin = OpenMayaMPx.MFnPlugin(obj, 'Avalanche Studios', '1.0', 'Any')
    try:
        plugin.registerCommand('StartLive', creatorStartLive)
        plugin.registerCommand('StopLive', creatorStopLive)
        
    except:
        raise RuntimeError, 'Failed to register commands'
 

def uninitializePlugin(obj):
    plugin = OpenMayaMPx.MFnPlugin(obj)
    try:
        plugin.deregisterCommand('StartLive')
        plugin.deregisterCommand('StopLive')
    except:
        raise RuntimeError, 'Failed to unregister commands'


#cmds.loadPlugin('D:\\Work\\Avalanche\\live_streaming\\liveStreamingPlugin.py', qt=False)
#cmds.unloadPlugin('liveStreamingPlugin.py')