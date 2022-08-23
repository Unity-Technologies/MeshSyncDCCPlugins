import os
import os.path
from os import path
import platform
import subprocess
import re
import bpy
from bpy.app.handlers import persistent

from bpy_extras.io_utils import ImportHelper
import MeshSyncClientBlender as ms

import addon_utils
import shutil

import json
import time

msb_context = ms.Context()
msb_cache = ms.Cache()


def msb_apply_scene_settings(self = None, context = None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.server_address = scene.meshsync_server_address
    ctx.server_port = scene.meshsync_server_port
    ctx.scale_factor = scene.meshsync_scale_factor
    ctx.sync_meshes = scene.meshsync_sync_meshes
    ctx.curves_as_mesh = scene.meshsync_curves_as_mesh
    ctx.make_double_sided = scene.meshsync_make_double_sided
    ctx.bake_modifiers = scene.meshsync_bake_modifiers
    ctx.bake_transform = scene.meshsync_bake_transform
    ctx.sync_bones = scene.meshsync_sync_bones
    ctx.sync_blendshapes = scene.meshsync_sync_blendshapes
    ctx.sync_textures = scene.meshsync_sync_textures
    ctx.sync_cameras = scene.meshsync_sync_cameras
    ctx.sync_lights = scene.meshsync_sync_lights
    return None

def msb_apply_animation_settings(self = None, context = None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.frame_step = scene.meshsync_frame_step
    return None


def msb_on_scene_settings_updated(self = None, context = None):
    msb_apply_scene_settings()
    if bpy.context.scene.meshsync_auto_sync:
        msb_context.setup(bpy.context)
        msb_context.export(msb_context.TARGET_OBJECTS)
    return None

def msb_on_bake_modifiers_updated(self = None, context = None):
    scene = bpy.context.scene
    if not scene.meshsync_bake_modifiers:
        scene.meshsync_bake_transform = False
    return msb_on_scene_settings_updated(self, context)

def msb_on_bake_transform_updated(self = None, context = None):
    scene = bpy.context.scene
    if scene.meshsync_bake_transform:
        scene.meshsync_bake_modifiers = True
    return msb_on_scene_settings_updated(self, context)

def msb_on_toggle_auto_sync(self = None, context = None):
    msb_apply_scene_settings()
    if bpy.context.scene.meshsync_auto_sync:
        if not msb_context.is_server_available:
            print("MeshSync: " + msb_context.error_message)
            bpy.context.scene.meshsync_auto_sync = False
    if bpy.context.scene.meshsync_auto_sync:
        msb_context.setup(bpy.context)
        msb_context.export(msb_context.TARGET_OBJECTS)
    return None

def msb_on_animation_settings_updated(self = None, context = None):
    # nothing to do for now
    return None

def msb_on_unity_project_path_updated(self = None, context = None):
    #TODO invoke callback for installing meshsync
    return None


def msb_initialize_properties():
    # sync settings
    bpy.types.Scene.meshsync_server_address = bpy.props.StringProperty(name = "Address", default = "127.0.0.1", update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(name = "Port", default = 8080, min = 0, max = 65535, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(name = "Scale Factor", default = 1.0, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_meshes = bpy.props.BoolProperty(name = "Sync Meshes", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_curves_as_mesh = bpy.props.BoolProperty(name = "Curves as Mesh", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_make_double_sided = bpy.props.BoolProperty(name = "Make Double Sided", default = False, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_bake_modifiers = bpy.props.BoolProperty(name = "Bake Modifiers", default = False, update = msb_on_bake_modifiers_updated)
    bpy.types.Scene.meshsync_bake_transform  = bpy.props.BoolProperty(name = "Bake Transform", default = False, update = msb_on_bake_transform_updated)
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(name = "Sync Bones", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_blendshapes = bpy.props.BoolProperty(name = "Sync Blend Shapes", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_textures = bpy.props.BoolProperty(name = "Sync Textures", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(name = "Sync Cameras", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_lights = bpy.props.BoolProperty(name = "Sync Lights", default = True, update = msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(name = "Auto Sync", default = False, update = msb_on_toggle_auto_sync)
    bpy.types.Scene.meshsync_frame_step = bpy.props.IntProperty(name = "Frame Step", default = 1, min = 1, update = msb_on_animation_settings_updated)
    bpy.types.Scene.meshsync_unity_project_path = bpy.props.StringProperty(name = "Unity Project Path", default= "C:/", subtype = 'DIR_PATH', update = msb_on_scene_settings_updated)

def add_meshsync_to_unity_manifest(path, entry):
    file = open(path, "r+");
    data = json.load(file);

    # check if MeshSync is installed on selected project
    # if not installed, install it
    found = False

    for package in data['dependencies']:
        if (package == 'com.unity.meshsync'):
            found = True

    if(found == False):
        #install for user
        dependencies = data["dependencies"];
        dependencies["com.unity.meshsync"] = entry
        if (entry == ""):
            del dependencies["com.unity.meshsync"]
        file.seek(0)
        file.truncate(0)
        json.dump(data, file)
        file.close()
        return "INSTALLED"
    else:
        file.close()
        return "ALREADY_INSTALLED"

def install_meshsync_to_unity_project(directory):

    #Modify the Unity Package manifest to add the MeshSync package from disk.
    manifest_path = directory + "/Packages/manifest.json";

    # This is for local testing. It should be the version of the package, i.e.
    #manifest entry = 0.14.0-preview
    # or some path to the plugin resources folder
    manifest_entry =  "file:C:/Users/Sean Dillon/MeshSync/MeshSync~/Packages/com.unity.meshsync"

    status = add_meshsync_to_unity_manifest(manifest_path, manifest_entry)
    return status

def get_editor_version(directory):
    project_version_path = directory + "/ProjectSettings/ProjectVersion.txt"
    with open(project_version_path, "r+") as file:
        first_line = file.readline()
        version = first_line[len("m_EditorVersion: "):]
        version = version[:-1]
        return version

def get_editor_path(editor_version):
    os = platform.system()
    if os == 'Windows':
        path = "C:\\Program Files\\Unity\\Hub\\Editor\\"+editor_version+"\\Editor\\Unity.exe"
    elif os == 'Darwin':
        path = "/Applications/Unity/Hub/Editor/"+editor_version+"/Unity.app/Contents/MacOS/Unity"
    elif os == 'Linux':
        path = "/Applications/Unity/Hub/Editor/"+editor_version+"/Unity.app/Contents/Linux/Unity"
    return path;

def launch_project(editor_path, project_path):
    os = platform.system()
    if os == 'Windows':
        path = editor_path + " -projectPath \"" + project_path + "\""
    elif os == 'Darwin':
        path = editor_path + " -projectPath " + project_path
    elif os == 'Linux':
        path = editor_path + " -projectPath " + project_path

    return subprocess.Popen(path)

unity_process = None

def start_unity_project (directory):
    
    global unity_process

    if unity_process is not None and unity_process.poll() is None:
        return 'ALREADY_STARTED'

    #Get the project version
    editor_version = get_editor_version(directory)

    #Get the editor path
    editor_path = get_editor_path(editor_version)

    #Launch the editor with the project
    unity_process = launch_project(editor_path, directory)

    return 'STARTED'

def validate_project_path(directory):
    project_version_path = directory +"/ProjectSettings/ProjectVersion.txt"
    if path.exists(project_version_path):
        return True
    return False

def prompt_user_unity_project():
    bpy.ops.meshsync.browse_files('INVOKE_DEFAULT')


@persistent
def on_scene_load(context):
    msb_context.clear()

@persistent
def on_scene_update(context):
    msb_context.flushPendingList()
    if(bpy.context.scene.meshsync_auto_sync):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context)
        msb_context.exportUpdatedObjects()

def MS_MessageBox(message = "", title = "MeshSync", icon = 'INFO'):
    def draw(self, context):
        self.layout.label(text=message)
    bpy.context.window_manager.popup_menu(draw, title = title, icon = icon)

class MESHSYNC_OT_SendObjects(bpy.types.Operator):
    bl_idname = "meshsync.send_objects"
    bl_label = "Export Objects"
    def execute(self, context):

        directory = context.scene.meshsync_unity_project_path
        #Validate that the path is a project or prompt user to enter a valid path
        if validate_project_path(directory) == False:
            # Prompt user to enter a valid path
            prompt_user_unity_project()
            
            directory = context.scene.meshsync_unity_project_path
            print('Selected dir is now:'+directory)
            if validate_project_path(directory) == False:
                return {'FINISHED'}

        #Try to install meshsync if not already installed
        install_meshsync_to_unity_project(directory)
        
        #Try starting unity if not already started
        start_status = start_unity_project(directory)

        #If the project was launched now, wait until the Editor server is available
        if start_status == 'STARTED':
            while(msb_context.is_editor_server_available is False):
                time.sleep(0.1)

        #If there is no server in the scene, add one
        msb_context.sendEditorCommand(1)

        # Export data
        msb_apply_scene_settings()
        msb_context.setup(bpy.context)
        msb_context.export(msb_context.TARGET_OBJECTS)

        return{'FINISHED'}

class MESHSYNC_OT_BrowseFiles(bpy.types.Operator, ImportHelper):
    bl_idname = "meshsync.browse_files"
    bl_label = "Brownse Files"

    def execute(self, context):
        bpy.context.scene.meshsync_unity_project_path = self.filepath 
        
        if validate_project_path(self.filepath) == True:
            bpy.ops.meshsync.send_objects('INVOKE_DEFAULT')
        
        return {'FINISHED'}


class MESHSYNC_OT_SendAnimations(bpy.types.Operator):
    bl_idname = "meshsync.send_animations"
    bl_label = "Export Animations"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_apply_animation_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_ANIMATIONS)
        return{'FINISHED'}
