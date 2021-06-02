AnimLiveBridge API
=====================

 Live Bridge is a high level wrapper for an animation data communication between two processes / threads using
At the moment API supports only a global shared memory communication.


Samples and Tests
--------------------------

There is a AnimLiveBridgeTest.cpp console application which is designed to test different features of the AnimLiveBridge API

AnimLiveBridgeTest.py - is a test python script to run and test different feature of animlivebridge python module
  Python module is build by using swig and is used in maya live bridge test plugin.


Quick start
--------------------------

First of all you need to prepare a pair name - this is a unique key that both server/client will refer to, like an address. In case you want to pair more devices together, you can create additional sessions but with different pair names.

Then you have to create a new session NewLiveSession. If call is succesfull, you will get a session id which internally is a index in a storage array where all needed information is stored (session properties, timeline manager, etc.)

Next step is to open hardware as a server or as a client.

Every device (eigher server or client) are first of all updating local session animation data and then they are making attemp to commit the data (put it into a shared memory or send throw the network)

Brief plan of an API running

Start a server/client
 - create a new session and get a session id
 - define communication properties and open a hardware
 - write down initial data and flush it to start a communication

Update loop
 - look for a recent data
 - modify or do something with a received information (update models)
 - flush - do attemp to write the data into a buffer and send event that data is updated

Close
 - write down a close trigger
 - stop hardware
 - free session and zero session id


TODO
--------------------------

 I need to think how to get a recent data at the beginning of the update. And how to commit data into a shared buffer, even if another remote device didn't trigger the event yet

 - python test for timeline
 - network communication implementation