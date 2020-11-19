# Requires the following environment var to be setup previously
# - MODO_SDK

string(REGEX MATCH "([0-9.]+)$" modo_version "${CMAKE_CURRENT_SOURCE_DIR}")
set(meshsync_client_modo "MeshSyncClientModo${modo_version}")

include(Modo)
setup_modo(${modo_version})

# Get sources
file(TO_CMAKE_PATH $ENV{MODO_SDK} modo_sdk_path)
file(GLOB sources 
    *.cpp 
    *.h 
    "${modo_sdk_path}/common/*.cpp"
)
list(FILTER sources EXCLUDE REGEX "initialize.cpp") 
list(FILTER sources EXCLUDE REGEX "clean.cpp") 

#DTolerance linker error
list(FILTER sources EXCLUDE REGEX "lxu_geometry_triangulation.cpp") 


add_library(${meshsync_client_modo} SHARED ${sources})
add_dependencies(${meshsync_client_modo} 
    MeshSyncClient MeshUtils MeshSync
)
target_include_directories(${meshsync_client_modo} PUBLIC
    "${CMAKE_SOURCE_DIR}"
    "$ENV{MODO_SDK}/include"
    "${MODO_${modo_version}_QT_INCLUDE_DIRS}"
    "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshSync/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}/MeshUtils/Include"
    "${MESHSYNC_PLUGIN_SRC_ROOT}"
)

# pch doesn't work currently because we are compiling MODO_SDK at the same time, so the dependencies get mixed up.
# target_precompile_headers(${meshsync_client_modo}  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

target_link_libraries(${meshsync_client_modo} MeshSyncClient MeshSync MeshUtils ${Poco_LIBRARIES} ${MODO_${modo_version}_QT_LIBRARIES})

if(ENABLE_ZSTD)
    target_link_libraries(${meshsync_client_modo} ${ZSTD_LIBRARY})
endif()

# if(LINUX)
#     target_link_libraries(${meshsync_client_modo} pthread "-Wl,--no-undefined")
# endif()
set_target_properties(${meshsync_client_modo} PROPERTIES PREFIX "")
set_target_properties(${meshsync_client_modo} PROPERTIES SUFFIX ".lx")
set_target_properties(${meshsync_client_modo} PROPERTIES OUTPUT_NAME "MeshSyncClientModo")

# Install/copying after build
set(dist_dir "${DIST_ROOT}/UnityMeshSync_${MESHSYNC_VER}_Modo_${PLATFORM}/Modo${modo_version}")
install(TARGETS ${meshsync_client_modo} DESTINATION ${dist_dir})

