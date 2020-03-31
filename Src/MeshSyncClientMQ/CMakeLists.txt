string(REGEX MATCH "([0-9.]+)$" MQ_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_METASEQUOIA "MeshSyncClientMQ${MQ_VERSION}")

# Setup Metasequoia SDK
include(Metasequoia)
get_mq(${MQ_VERSION})
setup_mq(${MQ_VERSION})


file(GLOB sources *.cpp *.h *.mm)

  
list(FILTER sources EXCLUDE REGEX "MeshSyncClientMQ3\\.(h|cpp)") 

add_library(${MESHSYNC_CLIENT_METASEQUOIA} MODULE ${sources})
#set_target_properties(${MESHSYNC_CLIENT_METASEQUOIA} PROPERTIES BUNDLE ON)
#set_target_properties(${MESHSYNC_CLIENT_METASEQUOIA} PROPERTIES OUTPUT_NAME "MeshSyncClientMQ4")

if(APPLE)
    set_target_properties(${name} PROPERTIES BUNDLE ON)
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


# Install/copying after build
set(DIST_DIR "${DIST_ROOT}/UnityMeshSync_${MESHSYNC_VER}_Metasequoia_${PLATFORM}/Metasequoia${MQ_VERSION}")
install(TARGETS ${MESHSYNC_CLIENT_METASEQUOIA} DESTINATION ${DIST_DIR})


#set(DIST_DIR "${CMAKE_INSTALL_PREFIX}/UnityMeshSync_Metasequoia_${PLATFORM}/Metasequoia${MQ_VERSION}")
#install(CODE "execute_process(COMMAND mkdir -p \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
#install(CODE "execute_process(COMMAND rm -rf \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
#install(CODE "execute_process(COMMAND cp -r \"${CMAKE_CURRENT_BINARY_DIR}/MeshSyncClientMQ4.bundle\" \"${DIST_DIR}/MeshSyncClientMQ4.plugin\")")
