cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)

project(ACT 	VERSION 1.0
				DESCRIPTION "Avalanche Content Toolset"
				LANGUAGES C CXX)


include_directories(".")
add_library(AnimLiveBridgeAPI SHARED AnimLiveBridge/AnimLiveBridge.cpp) 
    
#this says where to put the generated .dll relative to the directory "cmake" is executed from
set_target_properties(AnimLiveBridgeAPI PROPERTIES LIBRARY_OUTPUT_DIRECTORY "Result/.") 
    
  
#SWIG stuff:

find_package(SWIG REQUIRED COMPONENTS python)
    
#load the package that SWIG uses to generate Python

find_package(Python2 REQUIRED COMPONENTS Interpreter Development)
include(UseSWIG)
    
#point to python headers
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories("../AnimLiveBridge/.")
include_directories("C:/Avalanche/Python-2.7.3/Win64/include")

set_property(SOURCE AnimLiveBridge/AnimLiveBridge.i PROPERTY CPLUSPLUS ON)

#tell SWIG to create a new module, called AnimLiveBridge, 
#in Python and point to the SWIG interface file (the .i file)
SWIG_ADD_LIBRARY(AnimLiveBridge LANGUAGE python SOURCES AnimLiveBridge/AnimLiveBridge.i)
    
#link the above module to the API (the shared object) we just created
swig_link_libraries(AnimLiveBridge AnimLiveBridgeAPI)
    
#also link the above module to Python
swig_link_libraries(AnimLiveBridge ${PYTHON_LIBRARIES} )

set_property(
  TARGET AnimLiveBridge
  PROPERTY SWIG_INCLUDE_DIRECTORIES
    ${AnimLiveBridge-SWIG_INCLUDE_DIRS}
    ${AnimLiveBridge_INCLUDE_DIRS}
)

# MoBu Plugins

include("./cmake/MoBu.cmake")
ADD_MOBU_PLUGIN("MarkerWire")
ADD_MOBU_PLUGIN("RelationBoxes")
ADD_MOBU_FBX_PLUGIN("DrivenKeys_MB" "DrivenKeys_Shared")

ADD_MOBU_PLUGIN("Device_LiveBridge" "tinyxml")
TARGET_LINK_LIBRARIES( "Device_LiveBridge" AnimLiveBridgeAPI )

# Maya Plugins

option(MAYA_PLUGINS "Include maya plugins into a solution")
if (MAYA_PLUGINS)
	include("./cmake/Maya.cmake")
	ADD_MAYA_FBX_PLUGIN("DrivenKeys_Maya" "DrivenKeys_Shared")
endif()

