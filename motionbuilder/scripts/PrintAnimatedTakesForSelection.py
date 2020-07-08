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
# File: PrintAnimatedTakesForSelection.py
#
# This is an action script.
#  The script will print names of takes on which your selected models are animated
#
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