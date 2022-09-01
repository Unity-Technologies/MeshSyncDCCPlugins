# MeshSyncDCCPlugins Usage in Metasequioa

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
