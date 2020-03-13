
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
function(configure_python python_ver_no_dots)
    get_python_full_version(${python_ver_no_dots})

    set(PYTHON_${python_ver_no_dots}_SRC_ROOT "${CMAKE_BINARY_DIR}/Python-${PYTHON_FULL_VERSION}" CACHE STRING "Python ${python_ver_no_dots} root folder")    
    message("Configuring Python: ${PYTHON_${python_ver_no_dots}_SRC_ROOT}")
    if(NOT ${python_ver_no_dots}_CONFIGURED)
        execute_process(
            WORKING_DIRECTORY ${PYTHON_${python_ver_no_dots}_SRC_ROOT}
            COMMAND ./configure
        )
        set(${python_ver_no_dots}_CONFIGURED ON CACHE BOOL "${python_ver_no_dots} configured flag")
        mark_as_advanced(${python_ver_no_dots}_CONFIGURED)        
    endif()
    
    
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("Python_${python_ver_no_dots}"
        DEFAULT_MSG
        PYTHON_${python_ver_no_dots}_SRC_ROOT
    )
    
    if(NOT ${Python_${python_ver_no_dots}_FOUND})
        message(FATAL_ERROR "Failed to find and configure Python ${python_ver_no_dots}. \n"
            "  Path: ${PYTHON_SRC_ROOT}"
        )
    endif()    
        
endfunction()



