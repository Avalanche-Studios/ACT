
#
# This is a configuration to prepare project to compile with OpenReality SDK (MotionBuilder)

# check for platform configuration
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    message(FATAL_ERROR "Project is designed to be used for Windows x64 build only!")  
endif()

# we need env variable to or sdk

if(NOT $ENV{ADSK_MOBU_2017_64}x STREQUAL "x")
  set( MOBU_ROOT $ENV{ADSK_MOBU_2017_64} )
else()
	set( MOBU_ROOT "C:/Program Files/Autodesk/MotionBuilder 2017")
endif()


set(ORSDK_PATH ${MOBU_ROOT}/OpenRealitySDK CACHE STRING "MotionBuilder OR SDK Location")
set(ORSDK_PATH ${MOBU_ROOT}/OpenRealitySDK CACHE PATH "MotionBuilder OR SDK Location")


find_path(ORSDK_PATH "fbxsdk.h"
	PATHS
	"${ORSDK_PATH}/include"
)

if( ORSDK_PATH )
  set( ORSDK_FOUND 1 )
  message( STATUS "Found OpenRealitySDK!" )
else( ORSDK_PATH )
  set( ORSDK_FOUND 0 CACHE STRING "Set to 1 if ORSDK is found, 0 otherwise" )
  message( STATUS "Could not find OpenRealitySDK." )
endif( ORSDK_PATH )

# setup libraries
set( ORFB_LIBRARY ORFB_LIBRARY-NOTFOUND )
find_library( ORFB_LIBRARY fbsdk
  PATHS
  "${ORSDK_PATH}/lib/x64"
  NO_DEFAULT_PATH
  DOC "The directory where MotionBuilder fbsdk.lib resides" )

set( ORFBX_LIBRARY ORFBX_LIBRARY-NOTFOUND )
# back compatibility with old fbx sdk library name
find_library( ORFBX_LIBRARY fbxsdk
		PATHS
		"${ORSDK_PATH}/lib/x64"
		NO_DEFAULT_PATH
		DOC "The directory where MotionBuilder fbxsdk.lib resides" )

if (ORFBX_LIBRARY STREQUAL ORFBX_LIBRARY-NOTFOUND)
	find_library( ORFBX_LIBRARY libfbxsdk-adsk
  		PATHS
  		"${ORSDK_PATH}/lib/x64"
  		NO_DEFAULT_PATH
  		DOC "The directory where MotionBuilder libfbxsdk-adsk.lib resides" )	
endif()

set(ORSDK_LIBRARIES
 ${ORFB_LIBRARY} 
 ${ORFBX_LIBRARY} 
)


# TODO: append result path into MOTIONBUILDER_PLUGIN_PATH env
#option(REGISTER_BUILD_PATH "Register project build path in MoBu plugins path list")
#if (REGISTER_BUILD_PATH)
#endif()

#########################################################################
# Macro for making mobu plugin

macro(ADD_MOBU_PLUGIN)

	if( NOT ${ORSDK_FOUND} )
		message(FATAL_ERROR "Count not find OpenReality SDK")
	endif()

	set(PluginName ${ARGV0})
	set(MOBU_COMPILE_FLAGS "")

	if(${ARGC} GREATER 1)
		file(GLOB HEADERS "${PluginName}/*.h" "${ARGV1}/*.h")
		file(GLOB SOURCES "${PluginName}/*.cxx" "${ARGV1}/*.cpp")
		include_directories( ${ARGV1} )
		set( MOBU_COMPILE_FLAGS "/D \"FBXSDK_SHARED\" /D \"FBXSDK_NEW_API\"")

		add_library(${PluginName} SHARED ${HEADERS} ${SOURCES})
	else()
		file(GLOB HEADERS "${PluginName}/*.h")
		file(GLOB SOURCES "${PluginName}/*.cxx")
	
		add_library(${PluginName} SHARED ${HEADERS} ${SOURCES})
	endif()

	set_target_properties( ${PluginName}
	    PROPERTIES
	    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result"
	    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result"
	    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result"
	    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result"
	    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result"
	    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result"
	    COMPILE_FLAGS "${MOBU_COMPILE_FLAGS}"
	    PREFIX ""
	)

	# OR SDK headers
	target_include_directories(${PluginName} PUBLIC "${ORSDK_PATH}/include")

	# link OpenReality SDK to our project
	target_link_libraries(${PluginName} ${ORSDK_LIBRARIES} "opengl32.lib")

endmacro(ADD_MOBU_PLUGIN)

# this is a version of plugin for fbx extension

macro(ADD_MOBU_FBX_PLUGIN)

	if( NOT ${ORSDK_FOUND} )
		message(FATAL_ERROR "Count not find OpenReality SDK")
	endif()

	set(PluginName ${ARGV0})
	set(MOBU_COMPILE_FLAGS "")

	if(${ARGC} GREATER 1)
		file(GLOB HEADERS "${PluginName}/*.h" "${ARGV1}/*.h")
		file(GLOB SOURCES "${PluginName}/*.cxx" "${ARGV1}/*.cpp")
		include_directories( ${ARGV1} )
		set( MOBU_COMPILE_FLAGS "/D \"FBXSDK_SHARED\" /D \"FBXSDK_NEW_API\"")

		add_library(${PluginName} SHARED ${HEADERS} ${SOURCES})
	else()
		file(GLOB HEADERS "${PluginName}/*.h")
		file(GLOB SOURCES "${PluginName}/*.cxx")
	
		add_library(${PluginName} SHARED ${HEADERS} ${SOURCES})
	endif()

	set_target_properties( ${PluginName}
	    PROPERTIES
	    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result/mobu_fbx"
	    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result/mobu_fbx"
	    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/result/mobu_fbx"
	    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result/mobu_fbx"
	    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result/mobu_fbx"
	    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/result/mobu_fbx"
	    COMPILE_FLAGS "${MOBU_COMPILE_FLAGS}"
	    PREFIX ""
	)

	# OR SDK headers
	target_include_directories(${PluginName} PUBLIC "${ORSDK_PATH}/include")

	# link OpenReality SDK to our project
	target_link_libraries(${PluginName} ${ORSDK_LIBRARIES} "opengl32.lib")

endmacro(ADD_MOBU_FBX_PLUGIN)