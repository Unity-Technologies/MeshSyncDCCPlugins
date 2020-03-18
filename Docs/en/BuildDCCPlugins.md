# Building Plugins

1. [Windows](#building-on-windows)
1. [Mac OSX](#building-on-mac_osx)

## Building on Windows

### Prerequisites (Win)

1. Install [cmake](https://cmake.org/) 
1. Install Visual Studio 2017
1. Install git. For example: [SourceTree](https://www.sourcetreeapp.com/)
1. Build [Poco](https://pocoproject.org) (static libraries).  
   * Download [Poco 1.10.1](https://github.com/pocoproject/poco/archive/poco-1.10.1-release.zip) and extract the file in a folder
   * Start "Developer Command Prompt for VS 2017" and go to where Poco was extracted
   * Execute the following in the command prompt      
    ``` 
    $ mkdir cmake-build
    $ cd cmake-build
    $ cmake .. -DBUILD_SHARED_LIBS=OFF -G "Visual Studio 15 2017" -A x64
    $ cmake --build . --config MinSizeRel && cmake --build . --config Debug
    ```
    
    > To build Poco libraries with other configurations, see [Poco's Getting Started](https://pocoproject.org/docs/00200-GettingStarted.html).
    
1. Add *Poco_DIR* environment variable to point to the Poco root folder above
1. [Setup DCC tools](SetupDCC.md) for building
1. On Windows 10, allow regular users to [create symbolic links](CreateSymbolicLinksOnWindows10.md).


### Build Steps (Win)


Start "Developer Command Prompt for VS 2017" and execute the following:

``` 
$ git clone https://github.com/Unity-Technologies/MeshSyncDCCPlugin
$ cd MeshSyncDCCPlugin\Build
$ make_meshsync_dcc_plugin.bat <meshsync_version> [optional_arguments]
$ msbuild MeshSyncDCCPlugin.sln /t:Build /p:Configuration=MinSizeRel /p:Platform=x64 /m /nologo
$ cmake -DBUILD_TYPE=MinSizeRel -P cmake_install.cmake
```

There are two parameters in the script  
* `<meshsync_version>`  
  The MeshSync package version that we want the DCC plugins to work with.  
* `[optional_arguments]`   
  See [MakeOptionalArguments](MakeOptionalArguments.md) for more details.

> For a regular "Command Prompt", there is a script: *VsDevCmd_2017.bat* 
> under the *Build* folder, which if executed, will turn the prompt into a 
> "Developer Command Prompt for VS 2017".

The build results will be located in *Dist/MeshSyncClient_<meshsync_version>_<DCC_Tool>* folder.  
Refer to the [installation guide](Installation.md) to install the plugin for each DCC tool.

#### Notes

The build process will try to link againts Poco's release libraries in the following order:  
1. MinSizeRel  
1. Release  
1. RelWithDebInfo 

## Building on Mac OSX

### Prerequisites (Mac)

1. Install [cmake](https://cmake.org/) 
1. Install [XCode](https://developer.apple.com/xcode/)
1. Install XCode Command Line tools  
    ``` 
    xcode-select --install
    ```  
1. Install [Homebrew](https://brew.sh/)
1. Install git. For example: [SourceTree](https://www.sourcetreeapp.com/)
1. Build [Poco](https://pocoproject.org) (static libraries).  
   * Download [Poco 1.10.1](https://github.com/pocoproject/poco/archive/poco-1.10.1-release.zip) and extract the file in a folder
   * Open a terminal and go to where Poco was extracted
   * Execute the following in the command prompt      
    ``` 
    $ mkdir cmake-build
    $ cd cmake-build
    $ cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=MinSizeRel && cmake --build . 
    $ cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Debug && cmake --build . 
    ```
    > For other types of Poco configurations, see [Poco's Getting Started](https://pocoproject.org/docs/00200-GettingStarted.html).
1. Add *Poco_DIR* environment variable to point to the Poco root folder above. For example:  
    ``` 
    export Poco_DIR=~/MySDK/poco
    ```  
    It might also be good to add this command to *~/.bash_profile*
1. Install the following via Homebrew  
    ``` 
    $ brew install zstd
    $ brew install tbb
    ```  
    
    Currently, the used version of each is:
    * zstd: `stable 1.4.4`.
    * tbb:  `stable 2020_U1`.
    
1. [Setup DCC tools](SetupDCC.md) for building


### Build Steps (Mac)

Open a terminal and execute the following:

``` 
$ git clone https://github.com/Unity-Technologies/MeshSyncDCCPlugin
$ cd MeshSyncDCCPlugin/Build
$ ./make_meshsync_dcc_plugin <meshsync_version> [Custom Arguments]
$ xcodebuild -alltargets -configuration MinSizeRel build
```

There are two parameters in the script  
* `<meshsync_version>`    
  The MeshSync package version that we want the DCC plugins to work with.  
* `[optional_arguments]`  
  See [MakeOptionalArguments](MakeOptionalArguments.md) for more details.

The build results will be located in *Dist/MeshSyncClient_<meshsync_version>_<DCC_Tool>* folder.  
Refer to the [installation guide](Installation.md) to install the plugin for each DCC tool.



