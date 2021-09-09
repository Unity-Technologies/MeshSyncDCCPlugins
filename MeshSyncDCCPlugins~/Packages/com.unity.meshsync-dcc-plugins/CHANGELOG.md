# Changelog

## [0.8.0-preview] - 2020-07-22

### Added
* support com.unity.meshsync@0.8.x-preview

### Changed
* blender: update supported versions. 2.83.16, 2.91.2, 2.92.0, 2.93.1
* opt: optimize SetFrame in Blender plugin

### Fixed
* fix: compile errors when building against com.unity.meshsync@0.8.0-preview 

### Removed
* blender: remove support for Blender 2.79 - 2.82 

## [0.7.1-preview] - 2020-02-09

### Changed
* log output after exporting scene cache file 
* automatically add sc extension when exporting SceneCache

## [0.7.0-preview] - 2020-02-03

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

