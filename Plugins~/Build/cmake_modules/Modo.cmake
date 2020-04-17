# Set the following to cache:
# * ${MODO_${modo_ver}_QT_INCLUDE_DIRS}
# * ${MODO_${modo_ver}_QT_LIBRARIES}
# - ${MODO_${modo_ver}_FOUND} to TRUE or FALSE, depending on whether the header/libs are found
##
# This function also needs the paths to Modo application, and will try to look at the default location automatically.
# If not found, then MODO_APP_${modo_ver} environment variable is required. Examples:
# - MODO_APP_14
# - MODO_APP_13
function(setup_modo modo_ver)

    list(APPEND MODO_APP_SRC_PATHS
        $ENV{MODO_APP_${modo_ver}}        
        "/Applications/Modo${modo_ver}.2v2.app"        
        "/Applications/Modo${modo_ver}.2v1.app"        
        "/Applications/Modo${modo_ver}.0v1.app"        
    )
    
    # Possible paths for linux:
    # set(CMAKE_PREFIX_PATH
    #     "~/Modo13.0v1"
    #     "~/Modo12.2v2"
    # )
    
    
    # Library prefix/suffix
    if(APPLE)
        # Only try to find from Frameworks automatically as the last resort
        SET(CMAKE_FIND_FRAMEWORK LAST)
        
        set(modo_qt_library_prefix "")
        set(modo_qt_library_suffix "")
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(modo_qt_library_prefix ${CMAKE_SHARED_LIBRARY_PREFIX})
        set(modo_qt_library_suffix ${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
    endif()    
    
        
    # Qt include dirs
    find_path(MODO_${modo_ver}_QT_BASE_DIR
        NAMES
            QtCore.framework
        HINTS
            ${MODO_APP_SRC_PATHS}
        PATH_SUFFIXES 
            "Contents/Frameworks"
        NO_DEFAULT_PATH            
    )    
    mark_as_advanced(MODO_${modo_ver}_QT_BASE_DIR)

    set(MODO_${modo_ver}_QT_INCLUDE_DIRS
        "${MODO_${modo_ver}_QT_BASE_DIR}/QtCore.framework/Headers"
        "${MODO_${modo_ver}_QT_BASE_DIR}/QtGui.framework/Headers"
        CACHE PATH "Qt include dirs for Modo ${modo_ver}"
    )

    # Qt libraries
    foreach(QT_LIB QtCore QtGui)
        find_file(MODO_${modo_ver}_${QT_LIB}_LIBRARY 
            NAMES
                ${modo_qt_library_prefix}${QT_LIB}${modo_qt_library_suffix}
            PATHS
                ${MODO_${modo_ver}_QT_BASE_DIR}
            PATH_SUFFIXES
                ${QT_LIB}.framework
            NO_DEFAULT_PATH
        )
                
        mark_as_advanced(MODO_${modo_ver}_${QT_LIB}_LIBRARY)
                        
        if(MODO_${modo_ver}_${QT_LIB}_LIBRARY)
            list(APPEND MODO_${modo_ver}_QT_LIBRARIES ${MODO_${modo_ver}_${QT_LIB}_LIBRARY})
        endif()
    endforeach()

    set(MODO_${modo_ver}_QT_LIBRARIES ${MODO_${modo_ver}_QT_LIBRARIES} CACHE STRING "Modo ${modo_ver} libraries")
        
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("MODO_${modo_ver}"
        DEFAULT_MSG
        MODO_${modo_ver}_QT_BASE_DIR
        MODO_${modo_ver}_QT_INCLUDE_DIRS
        MODO_${modo_ver}_QT_LIBRARIES
    )
    
    if(NOT ${MODO_${modo_ver}_FOUND})
        message(FATAL_ERROR "MODO ${modo_ver} could not be detected. Please remove CMakeCache.txt and redo the build process")
    endif()
endfunction()

