
# Set
# - ${MOTIONBUILDER${motionbuilder_version}_FOUND} to TRUE or FALSE, depending on whether the header/libs are found
# - ${MOTIONBUILDER${motionbuilder_version}_INCLUDE_DIR}: the include directory of 3DSMax
# - $[MOTIONBUILDER${motionbuilder_version}_LIBRARIES}: path to the 3DSMax libraries
function(setup_motionbuilder motionbuilder_version)

	# The possible root paths of MotionBuilder
    list(APPEND MOTIONBUILDER${motionbuilder_version}_PATHS	
        $ENV{MOTIONBUILDER_SDK_${motionbuilder_version}}
        $ENV{MOTIONBUILDER_SDK_${motionbuilder_version}}/OpenRealitySDK
        "/usr/autodesk/MotionBuilder${motionbuilder_version}/OpenRealitySDK"
        "/opt/autodesk/MotionBuilder${motionbuilder_version}/OpenRealitySDK"		
		
    )
	
    # Header. Use find_path to store to cache
    find_path(MOTIONBUILDER${motionbuilder_version}_INCLUDE_DIR
        NAMES 
            fbsdk/fbsdk.h
        HINTS
            ${MOTIONBUILDER${motionbuilder_version}_PATHS}
        PATH_SUFFIXES 
		    include
			
    )
    mark_as_advanced(MOTIONBUILDER${motionbuilder_version}_INCLUDE_DIR)
	
    # Libs, and set cache at the end
    foreach(MOTIONBUILDER_LIB fbsdk)
        find_file(MOTIONBUILDER${motionbuilder_version}_${MOTIONBUILDER_LIB}_LIBRARY 
            NAMES 
                ${MOTIONBUILDER_LIB}.lib                     # Windows
				lib${MOBU_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} # Linux
            PATHS
                ${MOTIONBUILDER${motionbuilder_version}_PATHS}                
            PATH_SUFFIXES 
                lib/x64           # Windows
				lib/linux_64      # Linux
        )
				
        mark_as_advanced(MOTIONBUILDER${motionbuilder_version}_${MOTIONBUILDER_LIB}_LIBRARY)
        if(MOTIONBUILDER${motionbuilder_version}_${MOTIONBUILDER_LIB}_LIBRARY)
            list(APPEND MOTIONBUILDER${motionbuilder_version}_LIBRARIES ${MOTIONBUILDER${motionbuilder_version}_${MOTIONBUILDER_LIB}_LIBRARY})
        endif()
    endforeach()
    set(MOTIONBUILDER${motionbuilder_version}_LIBRARIES ${MOTIONBUILDER${motionbuilder_version}_LIBRARIES} CACHE STRING "Motion Builder ${motionbuilder_version} libraries")
    mark_as_advanced(MOTIONBUILDER${motionbuilder_version}_LIBRARIES)
	
	# message(${MOTIONBUILDER${motionbuilder_version}_LIBRARIES})
    
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args("MOTIONBUILDER${motionbuilder_version}"
        DEFAULT_MSG
        MOTIONBUILDER${motionbuilder_version}_INCLUDE_DIR
        MOTIONBUILDER${motionbuilder_version}_LIBRARIES
    )  	
	
    if(NOT ${MOTIONBUILDER${motionbuilder_version}_FOUND})  
        message(FATAL_ERROR "MotionBuilder ${motionbuilder_version} SDK could not be found. Please define MOTIONBUILDER_SDK_${motionbuilder_version}. \n"
            "Paths searched for MotionBuilder SDK: ${3DSMAX${motionbuilder_version}_PATHS}"
        )
    endif()	

endfunction()