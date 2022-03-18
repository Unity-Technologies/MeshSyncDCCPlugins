# Returns:
# * ZSTD_INCLUDE_DIR: where zstd.h is found
# * ZSTD_LIBRARY: zstd library path

function(setup_zstd meshsync_plugin_path)

    find_path(ZSTD_INCLUDE_DIR
        NAMES
            "zstd.h"
        PATHS
            ${meshsync_plugin_path}/External/zstd/include
        NO_DEFAULT_PATH                
    )

    mark_as_advanced(ZSTD_INCLUDE_DIR)

    # Decide the name of the zstd lib based on platform
    if(WIN32) 
        set(zstd_lib_filename "libzstd_static.lib")
        set(zstd_lib_path_suffix "win64")
    elseif(APPLE)
        set(zstd_lib_filename "libzstd.a")
        set(zstd_lib_path_suffix "osx/${CMAKE_SYSTEM_PROCESSOR}/")
    elseif(LINUX)
        set(zstd_lib_filename "libzstd.a")
        set(zstd_lib_path_suffix "linux64")
    endif()        


    find_file(
        ZSTD_LIBRARY 
        NAMES
            ${zstd_lib_filename}
        PATHS
            "${meshsync_plugin_path}/External/zstd/lib"
        PATH_SUFFIXES 
            ${zstd_lib_path_suffix}
        NO_DEFAULT_PATH        
    )

    mark_as_advanced(ZSTD_LIBRARY)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("ZSTD"
        DEFAULT_MSG
        ZSTD_INCLUDE_DIR
    	ZSTD_LIBRARY
    )
    
    if(NOT ${ZSTD_FOUND})  
        message(FATAL_ERROR "zstd could not be found. \n"
            "Paths searched for ztsd: ${meshsync_plugin_path}/External/zstd/lib/${zstd_lib_path_suffix}"
        )
    endif()
    
endfunction()
