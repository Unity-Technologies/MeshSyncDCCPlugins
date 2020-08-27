# Setting Up DCC Tools for Building Plugins

Please refer to the following sections to setup each DCC tool.

> Unless specified by [MakeOptionalArguments](MakeOptionalArguments.md), 
the build script will build MeshSync plugins for [all supported DCC tools](../../README.md), 
and therefore all supported DCC tools below have to be set up.

## Maya

Based on the version of [Maya](https://www.autodesk.com/products/maya/overview), set the following environment variable:
* `MAYA_SDK_<MAYA_VERSION>` : 
  where the [SDK or devkit](https://www.autodesk.com/developer-network/platform-technologies/maya) is installed.

> On Linux, no setup is necessary if Maya is installed in the default location.

As an example in MacOS:
``` 
export MAYA_SDK_2017=/Applications/Autodesk/maya2017/Maya.app/Contents
export MAYA_SDK_2018=/path/to/maya/sdk/2018
export MAYA_SDK_2019=/path/to/maya/sdk/2019
export MAYA_SDK_2020=/path/to/maya/sdk/2020
``` 
### Maya 2017 

In addition to the above, Maya 2017 requires `MAYA_APP_2017` environment variable to be set.  
As an example in MacOS:
``` 
export MAYA_APP_2017=/Applications/Autodesk/maya2017/Maya.app
``` 


## 3ds Max 

![Install3dsMaxSDK](../Images/Install3dsMaxSDK.png)

No setup is necessary if the SDK is installed using [3ds Max](https://www.autodesk.com/products/3ds-max/overview)
install application.  
However, if the SDK was downloaded directly from [3ds Max Developer Center](https://www.autodesk.com/developer-network/platform-technologies/3ds-max) 
on the Internet, then we need to set the following environment variable:
* `ADSK_3DSMAX_SDK_<3DSMAX_VERSION>` : where 3ds Max is installed.

## MotionBuilder

Based on the version of [MotionBuilder](https://www.autodesk.com/products/motionbuilder/overview), set the following environment variable:
* `MOTIONBUILDER_SDK_<MOTIONBUILDER_VERSION>` : where MotionBuilder is installed.

> On Linux, no setup is necessary if MotionBuilder is installed in the default location.

## Modo

1. Get [Modo Developer SDK](https://www.foundry.com/products/modo/download) and extract.
1. Add `MODO_SDK` environment variable to point to the path of the SDK.


### Additional Steps on Windows

Required steps on Windows, mostly to build Qt 4.8.7 64-bit libraries:
1. Choose the appropriate [Qt license](https://www.qt.io/licensing).
1. Install a `patch` command-line tool, for example by using [Cygwin](https://www.cygwin.com/).
1. Download Qt 4.8.7 source from [qt.io](https://download.qt.io/archive/qt/4.8/4.8.7/), and extract.
1. Add `QT_4_8_SDK` environment variable to point to the Qt source folder.
1. Copy [Qt 4.8.7 patch](../../External/Patches/qt-4.8.7-win.patch) 
   (thanks to [github.com/sandym](https://github.com/sandym/qt-patches/tree/master/windows/qt-4.8.7
   )) 
   to the Qt source folder.      
1. Patch Qt source code by executing `patch -p1 < qt-4.8.7-win.patch`  
1. Add the path to *rc.exe* to `PATH` environment variable.  
   Example: `C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64`
1. Open Developer Command Prompt for Visual Studio 2017, and execute the following:
   ```
   VC\Auxiliary\Build\vcvars64.bat
   cd c:\qt\sdk\path
   configure -platform win32-msvc2015
   nmake
   ```

### Additional Steps on Mac

If Modo is installed in a non-default path, 
then set `MODO_APP_<MODO_VERSION>` to point to where Modo is installed.

### Additional Steps on Linux

Required steps on Linux, mostly to configure Qt 4.8.7 64-bit libraries:
1. Choose the appropriate [Qt license](https://www.qt.io/licensing).
1. Download Qt 4.8.7 source from [qt.io](https://download.qt.io/archive/qt/4.8/4.8.7/), and extract.  
   Example: `unzip -a qt-everywhere-opensource-src-4.8.7.zip` 
1. Add environment variables:
   - `QT_4_8_SDK` : the path to the Qt source folder.
   - `MODO_APP_<MODO_VERSION>` : the path where Modo is installed.
1. Open a terminal, and execute the following:
   ```
   cd ~/qt/sdk/path
   ./configure -platform linux-g++-64
   ```




