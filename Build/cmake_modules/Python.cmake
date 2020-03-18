
include(Utilities)

# ----------------------------------------------------------------------------------------------------------------------

# returns 
# - PYTHON_FULL_VERSION
function(get_python_full_version python_ver_no_dots)    
    if(python_ver_no_dots STREQUAL "35")
        set(PYTHON_FULL_VERSION "3.5.9" PARENT_SCOPE)
    elseif(python_ver_no_dots STREQUAL "37")
        set(PYTHON_FULL_VERSION "3.7.7" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unsupported python version: ${python_ver_no_dots}")
    endif()
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

function(get_python python_ver_no_dots)

    get_python_full_version(${python_ver_no_dots})
    
    #Example Link: https://www.python.org/ftp/python/3.7.7/Python-3.7.7.tgz
    set(PYTHON_ARCHIVE_FILE "Python-${PYTHON_FULL_VERSION}.tgz")
    set(PYTHON_ARCHIVE_URL "https://www.python.org/ftp/python/${PYTHON_FULL_VERSION}/${PYTHON_ARCHIVE_FILE}")
    set(PYTHON_ARCHIVE_LOCAL_PATH "${CMAKE_BINARY_DIR}/${PYTHON_ARCHIVE_FILE}")

    download_and_extract(
        ${PYTHON_ARCHIVE_URL}
        ${PYTHON_ARCHIVE_LOCAL_PATH}
        "Downloading PYTHON ${PYTHON_FULL_VERSION}" 
        "Could not download PYTHON ${PYTHON_FULL_VERSION} !"
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

# Configure Python. Generate pyconfig.h
# Returns:
# - PYTHON_${python_ver_no_dots}_INCLUDE_DIRS: include dirs this version of Python
# - PYTHON_${python_ver_no_dots}_LIBRARY: (Windows only) the library of this version of Python 
function(configure_python python_ver_no_dots)
    get_python_full_version(${python_ver_no_dots})

    set(PYTHON_${python_ver_no_dots}_SRC_ROOT "${CMAKE_BINARY_DIR}/Python-${PYTHON_FULL_VERSION}" )    
    message("Configuring Python: ${PYTHON_${python_ver_no_dots}_SRC_ROOT}")

	
    if( ${PYTHON_${python_ver_no_dots}_LIBRARY} STREQUAL "PYTHON_${python_ver_no_dots}_LIBRARY-NOTFOUND" 
	     OR NOT ${python_ver_no_dots}_CONFIGURED)
        if(WIN32)
            execute_process(WORKING_DIRECTORY "${PYTHON_${python_ver_no_dots}_SRC_ROOT}/PCbuild" 
                COMMAND cmd.exe /c devenv pcbuild.sln /upgrade && build.bat -p x64
            )
        else()
            execute_process( WORKING_DIRECTORY ${PYTHON_${python_ver_no_dots}_SRC_ROOT}
                COMMAND ./configure
            )
        endif()
        set(${python_ver_no_dots}_CONFIGURED ON CACHE INTERNAL "${python_ver_no_dots} configured flag")
    endif()
    
    set(PYTHON_${python_ver_no_dots}_INCLUDE_DIRS  
            "${PYTHON_${python_ver_no_dots}_SRC_ROOT}" 
            "${PYTHON_${python_ver_no_dots}_SRC_ROOT}/Include"
        CACHE INTERNAL "Python ${python_ver_no_dots} include directories"
    )
    
    # Windows only. Shouldn't be required on other platforms
    if(WIN32) 
        set(PYTHON_${python_ver_no_dots}_INCLUDE_DIRS  
                "${PYTHON_${python_ver_no_dots}_INCLUDE_DIRS};" 
                "${PYTHON_${python_ver_no_dots}_SRC_ROOT}/PC"
            CACHE INTERNAL "Python ${python_ver_no_dots} include directories"
        )
        
        find_library(
            PYTHON_${python_ver_no_dots}_LIBRARY        
            NAMES             
                python${python_ver_no_dots}.lib
            HINTS 
                ${PYTHON_${python_ver_no_dots}_SRC_ROOT}
            PATH_SUFFIXES
                PCbuild/amd64            
        )
        
    endif()
    
    
        
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("Python_${python_ver_no_dots}"
        DEFAULT_MSG
        PYTHON_${python_ver_no_dots}_INCLUDE_DIRS        
    )
    
    if(NOT ${Python_${python_ver_no_dots}_FOUND})
        message(FATAL_ERROR "Failed to find and configure Python ${python_ver_no_dots}. \n"
            "  Path: ${PYTHON_SRC_ROOT}"
        )
    endif()    
        
endfunction()



