#Find Version
string(REGEX MATCH "([0-9.]+)$" BLENDER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_BLENDER "MeshSyncClientBlender${BLENDER_VERSION}")

# Setup Blender & Python source code
include(Blender)
get_blender(${BLENDER_VERSION})
setup_blender(${BLENDER_VERSION})

set(BLENDER_PYTHON_VERSION ${BLENDER${BLENDER_VERSION}_PYTHON_VERSION})
include(Python)
get_python(${BLENDER_PYTHON_VERSION})
configure_python(${BLENDER_PYTHON_VERSION})

set(BLENDER_PYTHON_VERSION ${BLENDER${BLENDER_VERSION}_PYTHON_VERSION})
string(REPLACE "." "_" BLENDER_MODULE_BLENDER_VERSION ${BLENDER_VERSION})

set(BLENDER_INCLUDE_DIRS
    "${BLENDER${BLENDER_VERSION}_INCLUDE_DIR}/blenkernel"
    "${BLENDER${BLENDER_VERSION}_INCLUDE_DIR}/blenlib"
    "${BLENDER${BLENDER_VERSION}_INCLUDE_DIR}/bmesh"
    "${BLENDER${BLENDER_VERSION}_INCLUDE_DIR}/makesdna"
    "${BLENDER${BLENDER_VERSION}_INCLUDE_DIR}/makesrna"
    "${BLENDER${BLENDER_VERSION}_INCLUDE_DIR}/python"
    ${PYTHON_${BLENDER_PYTHON_VERSION}_INCLUDE_DIRS}
)

file(GLOB_RECURSE sources *.cpp *.h *.c)
add_library(${MESHSYNC_CLIENT_BLENDER}  SHARED ${sources})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_dependencies(${MESHSYNC_CLIENT_BLENDER} MeshSyncClient )

target_precompile_headers(${MESHSYNC_CLIENT_BLENDER}  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
target_include_directories(${MESHSYNC_CLIENT_BLENDER} PRIVATE
    "${EXTERNAL_ROOT}"
    "${BLENDER_INCLUDE_DIRS}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshSync/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshUtils/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}"
    "${SRC_ROOT}/MeshSyncClient/Include"

)
target_link_libraries(${MESHSYNC_CLIENT_BLENDER} 
    PRIVATE
        MeshSyncClient
        ${Poco_LIBRARIES}
        ${PYTHON_${BLENDER_PYTHON_VERSION}_LIBRARY}    
)
  
if(ENABLE_ZSTD)
    target_link_libraries(${MESHSYNC_CLIENT_BLENDER} PRIVATE ${ZSTD_LIBRARY})
endif()
if(LINUX)
    target_link_libraries(${MESHSYNC_CLIENT_BLENDER} PRIVATE pthread)
    set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES SUFFIX ".cpython-${BLENDER_PYTHON_VERSION}m-x86_64-linux-gnu.so")
elseif(MAC)
    target_link_libraries(${MESHSYNC_CLIENT_BLENDER} "-undefined dynamic_lookup")    
    set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES SUFFIX ".cpython-${BLENDER_PYTHON_VERSION}m-darwin.so")
elseif(WIN32)
    set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES SUFFIX ".pyd")    
endif()
set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES PREFIX "")
set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES OUTPUT_NAME "MeshSyncClientBlender")

# Install/copying after build
set(DIST_DIR "${DIST_ROOT}/UnityMeshSync_${MESHSYNC_VER}_Blender_${PLATFORM}/blender-${BLENDER_VERSION}")
set(PYD_DIR "${DIST_DIR}/MeshSyncClientBlender")
install(FILES
    python/unity_mesh_sync_common.py
    python/${BLENDER_VERSION}/unity_mesh_sync.py
    DESTINATION ${DIST_DIR})
install(FILES python/__init__.py DESTINATION ${PYD_DIR})
install(TARGETS ${MESHSYNC_CLIENT_BLENDER} DESTINATION ${PYD_DIR})

