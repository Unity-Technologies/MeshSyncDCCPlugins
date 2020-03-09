CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(MeshSyncDCCPlugin)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Build/cmake_modules)
# set(src_root ${CMAKE_SOURCE_DIR}/Src)

# prerequisites. 

#CCache variable to hold MeshSync version
SET(MESHSYNC_VER "0.0.1-preview" CACHE STRING "MeshSync package version")

# Get MeshSync
include(MeshSync)

get_meshsync(${MESHSYNC_VER})

