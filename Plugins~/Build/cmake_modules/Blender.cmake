
include(Utilities)

# ----------------------------------------------------------------------------------------------------------------------

function(get_blender blender_ver)

    #Example Link: https://github.com/blender/blender/archive/v2.82a.tar.gz 
    set(BLENDER_ARCHIVE_FILE "v${blender_ver}.tar.gz")
    set(BLENDER_ARCHIVE_URL "https://github.com/blender/blender/archive/${BLENDER_ARCHIVE_FILE}")
    set(BLENDER_ARCHIVE_LOCAL_PATH "${CMAKE_BINARY_DIR}/Blender-${BLENDER_ARCHIVE_FILE}")
    set(BLENDER_ARCHIVE_EXTRACT_PATH "${CMAKE_BINARY_DIR}")
    
    download_and_extract(
        ${BLENDER_ARCHIVE_URL}
        ${BLENDER_ARCHIVE_LOCAL_PATH}
        ${BLENDER_ARCHIVE_EXTRACT_PATH}
        "Downloading Blender ${blender_ver}" 
        "Could not download Blender ${blender_ver} !"
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

# Set the following to cache
# - ${BLENDER${blender_ver}_INCLUDE_DIR}: Blender include directory
# - ${Blender${blender_ver}_FOUND} to TRUE or FALSE, depending on whether the header/libs are found
# - ${BLENDER${blender_ver}_PYTHON_VERSION}: the version of Python used by this Blender version
function(setup_blender blender_ver)

    list(APPEND BLENDER_SRC_PATHS
        "${CMAKE_BINARY_DIR}/Blender-${blender_ver}"
        "${CMAKE_BINARY_DIR}/blender-${blender_ver}"
    )
        
    # Header. Use find_path to store to cache
    find_path(BLENDER${blender_ver}_INCLUDE_DIR
        NAMES
            blenkernel/BKE_blender_version.h
        HINTS
            ${BLENDER_SRC_PATHS}
        PATH_SUFFIXES
            source/blender
        NO_DEFAULT_PATH            
    )
    
    mark_as_advanced(BLENDER${blender_ver}_INCLUDE_DIR)

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
            versions.cmake           # python 3.7
            ${PYTHON_CMAKE_PLATFORM} # python 3.5
        HINTS
            ${BLENDER_SRC_PATHS}
        PATH_SUFFIXES
            build_files/build_environment/cmake #python 3.7
            build_files/cmake/Modules    #python 3.5
            build_files/cmake/platform   #python 3.5
    )
    

    # Find Python version used in this Blender Version. 
    # Line: 
    # - set(PYTHON_VERSION 3.7.0
    # -	set(PYTHON_VERSION 3.5

    file(READ ${BLENDER${blender_ver}_CMAKE_VERSION} VERSION_CONTENTS)    
    string(REGEX MATCH "[Ss][Ee][Tt]\\(PYTHON_VERSION ([0-9]+).([0-9]+).?([0-9]*)" PYTHON_VERSION_SECTION ${VERSION_CONTENTS})        
    set(BLENDER${blender_ver}_PYTHON_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}" CACHE STRING "Python version used by Blender ${blender_ver}" FORCE)    
    message("Python version: ${BLENDER${blender_ver}_PYTHON_VERSION}")

        
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("Blender${blender_ver}"
        DEFAULT_MSG
        BLENDER${blender_ver}_INCLUDE_DIR
        BLENDER${blender_ver}_CMAKE_VERSION
        BLENDER${blender_ver}_PYTHON_VERSION 
    )
    message("Blender ${blender_ver} is using Python ${BLENDER${blender_ver}_PYTHON_VERSION}")
    
    if(NOT ${Blender${blender_ver}_FOUND})
        message(FATAL_ERROR "Blender ${blender_ver} could not be detected. Please remove CMakeCache.txt and redo the build process")
    endif()
endfunction()


