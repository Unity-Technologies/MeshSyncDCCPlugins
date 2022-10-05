# Usage in Metasequioa

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
