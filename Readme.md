![demo](Documentation~/images/Demo.gif)

# Latest official docs
- [English](https://docs.unity3d.com/Packages/com.unity.meshsync.dcc-plugins@latest)
- [日本語](https://docs.unity3d.com/ja/Packages/com.unity.meshsync.dcc-plugins@latest)

# MeshSync DCC Plugins

[![](https://badge-proxy.cds.internal.unity3d.com/b681f940-bd27-45c9-832f-e87e6282aa9f)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/build-info?branch=dev&testWorkflow=package-isolation)
[![](https://badge-proxy.cds.internal.unity3d.com/6b18f37e-3925-4b8d-a243-2582b0077f47)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/dependencies-info?branch=dev&testWorkflow=updated-dependencies)
[![](https://badge-proxy.cds.internal.unity3d.com/0db3f8f4-b2d4-40f8-b08a-0b65bcc02245)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/dependants-info)
[![](https://badge-proxy.cds.internal.unity3d.com/1c9fbd13-0736-40ed-96d5-89f237ce91ca)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/warnings-info?branch=dev)

![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/2e9a8300-389b-47be-9806-246c5121830b)
![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/6f4a7e27-d53f-4ad3-bef5-9d3961bb68fb)
# MeshSync DCC Plugins

MeshSync DCC Plugins is package that contains plugin binaries of DCC tools for using [MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest), 
which is another package for synchronizing meshes/models editing in DCC tools into Unity in real time.
This allows devs to immediately see how things will look in-game while modelling.

## Features

|                     | Maya                 | 3ds Max              | MotionBuilder        | Blender              |
| --------------------| -------------------- | -------------------- | -------------------- | -------------------- |
| Polygon mesh sync   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |
| Camera sync         | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |
| Light sync          | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |
| Double-sided Mesh   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |
| Negative Scale      | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: |
| Multi UV            |                      | :heavy_check_mark:   |                      | :heavy_check_mark:   |
| Scene Cache Export  | :heavy_check_mark:   | :heavy_check_mark:   |                      | :heavy_check_mark:   |
| Non-polygon shape   |                      |                      |                      |                      |
| Bidirectional Sync  |                      |                      |                      | :heavy_check_mark:   |

### Caveats

* Negative Scale: partially supported on some DCC Tools.  
  If all XYZ values have negative values, the mesh will sync properly, however if only one axis has a negative value,
  Unity will treat the mesh as though every axis has a negative value.
  Certain DCC tools may have *Bake Transform* option which can sync the mesh in this case, but it will lose any 
  deformer information.
  

## Supported DCC Tools

|                    | Windows            | Mac                | Linux              |
|--------------------| ------------------ | ------------------ |------------------- |
| Maya 2018          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya 2019          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya 2020          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya 2022          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya 2023          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya LT 2019 +     | :heavy_check_mark: |                    | :x:                |
| 3ds Max 2018       | :heavy_check_mark: | :x:                | :x:                |
| 3ds Max 2019       | :heavy_check_mark: | :x:                | :x:                |
| 3ds Max 2020       | :heavy_check_mark: | :x:                | :x:                |
| 3ds Max 2021       | :heavy_check_mark: | :x:                | :x:                |
| 3ds Max 2022       | :heavy_check_mark: | :x:                | :x:                |
| 3ds Max 2023       | :heavy_check_mark: | :x:                | :x:                |
| MotionBuilder 2017 | :heavy_check_mark: | :x:                | :heavy_check_mark: |
| MotionBuilder 2018 | :heavy_check_mark: | :x:                | :heavy_check_mark: |
| MotionBuilder 2019 | :heavy_check_mark: | :x:                | :heavy_check_mark: |
| MotionBuilder 2020 | :heavy_check_mark: | :x:                | :heavy_check_mark: |
| Blender 2.90       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 2.91       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 2.92       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 2.93       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 3.0        | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 3.1        | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 3.2        | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 3.3        | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 3.4        | :white_check_mark: | :white_check_mark: | :white_check_mark: |

Notes:
* :white_check_mark: : Supported
* :x: : Impossible to support (platform unsupported by the DCC, etc)
* empty : May be supported in the future

# DCC Plugin Installation

![MeshSyncPreferences](Documentation~/images/MeshSyncPreferences.png)

[MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest)'s Preferences page
provides easy installation on several DCC tools.  
Alternatively, [Manual Installation](Documentation~/Installation.md) is also available.

## Usage in DCC Tools

1. [Maya](Documentation~/Maya.md)
2. [3ds Max](Documentation~/3dsMax.md)
3. [MotionBuilder](Documentation~/MotionBuilder.md)
4. [Blender](Documentation~/Blender.md)
# Building
- [Building DCC Plugins](Plugins~/Docs/en/BuildDCCPlugins.md)

# License
- [License](LICENSE.md)
- [Third Party Notices](Third%20Party%20Notices.md)



*Auto-generated on Mon Jun 12 07:39:34 UTC 2023*
