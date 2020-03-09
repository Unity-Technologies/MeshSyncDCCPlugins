# Set ${Maya${maya_version}_FOUND} to TRUE or FALSE, depending on whether the header/libs are found
function(setup_maya maya_version)

    # The possible root paths of Maya
    list(APPEND MAYA${maya_version}_PATHS
        "/Applications/Autodesk/maya${maya_version}"
        "/Applications/Autodesk/maya${maya_version}/Maya.app/Contents"
        "/usr/autodesk/maya${maya_version}"
        "/opt/autodesk/maya${maya_version}"        
    )
    
    # Header
    find_path(MAYA${maya_version}_INCLUDE_DIR
        NAMES 
            maya/MGlobal.h
        HINTS
            ${MAYA${maya_version}_PATHS}
        PATH_SUFFIXES 
            include
    )
    mark_as_advanced(MAYA${maya_version}_INCLUDE_DIR)
    
    # message(${MAYA${maya_version}_INCLUDE_DIR})
    
    # Libs
    foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation)
        find_file(MAYA${maya_version}_${MAYA_LIB}_LIBRARY 
            NAMES 
                lib${MAYA_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} 
            PATHS
                ${MAYA${maya_version}_PATHS}                
            PATH_SUFFIXES 
                lib 
                MacOS
        )
        mark_as_advanced(MAYA${maya_version}_${MAYA_LIB}_LIBRARY)
        if(MAYA${maya_version}_${MAYA_LIB}_LIBRARY)
            list(APPEND MAYA${maya_version}_LIBRARIES ${MAYA${maya_version}_${MAYA_LIB}_LIBRARY})
        endif()
    endforeach()
    
    # message(${MAYA${maya_version}_LIBRARIES})

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("Maya${maya_version}"
        DEFAULT_MSG
        MAYA${maya_version}_INCLUDE_DIR
        MAYA${maya_version}_LIBRARIES
    )
    
    if(NOT ${Maya${maya_version}_FOUND})  
        message(FATAL_ERROR "Maya ${maya_version} SDK could not be found. Searched paths: ${MAYA${maya_version}_PATHS}")        
    endif()
endfunction()