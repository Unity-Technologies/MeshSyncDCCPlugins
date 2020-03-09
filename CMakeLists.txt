CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(MeshSyncDCCPlugin)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Build/cmake_modules)
set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Src)

# prerequisites. 

#CCache variable to hold MeshSync version
SET(MESHSYNC_VER "0.0.1-preview" CACHE STRING "MeshSync package version")

# MeshSync
include(MeshSync)
get_meshsync(${MESHSYNC_VER})
setup_meshsync(${MESHSYNC_VER})

# ----------------------------------------------------------------------------------------------------------------------
# Call add_subdirectory using symbolic link
function(add_subdirectory_link to from)
    if(NOT EXISTS "${SRC_ROOT}/${to}")
        execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMMAND ln -s "${SRC_ROOT}/${from}" "${SRC_ROOT}/${to}")
    endif()
    add_subdirectory("${SRC_ROOT}/${to}")
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

# Maya
if(BUILD_MAYA_ALL)
    option(BUILD_MAYA_2016_PLUGIN "Build MeshSync Plugin for Maya 2016" ON)
    option(BUILD_MAYA_2017_PLUGIN "Build MeshSync Plugin for Maya 2017" ON)
    option(BUILD_MAYA_2018_PLUGIN "Build MeshSync Plugin for Maya 2018" ON)
    option(BUILD_MAYA_2019_PLUGIN "Build MeshSync Plugin for Maya 2019" ON)
    option(BUILD_MAYA_2020_PLUGIN "Build MeshSync Plugin for Maya 2020" ON)
endif()

option(BUILD_MAYA_2016_PLUGIN "Build MeshSync Plugin for Maya 2016" OFF)
option(BUILD_MAYA_2017_PLUGIN "Build MeshSync Plugin for Maya 2017" OFF)
option(BUILD_MAYA_2018_PLUGIN "Build MeshSync Plugin for Maya 2018" OFF)
option(BUILD_MAYA_2019_PLUGIN "Build MeshSync Plugin for Maya 2019" OFF)
option(BUILD_MAYA_2020_PLUGIN "Build MeshSync Plugin for Maya 2020" OFF)

# ----------------------------------------------------------------------------------------------------------------------

if(BUILD_MAYA_2020_PLUGIN)
    add_subdirectory_link(MeshSyncClientMaya2020 MeshSyncClientMaya)
endif()


