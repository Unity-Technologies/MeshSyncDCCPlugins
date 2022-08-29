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
    
    bpy.utils.register_class(UnityVersion)
    bpy.types.Scene.meshsync_unity_version = bpy.props.CollectionProperty(type=UnityVersion)
    bpy.types.Scene.meshsync_unity_version_index = bpy.props.IntProperty()

    bpy.types.Scene.meshsync_selected_unity_version = bpy.props.StringProperty(name = "Unity Version", default = "")

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
    if directory[-1] == '/' or directory[-1] == '\\':
        manifest_path = directory + "Packages/manifest.json"
    else:
        manifest_path = directory + "/Packages/manifest.json"

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

def get_editor_path_suffix():
    os = platform.system()
    if os == 'Windows':
        path = "\\Editor\\Unity.exe"
    elif os == 'Darwin':
        path = "/Unity.app/Contents/MacOS/Unity"
    elif os == 'Linux':
        path = "/Unity.app/Contents/Linux/Unity"
    return path;

def get_editor_path_prefix():
    os = platform.system()
    if os == 'Windows':
        path = "C:\\Program Files\\Unity\\Hub\\Editor\\"
    elif os == 'Darwin':
        path = "/Applications/Unity/Hub/Editor/"
    elif os == 'Linux':
        path = "/Applications/Unity/Hub/Editor/"
    return path;

def get_editor_versions():
    base_path = get_editor_path_prefix()
    sub_dirs = [entry for entry in os.listdir(base_path)]
    return sub_dirs


def get_editor_path(editor_version):
    return get_editor_path_prefix() + editor_version + get_editor_path_suffix()

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

def unity_process_is_alive():
    global unity_process

    if unity_process is None or unity_process.poll() is not None:
        return False

    return True

def start_unity_project (directory):
    
    global unity_process

    #Check if we have launched the project as a subprocess
    if unity_process is not None and unity_process.poll() is None:
        return 'ALREADY_STARTED'

    #Check if there is an editor server listening from the target project
    if msb_context.is_editor_server_available:
        msb_context.sendEditorCommand(2)
        path = msb_context.editor_command_reply;
        if path == directory:
            return 'ALREADY_STARTED'

    if not path.exists(meshsync_unity_version):
        return 'BAD_UNITY_VERSION'

    #Launch the editor with the project
    unity_process = launch_project(editor_path, directory)

    return 'STARTED'

def validate_project_path(directory):
    project_version_path = directory +"/ProjectSettings/ProjectVersion.txt"
    if path.exists(project_version_path):
        return True
    return False

def package_manifest_exists(directory):
    project_manifest_path = directory + "/Packages/manifest.json"
    if path.exists(project_manifest_path):
        return True
    return False


def prompt_user_unity_project():
    bpy.ops.meshsync.browse_files('INVOKE_DEFAULT')

def update_project_path_from_server(context):
    if not msb_context.is_editor_server_available:
        return

    #Get the project path
    msb_context.sendEditorCommand(2)
    context.scene.meshsync_unity_project_path = msb_context.editor_command_reply;

def create_unity_project(directory):
    global unity_process
    #TODO handle multiple versions of unity editor installed with user prompt
    # For now assume only one version installed

    editor_path = get_editor_path(editor_version)    

    versions = get_editor_versions()

    #use the latest version
    version = versions[-1]
    editor_path = get_editor_path(version)

    path = editor_path + " -quit"
    os = platform.system()
    if os == 'Windows':
        path = path +" -createProject \"" + directory + "\""
    elif os == 'Darwin':
        path = path + " -createProject " + directory
    elif os == 'Linux':
        path = path + " -createProject " + directory


    unity_process = subprocess.Popen(path)


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
        bpy.ops.meshsync.select_unity_version('INVOKE_DEFAULT')
        return {'FINISHED'}

        directory = context.scene.meshsync_unity_project_path

        #Validate that the path is a project
        if validate_project_path(directory) == False:

            #Try to fill a valid path from an already running project
            update_project_path_from_server(context)
            directory = context.scene.meshsync_unity_project_path
            
            if validate_project_path(directory) == False:
                # Prompt user to enter a valid path
                prompt_user_unity_project()
            
                directory = context.scene.meshsync_unity_project_path
                if validate_project_path(directory) == False:
                    return {'FINISHED'}

        #Try to install meshsync if not already installed
        install_meshsync_to_unity_project(directory)
        
        #Try starting unity if not already started
        start_status = start_unity_project(directory)

        if start_status == 'BAD_UNITY_VERSION':
            versions = get_editor_versions()
            bpy.ops.meshsync.select_unity_version('INVOKE_DEFAULT')

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
        else:
            #TODO Prompt user to validate if they want to create a unity project in path
            #Create unity project in location
            create_unity_project(self.filepath)
            while unity_process_is_alive() or not package_manifest_exists(self.filepath):
                time.sleep(0.1)

            #install the package
            install_meshsync_to_unity_project(self.filepath)
            bpy.ops.meshsync.send_objects('INVOKE_DEFAULT')


        
        return {'FINISHED'}

class UnityVersion(bpy.types.PropertyGroup):
    version : bpy.props.StringProperty(name = "Version")




class MESHSYNC_OT_SelectUnityVersion(bpy.types.Operator):
    bl_idname = "meshsync.select_unity_version"
    bl_label = "Select Unity Version"

    def invoke(self, context, event):
        versions = get_editor_versions()
        prop = bpy.context.scene.meshsync_unity_version
        prop.clear()
        print(versions)
        for version in versions:
            item = prop.add()
            item.version = version
            item.name = version

        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):

        index = context.scene.meshsync_unity_version_index
        selected = context.scene.meshsync_unity_version[index].version
        context.scene.meshsync_selected_unity_version = selected

        return {'FINISHED'}
    def draw(self, context):
        scn = context.scene
        col = self.layout.column()
        col.template_list("UI_UL_list", "example_dialog", context.scene, "meshsync_unity_version", context.scene, "meshsync_unity_version_index",
            item_dyntip_propname = "version",rows=4)




class MESHSYNC_OT_SendAnimations(bpy.types.Operator):
    bl_idname = "meshsync.send_animations"
    bl_label = "Export Animations"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_apply_animation_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_ANIMATIONS)
        return{'FINISHED'}
