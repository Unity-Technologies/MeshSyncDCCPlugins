# Installation

1. [Maya](#maya)
1. [3ds Max](#3ds-max)
1. [MotionBuilder](#motionbuilder)
1. [Blender](#blender)
1. [Metasequoia](#metasequoia)
1. [Modo](#modo)

## Maya

<img align="right" src="../Images/MeshSyncClientMaya.png" height=400>

1. Get the latest Maya plugin with the same MeshSync version on the [release page](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases).  
   If it doesn't exist, please try [manual building](BuildDCCPlugins.md) or contact us by [creating a new issue](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/issues/new).
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
  
## 3ds Max

<img align="right" src="../Images/MeshSyncClient3dsMax.png" height=400>

1. Get the latest 3ds Max plugin with the same MeshSync version on the [release page](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases).  
   If it doesn't exist, please try [manual building](BuildDCCPlugins.md) or contact us by [creating a new issue](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/issues/new).
1. Start 3ds Max
1. Copy *MeshSyncClient3dsMax.dlu* that corresponds to the used version of 3ds Max into 
   one of the following directories:
   - **(Recommended)** A custom directory for plugins that has been added in 3ds Max by the following menu:
     * 3ds Max 2019 and earlier: Customize -> Configure System Paths. Then select *3rd Party Plug-Ins* tab.
     * 3ds Max 2020: Customize -> Configure User and System Paths. Then select *3rd Party Plug-Ins* tab.
   - The plugin path under the installation directory, e.g: `C:\Program Files\Autodesk\3ds Max 2019\Plugins`
1. Restart 3ds Max 
1. Confirm that "UnityMeshSync" has been added to the main menu bar.
   The settings window can be opened by clicking "Window". 
   > If we change the menu bar, "UnityMeshSync" will be added under Action, which 
   > still allows us to access MeshSync features.

## MotionBuilder

<img align="right" src="../Images/MeshSyncClientMotionBuilder.png" height=240>

1. Get the latest MotionBuilder plugin with the same MeshSync version on the [release page](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases).  
   If it doesn't exist, please try [manual building](BuildDCCPlugins.md) or contact us by [creating a new issue](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/issues/new).
1. Start MotionBuilder.
1. Add a path for custom plugins by clicking on Settings -> Preferences -> SDK menu.
1. Copy *MeshSyncClientMotionBuilder.dll* that corresponds to the used version of MotionBuilder into 
   the custom plugin path.
1. Restart MotionBuilder.
1. Confirm that "UnityMeshSync" is added in the AssetBrowser under Templates -> Devices folder.
1. Add "UnityMeshSync" to the scene.
1. Start playing around with various MeshSync settings and features by 
   selecting Devices -> UnityMeshSync in the Navigator.


## Blender
  
1. Get the latest Blender plugin with the same MeshSync version on the [release page](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases).  
   If it doesn't exist, please try [manual building](BuildDCCPlugins.md) or contact us by [creating a new issue](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/issues/new).
1. Start Blender
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


### Caveat

When installing a Blender plugin, the older version has to be uninstalled first if it exists.   
Steps to uninstall:

1. Select "Import-Export: Unity Mesh Sync" from the Add-ons menu.
1. Click "Remove" button.
1. Restart Blender. 
1. On Mac, delete the installed plugin files in the following folder:
    ``` 
    ~/Library/ApplicationSupport/Blender/2.83/scripts/addons/MeshSyncClientBlender
    ``` 


## Metasequoia
  
1. Get the latest Metasequoia plugin zip file with the same MeshSync version on the [release page](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/releases).  
   If it doesn't exist, please try [manual building](BuildDCCPlugins.md) or contact us by [creating a new issue](https://github.com/Unity-Technologies/MeshSyncDCCPlugins/issues/new).
1. Extract the zip file.   
1. Start Metasequoia.
1. Go to Help -> About Plug-ins, and click "Install" at the lower left of the Plug-win window.
1. Select the plugin file from the extracted zip file that corresponds to the version of Metasequoia.  
   For example: *MeshSyncClientMQ472.plugin*
1. Go to Panel -> Unity Mesh Sync.  
   If this menu doesn't exist, confirm that "Unity Mesh Sync" is checked in the Plug-in window.

![MeshSyncClientMQ](../Images/MeshSyncClientMQ.png)


## Modo

1. Due to Modo's licensing, [manual building](BuildDCCPlugins.md) is required at the moment.
1. Start Modo.
3. Go to System -> Add Plug-in, and select *MeshSyncClientModo.fx* for the applicable Modo version.
4. Go to Modo's tabs UI, click on *+*, and choose Application -> Custom View -> UnityMeshSync.
1. Confirm that UnityMeshSync view is added in the tabs UI.

![MeshSyncClientModo](../Images/MeshSyncClientModo.png)

### Caveat

When installing a newer version of the plugin, the existing version must be removed first if it exists.  
Steps to remove:
1. Close all UnityMeshSync views.
1. Restart Modo.

If installing a newer version still fails even after doing the above steps, 
then please try to remove manually after closing Modo.
1. Windows:
   - Open settings file using a text editor.  
     Example: `C:\Users\<user_name>\AppData\Roaming\Luxology\MODO<modo_version>.CFG`
   - Remove UnityMeshSync items like the following:
     ```
     <hash type="Server" key="12345678-abcd-1234-abcd-111122223333:UnityMeshSync">
       <atom type="Module">C:\ModoPlugins\Modo14\MeshSyncClientModo.lx</atom>
       <atom type="Direct">1</atom>
     </hash>
     ```
   - Save the settings file, and restart Modo.





