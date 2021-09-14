![demo](Documentation~/images/Demo.gif)

# Other Languages
- [日本語](Readme_JP.md)

# MeshSync DCC Plugins

[![](https://badge-proxy.cds.internal.unity3d.com/b681f940-bd27-45c9-832f-e87e6282aa9f)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/build-info?branch=dev&testWorkflow=package-isolation)
[![](https://badge-proxy.cds.internal.unity3d.com/6b18f37e-3925-4b8d-a243-2582b0077f47)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/dependencies-info?branch=dev&testWorkflow=updated-dependencies)
[![](https://badge-proxy.cds.internal.unity3d.com/0db3f8f4-b2d4-40f8-b08a-0b65bcc02245)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/dependants-info)
[![](https://badge-proxy.cds.internal.unity3d.com/1c9fbd13-0736-40ed-96d5-89f237ce91ca)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/warnings-info?branch=dev)

![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/2e9a8300-389b-47be-9806-246c5121830b)
![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/6f4a7e27-d53f-4ad3-bef5-9d3961bb68fb)

This repository provides the source to build DCC plugins for [MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest), 
which  is a package for synchronizing meshes/models editing in DCC tools into Unity in real time.  
This allows devs to immediately see how things will look in-game while modelling.  

## Features

|                     | Maya                 | 3ds Max              | MotionBuilder        | Blender              | Modo                 | Metasequoia          | 
| --------------------| -------------------- | -------------------- | -------------------- | -------------------- | -------------------- | -------------------- | 
| Polygon mesh sync   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| Camera sync         | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| Light sync          | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| Double-sided Mesh   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| Negative Scale      | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: |                      |
| Multi UV            |                      | :heavy_check_mark:   |                      | :heavy_check_mark:   |                      |                      |  
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
| Maya 2017           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya 2018           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya 2019           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya 2020           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya 2022           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya LT 2019 +      | :heavy_check_mark: |                    | :x:                | 
| 3ds Max 2017        | :heavy_check_mark: | :x:                | :x:                | 
| 3ds Max 2018        | :heavy_check_mark: | :x:                | :x:                | 
| 3ds Max 2019        | :heavy_check_mark: | :x:                | :x:                | 
| 3ds Max 2020        | :heavy_check_mark: | :x:                | :x:                | 
| 3ds Max 2021        | :heavy_check_mark: | :x:                | :x:                | 
| MotionBuilder 2017  | :heavy_check_mark: | :x:                | :heavy_check_mark: | 
| MotionBuilder 2018  | :heavy_check_mark: | :x:                | :heavy_check_mark: | 
| MotionBuilder 2019  | :heavy_check_mark: | :x:                | :heavy_check_mark: | 
| MotionBuilder 2020  | :heavy_check_mark: | :x:                | :heavy_check_mark: | 
| Blender 2.83        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Blender 2.90        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Blender 2.91        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Blender 2.92        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Blender 2.93        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Modo 12             | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Modo 13             | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Metasequoia 4.x     | :heavy_check_mark: | :heavy_check_mark: |                    | 

Notes:
* :heavy_check_mark: : Supported
* :x: : Impossible to support (platform unsupported by the DCC, etc)
* empty : May be supported in the future
  

# DCC Plugin Installation

![MeshSyncPreferences](Documentation~/images/MeshSyncPreferences.png)

[MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest)'s Preferences page
provides easy installation on several DCC tools.  
Alternatively, [Manual Installation](Documentation~/en/Installation.md) is also available.

# Building
- [Building DCC Plugins](Plugins~/Docs/en/BuildDCCPlugins.md)

# License
- [License](LICENSE.md)
- [Third Party Notices](Third%20Party%20Notices.md)

# Currently being reorganized

This document is currently being reorganized.
The previous version of the document is put below for reference.


## Guides
1. [Maya](#maya)
2. [Maya LT](#maya-lt)
3. [3ds Max](#3ds-max)
4. [MotionBuilder](#motionbuilder)
5. [Blender](#blender)
6. [Modo](#modo)
7. [Metasequoia](#Metaseq)
8. [Unity](#unity)

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971860-7f6d1330-573e-11e8-9a1e-9d95709cbd50.png" height=400>

### Maya
Confirmed functionality with Maya 2015, 2016, 2016.5, 2017, 2018, 2019 + Windows, Mac, and Linux (CentOS 7).
- Installation:
  - Download UnityMeshSync_Maya_*.zip from [releases](https://github.com/unity3d-jp/MeshSync/releases).
  - Windows: If %MAYA_APP_DIR% is already setup, copy the modules directory there, if not copy it to %USERPROFILE%\Documents\maya (← copy paste into the Explorer address bar).
    - For versions prior to 2016, copy to the version name directory (%MAYA_APP_DIR%\2016 etc.)
  - Mac: Copy the UnityMeshSync directory and .mod file to /Users/Shared/Autodesk/modules/maya.
  - Linux: Copy the modules directory to ~/maya/(Maya version).
- Start Maya, then go to Windows -> Settings/Preferences -> Plug-in Manager, and activate the plugin by checking Loaded under MeshSyncClient.
- Now that the UnityMeshSync shelf has been added, click on the gear icon to open the settings menu. 
- While "Auto Sync" is checked, any edits to the mesh will automatically be reflected in Unity. When Auto Sync is deactivated, click the  "Manual Sync" button to sync changes.
- Clicking Sync under Animations causes the timer to advance from the first frame to the final frame while baking the animation and sending it to Unity. 
- The other buttons correspond to their respective manual sync and animation sync functions. 

&nbsp;  

- Polygon mesh will carry skinning/bones (SkinCluster) and BlendShapes over to Unity as is.
  - MeshSync will attempt to apply any additional deformers, but if there is a SkinCluster before or after them they may not apply correctly. 
  - Check "Bake Deformers" to sync the results of applying all deformers. This will mostly sync the Mesh on both the Maya and Unity sides, but this will result in loss of Skinning and BlendShape information.
- Instancing is supported, but instancing for skinned meshes is currently not supported (on the Unity side they all end up in the same position as the original instance). 
- Commands are also registered to MEL, and all features can be accessed through MEL. See [the source code](https://github.com/unity3d-jp/MeshSync/blob/master/.MeshSync/Plugin/MeshSyncClientMaya/msmayaCommands.cpp) for details.


### Maya LT
Currently, only Windows is supported, and the tool is confirmed to work on Maya LT 2019 + Windows. Maya LT does not natively support outside plugins, so be aware that this may lead to problems. Even small version changes to Maya LT may lead to loss of compatibility.   
This is a separate package, but the process for installation and use is the same as [Non-LT Maya](#maya).


### 3ds Max
Confirmed functionality with 3ds Max 2016, 2017, 2018, 2019, 2020 + Windows.
- Installation: 
  - Copy MeshSyncClient3dsMax.dlu into the directory for plugin paths.
    - Plugin paths can be added in Max by going to Add under Customize -> Configure User and System Paths -> 3rd Party Plug-Ins
    - The default path (C:\Program Files\Autodesk\3ds Max 2019\Plugins) should be fine, but using a separate path is recommended
- After installing, "UnityMeshSync" will be added to the main menu bar, and the settings window can be opened by clicking "Window". 
  - If you change the menu bar, the "UnityMeshSync" category will be added under Action, where MeshSync features can also be accessed
- While "Auto Sync" is checked, changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to sync changes manually.  
- Clicking Sync under Animations will cause the timer to advance from the first frame to the final frame while baking the animation before sending it to Unity.

&nbsp;  

- Modifiers are mostly supported, but there are a few cases where they are not. Use the following rules.  
  - When there is no Morph or Skin, all modifiers will be applied during sync. 
  - If there is a Morph or Skin, all modifiers before them will be applied during sync.  
    - If there are multiple Morphs / Skins, the one at the bottom will be chosen as the base.
  - Morphs and Skins will sync on the Unity side as Blendshapes / Skins.
    - Unity applies them in order of Blendshape -> Skin, so if the order is reversed in Max, unintentional results may occur.
  - If "Bake Deformers" is checked, the results of applying all deformers will be sent to Unity. This will keep the content of the Mesh mostly consistent between Max and Unity, but will also result in the loss of Skinning and Blendshape information.
- Commands have also been added to the Max script, so all features can be accessed via the Max script. See [the source code](https://github.com/unity3d-jp/MeshSync/blob/master/.MeshSync/Plugin/MeshSyncClient3dsMax/msmaxEntryPoint.cpp) for details. 


<img align="right" src="https://user-images.githubusercontent.com/1488611/45682175-8a919100-bb7a-11e8-96a1-efe2e28146c3.png" height=200>

### MotionBuilder
Confirmed functionality with MotionBuilder 2015, 2016, 2017, 2018, 2019 + Windows, Linux (CentOS 7) 
- Installation:
  - Copy MeshSyncClientMotionBuilder.dll to the directory registered as a plugin path 
    - Plugin paths can be added in MotionBuilder under Settings -> Preferences -> SDK menu
- After installation an object called UnityMeshSync will be added to the Asset Browser under Templates -> Devices, so add it to the scene  
- The various settings and features can be accessed in the Navigator by selecting Devices -> UnityMeshSync 
- While "Auto Sync" is checked, any changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to manually reflect changes  
- Clicking Sync under Animations causes the timer to advance from the first frame to the final frame while baking the animation before sending it to Unity.  

&nbsp;  

- The Polygon mesh's skinning/bone and BlendShapes will be carried over to Unity unchanged. 


<img align="right" src="https://user-images.githubusercontent.com/1488611/49272332-79d39480-f4b4-11e8-8ca3-0ce0bc90a965.png" height=400>

### Blender
Functionality confirmed with Blender 2.79(a,b), 2.80 beta (2019-4-23) + Windows, Mac, Linux (CentOS 7). Be aware that depending on the implementation, **there is a high possibility that upgrading the Blender version will lead to a loss of compatibility**. Be especially careful when upgrading to the popular 2.8 versions. A supported version will be released when issues become known.
- Installation: 
  - In Blender, go to File -> User Preferences -> Add-ons (2.80 and after: Edit -> User Preferences), click "Install Add-on from file" at the bottom of the screen, and select the plugin zip file. 
  - **If an older version is already installed, it must be deleted beforehand**. Select "Import-Export: Unity Mesh Sync" from the Add-ons menu, **restart Blender after removing the older version** then follow the above steps.
- "Import-Export: Unity Mesh Sync" will be added to the menu, so select it to enable it.
- The MeshSync panel will also be added, where settings and manual sync can be accessed. 
  - The panel's location can be difficult to find in 2.8 versions. Use the screenshot to the right for reference.
- When "Auto Sync" is selected, changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, use the "Manual Sync" button to sync changes. 
- Pressing the Animations Sync button will cause the timer to advance from the first frame to the final frame while baking the animation, then send it to Unity. 

&nbsp;  

- The polygon mesh's skinning/bone (Armature) and Blendshape will be sent to Unity unchanged. Mirror deformers are also supported. Other deformers will be ignored. 
  - Check "Bake Modifiers" to sync the results of applying all modifiers. This will make the Mesh content mostly consistent between  Blender and Unity, but will also result in the loss of Skinning and Blendshape information.  
- Use "Convert To Mesh" to convert objects such as Nurbs into polygons, if they are able to, then sync. 


### Modo

  <img src="https://user-images.githubusercontent.com/1488611/55697991-d9135980-59fe-11e9-8e9f-8fcfba1b234f.png" height=300><img src="https://user-images.githubusercontent.com/1488611/55697990-d9135980-59fe-11e9-9312-29c95e20e5b0.png" height=300>

  Functionality confirmed with Modo 10, 12, 13 + Windows, Mac, Linux (CentOS 7).
  - Installation:
    - Designate MeshSyncClientModo.fx in Modo under System -> Add Plug-in
  - After installing, View will be added to the menu (Application -> Custom View -> UnityMeshSync), where varous options and settings can be accessed 
  - While "Auto Sync" is checked, changes made to the mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to sync changes
  - Clicking Sync under Animations will cause the timer to advance from the first frame to the final frame while baking the animation and sending it to Unity. 

  &nbsp;

  - Portions of Mesh Instance and Replicator are also supported.
  - Polygon mesh Skinning/Joints and Morph will carry over to Unity, but be aware of how deformers are handled.
    - MeshSync can only handle Joint + Weight Map skinning, or Morph deformers. Any other deformers will be ignored.
    - Checking "Bake Deformers" will send the results of applying all deformers to Unity. This will mostly synchronize the Mesh on the Unity side even with complex deformer compositions, but comes at the cost of losing skinning and Morph/Blendshape information. 
    - Mesh Instance and Replicator skinning won't display properly in Unity. "Bake Deformers" must be used.
  - MeshSync features can also be accessed via commands. Use unity.meshsync.settings to change settings, and unity.meshsync.export to export

  &nbsp;

As of Modo 13, the  [Mood Bridge for Unity](https://learn.foundry.com/modo/content/help/pages/appendices/modo_bridge.html) feature is available. This feature allows you to send Meshes and Materials directly to Unity.It has elements that are similar to MeshSync's features, with the following differences (as of 04/2019). 
  - Mood Bridge supports Modo <-> Unity sync in both directions. MeshSync only supports Modo -> Unity sync.
  - MeshSync can sync Replicator and Mesh Skinning/Morphs, and animations. Currently, Mood Bridge cannot.
  - MeshSync attempts to replicate the results of bringing data into Unity via FBX as much as possible. On the other hand Modo Bridge has big differences such as using a different coordinate system (Z direction is reversed), decompressing the Mesh index (a model with 1,000 triangles will have 3,000 vertices), etc.   


### Metasequoia
Supported in Windows for version 3 and 4 (32bit & 64bit) and Mac (version 4 only). All 3 versions are probably supported, but 4 versions must be 4.6.4 or later (bone output is not supported for earlier versions). 
Also, dll is different in version 4.7 and later. This is due to changes to the bone system after 4.7 which lead to a loss of plugin compatibility. Morph output is also supported in 4.7 and later. 
- Installation:
  - Go to Help -> About Plug-ins in Metasequoia, and select the plugin file under "Install" in the lower left of the dialogue. It's a Station plugin type. 
  - **If older versions are already installed, remove them manually before hand**. Delete the appropriate files before starting Metasequoia. 
- Panel -> Unity Mesh Sync will be added after installation, open this and check "Auto Sync".
- While "Auto Sync" is checked, changes to the mesh will automatically be reflected in Unity. If Auto Sync is disabled, use the "Manual Sync" button to sync changes. 
- Checking "Sync Camera" will sync the camera in Metasequoia. "Camera Path" is the camera path in Unity.
- Clicking "Import Unity Scene" will import the currently open Unity scene. Changes made to the scene can be reflected in real time. 

&nbsp;

- Mirroring and smooting will be reflected in Unity.
  - However, "reflective surfaces where the left and right are connected" type mirroring is not supported.
- Hidden objects in Metasequoia will also be hidden in Unity. Mesh details for hidden objects will not be sent to Unity, so when the number of objects in a scene makes sync heavy, making them hidden as appropriate should also speed up the sync process.  
- Materials will not be reflected in Unity, but they will be split into appropriate sub-meshes depending on the Material ID. 
- Subdivisions and metaballs will not be reflected in Unity until you freeze them. 
- Check "Sync Normals" to reflect vector changes supported by Metasequoia 4 versions. 
- Check "Sync Bones" to reflect bones supported by Metasequoia 4 versions. Checking "Sync Poses" will reflect the pose designated under "Skinning". 



##  Related
- [NormalPainter](https://github.com/unity3d-jp/NormalPainter): Tool for editing vectors in Unity
- [BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Tool for building BlendShapes in Unity

