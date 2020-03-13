# Find version
string(REGEX MATCH "([LT0-9.]+)$" MAYA_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
string(REGEX MATCH "([0-9.]+)$" MAYA_YEAR ${MAYA_VERSION})
string(REGEX MATCH "LT" MAYA_LT ${MAYA_VERSION})
set(MESHSYNC_CLIENT_MAYA "MeshSyncClientMAYA_${MAYA_VERSION}")

set(MESHSYNC_PLUGIN_MAYA_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include(Maya)
setup_maya(${MAYA_VERSION})

file(GLOB sources *.cpp *.h)
add_library(${MESHSYNC_CLIENT_MAYA} SHARED ${sources})
add_dependencies(${MESHSYNC_CLIENT_MAYA} MeshUtils MeshSync)

#Include, pch
target_precompile_headers(${MESHSYNC_CLIENT_MAYA}  PRIVATE "${MESHSYNC_PLUGIN_MAYA_DIR}/pch.h")
target_include_directories(${MESHSYNC_CLIENT_MAYA} PRIVATE 
    "${CMAKE_SOURCE_DIR}" 
    "${MAYA${MAYA_VERSION}_INCLUDE_DIR}"
    "${MESHSYNC_PLUGIN_SRC_ROOT}"
)

target_link_libraries(${MESHSYNC_CLIENT_MAYA} 
    MeshSync MeshUtils 
    ${Poco_LIBRARIES} 
    ${MAYA${MAYA_VERSION}_LIBRARIES}
)

if(ENABLE_ZSTD)
    target_link_libraries(${MESHSYNC_CLIENT_MAYA} ${ZSTD_LIBRARY})
endif()

if(${MAYA_VERSION} MATCHES "LT")
    add_definitions(-DMAYA_LT=1)
else()
    add_definitions(-DMAYA_LT=0)
endif()

if(LINUX)
    target_link_libraries(${MESHSYNC_CLIENT_MAYA} pthread "-Wl,--no-undefined")
    set(DEST_DIR "${DIST_ROOT}/UnityMeshSync_Maya_Linux/modules")
elseif(MAC)
    set_target_properties(${MESHSYNC_CLIENT_MAYA} PROPERTIES COMPILE_FLAGS -DOSMac_)
    set_target_properties(${MESHSYNC_CLIENT_MAYA} PROPERTIES SUFFIX ".bundle")
    set(DEST_DIR "${DIST_ROOT}/UnityMeshSync_Maya_Mac")
elseif(WIN32)
    set_target_properties(${MESHSYNC_CLIENT_MAYA} PROPERTIES SUFFIX ".mll")
    set(DEST_DIR "${DIST_ROOT}/UnityMeshSync_Maya_Windows/modules")
endif()

set(MESHSYNC_CLIENT_MAYA_OUTPUT_NAME "MeshSyncClientMaya")
set_target_properties(${MESHSYNC_CLIENT_MAYA} PROPERTIES PREFIX "")
set_target_properties(${MESHSYNC_CLIENT_MAYA} PROPERTIES OUTPUT_NAME ${MESHSYNC_CLIENT_MAYA_OUTPUT_NAME})

set(MEL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(MEL_DEST_DIR "${DEST_DIR}/UnityMeshSync/${MAYA_YEAR}${MAYA_LT}/scripts")
set(MLL_DEST_DIR "${DEST_DIR}/UnityMeshSync/${MAYA_YEAR}${MAYA_LT}/plug-ins")

# Folder structure
# modules
# |- UnityMeshSync.mod
# └─ UnityMeshSync
#       |- 2019
#       |   |- plug-ins 
#       |   |     └─ MeshSyncClientMaya.mll
#       |   |- scripts
#       |   |     └─ MEL scripts
#       |- 2020
#       |   |- plug-ins 
#       |   |     └─ MeshSyncClientMaya.mll
#       |   |- scripts
#       |   |     └─ MEL scripts

# Prepare the directory and MEL scripts
#file(MAKE_DIRECTORY ${MEL_DEST_DIR})
#file(MAKE_DIRECTORY ${MLL_DEST_DIR})

install(DIRECTORY scripts/ DESTINATION "${MEL_DEST_DIR}" )
install(DIRECTORY etc/ DESTINATION "${DEST_DIR}/" FILES_MATCHING PATTERN "*.mod")
install(TARGETS ${MESHSYNC_CLIENT_MAYA} DESTINATION "${MLL_DEST_DIR}")




