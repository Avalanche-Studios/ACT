cmake_minimum_required(VERSION 3.1)
project(ACT 	VERSION 1.0
				DESCRIPTION "Avalanche Content Toolset"
				LANGUAGES CXX)

# Live Bridge Library
include_directories(".")
ADD_LIBRARY( AnimLiveBridgeLib STATIC 
			AnimLiveBridge/AnimLiveBridge.cpp
)

# MoBu Plugins

include("./cmake/MoBu.cmake")
ADD_MOBU_PLUGIN("MarkerWire")
ADD_MOBU_PLUGIN("RelationBoxes")
ADD_MOBU_FBX_PLUGIN("DrivenKeys_MB" "DrivenKeys_Shared")

Add_MOBU_PLUGIN("Device_LiveBridge" "tinyxml")
TARGET_LINK_LIBRARIES("Device_LiveBridge" AnimLiveBridgeLib)

# Maya Plugins

option(MAYA_PLUGINS "Include maya plugins into a solution")
if (MAYA_PLUGINS)
	include("./cmake/Maya.cmake")
	ADD_MAYA_FBX_PLUGIN("DrivenKeys_Maya" "DrivenKeys_Shared")
endif()