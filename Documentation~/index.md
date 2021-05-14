# MeshSync DCC Plugins

MeshSync DCC Plugins is package that contains plugin binaries of DCC tools for using [MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest), 
which is another package for synchronizing meshes/models editing in DCC tools into Unity in real time.
This allows devs to immediately see how things will look in-game while modelling.

## Features

|                     | Maya                 | 3ds Max              | MotionBuilder        | Blender              | Modo                 | Metasequoia          | 
| --------------------| -------------------- | -------------------- | -------------------- | -------------------- | -------------------- | -------------------- | 
| Polygon mesh sync   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| Camera sync         | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| Light sync          | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| Double-sided Mesh   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| Negative Scale      | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: |                      |
| Multi UV            |                      |                      |                      | :heavy_check_mark:   |                      |                      |  
| Scene Cache Export  | :heavy_check_mark:   | :heavy_check_mark:   |                      | :heavy_check_mark:   | :heavy_check_mark:   |                      |  
| Non-polygon shape   |                      |                      |                      |                      |                      |                      |  


### Caveats

* Negative Scale: partially supported on some DCC Tools.  
  If all XYZ values have negative values, the mesh will sync properly, however if only one axis has a negative value,
  Unity will treat the mesh as though every axis has a negative value.
  Certain DCC tools may have *Bake Transform* option which can sync the mesh in this case, but it will lose any 
  deformer information.


## Supported DCC Tools

|                     | Windows            | Mac                | Linux              | 
| --------------------| ------------------ | ------------------ |------------------- | 
| Maya 2017           | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Maya 2018           | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Maya 2019           | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Maya 2020           | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Maya LT 2019 +      | :white_check_mark: |                    | :x:                | 
| 3ds Max 2017        | :white_check_mark: | :x:                | :x:                | 
| 3ds Max 2018        | :white_check_mark: | :x:                | :x:                | 
| 3ds Max 2019        | :white_check_mark: | :x:                | :x:                | 
| 3ds Max 2020        | :white_check_mark: | :x:                | :x:                | 
| MotionBuilder 2017  | :white_check_mark: | :x:                | :white_check_mark: | 
| MotionBuilder 2018  | :white_check_mark: | :x:                | :white_check_mark: | 
| MotionBuilder 2019  | :white_check_mark: | :x:                | :white_check_mark: | 
| MotionBuilder 2020  | :white_check_mark: | :x:                | :white_check_mark: | 
| Blender 2.83        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Blender 2.90        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Blender 2.91        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Blender 2.92        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Modo 12             | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Modo 13             | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Metasequoia 4.x     | :white_check_mark: | :white_check_mark: |                    | 

Notes:
* :white_check_mark: : Supported
* :x: : Impossible to support (platform unsupported by the DCC, etc)
* empty : May be supported in the future

# DCC Plugin Installation

![MeshSyncPreferences](images/MeshSyncPreferences.png)

[MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest)'s Preferences page
provides easy installation on several DCC tools.  
Alternatively, [Manual Installation](en/Installation.md) is also available.
