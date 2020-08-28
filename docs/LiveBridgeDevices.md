Live Bridge Devices
=====================

Server / Client device plugins for Autodesk MotionBuilder.
Designed to sync models animation between application instances.
That could help, for example, to animate face on a face rig in one motionbuilder instance
and have a live preview in a main scene with body animation in another motionbuilder instance

[![LiveBridgePreview](https://github.com/Avalanche-Studios/ACT/blob/master/docs/images/LiveBridge.gif)]()

Animation Live Bridge Library
---------------------
There is a library with common structures and functions to handle server/client pairing and data sharing.
That is designed to be used not only in motionbuilder, but in different other application
and with possible python connection as well with help of ctypes and being compiled as dynamic library.


How To Use
---------------------
Note! You should open every MotionBuilder instance with administrative rights!

On Server side (first MotionBuilder instance)
1) Create a server device
2) Prepare mapping (export selected hierarchy into xml and load it back)
3) Map device to objects in scene and make it live

On Client Side (second MotionBuilder instance)
1) Create a client device
2) Load prepared maping (load from xml)
3) Map device to a hierarchy of models in a scene and make it live

Features
---------------------
- custom joint mapping (export to xml, import from xml)
- linked timeline between server/client
- lookAt point handle on client to drive animation on server
- support for custom pair names

Third party used
---------------------
TinyXml