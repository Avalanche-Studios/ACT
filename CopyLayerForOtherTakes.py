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
# File: CopyLayerForOtherTakes.py
#
# This is an action script.
#  For every selected model the script will copy current layer TRS animation
#   into every other existing take in a scene
#
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
    

