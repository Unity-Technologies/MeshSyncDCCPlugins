set(MeshSyncClient_dir "${CMAKE_CURRENT_SOURCE_DIR}")


file(GLOB_RECURSE sources *.cpp *.h )
add_library(MeshSyncClient STATIC ${sources})
source_group(TREE ${MeshSyncClient_dir} FILES ${sources})

target_include_directories(MeshSyncClient PRIVATE
    "${MeshSyncClient_dir}"
    "${MeshSyncClient_dir}/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshSync/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshUtils/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}"
)

add_dependencies(MeshSyncClient 
    MeshUtils 
    MeshSync
)
target_precompile_headers(MeshSyncClient  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

target_link_libraries(MeshSyncClient 
    MeshSync 
    MeshUtils 
)



