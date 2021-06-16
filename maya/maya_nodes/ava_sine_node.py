
#
# Copyright (c) 2021 Avalanche Studios. All rights reserved.
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
# File: ava_sin_node.py
#
# The node is designed to be used for a procedural animal tail animation.
#
#-------------------------------------------------------------------------------------


import math
import sys

import maya.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx

kPluginNodeTypeName = "AvaSineNode"

AvaSineNodeId = OpenMaya.MTypeId(0x87000)

# Node definition


class AvaSineNode(OpenMayaMPx.MPxNode):
    # class variables
    inWeight = OpenMaya.MObject()

    inJointNo = OpenMaya.MObject()

    inGlobalTime = OpenMaya.MObject()
    inGlobalAmplitude = OpenMaya.MObject()
    inGlobalFrequency = OpenMaya.MObject()
    inGlobalDelay = OpenMaya.MObject()

    inAmplitude = OpenMaya.MObject()
    inFrequency = OpenMaya.MObject()
    inDelay = OpenMaya.MObject()

    output = OpenMaya.MObject()

    def __init__(self):
        OpenMayaMPx.MPxNode.__init__(self)

    def compute(self, plug, dataBlock):

        if (plug == AvaSineNode.output):

            # Overall weight/blend value of the calculation
            dataHandle_weight = dataBlock.inputValue(AvaSineNode.inWeight)
            weight_float = dataHandle_weight.asFloat()

            # Joint no. which is multiplied with delay down the chain
            # if there are three joints in the chain. for example. joint1, joint2, joint3 then joint1 should be 1, joint2 should be 2.. etc
            dataHandle_joint_no = dataBlock.inputValue(AvaSineNode.inJointNo)
            joint_no_float = dataHandle_joint_no.asFloat()

            # Connect the time1.outTime to this plug. Or else use cmds.currentTime.
            dataHandle_global_time = dataBlock.inputValue(AvaSineNode.inGlobalTime)
            global_time_float = dataHandle_global_time.asFloat()

            # the global amp for the node
            dataHandle_global_amplitude = dataBlock.inputValue(AvaSineNode.inGlobalAmplitude)
            global_amplitude_float = dataHandle_global_amplitude.asFloat()

            # the global frequency for the node
            dataHandle_global_frequency = dataBlock.inputValue(AvaSineNode.inGlobalFrequency)
            global_frequency_float = dataHandle_global_frequency.asFloat()

            # the global delay by frames for the node and this affects the child joint gradually
            dataHandle_global_delay = dataBlock.inputValue(AvaSineNode.inGlobalDelay)
            global_delay_float = dataHandle_global_delay.asFloat()

            # the individual amp which will be multiplied to the global amp
            dataHandle_amplitude = dataBlock.inputValue(AvaSineNode.inAmplitude)
            amplitude_float = dataHandle_amplitude.asFloat()

            # the individual frequency which will be multiplied to the global frequency
            dataHandle_frequency = dataBlock.inputValue(AvaSineNode.inFrequency)
            frequency_float = dataHandle_frequency.asFloat()

            # the individual delay which will be added to the global delay
            dataHandle_delay = dataBlock.inputValue(AvaSineNode.inDelay)
            delay_float = dataHandle_delay.asFloat()

            final_amp = amplitude_float * global_amplitude_float * 5.0
            final_fre = frequency_float * global_frequency_float * 0.0833 # / 12
            final_delay = (joint_no_float * global_delay_float) + delay_float

            result = weight_float * (math.sin((global_time_float * final_fre) + final_delay) * final_amp)

            outputHandle = dataBlock.outputValue(AvaSineNode.output)
            outputHandle.setFloat(result)
            dataBlock.setClean(plug)

        return OpenMaya.kUnknownParameter

# creator
def nodeCreator():
    return OpenMayaMPx.asMPxPtr(AvaSineNode())

# initializer
def nodeInitializer():
        # input

    nAttr = OpenMaya.MFnNumericAttribute()
    AvaSineNode.inWeight = nAttr.create("weight", "w", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    AvaSineNode.inJointNo = nAttr.create("jointno", "jo", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    AvaSineNode.inGlobalTime = nAttr.create("time", "time", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    AvaSineNode.inGlobalAmplitude = nAttr.create("global_amplitude", "gamp", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    AvaSineNode.inGlobalFrequency = nAttr.create("global_frequency", "gfre", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    AvaSineNode.inGlobalDelay = nAttr.create("global_delay", "gdelay", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    AvaSineNode.inAmplitude = nAttr.create("amplitude", "amp", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    AvaSineNode.inFrequency = nAttr.create("frequency", "fre", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    AvaSineNode.inDelay = nAttr.create("delay", "delay", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setReadable(1)
    nAttr.setWritable(1)
    nAttr.setStorable(1)
    nAttr.setKeyable(1)

    # output
    nAttr = OpenMaya.MFnNumericAttribute()
    AvaSineNode.output = nAttr.create("output", "out", OpenMaya.MFnNumericData.kFloat, 0.0)
    nAttr.setStorable(1)
    nAttr.setWritable(1)

    # add attributes
    AvaSineNode.addAttribute(AvaSineNode.inWeight)
    AvaSineNode.addAttribute(AvaSineNode.inJointNo)
    AvaSineNode.addAttribute(AvaSineNode.inGlobalTime)
    AvaSineNode.addAttribute(AvaSineNode.inGlobalAmplitude)
    AvaSineNode.addAttribute(AvaSineNode.inGlobalFrequency)
    AvaSineNode.addAttribute(AvaSineNode.inGlobalDelay)
    AvaSineNode.addAttribute(AvaSineNode.inAmplitude)
    AvaSineNode.addAttribute(AvaSineNode.inFrequency)
    AvaSineNode.addAttribute(AvaSineNode.inDelay)
    AvaSineNode.addAttribute(AvaSineNode.output)

    AvaSineNode.attributeAffects(AvaSineNode.inWeight, AvaSineNode.output)
    AvaSineNode.attributeAffects(AvaSineNode.inJointNo, AvaSineNode.output)
    AvaSineNode.attributeAffects(AvaSineNode.inGlobalTime, AvaSineNode.output)
    AvaSineNode.attributeAffects(AvaSineNode.inGlobalAmplitude, AvaSineNode.output)
    AvaSineNode.attributeAffects(AvaSineNode.inGlobalFrequency, AvaSineNode.output)
    AvaSineNode.attributeAffects(AvaSineNode.inGlobalDelay, AvaSineNode.output)
    AvaSineNode.attributeAffects(AvaSineNode.inAmplitude, AvaSineNode.output)
    AvaSineNode.attributeAffects(AvaSineNode.inFrequency, AvaSineNode.output)
    AvaSineNode.attributeAffects(AvaSineNode.inDelay, AvaSineNode.output)


# initialize the script plug-in
def initializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.registerNode(kPluginNodeTypeName, AvaSineNodeId, nodeCreator, nodeInitializer)
    except:
        sys.stderr.write("Failed to register node: %s" % kPluginNodeTypeName)
        raise


# uninitialize the script plug-in
def uninitializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.deregisterNode(AvaSineNodeId)
    except:
        sys.stderr.write("Failed to deregister node: %s" % kPluginNodeTypeName)
        raise
