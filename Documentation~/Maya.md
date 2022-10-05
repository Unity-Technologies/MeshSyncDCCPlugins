# MeshSyncDCCPlugins Usage in Maya

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971860-7f6d1330-573e-11e8-9a1e-9d95709cbd50.png" height=400>

- Start Maya, then go to Windows -> Settings/Preferences -> Plug-in Manager, and activate the plugin by checking Loaded under MeshSyncClient.
- Confirm that the UnityMeshSync shelf is added, and click on the gear icon to open the settings menu.
- While "Auto Sync" is checked, any edits to the mesh will automatically be reflected in Unity. When Auto Sync is deactivated, click the  "Manual Sync" button to sync changes.
- Clicking Sync under Animations causes the timer to advance from the first frame to the final frame while baking the animation and sending it to Unity.
- Clicking "Export Cache" will export all frame data into an *.sc file* for playback in Unity.   
  See the SceneCache feature in [MeshSync's documentation](https://docs.unity3d.com/Packages/com.unity.meshsync@latest) for more details.
- The other buttons correspond to their respective manual sync and animation sync functions.

&nbsp;

- Polygon mesh will carry skinning/bones (SkinCluster) and BlendShapes over to Unity as is.
   - MeshSync will attempt to apply any additional deformers, but if there is a SkinCluster before or after them they may not apply correctly.
   - Check "Bake Deformers" to sync the results of applying all deformers. This will mostly sync the Mesh on both the Maya and Unity sides, but this will result in loss of Skinning and BlendShape information.
   - Checking "Bake Transform" will apply the position/rotation/scale to the vertices of the Mesh 
     and reset the Transform to the default value in Unity. This option is only valid when "Bake Deformers" is enabled 
     and will be useful to work around cases where it's difficult to reproduce complex transforms involving pivots in Unity. 
        
- Instancing is supported, but instancing for skinned meshes is currently not supported (on the Unity side they all end up in the same position as the original instance).

## Maya LT

Currently, only Windows is supported, and the tool is confirmed to work on Maya LT 2019 + Windows. Maya LT does not natively support outside plugins, so be aware that this may lead to problems. Even small version changes to Maya LT may lead to loss of compatibility.   
This is a separate package, but the process for installation and use is the same as [Non-LT Maya](#maya).

