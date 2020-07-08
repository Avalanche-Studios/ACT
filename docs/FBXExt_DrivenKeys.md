FBX Extension Driven Keys
=====================

Maya import/export and MotionBuilder import fbx extension plugins to work with driven keys relations between objects

In FBX Container with help of new custom nodes CDAGNode and CDAGEdge we are storing / retrieving connection graph of relation keys.
In current implementation we are processing maya nodes and relations between them
 - joints
 - animation curve
 - unit conversion
 - blend weighted

[![MayaDrivenKeysGraph](https://github.com/Avalanche-Studios/ACT/blob/master/docs/images/maya_driven_keys_graph.jpg)]()

In MotionBuilder these nodes and relations are reconstructed with help of Relation Constraint Boxes.

[![MoBuDrivenKeysGraph](https://github.com/Avalanche-Studios/ACT/blob/master/docs/images/mobu_driven_keys_graph.jpg)]()

Notes
 * Work with joints
 * Work units in Maya should be in centimeters


Binaries
---------------------
In Release section of github repository you can find pre-compiled binaries


How to Compile
---------------------

Use CMake 3.1 or higher to prepare solution and project files. You should switch on a Maya_plugins option when doing a cmake configuration.

Installed
 * Maya DevKit 2017 or higher
 * MotionBuilder SDK 2017 or higher
 * FBX Extensions SDK 2017 or higher

Environment variables
 * MAYAPATH - specify your installed maya path, for example, C:\Program Files\Autodesk\Maya2017
 * ADSK_MOBU_2017_64 - motionbuilder path, for example, C:\Program Files\Autodesk\MotionBuilder 2017
 * FBXEXT_PATH - Fbx Extension SDK path, for example,  C:\Program Files\Autodesk\FBX\FBX Extensions SDK\2017.1


Third party used
---------------------
TinyXml

