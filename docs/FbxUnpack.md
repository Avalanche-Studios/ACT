FBX Unpack
=====================

 This is a console application to write down input fbx file information into a json format.
  The tool can output to a console or into a text file. It's also possible to write fbx file thumbnail image into a bmp file.


Command line
---------------------
 The command line arguments are:
 -h 		show help message
 -o FILE  	Output file path. Default is stdout
 -t FILE 	Output thumbnail picture path. Default is empty, no output
 -v 		Verbose output


Binaries
---------------------
In Release section of github repository you can find pre-compiled binaries


How to Compile
---------------------

Use CMake 3.20.3 or higher to prepare solution and project files.

Installed
 * rapidjson (should be downloaded by cmake from git repository)
 * FBX SDK


Third party used
---------------------
Rapid JSON
Autodesk FBX SDK
