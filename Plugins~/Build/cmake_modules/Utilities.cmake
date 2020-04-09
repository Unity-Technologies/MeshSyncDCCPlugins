# Various utilities for cmake

function(download_and_extract 
            archive_url
            archive_local_path 
            extract_path
            download_msg
            err_msg
        )
    if(NOT EXISTS ${archive_local_path})
        message(${download_msg})
        file(DOWNLOAD ${archive_url} ${archive_local_path} SHOW_PROGRESS)

        # Check if the download is successful
        file(SIZE ${archive_local_path} ARCHIVE_FILESIZE)
        if(0 EQUAL ${ARCHIVE_FILESIZE})
            file(REMOVE ${archive_local_path})
            message(FATAL_ERROR "${err_msg}")
        endif()
               
        # Extract
        message("   Extracting")
        file(MAKE_DIRECTORY ${extract_path})        
        execute_process(
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND tar -xf ${archive_local_path} -C ${extract_path}
        )
    endif()
    
endfunction()

# ----------------------------------------------------------------------------------------------------------------------

