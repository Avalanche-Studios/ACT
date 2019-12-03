Avalanche Content Toolset (ACT)
========================
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/nfrechette/acl/master/LICENSE)

Please post issues and feature requests to this [github repository issues section](https://github.com/Avalanche-Studios/ACT/issues)

MotionBuilder Toolset
---------------------

  This is set of scripts and plugins, mostly designed to enhance scene management and animation pipeline.

  More detailed information about every script and plugin can be found on github pages or directly in Docs folder.

### Compatibility
  Scripts are compatible with wide range of MotionBuilder versions from 2011-2019
If script has a Qt based UI, then Qt version 4 based tools compatibility is 2014-2018 or for Qt 5 based tools (2019+). Tool can have both versions for Qt4 and Qt5.

  Plugin source code is compatible with MotionBuilder 2017+ versions and has a project files for VS 2017
  Precompiled versions of plugins can be found in releases and can be installed with an installation package.


User Guide
----------

### MotionBuilder Scripts

#### How to use
 Scripts are splitted into actions and startup. Actions are scripts for a single execution. Startup scripts are additional modules that are registering shared functionality or tool. New tools can be found in Main Menu -> Python Tools

#### Manual Installation
  There are several ways to make motionbuilder execute scripts on startup:
Put into “my documents” directory for motionbuilder - as usual it’s <my documents>\MB\<version>\config\PythonStartup
Add a path into an environment variable MOTIONBUILDER_PYTHON_STARTUP. That task could be done with an GUI helper tool ConfigApp, that could be found in github repository - https://github.com/Neill3d/MoBu_ConfigApp 
Put directly into MotionBuilder installation directory - as usual it’s C:\Program Files\Autodesk\MotionBuilder <version>\bin\config\PythonStartup. This way is not preferable

#### Package Installation
 From a releases installation package, the scripts will be automatically copied into a motionbuilder startup folder in “my documents\MB\<version>\config\PythonStartup” directory

#### public_common
  This is a startup python script.
  The script is a set of functions and constants which are project / pipeline specified. Public version of this values are functions that have a general use case, but they could extend much other scripts functionality. Examples of such a common functions: get project path, get plugin path, make a project relative path, get project output path, etc.

#### Scripts documentation

This is detailed information about scripts in the repository.

Actions scripts:
- CopyLayerForOtherTakes
- FixTakesTimeSpan
- PrintAnimatedTakesForSelection
- RenderFromACameraSwitcher

Tools:
- CheckTakeTransitions

License
-------

 Copyright (c) Avalanche Studios. All rights reserved.
 Licensed under the MIT License.