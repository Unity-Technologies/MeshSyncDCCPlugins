# Installation

1. [Maya](#maya)

<img align="right" src="../Images/MeshSyncClientMaya.png" height=400>

## Maya

* Due to Autodesk's licensing, [manual building](BuildDCCPlugins.md) is required.
* Copy the plugins
  - Windows:   
    If `MAYA_APP_DIR` environment variable is setup, copy the *modules* directory there.  
    If not, go to `%USERPROFILE%\Documents\maya` in Windows Explorer, and copy the *modules* directory there.
  - Mac:   
    Copy the *UnityMeshSync* directory and *UnityMeshSync.mod* file to `/Users/Shared/Autodesk/modules/maya`.
  - Linux: 
    Copy the *modules* directory to `~/maya/<maya_version)`
  
  
- Start Maya, then go to Windows -> Settings/Preferences -> Plug-in Manager.
- Activate MeshSync DCC plugin by checking Loaded under *MeshSyncClient*.
- Notice that *UnityMeshSync* should now be available on the shelf UI.  
  Click on the gear icon to open the settings menu and start playing around with MeshSync.
  
  



