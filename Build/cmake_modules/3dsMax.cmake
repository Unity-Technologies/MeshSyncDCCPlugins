# Set
# - ${3DSMAX${3dsmax_version}_FOUND} to TRUE or FALSE, depending on whether the header/libs are found
# - 3DSMAX${3dsmax_version}_INCLUDE_DIR: the include directory of 3DSMax
function(setup_3dsmax 3dsmax_version)

    # The possible root paths of Maya
    list(APPEND 3DSMAX${3dsmax_version}_PATHS	
        $ENV{ADSK_3DSMAX_SDK_${3dsmax_version}}
        $ENV{ADSK_3DSMAX_SDK_${3dsmax_version}}/samples/modifiers/morpher
    )
    
    # Header. Use find_path to store to cache
    find_path(3DSMAX${3dsmax_version}_INCLUDE_DIR
        NAMES 
            max.h
        HINTS
            ${3DSMAX${3dsmax_version}_PATHS}
        PATH_SUFFIXES 
            include
    )
    mark_as_advanced(3DSMAX${3dsmax_version}_INCLUDE_DIR)
    
    # message(${3DSMAX${3dsmax_version}_INCLUDE_DIR})
    
    # Libs, and set cache at the end
    foreach(3DSMAX_LIB core geom mesh poly mnmath maxutil maxscrpt paramblk2 menus Morpher)
        find_file(3DSMAX${3dsmax_version}_${3DSMAX_LIB}_LIBRARY 
            NAMES 
                ${3DSMAX_LIB}.lib                 
            PATHS
                ${3DSMAX${3dsmax_version}_PATHS}                
            PATH_SUFFIXES 
                lib/x64/Release
                Lib/x64/Release				
        )
				
        mark_as_advanced(3DSMAX${3dsmax_version}_${3DSMAX_LIB}_LIBRARY)
        if(3DSMAX${3dsmax_version}_${3DSMAX_LIB}_LIBRARY)
            list(APPEND 3DSMAX${3dsmax_version}_LIBRARIES ${3DSMAX${3dsmax_version}_${3DSMAX_LIB}_LIBRARY})
        endif()
    endforeach()
    set(3DSMAX${3dsmax_version}_LIBRARIES ${3DSMAX${3dsmax_version}_LIBRARIES} CACHE STRING "Maya ${3dsmax_version} libraries")
    mark_as_advanced(3DSMAX${3dsmax_version}_LIBRARIES)
    
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("3DSMAX${3dsmax_version}"
        DEFAULT_MSG
        3DSMAX${3dsmax_version}_INCLUDE_DIR
        3DSMAX${3dsmax_version}_LIBRARIES
    )    
   
    if(NOT ${3DSMAX${3dsmax_version}_FOUND})  
        message(FATAL_ERROR "Maya ${3dsmax_version} SDK could not be found. Please define 3DSMAX_SDK_${3dsmax_version}. \n"
            "Paths searched for Maya SDK: ${3DSMAX${3dsmax_version}_PATHS}"
        )
    endif()
endfunction()