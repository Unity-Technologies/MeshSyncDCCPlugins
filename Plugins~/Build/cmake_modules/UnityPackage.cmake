
# Set the following to cache
# - ${UNITY_PACKAGE_VERSION}

function(setup_unity_package_version)

    find_file(UNITY_PACKAGE_JSON
        NAMES package.json
        HINTS "../"
    )    

    file(READ ${UNITY_PACKAGE_JSON} package_contents)    
    
    # Convert the whole text to lines
    STRING(REGEX REPLACE "\n" ";" split_contents "${package_contents}")    
    
    foreach(package_line in ${split_contents})
        string(REGEX MATCH "\"version\": \"(.*)\"" PACKAGE_VERSION ${package_line})        
        if (PACKAGE_VERSION)
        
            message("Unity Package Version: ${CMAKE_MATCH_1}")
            set(UNITY_PACKAGE_VERSION ${CMAKE_MATCH_1} CACHE STRING "Unity package version" FORCE )    
            break()
        
        endif()
    endforeach()
        
    # error check
    if(NOT UNITY_PACKAGE_VERSION)
        message(FATAL_ERROR "Could not be detect the version of Unity Package")
    endif()
    
    
         
endfunction()
