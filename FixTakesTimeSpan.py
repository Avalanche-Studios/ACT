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
# File: FillTakesTimeSpan.py
#
# This is an action script.
#   For selected models the script will go through every scene take
#  and adjust timeline range to actual models keyframes time range
#
#-------------------------------------------------------------------------------------

from pyfbsdk import *

g_System = FBSystem()
g_Scene = g_System.Scene
g_Player = FBPlayerControl()

#

def CalculateTakeTimeSpan(ref_model):
    
    time_span = FBTimeSpan(g_Player.LoopStart, g_Player.LoopStop)
    
    if ref_model is not None:
    
        anim_node = ref_model.Translation.GetAnimationNode()
        if anim_node is not None and anim_node.KeyCount > 0 and anim_node.Nodes[0].FCurve is not None:
            
            curve = anim_node.Nodes[0].FCurve
            start_time = curve.Keys[0].Time
            stop_time = curve.Keys[len(curve.Keys)-1].Time
        
            time_span.Set(start_time, stop_time)
    
    return time_span

#

current_take = g_System.CurrentTake

g_Models = FBModelList()
FBGetSelectedModels(g_Models)

if len(g_Models) > 0:

    for take in g_Scene.Takes:
        
        g_System.CurrentTake = take
        g_Scene.Evaluate()
    
        time_span = CalculateTakeTimeSpan(g_Models[0])
        
        g_Player.LoopStart = time_span.GetStart()
        g_Player.LoopStop = time_span.GetStop()
        
g_System.CurrentTake = current_take