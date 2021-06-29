# Make: Optional Arguments

The build process is setup so that all MeshSync plugins for [all supported DCC tools](../../../Documentation~/index.md) 
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
* Blender 2.83.16: `-DBUILD_BLENDER_2.83.16_PLUGIN=ON`
* Blender 2.90.1 : `-DBUILD_BLENDER_2.90.1_PLUGIN=ON`
* Blender 2.91.2 : `-DBUILD_BLENDER_2.91.2_PLUGIN=ON`
* Blender 2.92.0 : `-DBUILD_BLENDER_2.92.0_PLUGIN=ON`
* Blender 2.93.1 : `-DBUILD_BLENDER_2.93.1_PLUGIN=ON`

## Modo

* All supported Blender versions: `-DBUILD_MODO_ALL=ON`
* MODO_12: `-DBUILD_MODO_12_PLUGIN=ON`
* MODO_13: `-DBUILD_MODO_13_PLUGIN=ON`
* MODO_14: `-DBUILD_MODO_14_PLUGIN=ON`

## Metasequoia

* All supported Blender versions: `-DBUILD_METASEQUOIA_ALL=ON`
* Metasequoia 4.72 and up: `-DBUILD_MQ_472_PLUGIN=ON`
* Metasequoia 4.71: `-DBUILD_MQ_471_PLUGIN=ON`
* Metasequoia 4.70: `-DBUILD_MQ_470_PLUGIN=ON`


# Tips

Multiple optional arguments can be joined together to build MeshSync plugins for a specific combination of 
DCC tools. For example:

* `-DBUILD_MAYA_2017_PLUGIN=ON -DBUILD_MAYA_2020_PLUGIN=ON`  
  Will build plugins for Maya 2017 and Maya 2020.
