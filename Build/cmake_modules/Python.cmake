
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

    set(PYTHON_SRC_ROOT "${CMAKE_BINARY_DIR}/Python-${PYTHON_FULL_VERSION}")    
    message("Configuring Python: ${PYTHON_SRC_ROOT}")
    execute_process(
        WORKING_DIRECTORY ${PYTHON_SRC_ROOT}
        COMMAND ./configure
    )
        
endfunction()



