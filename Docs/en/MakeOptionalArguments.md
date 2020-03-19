# Make: Optional Arguments

The build process is setup so that all MeshSync plugins for [all supported DCC tools](../../Readme.md) 
will be built by default.

To build plugins for only specific DCC tools, 
replace `[optional_arguments]` in the [manual building page](BuildDCCPlugins.md)
with commands described in the following sections.

## Maya

* All supported Maya versions: `-DBUILD_MAYA_ALL=ON`
* Maya 2017: `-DBUILD_MAYA_2017_PLUGIN=ON`
* Maya 2018: `-DBUILD_MAYA_2018_PLUGIN=ON`
* Maya 2019: `-DBUILD_MAYA_2019_PLUGIN=ON`
* Maya 2020: `-DBUILD_MAYA_2020_PLUGIN=ON`

## 3ds Max

* All supported 3ds Max versions: `-DBUILD_3DSMAX_ALL=ON`
* 3ds Max 2017: `-DBUILD_3DSMAX_2017_PLUGIN=ON`
* 3ds Max 2018: `-DBUILD_3DSMAX_2018_PLUGIN=ON`
* 3ds Max 2019: `-DBUILD_3DSMAX_2019_PLUGIN=ON`
* 3ds Max 2020: `-DBUILD_3DSMAX_2020_PLUGIN=ON`

## MotionBuilder

* All supported MotionBuilder versions: `-DBUILD_MOTIONBUILDER_ALL=ON`
* Motion Builder 2017: `-DBUILD_MOTIONBUILDER_2017_PLUGIN=ON`
* Motion Builder 2018: `-DBUILD_MOTIONBUILDER_2018_PLUGIN=ON`
* Motion Builder 2019: `-DBUILD_MOTIONBUILDER_2019_PLUGIN=ON`
* Motion Builder 2020: `-DBUILD_MOTIONBUILDER_2020_PLUGIN=ON`

## Blender

* All supported Blender versions: `-DBUILD_BLENDER_ALL=ON`
* Blender 2.79: `-DBLENDER_2.79_PLUGIN=ON`
* Blender 2.80: `-DBLENDER_2.80_PLUGIN=ON`

# Tips

Multiple optional arguments can be joined together to build MeshSync plugins for a specific combination of 
DCC tools. For example:

* `-DBUILD_MAYA_2017_PLUGIN=ON -DBUILD_MAYA_2020_PLUGIN=ON`  
  Will build plugins for Maya 2017 and Maya 2020.
