# Changelog

## [0.18.0-preview] - 2023-08-01

### Added
* feat: Blender 3.5 and Blender 3.6.2 support
* feat: Improved instancing support

### Changed
* change: instances are now sent in a way that allows the same object to be instanced on multiple parents and keep that hierarchy in Unity

### Fixed
* fix: various instancing fixes

## [0.17.1-preview] - 2023-06-13

### Added
* blender-feat: adding option to only sync selected objects. 
* blender-feat: handle node group for materials
* doc: mention support for Blender 3.4 

### Changed
* doc: remove Modo and Metasequoia sections

### Fixed

* blender-fix: Spline crash fix in Blender 3.3+ 
* blender-fix: material index crash fix 
* blender-fix: fix for race condition when applying properties 
* blender-fix: crash fix when removing an empty instance object 
* blender-fix: fix for debug logs and uninitialized colorValue variable
* blender-fix: apply mirror Modifier for instances when applicable 
* blender-fix: null reference check in visible_in_collection() 
* fix: use path provider to ensure instances use their correct path for properties 
* blender-fix: fix for image paths coming from libraries
* doc-blender: fix anchor links (lower case) 

## [0.17.0-preview] - 2023-02-07

### Added
* Blender 3.4 support
* blender-feat: Material baking
* blender-feat: displacement maps baking
* blender-feat: AO baking for materials
* blender-feat: implementing orthographic size for camera
* blender-feat: Adding texel density setting for baking

### Changed
* blender-change: bake modifiers in Edit Mode
* blender: Making deduplication of meshes optional 
* blender-opt: optimisation to avoid making unique objects when the source object is the same.
* blender-doc: update docs to include all new material baking settings 
* blender: use all available UV layers in Edit mode  
* blender: updated UI to show materials panel. Reordered panels and removed Unity Project panel 
* blender: added cancel and reset modes to increment progress. Used reset mode when resetting and cancel mode when cancelling
* doc: update Table of contents with Blender Installer page

### Fixed
* blender-fix: export normals correctly in edit mode, taking polygon smooth into account 
* blender-fix: dangling pointers on Mesh data
* blender-fix: avoid copies of linked duplicates 
* blender-fix: fix for materials not referencing correctly and smoothness map being roughness.
* blender-fix for crash on empty material slots.
* blender-fix: Fix for crashes when instantiating curves through geonodes
* blender: geometry node fixes 

## [0.16.0-preview] - 2022-11-25

### Added
* blender-feat: material support 
* blender-feat: support exporting empty objects

### Changed
* blender-doc: update installation docs 

### Fixed
* blender-fix: make all object types (incl. lights) instantiable for geometry nodes 
* blender-fix: fix for exporting keyframes in 3.3.0 and above 
* blender-fix: null check for geometry node properties

### Removed
* blender: removed editor path property from project panel

## [0.15.2-preview] - 2022-11-16

### Added
* blender: preferences UI for auto-detecting the hub and the editors folder
* blender: use hub to select or create project 

### Changed
* blender-change: Handled local packages 

### Fixed

* fix: fix for crash when exporting baked transforms
* blender-fix: fix for broken uvs on edit mode 
* blender-fix: get latest compatible version of MeshSync package in AutoSetup
* blender-fix: More reliable way to check if project is already running 
* blender-fix: give feedback when selecting project
* blender-fix: give feedback for project creation  

## [0.15.1-preview] - 2022-10-07

### Added
* blender-feat: add Add-on Preferences file
* blender-doc: add Unity project panel documentation

### Changed
* doc: update Installation page (TOC, typo fixes) and add missing English docs in DCC pages
* doc: put Unity setup via DCC section below in the Installation page

### Fixed
* blender-fix: avoid overwriting manifest package and report error if installed package is not supported
* blender-fix: do not reset Server port if server is available
* blender-fix: use default ports if possible in auto-config
* blender-fix: fix for crash in blender 3.3 

### Removed
* doc: remove obsolete installation steps in DCC pages
* doc: remove Japanese documentation. Moved to the official page

## [0.15.0-preview] - 2022-09-29

### Added
* blender-feat: support Blender 3.3.0
* blender-feat: Editor commands in Blender clients 
* blender-feat: install MeshSync from Blender
* blender-feat: auto-config of ports for AutoSetup via Blender
* blender-doc: add doc for installing MeshSync via Blender

### Changed
* change: change plugins folder to Plugins to reveal it in the Unity Editor
* doc: organize usage in DCC tools into separate pages
* doc: reorganize and update installation documentation
* blender-doc: update the section about BakeModifiers 
* blender-doc: add Bidirectional Sync section in the Blender documentation 

### Fixed
* blender-fix: Scale instance transforms with meshsync scale factor to ensure correct positioning. 
* blender-fix: fix instance scaling
* blender-fix: set camera visibility based on the active scene camera
* blender-fix: export geometry node instances when frame changes
* blender-fix: fix for material index bug 


## [0.14.1-preview] - 2022-07-29

### Fixed
* maya-fix: load plugin for Maya 2023 properly 
* 3dsmax-fix: export vertex alpha 
* blender-fix: fix to export curves 

## [0.14.0-preview] - 2022-07-26

### Added
* blender-feat: support for Blender 3.2.0 
* blender-feat: send particle instance meshes
* blender-feat: Unity->Blender communication
* maya-feat: support for Maya 2023

### Changed
* change: set minimum Unity requirement to 2020.3
* blender-change: convert sun light energy to Unity light intensity directly
* licensed several components under GPL v3

### Fixed
* blender-fix: ensure layers that are hidden for render in blender, are also hidden in unity
* blender-fix: Fix for accessing deform groups in Blender 3.0 and above
* blender-fix: Blender settings apply differently depending on the order of using Manual Sync vs Animation Sync 
* blender-fix: fix for calllback getting registered multiple times. Ignore updates when not active
* blender-fix: ceil floats to avoid hashing issues 

### Removed
* blender-change: drop support for Blender 2.83


## [0.13.1-preview] - 2022-05-31

### Fixed
* blender-fix: do not handle already handled records when extracting geometry node instances
* 3dsmax-fix: consider "Enable in Renderer" option for syncing with "Ignore Non-Renderable" set to on 

## [0.13.0-preview] - 2022-04-26

### Added
* blender: Geometry nodes support
* 3ds Max: 3ds Max 2022, 2023+ compatibility 

### Changed
* chore: ensure to build Mac plugins targeting 10.15 at minimum

### Fixed
* blender: consider collection visibility for syncing 

## [0.12.1-preview] - 2022-04-06

### Fixed
* blender: could not export SceneCache in Blender 3.1

## [0.12.0-preview] - 2022-03-18

### Added
* blender: add support for Blender 3.1 

### Changed
* blender: use the appropriate Python version for building different versions of Blender plugins
* chore: update code to adhere to MeshSync@0.12.x-preview

### Fixed
* blender: fix auto sync status when file is loaded 

## [0.11.0-preview] - 2022-02-08

### Added
* feat: blender3.0 support 
* blender: show a message box after exporting a SceneCache file

### Changed
* blender: export only the materials of selected objects during scene cache export

## [0.10.1-preview] - 2022-01-27

### Changed
* blender: use 2.83.18, 2.93.7 for building 

### Fixed
* fix: Blender plugin wouldn't install on Mac for Blender 2.93.x

## [0.10.0-preview] - 2021-11-11

### Added
* support com.unity.meshsync@0.10.x-preview

### Removed
* remove support for 3dsMax 2017 and Maya 2017

## [0.9.0-preview] - 2021-09-15

### Added
* support com.unity.meshsync@0.9.x-preview
* feat: add support for Maya 2022 
* feat: support for 3ds Max 2021

### Changed
* blender: optimize setting the frame back after exporting scene cache

### Fixed
* fix: show the correct plugin version for DCC plugins

## [0.8.1-preview] - 2021-09-9

### Added
* feat: multi UV for 3ds Max 

### Changed
* blender: use 2.83.17, 2.93.4 for building

## [0.8.0-preview] - 2021-07-22

### Added
* support com.unity.meshsync@0.8.x-preview

### Changed
* blender: update supported versions. 2.83.16, 2.91.2, 2.92.0, 2.93.1
* opt: optimize SetFrame in Blender plugin

### Fixed
* fix: compile errors when building against com.unity.meshsync@0.8.0-preview 

### Removed
* blender: remove support for Blender 2.79 - 2.82 

## [0.7.1-preview] - 2021-02-09

### Changed
* log output after exporting scene cache file 
* automatically add sc extension when exporting SceneCache

## [0.7.0-preview] - 2021-02-03

### Added
* support com.unity.meshsync@0.7.x-preview

## [0.6.0-preview] - 2020-12-15

### Added
* support Blender 2.91.0 and 2.83.10 

## [0.5.4-preview] - 2020-11-25

### Fixed
* maya: fix plugin for Maya 2017 (initializePlugin failed)
* fix: use com.unity.meshsync@0.5.4-preview with its fixes 

## [0.5.3-preview] - 2020-11-24

### Changed
* blender: update Blender support to Blender 2.83.9 and enable Python optimization
* apply changes to the plugin source code in com.unity.meshsync@0.5.3
* update contributing policy according to the license 

### Fixed
* blender: fix Blender plugin causing crash when quitting application on Mac
* fix: update the source of ISPC library and update to use ISPC 1.14.1 
* fix: build pipeline for XCode 12 and update the docs for installing/removing Blender plugin

## [0.4.0-preview] - 2020-09-29

### Added
* blender: add support for Blender 2.90 

### Changed
* modify plugin source code to conform to MeshSync@0.4.0-preview
* update the zip files for MeshSync@0.4.0-preview

### Fixed
* blender: check the number of UV sets in blender before getting the data


## [0.3.3-preview] - 2020-09-08

### Changed
* modify plugin source code to conform to MeshSync@0.3.3-preview
* update the zip files for MeshSync@0.3.3-preview

## [0.3.2-preview] - 2020-09-07

### Added
* blender: add multi UV support for Blender

### Changed
* modify plugin source code to conform to MeshSync@0.3.2-preview
* update the zip files for MeshSync@0.3.2-preview

## [0.2.0-preview] - 2020-06-18

### Added
* blender: add support for Blender 2.83 

### Changed
* Updates the zip files for MeshSync 0.2.0-preview


## [0.0.1-preview] - 2020-04-10

### Added
* The first release of *MeshSyncDCCPlugin*, which contains plugin binaries of DCC tools for using *MeshSync* 0.0.3-preview

