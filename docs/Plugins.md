
Maya Plugins
============

Live Streaming
-------------------

  This is a python based plugin to start streaming joints transform and float properties values as a server for live bridge.

  Python module AnimLiveBridge.py and dynamic library _AnimLiveBridge.pyd you should put into a place where Maya could catch it.

  To work with a plugin you should select joint objects.
  Float properties that you would like to stream should start with "float_"
  
  The Plugin registers two commands StartLive() / StopLive()


MotionBuilder Plugins
=====================

 Plugins are designed for 2017+ version of software (MotionBuilder, Maya)


Fbx Extension Driven Keys
--------------------------

There are two plugins, one for MotionBuilder and one for Maya.
Plugins show a way how with help of fbx extensions, we can store/retrieve and exchange between apps animation driven keys relations.

[![MayaDrivenKeysGraph](https://github.com/Avalanche-Studios/ACT/blob/master/docs/images/maya_driven_keys_graph.jpg)]()

For more detailed information, please, have a look at a FBExt_DrivenKeys document.

Live Bridge Devices
-------------------

Server / Client device plugins for Autodesk MotionBuilder.
Designed to sync models animation between application instances.
That could help, for example, to animate face on a face rig in one motionbuilder instance
and have a live preview in a main scene with body animation in another motionbuilder instance

[![LiveBridgePreview](https://github.com/Avalanche-Studios/ACT/blob/master/docs/images/LiveBridge.gif)]()

For more detailed information, please, have a look at a LiveBridgeDevices document.

Marker Wire
-------------------

[![MarkerWire](https://github.com/Avalanche-Studios/ACT/blob/master/docs/images/marker_wire.JPG)]()

This is a Custom Marker Scene Object which extends functionality of normal Marker object with visible wire connections to specified list of objects.
Could be useful to track look at point direction, for example.


Relation Boxes
-------------------

The project extends relation constraint functionality with some custom boxes.
* SmartMix - the box comes from an animation pipeline with mix of mocap source data and some additional keyframing on top of it. To avoid double offset that comes from user control element and mocap source, the smart mix box is trying to mix together offsets which go in a same direction.
* RayPlaneIntersect - the box to compute intersect point between ray and a plane.

How to build
--------------------------

Use CMake 3.1 or higher to prepare solution and project files to build plugins
To Build MotionBuilder plugins you need OpenReality SDK which is always included in a MotinBuilder distributive.
To build Maya plugins you need Maya DevKit and to build Maya FBX Extension plugin you need FBX Extension SDK.

CMake Configuration
--------------------------

On first run of configuration, don't forget to choose a x64 compiler version.

To compile motionbuilder plugin, you should set paths to your MotionBuilder OpenReality SDK sub folders
ORSDK_INCLUDE_PATH - <your OpenReality SDK>/include
ORSDK_LIB_PATH - <your OpenReality SDK>/lib/x64

To copile maya plugin, you should set paths to you Maya DevKit sub folders
DEVKIT_INCLUDE_PATH - <your DevKit>/include
DEVKIT_LIB_PATH - <your DevKit>/lib

When press "Configure", CMake script will do check for needed header and library files and assign all other variables automaticaly.

Default Plugin locations
--------------------------

 These are locations or environment variables that could help you to make Maya or MotionBuilder start using your binaries.

For plugins:
Maya:        plugin you can load by using Windows->Settings/Preferences->Plugin-in Manager
MotionBuilder: <MB install folder>/bin/x64/plugins

For FBX Extensions
Maya:          <Maya install folder>/plug-ins/fbx/plug-ins/FBX/
MotionBuilder: <MB install folder>/bin/x64/plugins/FBX/


Environment Variables
--------------------------
There are environment variables which could help you to make MotionBuilder or Maya start using plugins without a need to copy them into a program files directory.

For plugins

Maya:           MAYA_PLUG_IN_PATH
MotionBuilder:  MOTIONBUILDER_PLUGIN_PATH

For FBX Extension plugins

Maya:            MAYA_FBX_EXTENSION_PATH
MotionBuilder:   MOBU_FBX_EXTENSION_PATH

 There is a small manager, that could help you to set up environment variables for MotionBuilder - https://github.com/Neill3d/MoBu_ConfigApp