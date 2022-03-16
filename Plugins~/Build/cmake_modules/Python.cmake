
include(Utilities)

# ----------------------------------------------------------------------------------------------------------------------

function(get_python python_full_version)
   
    #Example Link: https://www.python.org/ftp/python/3.7.7/Python-3.7.7.tgz
    set(PYTHON_ARCHIVE_FILE "Python-${python_full_version}.tgz")
    set(PYTHON_ARCHIVE_URL "https://www.python.org/ftp/python/${python_full_version}/${PYTHON_ARCHIVE_FILE}")
    set(PYTHON_ARCHIVE_LOCAL_PATH "${CMAKE_BINARY_DIR}/${PYTHON_ARCHIVE_FILE}")
    set(PYTHON_ARCHIVE_EXTRACT_PATH "${CMAKE_BINARY_DIR}")
    
    download_and_extract(
        ${PYTHON_ARCHIVE_URL}
        ${PYTHON_ARCHIVE_LOCAL_PATH}
        ${PYTHON_ARCHIVE_EXTRACT_PATH}
        "Downloading PYTHON ${python_full_version}" 
        "Could not download PYTHON ${python_full_version} !"
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

# Configure Python. Generate pyconfig.h
# Returns:
# - PYTHON_${python_full_version}_INCLUDE_DIRS: include dirs of this version of Python
# - PYTHON_${python_full_version}_LIBRARY: (Windows only) the library of this version of Python 
function(configure_python python_full_version)

    set(PYTHON_${python_full_version}_SRC_ROOT "${CMAKE_BINARY_DIR}/Python-${python_full_version}" )    
    message("Configuring Python: ${PYTHON_${python_full_version}_SRC_ROOT}")

	
    if( (DEFINED ${PYTHON_${python_full_version}_LIBRARY} AND ${PYTHON_${python_full_version}_LIBRARY} STREQUAL "PYTHON_${python_full_version}_LIBRARY-NOTFOUND") 
	     OR NOT PYTHON_${python_full_version}_CONFIGURED
	  )
        if(WIN32)
            execute_process(WORKING_DIRECTORY "${PYTHON_${python_full_version}_SRC_ROOT}/PCbuild" 
                COMMAND cmd.exe /c devenv pcbuild.sln /upgrade && build.bat -p x64 
            )
        else()
            execute_process( WORKING_DIRECTORY ${PYTHON_${python_full_version}_SRC_ROOT}
                COMMAND ./configure --enable-optimizations
            )
        endif()
        set(PYTHON_${python_full_version}_CONFIGURED ON CACHE INTERNAL "Python ${python_full_version} configured flag")
    endif()
    
    set(PYTHON_${python_full_version}_INCLUDE_DIRS  
            "${PYTHON_${python_full_version}_SRC_ROOT}" 
            "${PYTHON_${python_full_version}_SRC_ROOT}/Include"
        CACHE INTERNAL "Python ${python_full_version} include directories"
    )
    
    # Windows only. Shouldn't be required on other platforms
    if(WIN32) 
        set(PYTHON_${python_full_version}_INCLUDE_DIRS  
                "${PYTHON_${python_full_version}_INCLUDE_DIRS};" 
                "${PYTHON_${python_full_version}_SRC_ROOT}/PC"
            CACHE INTERNAL "Python ${python_full_version} include directories"
        )
        string(REGEX MATCH "([0-9]+).([0-9]+).([0-9]+)" python_ver ${python_full_version})        
        set(python_ver_no_dots "${CMAKE_MATCH_1}${CMAKE_MATCH_2}" )
        
        find_library(
            PYTHON_${python_full_version}_LIBRARY
            NAMES             
                python${python_ver_no_dots}.lib
            HINTS 
                ${PYTHON_${python_full_version}_SRC_ROOT}
            PATH_SUFFIXES
                PCbuild/amd64            
        )
        
        if(NOT DEFINED PYTHON_${python_full_version}_LIBRARY)
            message(FATAL_ERROR "Failed to find and configure Python libraries for ${python_full_version}. \n"
                "  Path: ${PYTHON_SRC_ROOT}"
            )
        endif()
    endif()
        
        
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("Python_${python_full_version}"
        DEFAULT_MSG
        PYTHON_${python_full_version}_INCLUDE_DIRS
    )
    
    if(NOT ${Python_${python_full_version}_FOUND})
        message(FATAL_ERROR "Failed to find and configure Python ${python_full_version}. \n"
            "  Path: ${PYTHON_SRC_ROOT}"
        )
    endif()    

        
endfunction()



