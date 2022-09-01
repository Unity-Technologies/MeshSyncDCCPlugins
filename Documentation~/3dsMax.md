# MeshSyncDCCPlugins Usage in 3dsMax

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
