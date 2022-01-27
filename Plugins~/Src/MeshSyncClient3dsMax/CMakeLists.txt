# Find version
string(REGEX MATCH "([0-9.]+)$" 3DSMAX_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_3DSMAX "MeshSyncClient3DSMAX_${3DSMAX_VERSION}")

set(MESHSYNC_PLUGIN_3DSMAX_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include(3dsMax)
setup_3dsmax(${3DSMAX_VERSION})

file(GLOB sources *.cpp *.h *.rc)
set_source_files_properties(*.rc PROPERTIES LANGUAGE RC)

add_library(${MESHSYNC_CLIENT_3DSMAX} SHARED ${sources})
add_dependencies(${MESHSYNC_CLIENT_3DSMAX} MeshSyncClient )

#Include, pch
target_precompile_headers(${MESHSYNC_CLIENT_3DSMAX}  PRIVATE "${MESHSYNC_PLUGIN_3DSMAX_DIR}/pch.h")
target_include_directories(${MESHSYNC_CLIENT_3DSMAX} PRIVATE 
    "${CMAKE_SOURCE_DIR}" 
    "${3DSMAX${3DSMAX_VERSION}_INCLUDE_DIR}"
    "${3DSMAX${3DSMAX_VERSION}_INCLUDE_DIR}/../samples"
)

target_link_libraries(${MESHSYNC_CLIENT_3DSMAX} 
    PRIVATE
        MeshSyncClient 
        ${3DSMAX${3DSMAX_VERSION}_LIBRARIES}
)

# Installation
set(DEST_DIR "${DIST_ROOT}/UnityMeshSync_${UNITY_PACKAGE_VERSION}_3DSMAX_${PLATFORM}")
set_target_properties(${MESHSYNC_CLIENT_3DSMAX} PROPERTIES SUFFIX ".dlu")

set(MESHSYNC_CLIENT_3DSMAX_OUTPUT_NAME "MeshSyncClient3dsMax")
set_target_properties(${MESHSYNC_CLIENT_3DSMAX} PROPERTIES PREFIX "")
set_target_properties(${MESHSYNC_CLIENT_3DSMAX} PROPERTIES OUTPUT_NAME ${MESHSYNC_CLIENT_3DSMAX_OUTPUT_NAME})

set(DLU_DEST_DIR "${DEST_DIR}/3dsMax${3DSMAX_VERSION}")

# Folder structure
# modules
# |- 3dsMax2017
# |     └─ MeshSyncClient3dsMax.dlu
# |- 3dsMax2018
# |     └─ MeshSyncClient3dsMax.dlu
# |- 3dsMax2019
# |     └─ MeshSyncClient3dsMax.dlu


# copy files to destination, as part of the install process.
install(TARGETS ${MESHSYNC_CLIENT_3DSMAX}  DESTINATION "${DLU_DEST_DIR}")




