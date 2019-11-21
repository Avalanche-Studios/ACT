
#--------------------------------------------------------------------------------------
# File: CopyLayerForOtherTakes.py
#
# This is an action script.
#  For every selected model the script will copy current layer TRS animation
#   into every other existing take in a scene
#
# Copyright (c) Avalanche Studios. All rights reserved.
# Licensed under the MIT License.
#-------------------------------------------------------------------------------------

from pyfbsdk import *

g_System = FBSystem()
g_Scene = g_System.Scene

models = FBModelList()
FBGetSelectedModels(models)

curr_take = g_System.CurrentTake
curr_layer = curr_take.GetCurrentLayer()

def PopulateCurveList(curve_list, anim_node):
    if anim_node is not None:
        for i in range(3):
            curve_list.append(anim_node.Nodes[i].FCurve)

def ReplaceWithCurves(curve_list, anim_node):
    if anim_node is not None and len(curve_list) > 0:
        for i in range(3):
            anim_node.Nodes[i].FCurve.KeyReplaceBy(curve_list[i])

for model in models:
    
    pos_curves = []
    rot_curves = []
    scl_curves = []
    
    PopulateCurveList(pos_curves, model.Translation.GetAnimationNode())
    PopulateCurveList(rot_curves, model.Rotation.GetAnimationNode())
    PopulateCurveList(scl_curves, model.Scaling.GetAnimationNode())
        
    for take in g_Scene.Takes:
        if take != curr_take:
            
            g_System.CurrentTake = take
            
            count = take.GetLayerCount()
            if count <= curr_layer:
                take.CreateNewLayer()
                take.SetCurrentLayer(count)
            else:
                take.SetCurrentLayer(curr_layer)
            
            ReplaceWithCurves(pos_curves, model.Translation.GetAnimationNode())
            ReplaceWithCurves(rot_curves, model.Rotation.GetAnimationNode())
            ReplaceWithCurves(scl_curves, model.Scaling.GetAnimationNode())                
            
            
    g_System.CurrentTake = curr_take
    

