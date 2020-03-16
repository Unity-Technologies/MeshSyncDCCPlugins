# Installation

1. [Maya](#maya)

## Maya

<img align="right" src="../Images/MeshSyncClientMaya.png" height=400>

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
  
## Blender
  
![MeshSyncClientBlender_Installation](../Images/MeshSyncClientBlender_Installation.png)

* Download Blender plugin that corresponds to MeshSync with the same version on the [release page](https://github.com/Unity-Technologies/MeshSyncDCCPlugin/releases) 
* Depending on the Blender version, the way to install is different.
  - Blender 2.79:
    * Go to File -> User Preferences -> Add-ons.
    * Click "Install Add-on from file" at the bottom of the window, and select the plugin zip file
    * "Import-Export: Unity Mesh Sync" will be added to the menu.
  - Blender 2.80 or after:
    * Go to Edit -> User Preferences. 
    * Click "Install" at the top right of the window, and select the plugin zip file. 
    * "Import-Export: Unity Mesh Sync" will be added to the Add-ons tab.
* Check "Import-Export: Unity Mesh Sync"

<img align="right" src="../Images/MeshSyncClientBlender.png" height=400>

> If the plugin can't be found on the release page, please refer to [manual building](BuildDCCPlugins.md) to build manually.

### Caveat

If an older version of the Blender plugin is already installed, it must be deleted before installing a new version. 
To delete:
* Select "Import-Export: Unity Mesh Sync" from the Add-ons menu
* Click "Remove" button
* Restart Blender 

