# Installation

1. [Maya](#maya)
1. [Blender](#blender)

## Maya

<img align="right" src="../Images/MeshSyncClientMaya.png" height=400>

1. Due to Autodesk's licensing, [manual building](BuildDCCPlugins.md) is required.
1. Copy the plugins
   - Windows:   
     If `MAYA_APP_DIR` environment variable is setup, copy the *modules* directory there.  
     If not, go to `%USERPROFILE%\Documents\maya` in Windows Explorer, and copy the *modules* directory there.
   - Mac:   
     Copy the *UnityMeshSync* directory and *UnityMeshSync.mod* file to `/Users/Shared/Autodesk/modules/maya`.
   - Linux:  
     Copy the *modules* directory to `~/maya/<maya_version)`
  
  
1. Start Maya, then go to Windows -> Settings/Preferences -> Plug-in Manager.
1. Activate MeshSync DCC plugin by checking Loaded under *MeshSyncClient*.
1. Notice that *UnityMeshSync* should now be available on the shelf UI.  
  Click on the gear icon to open the settings menu and start playing around with MeshSync.
  
## Blender
  
1. Download Blender plugin that corresponds to the used MeshSync version on the [release page](https://github.com/Unity-Technologies/MeshSyncDCCPlugin/releases).
1. Depending on the Blender version, the installation is a bit different.
   - Blender 2.79:
     * Go to File -> User Preferences -> Add-ons
     * Click "Install Add-on from file" at the bottom of the window, and select the plugin zip file.
     * Confirm that "Import-Export: Unity Mesh Sync" is added to the menu.
   - Blender 2.80 or after:
     * Go to Edit -> User Preferences
     * Click "Install" at the top right of the window, and select the plugin zip file.
     * Confirm that "Import-Export: Unity Mesh Sync" is added to the Add-ons tab.
     
     ![MeshSyncClientBlender_Installation](../Images/MeshSyncClientBlender_Installation.png)

1. Check "Import-Export: Unity Mesh Sync" to enable it.
1. Confirm that MeshSync panel is added.
  
![MeshSyncClientBlender](../Images/MeshSyncClientBlender.png)


> If the plugin for the used MeshSync version can't be found on the [release page](https://github.com/Unity-Technologies/MeshSyncDCCPlugin/releases), 
  please refer to [the manual building page](BuildDCCPlugins.md) to build manually.

### Caveat

When installing a Blender plugin, the older version has to be uninstalled first if it exists.   
Steps to uninstall:

1. Select "Import-Export: Unity Mesh Sync" from the Add-ons menu.
1. Click "Remove" button.
1. Restart Blender. 

