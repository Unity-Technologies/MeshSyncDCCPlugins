# Setting Up DCC

Please refer to the following sections to setup each DCC tool.

Unless specified by [MakeOptionalArguments](MakeOptionalArguments.md), 
the build script will build MeshSync plugins for [all supported DCC tools](../../README.md).

## [Maya](https://www.autodesk.com/products/maya/overview)

Based on the version of Maya, set `MAYA_SDK_<MAYA_VERSION>` environment variable that point to 
where the [SDK/devkit](https://www.autodesk.com/developer-network/platform-technologies/maya) 
is installed.

For example in MacOS:
``` 
export MAYA_SDK_2019=your_path
export MAYA_SDK_2020=your_path
``` 
