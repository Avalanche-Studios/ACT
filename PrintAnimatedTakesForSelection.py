
#--------------------------------------------------------------------------------------
# File: PrintAnimatedTakesForSelection.py
#
# This is an action script.
#  The script will print names of takes on which your selected models are animated
#
# Copyright (c) Avalanche Studios. All rights reserved.
# Licensed under the MIT License.
#-------------------------------------------------------------------------------------

from pyfbsdk import *

g_System = FBSystem()
g_Scene = g_System.Scene

##

def AnyKeyframesOnNode(node):
    
    for the_child in node.Nodes:
        if AnyKeyframesOnNode(the_child) > 0:
            return 1
    
    if node.KeyCount > 0:
        return 1
            
    return 0

def GetAnimatedTakesForModels(models):
    
    takes = []
    
    for the_take in g_Scene.Takes:
        
        g_System.CurrentTake = the_take
        g_Scene.Evaluate()
        
        any_keyframes = False
        
        for the_model in models:

            if AnyKeyframesOnNode(the_model.AnimationNode) > 0:
                any_keyframes = True
                break
                
        if any_keyframes is True:
            takes.append(the_take.Name)
    
    return takes
    
##
  
g_Models = FBModelList()
FBGetSelectedModels(g_Models)

curr_take = g_System.CurrentTake
  
print GetAnimatedTakesForModels(g_Models)

g_System.CurrentTake = curr_take