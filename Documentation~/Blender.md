# MeshSyncDCCPlugins Usage in Blender


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
