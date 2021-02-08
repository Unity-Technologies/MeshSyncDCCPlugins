set(MeshSyncClient_dir "${CMAKE_CURRENT_SOURCE_DIR}")


file(GLOB_RECURSE sources *.cpp *.h )
add_library(MeshSyncClient STATIC ${sources})
source_group(TREE ${MeshSyncClient_dir} FILES ${sources})

target_include_directories(MeshSyncClient 
PUBLIC
    "${MeshSyncClient_dir}/Include"
PRIVATE
    "${MeshSyncClient_dir}"
    "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshSync/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshUtils/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}"
    "${Poco_INCLUDE_DIRS}"

)

add_dependencies(MeshSyncClient 
    MeshSync
)
target_precompile_headers(MeshSyncClient  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

target_link_libraries(MeshSyncClient 
PUBLIC
    MeshSync 
)




