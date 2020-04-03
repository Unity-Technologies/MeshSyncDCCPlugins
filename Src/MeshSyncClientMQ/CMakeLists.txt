string(REGEX MATCH "([0-9.]+)$" MQ_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_METASEQUOIA "MeshSyncClientMQ${MQ_VERSION}")

# Setup Metasequoia SDK
include(Metasequoia)
get_mq(${MQ_VERSION})
setup_mq(${MQ_VERSION})


file(GLOB sources *.cpp *.h)  
list(FILTER sources EXCLUDE REGEX "MeshSyncClientMQ3\\.(h|cpp)") 
if(APPLE)
    file(GLOB sources *.mm)
endif()


add_library(${MESHSYNC_CLIENT_METASEQUOIA} MODULE ${sources})
#set_target_properties(${MESHSYNC_CLIENT_METASEQUOIA} PROPERTIES OUTPUT_NAME "MeshSyncClientMQ4")

if(APPLE)
    set_target_properties(${MESHSYNC_CLIENT_METASEQUOIA} PROPERTIES BUNDLE ON)
    set_target_properties(${MESHSYNC_CLIENT_METASEQUOIA} PROPERTIES BUNDLE_EXTENSION "plugin")        
endif()

add_dependencies(${MESHSYNC_CLIENT_METASEQUOIA} MeshUtils MeshSync)

target_precompile_headers(${MESHSYNC_CLIENT_METASEQUOIA}  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

#ignore pch for the objective-c files
set_source_files_properties(sdk_StringUtil.mm
  PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

target_include_directories(${MESHSYNC_CLIENT_METASEQUOIA} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${MQSDK_${MQ_VERSION}_DIR}"
    ${MESHSYNC_PLUGIN_SRC_ROOT}    
)

target_link_libraries(${MESHSYNC_CLIENT_METASEQUOIA} 
    MeshSync MeshUtils ${Poco_LIBRARIES} 
    ${COCOA_LIBRARY}
)

if(ENABLE_ZSTD)
    target_link_libraries(${MESHSYNC_CLIENT_METASEQUOIA} ${ZSTD_LIBRARY})
endif()



# Zip
# [TODO-sin: 2020-4-3] MinSizeRel should be replaced with ${CMAKE_BUILD_TYPE}
set(ZIP_FILE "UnityMeshSync_${MESHSYNC_VER}_Metasequoia${MQ_VERSION}_${PLATFORM}.zip")
set(ZIP_FULL_PATH "${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel/${ZIP_FILE}")
file(REMOVE ${ZIP_FULL_PATH})
add_custom_target(CREATE_ZIP_${MESHSYNC_CLIENT_METASEQUOIA} COMMAND
    ${CMAKE_COMMAND} -E tar "cfv" "${ZIP_FILE}" --format=zip
       "${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel/*"
WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel"
DEPENDS
    ${MESHSYNC_CLIENT_METASEQUOIA}
)

# Install/copying after build
#set(DIST_DIR "${DIST_ROOT}/UnityMeshSync_${MESHSYNC_VER}_Metasequoia_${PLATFORM}/Metasequoia${MQ_VERSION}")
#install(TARGETS ${MESHSYNC_CLIENT_METASEQUOIA} DESTINATION ${DIST_DIR})

install(FILES "${ZIP_FULL_PATH}" DESTINATION ${DIST_ROOT})

#install(TARGETS CREATE_ZIP_${MESHSYNC_CLIENT_METASEQUOIA} DESTINATION ${DIST_ROOT})   
#set(DIST_DIR "${CMAKE_INSTALL_PREFIX}/UnityMeshSync_Metasequoia_${PLATFORM}/Metasequoia${MQ_VERSION}")
#install(CODE "execute_process(COMMAND mkdir -p \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
#install(CODE "execute_process(COMMAND rm -rf \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
#install(CODE "execute_process(COMMAND cp -r \"${CMAKE_CURRENT_BINARY_DIR}/MeshSyncClientMQ4.bundle\" \"${DIST_DIR}/MeshSyncClientMQ4.plugin\")")
