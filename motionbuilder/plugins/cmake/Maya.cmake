
#
# This is a configuration to prepare project to compile with Maya DevKit

# check for platform configuration
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    message(FATAL_ERROR "Project is designed to be used for Windows x64 build only!")  
endif()

# FBX Extension SDK

if(NOT $ENV{FBXEXT_PATH}x STREQUAL "x")
 	set( FBXEXT_ROOT $ENV{FBXEXT_PATH} )
else()
	set( FBXEXT_ROOT "C:/Program Files/Autodesk/FBX/FBX Extensions SDK/2020.0.1")
endif()

set(FBXEXT_PATH ${FBXEXT_ROOT} CACHE STRING "FBX Extension SDK Location")
set(FBXEXT_PATH ${FBXEXT_ROOT} CACHE PATH "FBX Extension SDK Location")

set(FBX_COMPILER "vs2015")
set(FBX_EXT_LIB FBX_EXT_LIB-NOTFOUND)

find_file(FBX_EXT_LIB "libfbxsdk-adsk.lib"
	PATHS
	"${FBXEXT_PATH}/lib/${FBX_COMPILER}/"
)

if (FBX_EXT_LIB STREQUAL FBX_EXT_LIB-NOTFOUND)
	set(FBX_COMPILER "vs2015")
	find_file(FBX_EXT_LIB "fbxmaya_amd64.lib"
		PATHS
		"${FBXEXT_PATH}/lib/${FBX_COMPILER}/"
	)
endif()

if (FBX_EXT_LIB STREQUAL FBX_EXT_LIB-NOTFOUND)
	set(FBX_COMPILER "vs2012")
	find_file(FBX_EXT_LIB "fbxmaya_amd64.lib"
		PATHS
		"${FBXEXT_PATH}/lib/${FBX_COMPILER}/"
	)
endif()

if (FBX_EXT_LIB STREQUAL FBX_EXT_LIB-NOTFOUND)
    message("FBX SDK library is not found!")  
endif()

# we need env variable to or sdk

if(NOT $ENV{MAYAPATH}x STREQUAL "x")
  set( MAYA_ROOT $ENV{MAYAPATH} )
else()
	set( MAYA_ROOT "C:/Program Files/Autodesk/Maya 2020")
endif()


set(DEVKIT_INCLUDE_PATH ${MAYA_ROOT} CACHE STRING "Maya DevKit Include Location")
set(DEVKIT_LIB_PATH ${MAYA_ROOT} CACHE PATH "Maya DevKit Library Location")


find_path(DEVKIT_INCLUDE_PATH "mglobal.h"
	PATHS
	"${DEVKIT_INCLUDE_PATH}/maya"
)

if( DEVKIT_INCLUDE_PATH )
  set( DEVKIT_FOUND 1 )
  message( STATUS "Found Maya DevKit!" )
else( DEVKIT_INCLUDE_PATH )
  set( DEVKIT_FOUND 0 CACHE STRING "Set to 1 if DevKit is found, 0 otherwise" )
  message( STATUS "Could not find Maya DevKit." )
endif( DEVKIT_INCLUDE_PATH )

# setup libraries
set( OPENMAYAUI_LIBRARY OPENMAYAUI_LIBRARY-NOTFOUND )
find_library( OPENMAYAUI_LIBRARY OpenMayaUI
  PATHS
  "${DEVKIT_LIB_PATH}"
  NO_DEFAULT_PATH
  DOC "The directory where OpenMayaUI.lib resides" )

set( OPENMAYA_LIBRARY OPENMAYA_LIBRARY-NOTFOUND )
find_library( OPENMAYA_LIBRARY OpenMaya
  PATHS
  "${DEVKIT_LIB_PATH}"
  NO_DEFAULT_PATH
  DOC "The directory where OpenMaya.lib resides" )

set( OPENMAYAANIM_LIBRARY OPENMAYAANIM_LIBRARY-NOTFOUND )
find_library( OPENMAYAANIM_LIBRARY OpenMayaAnim
  PATHS
  "${DEVKIT_LIB_PATH}"
  NO_DEFAULT_PATH
  DOC "The directory where OpenMayaAnim.lib resides" )

set( OPENMAYARENDER_LIBRARY OPENMAYARENDER_LIBRARY-NOTFOUND )
find_library( OPENMAYARENDER_LIBRARY OpenMayaRender
  PATHS
  "${DEVKIT_LIB_PATH}"
  NO_DEFAULT_PATH
  DOC "The directory where OpenMayaRender.lib resides" )

set( MAYAIMAGE_LIBRARY MAYAIMAGE_LIBRARY-NOTFOUND )
find_library( MAYAIMAGE_LIBRARY Image
  PATHS
  "${DEVKIT_LIB_PATH}"
  NO_DEFAULT_PATH
  DOC "The directory where Maya's Image.lib resides" )

set( MAYAFOUNDATION_LIBRARY MAYAFOUNDATION_LIBRARY-NOTFOUND )
find_library( MAYAFOUNDATION_LIBRARY Foundation
  PATHS
  "${DEVKIT_LIB_PATH}"
  NO_DEFAULT_PATH
  DOC "The directory where Maya's Foundation.lib resides" )

set(DEVKIT_LIBRARIES
 ${OPENMAYAUI_LIBRARY} 
 ${OPENMAYA_LIBRARY} 
 ${OPENMAYAANIM_LIBRARY}
 ${OPENMAYARENDER_LIBRARY}
 ${MAYAIMAGE_LIBRARY}
 ${MAYAFOUNDATION_LIBRARY}
)

# TODO: append result path into MAYA_PLUG_IN_PATH env
#option(REGISTER_BUILD_PATH "Register project build path in Maya plugins path list")
#if (REGISTER_BUILD_PATH)
#endif()

#########################################################################
# Macro for making maya plugin

macro(ADD_MAYA_FBX_PLUGIN)

	if( NOT ${DEVKIT_FOUND} )
		message(FATAL_ERROR "Count not find Maya DevKit")
	endif()

	set(PluginName ${ARGV0})
	set(MAYA_COMPILE_FLAGS "")

	if(${ARGC} GREATER 1)
		file(GLOB HEADERS "${PluginName}/*.h" "${ARGV1}/*.h")
		file(GLOB SOURCES "${PluginName}/*.cxx" "${ARGV1}/*.cpp")
		include_directories( ${ARGV1} )
		set( MAYA_COMPILE_FLAGS "/D \"FBXSDK_SHARED\" /D \"FBXSDK_NEW_API\"")

		add_library(${PluginName} SHARED ${HEADERS} ${SOURCES})
	else()
		file(GLOB HEADERS "${PluginName}/*.h")
		file(GLOB SOURCES "${PluginName}/*.cxx")
	
		add_library(${PluginName} SHARED ${HEADERS} ${SOURCES})
	endif()

	set_target_properties( ${PluginName}
	    PROPERTIES
	    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result/maya_fbx"
	    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result/maya_fbx"
	    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result/maya_fbx"
	    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result/maya_fbx"
	    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result/maya_fbx"
	    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result/maya_fbx"
	    COMPILE_FLAGS "${MAYA_COMPILE_FLAGS}"
	    PREFIX ""
	)

	# project specified includes
	target_include_directories(${PluginName} PUBLIC "${FBXEXT_PATH}/include")
	target_include_directories(${PluginName} PUBLIC "${DEVKIT_INCLUDE_PATH}")

	# link OpenReality SDK to our project
	target_link_libraries(${PluginName} ${FBX_EXT_LIB} ${DEVKIT_LIBRARIES})

endmacro(ADD_MAYA_FBX_PLUGIN)