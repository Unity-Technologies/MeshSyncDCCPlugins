# Make: Optional Arguments

Following the steps in [Building DCC Plugins](BuildDCCPlugins.md) will build 
MeshSync plugins for [all supported DCC tools](../../Readme.md) by default.

To build plugins for specific DCC tools, replace `[optional_arguments]` with commands described
in the following sections.

## Maya

* All supported Maya versions: `-DBUILD_MAYA_ALL=ON`
* Maya 2017: `-DBUILD_MAYA_2017_PLUGIN=ON`
* Maya 2018: `-DBUILD_MAYA_2018_PLUGIN=ON`
* Maya 2019: `-DBUILD_MAYA_2019_PLUGIN=ON`
* Maya 2020: `-DBUILD_MAYA_2020_PLUGIN=ON`

## Blender

* All supported Blender versions: `-DBUILD_BLENDER_ALL=ON`
* Blender 2.79: `-DBLENDER_2.79=ON`
* Blender 2.80: `-DBLENDER_2.80=ON`

# Tips

Multiple optional arguments can be joined together to build MeshSync plugins for a specific combination of 
DCC tools. For example:

* `-DBUILD_MAYA_2017_PLUGIN=ON -DBUILD_MAYA_2020_PLUGIN=ON`  
  Will build plugins for Maya 2017 and Maya 2020.
