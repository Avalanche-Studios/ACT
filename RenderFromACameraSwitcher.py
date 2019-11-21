
#--------------------------------------------------------------------------------------
# File: RenderFromACameraSwitcher.py
#
# The script has a project/pipeline specified functions, the legacy realization of them are listened in Common_Public.py. Please see “How to Use” section for more information.
#
# This is an action script.
#  The script will switch to camera switcher and render scene into a file
# Features:
# * It will use *.mov format and x264 codec
# * If you have p4 (perforce) module installed, you can add renderer file into a new change list
# * in Common_Public.py you can find general pipeline function for sanitize relative path to your project
#      and a function to build an export file path out of your scene file
#  Customize these function depends on your needs.
#
# Copyright (c) Avalanche Studios. All rights reserved.
# Licensed under the MIT License.
#-------------------------------------------------------------------------------------

import os
from pyfbsdk import *

import Common_Public

g_System = FBSystem()
g_App = FBApplication()

g_TimeStep = 1 # for 60 fps animation file, put 2 here to have 30 fps output video
g_RenderAudio = True

# if we have perforce, lets create a new change list automaticaly
try:
    import p4cmd
    
    def AddFileToChangeList(f):
        # Create a fancy file name
        rel_path = GetSanitizedRelPath()
        # Only change changelist if the changelist is default
        # Adds the files to the changelist if theyre not already checked out
        p4cmd.edit(str(f), timeout=10, description="Motionbuilder Export from <{}>".format(str(rel_path)))

except:
    def AddFileToChangeList(f):
        print('[RenderFromCameraSwitcher] Perforce library is not found')
        pass


# DONE: Render camera switcher
def RenderTake(file_name):
    
    options = FBVideoGrabber().GetOptions()
                  
    # Set VideoCodec Option:
    VideoManager = FBVideoCodecManager()
    VideoManager.VideoCodecMode = FBVideoCodecMode.FBVideoCodecStored
    
    codeclist = FBStringList()
    codeclist = VideoManager.GetCodecIdList('MOV')

    for item in codeclist:
        if item.find('x264') >= 0: 
            VideoManager.SetDefaultCodec('MOV', item)
    VideoManager.SetDefaultCodec('MOV', 'x264')
        
    # DONE: take a scene filename, otherwise ask user for a file path

    options.OutputFileName = file_name
    options.CameraResolution = FBCameraResolutionMode.kFBResolutionFullScreen
    options.AntiAliasing = True
    options.ShowTimeCode = True
    options.ShowCameraLabel = True
    # for 60 fps lets white video in half frame rate
    options.TimeSpan = FBTimeSpan(FBTime(0,0,0,0), FBTime(0,0,0,0))
    options.TimeSteps = FBTime(0,0,0, g_TimeStep) 
    
    # Audio
    options.RenderAudio = g_RenderAudio
    
    if g_RenderAudio:

        lNewFormat  = FBAudioFmt_ConvertBitDepthMode( FBAudioBitDepthMode.kFBAudioBitDepthMode_16 )
        lNewFormat |= FBAudioFmt_ConvertRateMode( FBAudioRateMode.kFBAudioRateMode_48000 )
        lNewFormat |= FBAudioFmt_ConvertChannelMode( FBAudioChannelMode.kFBAudioChannelModeStereo )
        options.AudioRenderFormat = lNewFormat
        
    # On Mac OSX, QuickTime renders need to be in 32 bits.

    lRenderFileFormat = '.mov'
    # Only windows supports mov.
    if lRenderFileFormat == '.mov' and os.name != 'nt':
        options.BitsPerPixel = FBVideoRenderDepth.FBVideoRender32Bits
    
    # Do the render. This will always be done in uncompressed mode.
    g_App.FileRender( options )
    
#
#

file_name = g_App.FBXFileName
if len(file_name) == 0:
    FBMessageBox('Render Scene', "Sorry, the current scene doesn't have a name", "Ok")
else:
    export_path = BuildAnExportPath(file_name)
    
    # switch to a camera switcher on a current pane
    index = g_System.Renderer.GetSelectedPaneIndex()
    if g_System.Renderer.IsCameraSwitcherInPane(index) == False:
        g_System.Renderer.SetCameraSwitcherInPane(index, True)    
    
    RenderTake(export_path)
    AddFileToChangeList(export_path)
