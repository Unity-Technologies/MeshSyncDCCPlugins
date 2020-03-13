
include(Utilities)

# ----------------------------------------------------------------------------------------------------------------------

function(get_python python_ver_no_dots)

    
    if(python_ver_no_dots STREQUAL "35")
        set(PYTHON_VERSION "3.5.9")
    elseif(python_ver_no_dots STREQUAL "37")
        set(PYTHON_VERSION "3.7.7")
    else()
        message(FATAL_ERROR "Unsupported python version: ${python_ver_no_dots}")
    endif()
    
    #Example Link: https://www.python.org/ftp/python/3.7.7/Python-3.7.7.tgz
    set(PYTHON_ARCHIVE_FILE "Python-${PYTHON_VERSION}.tgz")
    set(PYTHON_ARCHIVE_URL "https://www.python.org/ftp/python/${PYTHON_VERSION}/${PYTHON_ARCHIVE_FILE}")
    set(PYTHON_ARCHIVE_LOCAL_PATH "${CMAKE_BINARY_DIR}/${PYTHON_ARCHIVE_FILE}")

    download_and_extract(
        ${PYTHON_ARCHIVE_URL}
        ${PYTHON_ARCHIVE_LOCAL_PATH}
        "Downloading PYTHON ${PYTHON_VERSION}" 
        "Could not download PYTHON ${PYTHON_VERSION} !"
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------


