
#--------------------------------------------------------------------------------------
# File: CheckTakeTransitions.py
#
#This is a tool script.
# Animations should be stored as scene takes 
# and the script will help to preview transitions between them by automating setup in Story Editor.
# 
# Features:
#   * automate process of story clips managing to preview transition between animations
#   * store/retreive a motion transition graph
#   * make a blend speed transition between motions (to test phase blend between walk and run, for example)
#
# Copyright (c) Avalanche Studios. All rights reserved.
# Licensed under the MIT License.
#-------------------------------------------------------------------------------------

from pyfbsdk import *
from pyfbsdk_additions import *
import re
from xml.dom import minidom
import math
import os.path

lApp = FBApplication()
lStory = FBStory()
lSystem = FBSystem()
lScene = lSystem.Scene
lPlayer = FBPlayerControl()

gDevelopment = False
gFilter = ''

gListAllNames = []
gListAllTakes = FBList()

gListSourceTakes = FBList()
gListDestTakes = FBList()

# this is a global definition for a character pipeline
TOOL_TITLE              = 'Check Transitions Tool'
CHAR_NAMESPACE          = '' # character namespace
CHAR_ROOT               = 'Hips' # this is a name of a root node
FOOTSTEP_MARK_NAME      = 'LEG_LIFT'

gGraphFilePath = os.path.join( lSystem.UserConfigPath, 'TransitionsGraph.xml' )


# global dictionary that stores transitions between motions
gLastTake = "Take 001"
gTransitions = {}

def LoadGraph(fname):
    
    global gLastTake
    global gTransitions
    gTransitions = {}
    
    
    xmldoc = minidom.parse(fname)
    allTakeElems = xmldoc.getElementsByTagName("Take")
    for takeElem in allTakeElems:
        takeName = takeElem.getAttribute("Name").encode('utf8')
        
        connections_elem = takeElem.getElementsByTagName("Connection")
        dstArray = []
             
        for conn_elem in connections_elem:
            
            conn_name = conn_elem.getAttribute('Name').encode('utf8')
            dstArray.append(conn_name)
            
        if len(dstArray) > 0:
            gTransitions[takeName] = dstArray
             
    EventChangeSourceTake(gListSourceTakes, None)
    
def SaveGraph(fname):
    
    impl = minidom.getDOMImplementation()
    
    newdoc = impl.createDocument(None, "TransitionGraph", None)
    top_element = newdoc.documentElement
    text = newdoc.createTextNode(lApp.FBXFileName)
    top_element.appendChild(text)
    
    elem_takes = newdoc.createElement('Takes')
    
    for key in gTransitions.keys():
        
        elem = newdoc.createElement('Take')
        elem.setAttribute('Name', key)
        
        elem_connectors = newdoc.createElement('Connections')
        
        dstArray = gTransitions[key]
        for value in dstArray:
            elem_dst = newdoc.createElement('Connection')
            elem_dst.setAttribute('Name', value)
            elem_connectors.appendChild(elem_dst)
        
        if len(dstArray) > 0:
            elem.appendChild(elem_connectors)
            
        elem_takes.appendChild(elem)
    
    top_element.appendChild(elem_takes)

    #    
    with open(fname, "w") as f:
        f.writelines(newdoc.toprettyxml())


def ClearStoryFolder(folder):
    pass

def ClearStory():
    
    ClearStoryFolder(lStory.RootFolder)
    ClearStoryFolder(lStory.RootEditFolder)

    list_to_delete = []

    for comp in lScene.Components:
        if isinstance(comp, FBStoryTrack) or isinstance(comp, FBStoryClip):
            list_to_delete.append(comp)
            
    map(FBComponent.FBDelete, list_to_delete)

def AddModelToList(model_list, root):

    model_list.append(root)
    
    for child in root.Children:
        AddModelToList(model_list, child)

def FindRootModel(char_ns=CHAR_NAMESPACE):
    root_name = '{}:{}'.format(char_ns, CHAR_ROOT) if char_ns != '' else CHAR_ROOT
    return FBFindModelByLabelName(root_name)

def ConnectNSModelsToProp(prop, char_ns):
    
    model_list = []
    root_model = FindRootModel(char_ns)
    
    if root_model is None:
        raise Exception('Root model is not found, please check tool documentation')
    
    AddModelToList(model_list, root_model)
    
    for obj in model_list:
        FBConnect(obj, prop)

def CalculateTakeTimeSpan(char_ns):
    
    time_span = FBTimeSpan(lPlayer.LoopStart, lPlayer.LoopStop)
    
    root_model = FindRootModel(char_ns)    
    if root_model is None:
        raise Exception('Root model is not found, please check tool documentation')
        
    anim_node = root_model.Translation.GetAnimationNode()
    if anim_node is not None and anim_node.KeyCount > 0 and anim_node.Nodes[0].FCurve is not None:
    
        fcurve = anim_node.Nodes[0].FCurve
        start_time = fcurve.Keys[0].Time
        stop_time = fcurve.Keys[len(fcurve.Keys)-1].Time
    
        time_span.Set(start_time, stop_time)
    else:
        raise Exception('Root model doesnt have any animation in a current take')
    
    return time_span
        

def MakeTransition(char_ns, src, dst):
    
    clips = []
    
    lTrack = FBStoryTrack(FBStoryTrackType.kFBStoryTrackAnimation)
    
    prop = lTrack.PropertyList.Find('Track Content')
    if prop is not None:
        ConnectNSModelsToProp(prop, char_ns)
    
    src_take = None
    dst_take = None
    
    for take in lScene.Takes:
        if take.Name == src:
            src_take = take
        elif take.Name == dst:
            dst_take = take    
    
    if src_take is not None and dst_take is not None:
    
        lSrcSpan = src_take.ReferenceTimeSpan #LocalTimeSpan
        lDstSpan = dst_take.ReferenceTimeSpan #LocalTimeSpan
        
        currTake = lSystem.CurrentTake
        
        lSystem.CurrentTake = src_take
        lScene.Evaluate()
        lSrcSpan = CalculateTakeTimeSpan(char_ns)
        lPlayer.LoopStart = lSrcSpan.GetStart()
        lPlayer.LoopStop = lSrcSpan.GetStop()
                
        src_clip = lTrack.CopyTakeIntoTrack(lSrcSpan, src_take)
        
        lSystem.CurrentTake = dst_take
        lScene.Evaluate()
        lDstSpan = CalculateTakeTimeSpan(char_ns)
        lPlayer.LoopStart = lDstSpan.GetStart()
        lPlayer.LoopStop = lDstSpan.GetStop()

        dst_clip = lTrack.CopyTakeIntoTrack(lDstSpan, dst_take, lSrcSpan.GetStop())
        
        lSystem.CurrentTake = currTake
        lScene.Evaluate()
        
        dst_clip.Match('{}:{}'.format(char_ns, CHAR_ROOT), 
            FBStoryClipMatchingTimeType.kFBStoryClipMatchingTimeEndOfPreviousClip, 
            FBStoryClipMatchingTranslationType.kFBStoryClipMatchingTranslationGravityXZ, 
            FBStoryClipMatchingRotationType.kFBStoryClipMatchingRotationXYZ) 

        clips = [src_clip, dst_clip]

    return clips

def MakeTransitionB(char_ns, src, dst):
    
    # just add a dst take at the end of existing list of clips

    track = None
    
    for comp in lScene.Components:
        if isinstance(comp, FBStoryTrack) and comp.Type == FBStoryTrackType.kFBStoryTrackAnimation:
            track = comp
            
    if track is None:
        MakeTransition(char_ns, src, dst)
    else:
        
        lSrcTake = None
        lDstTake = None
    
        for take in lScene.Takes:
            if take.Name == src:
                lSrcTake = take
            elif take.Name == dst:
                lDstTake = take 
        
        if lDstTake is not None:
        
            stop_time = FBTime()
            
            for clip in track.Clips:
                if stop_time < clip.Stop:
                    stop_time = clip.Stop
            
            currTake = lSystem.CurrentTake
            lSystem.CurrentTake = lDstTake
            lScene.Evaluate()
            lDstSpan = CalculateTakeTimeSpan(char_ns)
            lPlayer.LoopStart = lDstSpan.GetStart()
            lPlayer.LoopStop = lDstSpan.GetStop()
    
            lDstClip = track.CopyTakeIntoTrack(lDstSpan, lDstTake, stop_time)
            
            lSystem.CurrentTake = currTake
            lScene.Evaluate()
            
            lDstClip.Match('{}:{}'.format(char_ns, CHAR_ROOT), 
                FBStoryClipMatchingTimeType.kFBStoryClipMatchingTimeEndOfPreviousClip, 
                FBStoryClipMatchingTranslationType.kFBStoryClipMatchingTranslationGravityXZ, 
                FBStoryClipMatchingRotationType.kFBStoryClipMatchingRotationXYZ) 

def EventButtonMakeCurrentClick(control, event):
    
    if len(gListAllTakes.Items) > 0 and gListAllTakes.ItemIndex >= 0:
        takeName = gListAllNames[gListAllTakes.ItemIndex]
        
        switchToTake = None
        for take in lScene.Takes:
        
            scene_take_name = take.Name
            if scene_take_name == takeName:
                switchToTake = take
                break
                
        if switchToTake is not None:
            lSystem.CurrentTake = switchToTake

def EventButtonLoadGraphClick(control, event):
    
    dialog = FBFilePopup()
    dialog.Filter = '*.xml'
    dialog.Style = FBFilePopupStyle.kFBFilePopupOpen
    dialog.Path = os.path.dirname(gGraphFilePath)

    if dialog.Execute():    
        LoadGraph(dialog.FullFilename)

def EventButtonSaveGraphClick(control, event):

    dialog = FBFilePopup()
    dialog.Filter = '*.xml'
    dialog.Style = FBFilePopupStyle.kFBFilePopupSave
    dialog.Path = os.path.dirname(gGraphFilePath)
    
    if dialog.Execute():    
        SaveGraph(dialog.FullFilename)

def EventEditFilterChange(control, event):
    
    global gFilter
    gFilter = control.Text
    RefreshUI()
    print 'event'

def EventChangeSourceTake(control, event):

    # load last dst user changes
    # load new dst list
    
    global gLastTake
    print gTransitions
    
    if len(control.Items) > 0 and control.ItemIndex >= 0:

        takeName = control.Items[control.ItemIndex]
        
        #if gLastTake != takeName:
            
        dstArray = []
        for item in gListDestTakes.Items:
            dstArray.append(item)
        gTransitions[gLastTake] = dstArray
        
        gLastTake = takeName
        #

        gListDestTakes.Items.removeAll()
        
        if takeName in gTransitions:
            dstArray = gTransitions[takeName]
            
            for item in dstArray:
                gListDestTakes.Items.append(item)

def EventButtonDstToSrcClick(control, event):
    
    if len(gListDestTakes.Items) > 0 and gListDestTakes.ItemIndex >= 0:
        take_name = gListDestTakes.Items[gListDestTakes.ItemIndex]
        
        itemIndex = -1        
        
        for i, srcName in enumerate(gListSourceTakes.Items):
            if take_name == srcName:
                itemIndex = i
                break

        if itemIndex >= 0:
            gListSourceTakes.ItemIndex = itemIndex
            EventChangeSourceTake(gListSourceTakes, None)

def EventButtonAddSourceTakeClick(control, event):
    
    itemIndex = -1
    
    if len(gListAllTakes.Items) > 0 and gListAllTakes.ItemIndex >= 0:
        takeName = gListAllNames[gListAllTakes.ItemIndex]
        
        for i, srcName in enumerate(gListSourceTakes.Items):
            if takeName == srcName:
                itemIndex = i
                break
    
    if itemIndex >= 0:
        gListSourceTakes.ItemIndex = itemIndex
        EventChangeSourceTake(gListSourceTakes, None)

def EventButtonAddDstTakeClick(control, event):
    
    itemIndex = -1
    
    if len(gListAllTakes.Items) > 0 and gListAllTakes.ItemIndex >= 0:
        
        for i, item in enumerate(gListAllTakes.Items):
        
            if gListAllTakes.IsSelected(i):
                gListDestTakes.Items.append(gListAllNames[i])

def EventButtonRemoveDstTakeClick(control, event):
    
    if len(gListDestTakes.Items) > 0 and gListDestTakes.ItemIndex >= 0:

        itemIndex = gListDestTakes.ItemIndex
        items = []
        for name in gListDestTakes.Items:
            items.append(name)
            
        gListDestTakes.Items.removeAll()
        
        for i, name in enumerate(items):
            
            if i != itemIndex:
                gListDestTakes.Items.append(name)
    
def EventButtonRemoveAllDstTakeClick(control, event):

    if len(gListDestTakes.Items) > 0:
        gListDestTakes.Items.removeAll()

def EventButtonUpDstTakeClick(control, event):

    itemIndex = gListDestTakes.ItemIndex
    if len(gListDestTakes.Items) > 1 and itemIndex >= 1:
        
        newIndex = itemIndex-1
        
        items = []
        for name in gListDestTakes.Items:
            items.append(name)
        
        temp = items[newIndex]
        items[newIndex] = items[itemIndex]
        items[itemIndex] = temp
        
        gListDestTakes.Items.removeAll()
        for name in items:
            gListDestTakes.Items.append(name)
        
        gListDestTakes.ItemIndex = newIndex
        

def EventButtonDownDstTakeClick(control, event):
    
    itemIndex = gListDestTakes.ItemIndex
    count = len(gListDestTakes.Items)
    
    if count > 1 and itemIndex <= count-2:
        
        newIndex = itemIndex+1
        
        items = []
        for name in gListDestTakes.Items:
            items.append(name)
        
        temp = items[newIndex]
        items[newIndex] = items[itemIndex]
        items[itemIndex] = temp
        
        gListDestTakes.Items.removeAll()
        for name in items:
            gListDestTakes.Items.append(name)
        
        gListDestTakes.ItemIndex = newIndex

def EventButtonMakeTransitionClick(control, event):
    
    if len(gListSourceTakes.Items) > 0 and gListSourceTakes.ItemIndex >= 0 and len(gListDestTakes.Items) > 0:
        
        srcTake = gListSourceTakes.Items[gListSourceTakes.ItemIndex]        
        dstTake = gListDestTakes.Items[gListDestTakes.ItemIndex] if gListDestTakes.ItemIndex >= 0 else 0
        
        MakeTransition(CHAR_NAMESPACE, srcTake, dstTake)

def EventButtonMakeTransitionBClick(control, event):
    
    if len(gListSourceTakes.Items) > 0 and gListSourceTakes.ItemIndex >= 0 and len(gListDestTakes.Items) > 0:
        
        srcTake = gListSourceTakes.Items[gListSourceTakes.ItemIndex]        
        dstTake = gListDestTakes.Items[gListDestTakes.ItemIndex] if gListDestTakes.ItemIndex >= 0 else 0
        
        try:
        
        MakeTransitionB(CHAR_NAMESPACE, srcTake, dstTake)

        except Exception as e:
            FBMessageBox(TOOL_TITLE, e.message, 'Ok')
            raise
            

def EventButtonPutSegAClick(control, event):
    
    localTime = lSystem.LocalTime
    currTake = lSystem.CurrentTake
    
    idx = currTake.AddTimeMark(localTime)
    currTake.SetTimeMarkName(idx, FOOTSTEP_MARK_NAME)
    
def EventButtonPutSegBClick(control, event):
    
    # TODO: at the moment we do only one label
    EventButtonPutSegAClick(control, event)
    
def GetRootDistance(model, timeA, timeB):
    
    v1 = FBVector3d()
    v2 = FBVector3d()
    
    animNode = model.Translation.GetAnimationNode()

    for i in range(3):
        v1[i] = animNode.Nodes[i].FCurve.Evaluate(timeA)
        v2[i] = animNode.Nodes[i].FCurve.Evaluate(timeB)
    
    v = FBVector3d(v2[0]-v1[0], v2[1]-v1[1], v2[2]-v1[2])
    
    return math.sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2])

def EventButtonComputeSpeedClick(control, event):
    
    dist1 = 0.0
    speed1 = 0.0

    try:

        root_model = FindRootModel()
        if root_model is None:
            raise Exception('failed to find a root node')
        
        src_take = lSystem.CurrentTake
        number_of_marks = src_take.GetTimeMarkCount()
        if number_of_marks < 2:
            raise Exception('failed to find two time marks in src clip {}'.format(src_take.Name))

        segA = [src_take.GetTimeMarkTime(0), src_take.GetTimeMarkTime(1)]   
        
        dist1 = GetRootDistance(root_model, segA[0], segA[1])
        time1 = segA[1].GetSecondDouble() - segA[0].GetSecondDouble()
        speed1 = dist1 / time1
  
    except Exception as e:
        FBMessageBox(TOOL_TITLE, e.message, 'Ok')
        raise
    else:
        FBMessageBox(TOOL_TITLE, 'Root distance {} and speed {}'.format(dist1, speed1), 'Ok')

def ChangeSourceWarp(warpNode, origin, originValue, speedA, speedB, transition_time, is_reverse):

    deltaSpeed = abs(speedB - speedA)
    if deltaSpeed < 0.01:
        print 'not big difference in speed, no need in time wrap'
        return
    
    speed1 = 1.0
    speed2 = speedB / speedA
    
    a = (speed2 - speed1) / 2.0
    
    
    fCurve = warpNode.FCurve
    fCurve.EditBegin()
    
    fps = 30.0
    
    scale = speedA / speedB
    
    lTime = FBTime(0)
    while lTime <= transition_time:

        t = 1.0 * lTime.GetSecondDouble() / transition_time.GetSecondDouble()
        print t
        s = a * t * t / 2.0
        print s
        
        #deltaValue = (speedA + deltaSpeed) / speedA * t.GetSecondDouble()
        deltaFrame = 1.0 * lTime.GetFrame()
        
        if is_reverse:
            deltaFrame = deltaFrame * scale
        
        value = originValue.GetFrame() + deltaFrame
        
        # convert to frames
        value = value / fps + s
        
        # clamp and loop motion
        if value > 32.0:
            value = 32.0

        print value
        #warpNode.KeyAdd(origin + lTime, value)
        fCurve.KeyAdd(origin + lTime, value)
        
        lTime = lTime + FBTime(0,0,0,1)
    
    fCurve.EditEnd()

def EventButtonTestClick(control, event):
    
    default_blend_time = FBTime(0,0,0, 12)
    
    tempTake = lSystem.CurrentTake
    
    if len(gListSourceTakes.Items) > 0 and gListSourceTakes.ItemIndex >= 0 and len(gListDestTakes.Items) > 0:
        
        srcTakeName = gListSourceTakes.Items[gListSourceTakes.ItemIndex]
        dstTakeName = gListDestTakes.Items[gListDestTakes.ItemIndex] if gListDestTakes.ItemIndex >= 0 else 0
        
        lSrcTake = None
        lDstTake = None
        
        for take in lScene.Takes:
            if take.Name == srcTakeName:
                lSrcTake = take
            elif take.Name == dstTakeName:
                lDstTake = take   
        
        # get segments for A        
        numberOfMarks = lSrcTake.GetTimeMarkCount()
        if numberOfMarks < 2:
            print 'failed to find two segments in src clip'
            return
        
        segA = [lSrcTake.GetTimeMarkTime(0), lSrcTake.GetTimeMarkTime(1)]
        
        # root node
        
        root_model = FindRootModel()
        if root_model is None:
            print 'failed to find a root node'
            return
        
        # get speed of A
        
        lSystem.CurrentTake = lSrcTake
        
        distA = GetRootDistance(root_model, segA[0], segA[1])
        timeA = segA[1].GetSecondDouble() - segA[0].GetSecondDouble()
        speedA = distA / timeA
        
        # get segments for B
        
        numberOfMarks = lDstTake.GetTimeMarkCount()
        if numberOfMarks < 2:
            print 'failed to find two segments in dst clip'
            return
        
        segB = [lDstTake.GetTimeMarkTime(0), lDstTake.GetTimeMarkTime(1)]
        
        lSystem.CurrentTake = lDstTake
        
        distB = GetRootDistance(root_model, segB[0], segB[1])
        timeB = segB[1].GetSecondDouble() - segB[0].GetSecondDouble()
        speedB = distB / timeB
        
        print speedA
        print speedB
    
        track = None
        clip = None
        originTime = segA[0]
        
        for comp in lScene.Components:
            if isinstance(comp, FBStoryTrack) and comp.Type == FBStoryTrackType.kFBStoryTrackAnimation:
                track = comp
                
                if len(track.Clips) > 0:
                    clip = track.Clips[0]
                    
        if clip is None:
            print "couldn't find a clip"
            return

        propFadeStart = clip.PropertyList.Find('Fade Out Start')
        propFadeStop = clip.PropertyList.Find('Fade Out Stop')
        
        transition_time = FBTime(0,0,0,8)
        
        if propFadeStart is not None and propFadeStop is not None:
            fadeStart = propFadeStart.Data
            fadeStop = propFadeStop.Data
            
            transition_time = fadeStop - fadeStart
            
        clip.TimeWarpEnabled = True
        clip.TimeWarpInterpolatorType = FBStoryClipTimeWarpInterpolatorType.kFBStoryClipTimeWarpInterpolatorLinear        
        clip.TimeWarpInterpolatorType = FBStoryClipTimeWarpInterpolatorType.kFBStoryClipTimeWarpInterpolatorCustom
        warpNode = clip.CustomTimeWarp
        
        ChangeSourceWarp(warpNode, originTime, speedA, speedB, transition_time)
    

    lSystem.CurrentTake = tempTake

def EventButtonMakeBlendClick(control, event):
    
    tempTake = lSystem.CurrentTake
    
    # 12 frames blend time
    default_blend_time = FBTime(0,0,0, 12)
    
    if len(gListSourceTakes.Items) > 0 and gListSourceTakes.ItemIndex >= 0 and len(gListDestTakes.Items) > 0:
        
        srcTakeName = gListSourceTakes.Items[gListSourceTakes.ItemIndex]
        dstTakeName = gListDestTakes.Items[gListDestTakes.ItemIndex] if gListDestTakes.ItemIndex >= 0 else 0
        
        lSrcTake = None
        lDstTake = None
        
        for take in lScene.Takes:
            if take.Name == srcTakeName:
                lSrcTake = take
            elif take.Name == dstTakeName:
                lDstTake = take   
        
        # get segments for A        
        numberOfMarks = lSrcTake.GetTimeMarkCount()
        if numberOfMarks < 2:
            print 'failed to find two segments in src clip'
            return
        
        segA = [lSrcTake.GetTimeMarkTime(0), lSrcTake.GetTimeMarkTime(1)]
        
        # root node
        
        root_model = FindRootModel()
        if root_model is None:
            print 'failed to find a root node'
            return
        
        # get speed of A
        
        lSystem.CurrentTake = lSrcTake
        
        distA = GetRootDistance(root_model, segA[0], segA[1])
        timeA = segA[1].GetSecondDouble() - segA[0].GetSecondDouble()
        speedA = distA / timeA
        
        # get segments for B
        
        numberOfMarks = lDstTake.GetTimeMarkCount()
        if numberOfMarks < 2:
            print 'failed to find two segments in dst clip'
            return
        
        segB = [lDstTake.GetTimeMarkTime(0), lDstTake.GetTimeMarkTime(1)]
        
        lSystem.CurrentTake = lDstTake
        
        distB = GetRootDistance(root_model, segB[0], segB[1])
        timeB = segB[1].GetSecondDouble() - segB[0].GetSecondDouble()
        speedB = distB / timeB
        
        print speedA
        print speedB
        
        ## Story Blend
        
        clips = MakeTransition(CHAR_NAMESPACE, srcTakeName, dstTakeName)
    
        lPlayer.Goto(segA[0])
        clips[0].Stop = segA[1]
        
        deltaTime = FBTime(0,0,0, -segB[0].GetFrame())
        
        #
        # warp source
        
        clips[0].TimeWarpEnabled = True
        clips[0].TimeWarpInterpolatorType = FBStoryClipTimeWarpInterpolatorType.kFBStoryClipTimeWarpInterpolatorCustom
        warpNode = clips[0].CustomTimeWarp
        
        propFadeStart = clips[0].PropertyList.Find('Fade Out Start')
        propFadeStop = clips[0].PropertyList.Find('Fade Out Stop')
        
        transition_time = FBTime(0,0,0,8)
        
        if propFadeStart is not None and propFadeStop is not None:
            fadeStart = propFadeStart.Data
            fadeStop = propFadeStop.Data
            
            transition_time = fadeStop - fadeStart
        
        ChangeSourceWarp(warpNode, segA[0], segA[0], speedA, speedB, transition_time, False)

        #
        # warp dst
        clips[1].Start = segB[0]
        clips[1].MoveTo(segA[0], True)
        
        #clips[1].Speed = speedA / speedB
        
        clips[1].TimeWarpEnabled = True
        clips[1].TimeWarpInterpolatorType = FBStoryClipTimeWarpInterpolatorType.kFBStoryClipTimeWarpInterpolatorCustom
        warpNode = clips[1].CustomTimeWarp
        
        propFadeStart = clips[1].PropertyList.Find('Fade Out Start')
        propFadeStop = clips[1].PropertyList.Find('Fade Out Stop')
        
        transition_time = FBTime(0,0,0,8)
        
        if propFadeStart is not None and propFadeStop is not None:
            fadeStart = propFadeStart.Data
            fadeStop = propFadeStop.Data
            
            transition_time = fadeStop - fadeStart
        
        ChangeSourceWarp(warpNode, segB[0], segA[0], speedA, speedB, transition_time, True )

        #
        # root translation align

        clips[1].Match(root_model.LongName, 
            FBStoryClipMatchingTimeType.kFBStoryClipMatchingTimeCurrentTime, 
            FBStoryClipMatchingTranslationType.kFBStoryClipMatchingTranslationGravityXZ, 
            FBStoryClipMatchingRotationType.kFBStoryClipMatchingRotationXYZ) 
        
    lSystem.CurrentTake = tempTake
    #

def RefreshUI():
    
    global gListAllNames
    gListAllNames = []
    
    gListAllTakes.Items.removeAll()
    
    source_name = ''
    if len(gListSourceTakes.Items) > 0 and gListSourceTakes.ItemIndex >= 0:
        source_name = gListSourceTakes.Items[gListSourceTakes.ItemIndex]
        
    gListSourceTakes.Items.removeAll()
    
    for take in lScene.Takes:
        
        takeName = take.Name
        addTake = True

        if len(gFilter) > 0:
            if not re.search(gFilter, takeName):        
                addTake = False

        if addTake is True:
            gListAllNames.append(takeName)
            
            time_span = take.LocalTimeSpan
            
            adv_name = takeName + '\t[' + str(time_span.GetStart().GetFrame()) + ' - ' + str(time_span.GetStop().GetFrame()) + ']'
            gListAllTakes.Items.append(adv_name)

        gListSourceTakes.Items.append(takeName)

    if len(gListSourceTakes.Items) > 0 and gListSourceTakes.ItemIndex >= 0:

        for i, item in enumerate(gListSourceTakes.Items):
            if item == source_name:
                gListSourceTakes.ItemIndex = i
                break

    EventChangeSourceTake(gListSourceTakes, None)

def EventButtonRefreshClick(control, event):
    RefreshUI()

def EventShowTool(control, event):
    
    if event.Shown is True:
        RefreshUI()

def PopulateLayout(mainLyt):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBHBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    # TODO: add more controls
    
    lyt = FBVBoxLayout()
    
    
    toolbar = FBHBoxLayout()
    
    b = FBEdit()
    b.OnChange.Add(EventEditFilterChange)
    toolbar.Add(b, 180)
    
    
    b = FBButton()
    b.Caption = 'Transition A->B'
    b.Hint = ''
    b.OnClick.Add( EventButtonMakeTransitionClick )
    toolbar.Add(b, 120)

    b = FBButton()
    b.Caption = 'Transition ->B'
    b.Hint = ''
    b.OnClick.Add( EventButtonMakeTransitionBClick )
    toolbar.Add(b, 120)

    b = FBButton()
    b.Caption = 'Make Current'
    b.Hint = ''
    b.OnClick.Add( EventButtonMakeCurrentClick )
    toolbar.Add(b, 120)

    
    b = FBButton()
    b.Caption = 'Load Graph'
    b.Hint = ''
    b.OnClick.Add( EventButtonLoadGraphClick )
    toolbar.Add(b, 120)
    
    b = FBButton()
    b.Caption = 'Save Graph'
    b.Hint = ''
    b.OnClick.Add( EventButtonSaveGraphClick )
    toolbar.Add(b, 120)
    
    #
    lyt.Add(toolbar, 25)
    
    ## Blend transition toolbar
    toolbar = FBHBoxLayout()
    
    b = FBButton()
    b.Caption = 'Refresh'
    b.OnClick.Add( EventButtonRefreshClick )
    toolbar.Add(b, 120)
    b = FBLabel()
    b.Caption = ''
    toolbar.Add(b, 100)
    
    
    b = FBButton()
    b.Caption = 'Mark Seg Start'
    b.Hint = 'Put a start segment into a current take'
    b.OnClick.Add( EventButtonPutSegAClick )
    toolbar.Add(b, 120)

    b = FBButton()
    b.Caption = 'Mark Seg Stop'
    b.Hint = 'Put a stop segment into a current take'
    b.OnClick.Add( EventButtonPutSegBClick )
    toolbar.Add(b, 120)

    b = FBButton()
    b.Caption = 'Make Blend A->B'
    b.Hint = ''
    b.OnClick.Add( EventButtonMakeBlendClick )
    toolbar.Add(b, 120)
    
    b = FBButton()
    b.Caption = 'Compute Speed'
    b.Hint = ''
    b.OnClick.Add( EventButtonComputeSpeedClick )
    toolbar.Add(b, 120)
    
    b = FBButton()
    b.Caption = 'Test'
    b.Hint = ''
    b.OnClick.Add( EventButtonTestClick )
    toolbar.Add(b, 120)    

    #
    lyt.Add(toolbar, 25)
    
    #
    #
        
    mainLyt = FBHBoxLayout()
    
    gListAllTakes.Style = FBListStyle.kFBVerticalList
    gListAllTakes.MultiSelect = True
    mainLyt.AddRelative(gListAllTakes, 0.4)
    
    gUserLyt = FBVBoxLayout()
    
    b = FBLabel()
    b.Caption = 'Source Take:'
    gUserLyt.Add(b, 25)
 
    ## src lyt
    gSrcLyt = FBHBoxLayout()
    
    b = FBButton()
    b.Caption = '>>'
    b.OnClick.Add(EventButtonAddSourceTakeClick)
    gSrcLyt.Add(b, 25)
       
    gListSourceTakes.Style = FBListStyle.kFBDropDownList
    gListSourceTakes.OnChange.Add(EventChangeSourceTake)
    gSrcLyt.AddRelative(gListSourceTakes, 0.9)
    
    gUserLyt.Add(gSrcLyt, 25)
    
    b = FBLabel()
    b.Caption = 'Destination Takes:'
    gUserLyt.Add(b, 25)
    
    ## dst lyt
    gDstLyt = FBHBoxLayout()
    
    gDstToolbar = FBVBoxLayout()
    
    b = FBButton()
    b.Caption = '>>'
    b.OnClick.Add(EventButtonAddDstTakeClick)
    gDstToolbar.Add(b, 25)
    
    b = FBButton()
    b.Caption = 'x'
    b.OnClick.Add(EventButtonRemoveDstTakeClick)
    gDstToolbar.Add(b, 25)

    b = FBButton()
    b.Caption = 'Clear'
    b.OnClick.Add(EventButtonRemoveAllDstTakeClick)
    gDstToolbar.Add(b, 25)
    
    b = FBButton()
    b.Caption = 'Up'
    b.OnClick.Add(EventButtonUpDstTakeClick)
    gDstToolbar.Add(b, 25)

    b = FBButton()
    b.Caption = 'Down'
    b.OnClick.Add(EventButtonDownDstTakeClick)
    gDstToolbar.Add(b, 25)
    
    b = FBButton()
    b.Caption = '^'
    b.Hint = 'Assign current destination take as a source take'
    b.OnClick.Add(EventButtonDstToSrcClick)
    gDstToolbar.Add(b, 25)
    
    gDstLyt.Add(gDstToolbar, 40)
    
    #
    
    gListDestTakes.Style = FBListStyle.kFBVerticalList
    gDstLyt.AddRelative(gListDestTakes, 0.9)
    
    gUserLyt.AddRelative(gDstLyt, 0.6)
    
    mainLyt.AddRelative(gUserLyt, 0.6)
    
    lyt.AddRelative(mainLyt, 0.9)
    
    main.AddRelative(lyt, 1.0)
    
    #RefreshUI()

def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool(TOOL_TITLE)
    PopulateLayout(t)
    t.StartSizeX = 800
    t.StartSizeY = 600
    t.OnShow.Add( EventShowTool )

    if gDevelopment:
        ShowTool(t)

CreateTool()