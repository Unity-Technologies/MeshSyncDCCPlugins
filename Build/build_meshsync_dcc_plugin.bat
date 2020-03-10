@ECHO OFF

REM  The default will build all
SET CMAKE_BUILD_ARGS=-DBUILD_MAYA_ALL=ON -DBUILD_3DSMAX_ALL=ON -DBUILD_BLENDER_ALL=ON

set argCount=0
for %%x in (%*) do (
   set /A argCount+=1
   set "argVec[!argCount!]=%%~x"
)

echo Number of processed arguments: %argCount%


REM Invalid arguments check
IF %argCount% LEQ  0 (
    echo Usage: build_meshsync_dcc_plugin [meshsync_version]
    exit
)

SET MESHSYNC_VER=%1

REM Custom build arguments
IF %argCount% GEQ 2 (
    shift
    SET CMAKE_BUILD_ARGS=%*   
)

SET BUILD_SYSTEM=-G "Visual Studio 15 2017"

ECHO cmake build Arguments: %CMAKE_BUILD_ARGS%
cmake  -DMESHSYNC_VER:STRING=%MESHSYNC_VER% %CMAKE_BUILD_ARGS% %BUILD_SYSTEM% ..
