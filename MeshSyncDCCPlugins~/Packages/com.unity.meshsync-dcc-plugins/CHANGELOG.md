# Changelog

## [0.14.0-preview] - 2022-07-26

TBD

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

