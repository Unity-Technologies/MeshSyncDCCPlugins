@ECHO OFF

REM  The default will build all
SET CMAKE_BUILD_ARGS=-DBUILD_MAYA_ALL=ON -DBUILD_3DSMAX_ALL=ON -DBUILD_BLENDER_ALL=ON -DBUILD_MOTIONBUILDER_ALL=ON -DBUILD_METASEQUOIA_ALL=ON -DBUILD_MODO_ALL=ON

set ARG_COUNT=0
for %%x in (%*) do (
   set /A ARG_COUNT+=1
)

REM Invalid arguments check
IF %ARG_COUNT% LEQ  0 (
    echo Usage: build_meshsync_dcc_plugin [meshsync_version]
    exit /B
)

SET MESHSYNC_VER=%1

REM Custom build arguments
IF %ARG_COUNT% GEQ 2 (
    shift
    SET CMAKE_BUILD_ARGS=%*   
)

SET BUILD_SYSTEM=-G "Visual Studio 17 2022" -A x64

ECHO cmake build Arguments: %CMAKE_BUILD_ARGS%
cmake -UBUILD_*_PLUGIN -UBUILD_*_ALL -DMESHSYNC_VER:STRING=%MESHSYNC_VER% %CMAKE_BUILD_ARGS% %BUILD_SYSTEM% ..
