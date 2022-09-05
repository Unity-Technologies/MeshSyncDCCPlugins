# MeshSyncDCCPlugins Usage in MotionBuilder

<img align="right" src="https://user-images.githubusercontent.com/1488611/45682175-8a919100-bb7a-11e8-96a1-efe2e28146c3.png" height=200>

- Installation:
   - Copy MeshSyncClientMotionBuilder.dll to the directory registered as a plugin path
      - Plugin paths can be added in MotionBuilder under Settings -> Preferences -> SDK menu
- After installation an object called UnityMeshSync will be added to the Asset Browser under Templates -> Devices, so add it to the scene
- The various settings and features can be accessed in the Navigator by selecting Devices -> UnityMeshSync
- While "Auto Sync" is checked, any changes to the Mesh will automatically be reflected in Unity. If Auto Sync is disabled, the "Manual Sync" button can be used to manually reflect changes
- Clicking Sync under Animations causes the timer to advance from the first frame to the final frame while baking the animation before sending it to Unity.

&nbsp;

- The Polygon mesh's skinning/bone and BlendShapes will be carried over to Unity unchanged.

