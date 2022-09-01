# MeshSyncDCCPlugins Usage in Blender

![](images/MeshSyncClientBlender.png)

After [installing the plugin](Installation.md), sync controls will be displayed in the MeshSync panel
as can be seen above, where most of the options are self-explanatory.

|**Options** |**Description** |
|:---       |:---|
| **Bake Modifiers**        | An option to sync the results after applying all modifiers. This will make the content mostly consistent between Blender and Unity, but will also result in the loss of mesh properties, such as skinning and blend shapes.|


|**Buttons** |**Description** |
|:---       |:---|
| **Auto Sync**             | A toggle that will automatically reflect mesh changes to Unity.|
| **Manual Sync**           | Use the **Manual Sync** button to reflect mesh changes when **Auto Sync** is inactive.|
| Animation &rarr; **Sync** | Bake animations by advancing the timer from the first frame to the final frame, and then send them to Unity.|
| **Export Cache** | Export into an *.sc* file. Please refer to the SceneCache feature in [MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest).|

The following properties are supported:
1. The polygon mesh's skinning/bone (Armature) 
2. Blend shapes
3. Mirror deformers 


