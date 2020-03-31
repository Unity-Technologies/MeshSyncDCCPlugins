
include(Utilities)

# ----------------------------------------------------------------------------------------------------------------------

function(get_mq mq_ver)

    #Example Link: http://www.metaseq.net/metaseq/mqsdk470.zip 
    set(MQ_ARCHIVE_FILE "mqsdk${mq_ver}.zip")
    set(MQ_ARCHIVE_URL "http://www.metaseq.net/metaseq/${MQ_ARCHIVE_FILE}")
    set(MQ_ARCHIVE_LOCAL_PATH "${CMAKE_BINARY_DIR}/${MQ_ARCHIVE_FILE}")
    set(MQ_ARCHIVE_EXTRACT_PATH "${CMAKE_BINARY_DIR}/mqsdk-${mq_ver}")
    
    message("Downloading ${MQ_ARCHIVE_URL} and extracting to ${MQ_ARCHIVE_EXTRACT_PATH}")

    download_and_extract(
        ${MQ_ARCHIVE_URL}
        ${MQ_ARCHIVE_LOCAL_PATH}
        ${MQ_ARCHIVE_EXTRACT_PATH}
        "Downloading MQ ${mq_ver}" 
        "Could not download MQ ${mq_ver} !"
    )
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

# Set the following to cache
# - ${MQ${mq_ver}_DIR}: MQ directory
# - ${MQ${mq_ver}_FOUND} to TRUE or FALSE, depending on whether the header/libs are found
function(setup_mq mq_ver)

    list(APPEND MQ_SRC_PATHS
        "${CMAKE_BINARY_DIR}/mqsdk-${mq_ver}"
    )
        
    # Header. Use find_path to store to cache
    find_path(MQ${mq_ver}_DIR
        NAMES
            MQPlugin.h
        HINTS
            ${MQ_SRC_PATHS}
        PATH_SUFFIXES
            mqsdk
        NO_DEFAULT_PATH            
    )
    
    mark_as_advanced(MQ${mq_ver}_DIR)
        
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("MQ${mq_ver}"
        DEFAULT_MSG
        MQ${mq_ver}_DIR
    )
    
    if(NOT ${MQ${mq_ver}_FOUND})
        message(FATAL_ERROR "MQ ${mq_ver} could not be detected. Please remove CMakeCache.txt and redo the build process")
    endif()
endfunction()


