include(CMakeParseArguments)

function(get_meshsync meshsync_ver)

    #Example Link: https://github.com/unity3d-jp/MeshSync/archive/0.0.1-preview.2.zip
    set(MESHSYNC_ARCHIVE_FILE "${meshsync_ver}.zip")
    set(MESHSYNC_ARCHIVE_URL "https://github.com/unity3d-jp/MeshSync/archive/${MESHSYNC_ARCHIVE_FILE}")
    set(MESHSYNC_ARCHIVE_LOCAL_PATH "${CMAKE_BINARY_DIR}/MeshSync-${MESHSYNC_ARCHIVE_FILE}")

   if(NOT EXISTS ${MESHSYNC_ARCHIVE_LOCAL_PATH})
       message("Downloading MeshSync " ${meshsync_ver})
       file(DOWNLOAD ${MESHSYNC_ARCHIVE_URL} ${MESHSYNC_ARCHIVE_LOCAL_PATH} SHOW_PROGRESS)

       # Check if the download is successful
       file(SIZE ${MESHSYNC_ARCHIVE_LOCAL_PATH} MESHSYNC_ARCHIVE_FILESIZE)
       if(0 EQUAL ${MESHSYNC_ARCHIVE_FILESIZE})
           file(REMOVE ${MESHSYNC_ARCHIVE_LOCAL_PATH})
           message(FATAL_ERROR "Could not download MeshSync ${meshsync_ver} !")
       endif()
   endif()

   
   execute_process(
       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
       COMMAND tar -xf ${MESHSYNC_ARCHIVE_LOCAL_PATH}
   )

endfunction()

