
# Setup Modo on Windows. Sets the following to cache:
# * ${MODO_${modo_ver}_QT_INCLUDE_DIRS}
# * ${MODO_${modo_ver}_QT_LIBRARIES}
function(setup_modo_win modo_ver)

    # Qt include dirs
    file(TO_CMAKE_PATH $ENV{QT_4_8_SDK} qt_sdk_path)
    set(MODO_${modo_ver}_QT_INCLUDE_DIRS
        "${qt_sdk_path}/include"
        "${qt_sdk_path}/include/QtCore"
        "${qt_sdk_path}/include/QtGui"
        CACHE PATH "Qt include dirs for Modo ${modo_ver}"
    )
        
    #Qt libraries
    list(APPEND qt_libraries_list QtCore4 QtGui4)     
    foreach(QT_LIB IN LISTS qt_libraries_list)
        find_file(MODO_${modo_ver}_${QT_LIB}_LIBRARY 
            NAMES
                ${QT_LIB}.lib
            PATHS
                ${qt_sdk_path}
            PATH_SUFFIXES
                lib
            NO_DEFAULT_PATH
        )                
        mark_as_advanced(MODO_${modo_ver}_${QT_LIB}_LIBRARY)
                        
        if(MODO_${modo_ver}_${QT_LIB}_LIBRARY)
            list(APPEND MODO_${modo_ver}_QT_LIBRARIES ${MODO_${modo_ver}_${QT_LIB}_LIBRARY})
        endif()
    endforeach()

    set(MODO_${modo_ver}_QT_LIBRARIES ${MODO_${modo_ver}_QT_LIBRARIES} CACHE STRING "Modo ${modo_ver} libraries")
endfunction()

#-----------------------------------------------------------------------------------------------------------------------
# Setup Modo on Linux. Sets the following to cache:
# * ${MODO_${modo_ver}_QT_INCLUDE_DIRS}
# * ${MODO_${modo_ver}_QT_LIBRARIES}
function(setup_modo_linux modo_ver)

    # Qt include dirs
    file(TO_CMAKE_PATH $ENV{QT_4_8_SDK} qt_sdk_path)
    set(MODO_${modo_ver}_QT_INCLUDE_DIRS
        "${qt_sdk_path}/include"
        "${qt_sdk_path}/include/QtCore"
        "${qt_sdk_path}/include/QtGui"
        CACHE PATH "Qt include dirs for Modo ${modo_ver}"
    )
        
    #Qt libraries
    list(APPEND qt_libraries_list QtCore QtGui)            
        
    # Qt libraries. Use shared libraries inside Modo app
    foreach(QT_LIB IN LISTS qt_libraries_list)

        find_file(MODO_${modo_ver}_${QT_LIB}_LIBRARY 
            NAMES
                ${CMAKE_SHARED_LIBRARY_PREFIX}${QT_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX}
            PATHS
                $ENV{MODO_APP_${modo_ver}}        
                "~/Modo${modo_ver}.2v2"
                "~/Modo${modo_ver}.2v1"
                "~/Modo${modo_ver}.0v1"        
            PATH_SUFFIXES
            NO_DEFAULT_PATH
        )
                        
        mark_as_advanced(MODO_${modo_ver}_${QT_LIB}_LIBRARY)
                        
        if(MODO_${modo_ver}_${QT_LIB}_LIBRARY)
            list(APPEND MODO_${modo_ver}_QT_LIBRARIES ${MODO_${modo_ver}_${QT_LIB}_LIBRARY})
        endif()
    endforeach()


    set(MODO_${modo_ver}_QT_LIBRARIES ${MODO_${modo_ver}_QT_LIBRARIES} CACHE STRING "Modo ${modo_ver} libraries")
endfunction()

#-----------------------------------------------------------------------------------------------------------------------

# Set the following to cache:
# * ${MODO_${modo_ver}_QT_INCLUDE_DIRS}
# * ${MODO_${modo_ver}_QT_LIBRARIES}
#
# This function also needs the paths to Modo application, and will try to look at the default location automatically.
# If not found, then MODO_APP_${modo_ver} environment variable is required. Examples:
# - MODO_APP_14
# - MODO_APP_13
function(setup_modo_mac modo_ver)
    list(APPEND MODO_APP_SRC_PATHS
        $ENV{MODO_APP_${modo_ver}}        
        "/Applications/Modo${modo_ver}.2v2.app"        
        "/Applications/Modo${modo_ver}.2v1.app"        
        "/Applications/Modo${modo_ver}.0v1.app"
    )
    
    
    # Library prefix/suffix and include dirs
    # Only try to find from Frameworks automatically as the last resort
    SET(CMAKE_FIND_FRAMEWORK LAST)
    
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

    list(APPEND qt_libraries_list QtCore QtGui)            
        
    # Qt libraries
    foreach(QT_LIB IN LISTS qt_libraries_list)
        find_file(MODO_${modo_ver}_${QT_LIB}_LIBRARY 
            NAMES
                ${QT_LIB}
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
        
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

# Set the following to cache:
# * ${MODO_${modo_ver}_QT_INCLUDE_DIRS}
# * ${MODO_${modo_ver}_QT_LIBRARIES}
# - ${MODO_${modo_ver}_FOUND} to TRUE or FALSE, depending on whether the header/libs are found
function(setup_modo modo_ver)
    
    include(FindPackageHandleStandardArgs)
    if(APPLE)
        setup_modo_mac(${modo_ver})
    elseif(LINUX)
        setup_modo_linux(${modo_ver})
    else()
        setup_modo_win(${modo_ver})    
    endif()    

    find_package_handle_standard_args("MODO_${modo_ver}"
        DEFAULT_MSG
        MODO_${modo_ver}_QT_INCLUDE_DIRS
        MODO_${modo_ver}_QT_LIBRARIES
    )
    
    if(NOT ${MODO_${modo_ver}_FOUND})
        message(FATAL_ERROR "MODO ${modo_ver} could not be detected. Please remove CMakeCache.txt and redo the build process")
    endif()
endfunction()

