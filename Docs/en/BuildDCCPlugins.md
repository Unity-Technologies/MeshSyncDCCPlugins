# Building on Mac OSX

## Prerequisites (Mac)

1. Install [cmake](https://cmake.org/) 
1. Install [XCode](https://developer.apple.com/xcode/)
1. Install [Homebrew](https://brew.sh/)
1. Install git. For example: [SourceTree](https://www.sourcetreeapp.com/)
1. Download and build the debug and release versions of [Poco](https://pocoproject.org) (static libraries).  
    ``` 
    $ git clone -b master https://github.com/pocoproject/poco.git
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
1. Install [zstd](https://github.com/facebook/zstd/releases)  via Homebrew  
    ``` 
    $ brew install zstd
    ```  



## Build Steps (Mac)

Open a terminal and execute the following.
While doing that, replace `[meshsync_version]` withe the actual version, for example `0.0.1-preview`

``` 
$ git clone https://github.com/Unity-Technologies/MeshSyncDCCPlugin
$ cd MeshSyncDCCPlugin/Build
$ ./build_meshsync_dcc_plugin [meshsync_version]
```
