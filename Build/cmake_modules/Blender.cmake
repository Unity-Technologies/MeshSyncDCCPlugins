# * BLENDER2.80_PYTHON_VERSION
# * BLENDER2.80_PYTHON_INCLUDE_DIR

include(Utilities)

# ----------------------------------------------------------------------------------------------------------------------

function(get_blender blender_ver)

    #Example Link: https://github.com/blender/blender/archive/v2.82a.zip 
    set(BLENDER_ARCHIVE_FILE "v${blender_ver}.zip")
    set(BLENDER_ARCHIVE_URL "https://github.com/blender/blender/archive/${BLENDER_ARCHIVE_FILE}")
    set(BLENDER_ARCHIVE_LOCAL_PATH "${CMAKE_BINARY_DIR}/Blender-${BLENDER_ARCHIVE_FILE}")

    download_and_extract(
        ${BLENDER_ARCHIVE_URL}
        ${BLENDER_ARCHIVE_LOCAL_PATH}
        "Downloading Blender ${blender_ver}" 
        "Could not download Blender ${blender_ver} !"
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

# Set ${Blender${blender_ver}_FOUND} to TRUE or FALSE, depending on whether the header/libs are found
function(setup_blender blender_ver)

    set(BLENDER_SRC_ROOT "${CMAKE_BINARY_DIR}/Blender-${blender_ver}")

    # The possible root paths of Blender
    list(APPEND BLENDER${blender_ver}_PATHS
        ${BLENDER_SRC_ROOT}/source/blender
    )
        
    # Header. Use find_path to store to cache
    find_path(BLENDER${blender_ver}_INCLUDE_DIR
        NAMES
            BKE_blender_version.h
        HINTS
            ${BLENDER${blender_ver}_PATHS}
        PATH_SUFFIXES
            blenkernel
    )
    mark_as_advanced(BLENDER${blender_ver}_INCLUDE_DIR)
    #
    # # message(${BLENDER${blender_ver}_INCLUDE_DIR})
    #
    # # Libs, and set cache at the end
    # foreach(BLENDER_LIB OpenBlenderAnim OpenBlenderFX OpenBlenderRender OpenBlenderUI OpenBlender Foundation)
    #     find_file(BLENDER${blender_ver}_${BLENDER_LIB}_LIBRARY
    #         NAMES
    #             ${BLENDER_LIB}.lib     # Multithreaded-DLL, Windows
    #             lib${BLENDER_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX}
    #         PATHS
    #             ${BLENDER${blender_ver}_PATHS}
    #         PATH_SUFFIXES
    #             lib
    #             MacOS
    #     )
    #     mark_as_advanced(BLENDER${blender_ver}_${BLENDER_LIB}_LIBRARY)
    #     if(BLENDER${blender_ver}_${BLENDER_LIB}_LIBRARY)
    #         list(APPEND BLENDER${blender_ver}_LIBRARIES ${BLENDER${blender_ver}_${BLENDER_LIB}_LIBRARY})
    #     endif()
    # endforeach()
    # set(BLENDER${blender_ver}_LIBRARIES ${BLENDER${blender_ver}_LIBRARIES} CACHE STRING "Blender ${blender_ver} libraries")
    # mark_as_advanced(BLENDER${blender_ver}_LIBRARIES)
    #

    # CXX Setup
    if(APPLE)
        set(PYTHON_CMAKE_PLATFORM "platform_apple.cmake")
    elseif(WIN32)
        set(PYTHON_CMAKE_PLATFORM "platform_win32_msvc.cmake")
    else()
        set(PYTHON_CMAKE_PLATFORM "FindPythonLibsUnix.cmake")
    endif()
    
    # Find the version of python required
    find_file(BLENDER${blender_ver}_CMAKE_VERSION
        NAMES
            versions.cmake
            ${PYTHON_CMAKE_PLATFORM} # python 3.5
        HINTS
            ${BLENDER_SRC_ROOT}
        PATH_SUFFIXES
            build_files/build_environment/cmake #python 3.7
            build_files/cmake/Modules    #python 3.5
            build_files/cmake/platform   #python 3.5
    )
    
    message(${PYTHON_CMAKE_PLATFORM})
    message(${PYTHON_CMAKE_PLATFORM})
    message(${BLENDER_SRC_ROOT}/build_files/cmake/platform/${PYTHON_CMAKE_PLATFORM})

    message(${BLENDER${blender_ver}_CMAKE_VERSION})
    message(${BLENDER${blender_ver}_CMAKE_VERSION})
    message(${BLENDER${blender_ver}_CMAKE_VERSION})

    # Find Python version. 
    # Line: 
    # - set(PYTHON_VERSION 3.7.0)
    # -	set(PYTHON_VERSION 3.5)

    file(READ ${BLENDER${blender_ver}_CMAKE_VERSION} VERSION_CONTENTS)    
    string(REGEX MATCH "set\\(PYTHON_VERSION ([0-9]+).([0-9]+).?([0-9]*)\\)" _ ${VERSION_CONTENTS})        
    set(BLENDER${blender_ver}_PYTHON_VERSION ${CMAKE_MATCH_1}${CMAKE_MATCH_2} CACHE STRING "Python version used by Blender ${blender_ver}")    
    #message("Python version: ${BLENDER${blender_ver}_PYTHON_VERSION}")
        
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("Blender${blender_ver}"
        DEFAULT_MSG
        BLENDER${blender_ver}_INCLUDE_DIR
        BLENDER${blender_ver}_CMAKE_VERSION
        BLENDER${blender_ver}_PYTHON_VERSION 
#        BLENDER${blender_ver}_LIBRARIES
    )
    message("Blender ${blender_ver} is using Python ${BLENDER${blender_ver}_PYTHON_VERSION}")
    
    if(NOT ${Blender${blender_ver}_FOUND})
        message(FATAL_ERROR "Blender ${blender_ver} could not be detected. Please remove CMakeCache.txt and redo the build process")
    endif()
endfunction()


# set(BLENDER2.80_PYTHON_VERSION 37 CACHE STRING "")
# mark_as_advanced(BLENDER2.80_PYTHON_VERSION)
#
# find_path(BLENDER2.80_PYTHON_INCLUDE_DIR
#     NAMES
#         "Python.h"
#     PATHS
#         "/opt/rh/rh-python37/root/usr/include"
#         "/usr/local/include"
#     PATH_SUFFIXES
#         "python3.7m"
#     NO_DEFAULT_PATH
# )
# mark_as_advanced(BLENDER2.80_PYTHON_INCLUDE_DIR)
#
# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args("Blender"
#     DEFAULT_MSG
#     BLENDER2.80_PYTHON_INCLUDE_DIR
# )
