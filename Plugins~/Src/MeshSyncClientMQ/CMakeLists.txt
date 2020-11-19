string(REGEX MATCH "([0-9.]+)$" MQ_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(meshsync_client_metasequoia "MeshSyncClientMQ${MQ_VERSION}")

# Setup Metasequoia SDK
include(Metasequoia)
get_mq(${MQ_VERSION})
setup_mq(${MQ_VERSION})

 
file(GLOB sources *.cpp *.h)  
list(FILTER sources EXCLUDE REGEX "MeshSyncClientMQ3\\.(h|cpp)") 
if(APPLE)
    file(GLOB mm_files *.mm)
    list(APPEND sources ${mm_files})    
endif()

add_library(${meshsync_client_metasequoia} MODULE ${sources})
set_target_properties(${meshsync_client_metasequoia}  PROPERTIES OUTPUT_NAME "MeshSyncClientMQ")

if(APPLE)
    set_target_properties(${meshsync_client_metasequoia} PROPERTIES BUNDLE ON)
    set_target_properties(${meshsync_client_metasequoia} PROPERTIES BUNDLE_EXTENSION "plugin")        
endif()

add_dependencies(${meshsync_client_metasequoia} MeshSyncClient )

target_precompile_headers(${meshsync_client_metasequoia}  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

#ignore pch for the objective-c files
set_source_files_properties(sdk_StringUtil.mm
  PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

target_include_directories(${meshsync_client_metasequoia} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${MQSDK_${MQ_VERSION}_DIR}"
)

target_link_libraries(${meshsync_client_metasequoia} 
    MeshSyncClient 
#    ${COCOA_LIBRARY}
)


# Install/copying after build
set(DIST_DIR "${DIST_ROOT}/UnityMeshSync_${MESHSYNC_VER}_Metasequoia_${PLATFORM}/Metasequoia${MQ_VERSION}")
install(TARGETS ${meshsync_client_metasequoia} DESTINATION ${DIST_DIR})


#set(DIST_DIR "${CMAKE_INSTALL_PREFIX}/UnityMeshSync_Metasequoia_${PLATFORM}/Metasequoia${MQ_VERSION}")
#install(CODE "execute_process(COMMAND mkdir -p \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
#install(CODE "execute_process(COMMAND rm -rf \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
#install(CODE "execute_process(COMMAND cp -r \"${CMAKE_CURRENT_BINARY_DIR}/MeshSyncClientMQ4.bundle\" \"${DIST_DIR}/MeshSyncClientMQ4.plugin\")")

