# MeshSyncDCCPlugins Usage in 3dsMax

- Installation:
   - Copy MeshSyncClient3dsMax.dlu into the directory for plugin paths.
      - Plugin paths can be added in Max by going to Add under Customize -> Configure User and System Paths -> 3rd Party Plug-Ins
      - The default path (C:\Program Files\Autodesk\3ds Max 2019\Plugins) should be fine, but using a separate path is recommended
- After installing, "UnityMeshSync" will be added to the main menu bar, and the settings window can be opened by clicking "Window".
   - If you change the menu bar, the "UnityMeshSync" category will be added under Action, where MeshSync features can also be accessed
- While "Auto Sync" is checked, changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to sync changes manually.
- Clicking Sync under Animations will cause the timer to advance from the first frame to the final frame while baking the animation before sending it to Unity.
- Clicking "Export Cache" will export all frame data into an *.sc file* for playback in Unity.   
  See the SceneCache feature in [MeshSync's documentation](https://docs.unity3d.com/Packages/com.unity.meshsync@latest) for more details.


&nbsp;

- Modifiers are mostly supported, but there are a few cases where they are not. Use the following rules.
   - When there is no Morph or Skin, all modifiers will be applied during sync.
   - If there is a Morph or Skin, all modifiers before them will be applied during sync.
      - If there are multiple Morphs / Skins, the one at the bottom will be chosen as the base.
   - Morphs and Skins will sync on the Unity side as Blendshapes / Skins.
      - Unity applies them in order of Blendshape -> Skin, so if the order is reversed in Max, unintentional results may occur.
   - If "Bake Modifiers" is checked, the results of applying all deformers will be sent to Unity. This will keep the content of the Mesh mostly consistent between Max and Unity, but will also result in the loss of Skinning and Blendshape information.
   - Checking "Bake Transform" will apply the position/rotation/scale to the vertices of the Mesh
     and reset the Transform to the default value in Unity. This option is only valid when "Bake Modifiers" is enabled
     and will be useful to work around cases where it's difficult to reproduce complex transforms involving pivots in Unity.
   - Check "Use Render Meshes" to extract data from the meshes for rendering. 
     For example, Turbo Smooth will reflect the Iteration for rendering on the Unity side. 
     In addition, this will also correctly reflect meshes that appear only when rendering, such as Fluid, and Space Warps, etc.
- Check "Ignore Non-Renderable" to ignore meshes that can not be rendered. An example of such as mesh is a pyramid-like shape in a bone's viewport display.
