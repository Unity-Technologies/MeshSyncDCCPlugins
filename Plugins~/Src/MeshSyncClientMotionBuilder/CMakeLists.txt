string(REGEX MATCH "([0-9.]+)$" MOTIONBUILDER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_MOTIONBUILDER "MeshSyncClientMotionBuilder${MOTIONBUILDER_VERSION}")

include(MotionBuilder)
setup_motionbuilder(${MOTIONBUILDER_VERSION})
add_definitions(-DMOTIONBUILDER_VERSION=${MOTIONBUILDER_VERSION})

file(GLOB sources *.cpp *.h)
add_library(${MESHSYNC_CLIENT_MOTIONBUILDER} SHARED ${sources})
add_dependencies(${MESHSYNC_CLIENT_MOTIONBUILDER} MeshSyncClient MeshUtils MeshSync)
target_precompile_headers(${MESHSYNC_CLIENT_MOTIONBUILDER}  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
target_include_directories(${MESHSYNC_CLIENT_MOTIONBUILDER} 
    PUBLIC 
        "${MOTIONBUILDER${MOTIONBUILDER_VERSION}_INCLUDE_DIR}"     
        "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshSync/Include"
        "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshUtils/Include"
        "${MESHSYNC_PLUGIN_SRC_ROOT}"
)

# message(abc: ${MOTIONBUILDER${MOTIONBUILDER_VERSION}_LIBRARIES})
target_link_libraries(${MESHSYNC_CLIENT_MOTIONBUILDER} 
    MeshSyncClient 
    MeshSync 
    MeshUtils 
    ${Poco_LIBRARIES} 
    ${MOTIONBUILDER${MOTIONBUILDER_VERSION}_LIBRARIES}
)



if(ENABLE_ZSTD)
    target_link_libraries(${MESHSYNC_CLIENT_MOTIONBUILDER} ${ZSTD_LIBRARY})
endif()

if(LINUX)
    target_link_libraries(${MESHSYNC_CLIENT_MOTIONBUILDER} pthread "-Wl,--no-undefined")
elseif(MAC)
    set_target_properties(${MESHSYNC_CLIENT_MOTIONBUILDER} PROPERTIES COMPILE_FLAGS -DOSMac_)
    set_target_properties(${MESHSYNC_CLIENT_MOTIONBUILDER} PROPERTIES SUFFIX ".bundle")
elseif(WIN32)   
    set_target_properties(${MESHSYNC_CLIENT_MOTIONBUILDER} PROPERTIES SUFFIX ".dll")
endif()
set_target_properties(${MESHSYNC_CLIENT_MOTIONBUILDER} PROPERTIES PREFIX "")
set_target_properties(${MESHSYNC_CLIENT_MOTIONBUILDER} PROPERTIES OUTPUT_NAME "MeshSyncClientMotionBuilder")

set(DEST_DIR "${DIST_ROOT}/UnityMeshSync_${MESHSYNC_VER}_MotionBuilder_${PLATFORM}")
set(CONTENT_DIR "${DEST_DIR}/MotionBuilder${MOTIONBUILDER_VERSION}")

install(TARGETS ${MESHSYNC_CLIENT_MOTIONBUILDER} DESTINATION "${CONTENT_DIR}/")
