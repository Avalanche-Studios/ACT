
#--------------------------------------------------------------------------------------
# File: FillTakesTimeSpan.py
#
# This is an action script.
#   For selected models the script will go through every scene take
#  and adjust timeline range to actual models keyframes time range
#
# Copyright (c) Avalanche Studios. All rights reserved.
# Licensed under the MIT License.
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