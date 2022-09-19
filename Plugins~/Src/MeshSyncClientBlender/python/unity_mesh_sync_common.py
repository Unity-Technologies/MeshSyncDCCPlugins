import os
import re
import bpy
from bpy.app.handlers import persistent
from . import MeshSyncClientBlender as ms

msb_context = ms.Context()
msb_cache = ms.Cache()

EDITOR_COMMAND_ADD_SERVER = 1
EDITOR_COMMAND_GET_PROJECT_PATH = 2

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

def msb_get_editor_path_prefix():
    return bpy.context.scene.meshsync_unity_editors_path

def msb_get_editor_path_prefix_default():
    os = platform.system()
    if os == 'Windows':
        path = "C:\\Program Files\\Unity\\Hub\\Editor"
    elif os == 'Darwin' or os == 'Linux':
        path = "/Applications/Unity/Hub/Editor"
    return path;

def msb_get_editor_path_suffix():
    os = platform.system()
    if os == 'Windows':
        path = "\\Editor\\Unity.exe"
    elif os == 'Darwin':
        path = "/Unity.app/Contents/MacOS/Unity"
    elif os == 'Linux':
        path = "/Unity.app/Contents/Linux/Unity"
    return path

def msb_get_editor_path(editor_version):
    return msb_get_editor_path_prefix() + editor_version + msb_get_editor_path_suffix()

def msb_validate_project_path(directory):
    project_version_path = directory +"/ProjectSettings/ProjectVersion.txt"
    if path.exists(project_version_path):
        return True
    return False

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
    bpy.types.Scene.meshsync_unity_project_path = bpy.props.StringProperty(name = "Path", default= "C:/", subtype = 'DIR_PATH', update = msb_on_scene_settings_updated)
    
    default_hub_path = msb_get_editor_path_prefix_default()
    bpy.types.Scene.meshsync_unity_editors_path = bpy.props.StringProperty(name = "Unity Editors", default= default_hub_path, subtype = 'DIR_PATH', update = msb_on_scene_settings_updated)

unity_process = None
click_source = None


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

class MESHSYNC_OT_CheckProjectPath_Auto(bpy.types.Operator):
    bl_idname = "meshsync.check_project_path_auto"
    bl_label = "Ensure Unity project path is pointing to a Unity project"
    
    def execute(self, context):
        global click_source
        click_source = 'Auto'
        bpy.ops.meshsync.check_project_path('INVOKE_DEFAULT')
        return {'FINISHED'}

class MESHSYNC_OT_CheckProjectPath_Manual(bpy.types.Operator):
    bl_idname = "meshsync.check_project_path_manual"
    bl_label = "Ensure Unity project path is pointing to a Unity project"
    
    def execute(self, context):
        global click_source
        click_source = 'Manual'
        bpy.ops.meshsync.check_project_path('INVOKE_DEFAULT')
        return {'FINISHED'}


class MESHSYNC_OT_CheckProjectPath(bpy.types.Operator):
    bl_idname = "meshsync.check_project_path"
    bl_label = "Ensure Unity project path is pointing to a Unity project"

    def execute(self, context):
        directory = context.scene.meshsync_unity_project_path
        #Validate that the path is a project
        if msb_validate_project_path(directory) == False:
            #Try to fill a valid path from an already running project
            bpy.ops.meshsync.path_from_server('INVOKE_DEFAULT')
        else:
            bpy.ops.meshsync.install_meshsync('INVOKE_DEFAULT')

        return {'FINISHED'}

class MESHSYNC_OT_TryGetPathFromServer(bpy.types.Operator):
    bl_idname = "meshsync.path_from_server"
    bl_label = "Try to get the unity path from a running server"

    def execute(self, context):
        if not msb_context.is_editor_server_available:
            bpy.ops.meshsync.prompt_project_path('INVOKE_DEFAULT')
            return {'FINISHED'}

        #Get the project path
        msb_context.sendEditorCommand(EDITOR_COMMAND_GET_PROJECT_PATH)
        server_reply = msb_context.editor_command_reply
        directory = context.scene.meshsync_unity_project_path

        if msb_validate_project_path(server_reply) == False:
            # Prompt user to enter a valid path
            message = directory + " is not a Unity project"
            MS_MessageBox(message = message)
            print(message)
        else:
            context.scene.meshsync_unity_project_path = server_reply
            bpy.ops.meshsync.create_server('INVOKE_DEFAULT')
            
        return {'FINISHED'}

class MESHSYNC_OT_PromptProjectPath(bpy.types.Operator, ImportHelper):
    bl_idname = "meshsync.prompt_project_path"
    bl_label = "Unity Project"

    filepath : bpy.props.StringProperty(name = "filepath")

    def execute(self, context):

        context.scene.meshsync_unity_project_path = self.filepath

        if msb_validate_project_path(self.filepath) == False:
            message = self.filepath + " is not a Unity project"
            MS_MessageBox(message = message)
            print(message)
            return {'FINISHED'}
        else:
            bpy.ops.meshsync.install_meshsync('INVOKE_DEFAULT')

        return {'FINISHED'}

class MESHSYNC_OT_InstallMeshSync(bpy.types.Operator):

    bl_idname = "meshsync.install_meshsync"
    bl_label = "Try to get the unity path from a running server"

    def add_meshsync_to_unity_manifest(self, path, entry):
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

    def install_meshsync_to_unity_project(self, directory):

        #Modify the Unity Package manifest to add the MeshSync package from disk.
        if directory[-1] == '/' or directory[-1] == '\\':
            manifest_path = directory + "Packages/manifest.json"
        else:
            manifest_path = directory + "/Packages/manifest.json"

        #TODO replace with release that has the Editor Commands changes
        manifest_entry = "0.14.5-preview"

        self.add_meshsync_to_unity_manifest(manifest_path, manifest_entry)

    def execute(self, context):
        directory = context.scene.meshsync_unity_project_path
        self.install_meshsync_to_unity_project(directory)
        bpy.ops.meshsync.start_unity('INVOKE_DEFAULT')
        return {'FINISHED'}

class MESHSYNC_OT_StartUnity(bpy.types.Operator):
    bl_idname = "meshsync.start_unity"
    bl_label = "Export Objects"

    def get_editor_version(self, directory):
        project_version_path = directory + "/ProjectSettings/ProjectVersion.txt"
        with open(project_version_path, "r+") as file:
            first_line = file.readline()
            version = first_line[len("m_EditorVersion: "):]
            version = version[:-1]
            return version

    def launch_project(self, editor_path, project_path):
        os = platform.system()
        if os == 'Windows':
            path = editor_path + " -projectPath \"" + project_path + "\""
        elif os == 'Darwin' or os == 'Linux':
            path = editor_path + " -projectPath " + project_path

        return subprocess.Popen(path)

    def start_unity_project (self, directory):
        
        global unity_process

        #Check if we have launched the project as a subprocess
        if unity_process is not None and unity_process.poll() is None:
            return 'ALREADY_STARTED'

        #Check if there is an editor server listening from the target project
        if msb_context.is_editor_server_available:
            msb_context.sendEditorCommand(EDITOR_COMMAND_GET_PROJECT_PATH)
            reply_path = msb_context.editor_command_reply;
            if reply_path == directory:
                return 'ALREADY_STARTED'


        editor_version = self.get_editor_version(directory)
        editor_path = msb_get_editor_path(editor_version)

        if not path.exists(editor_path):
            return 'EDITOR_NOT_EXISTS'

        #Launch the editor with the project
        unity_process = self.launch_project(editor_path, directory)

        return 'STARTED'


    def execute(self, context):
        #Try starting unity if not already started
        directory = context.scene.meshsync_unity_project_path
        start_status = self.start_unity_project(directory)

        #If the project was launched now, wait until the Editor server is available
        if start_status == 'STARTED':
            while(msb_context.is_editor_server_available is False):
                time.sleep(0.1)
        elif start_status == 'EDITOR_NOT_EXISTS':
            editor_version = self.get_editor_version(directory)
            editor_path = msb_get_editor_path(editor_version)
            message = "Editor path " + editor_path + " does not exist." 
            MS_MessageBox(message = message)
            print(message)
            return {'FINISHED'}

        bpy.ops.meshsync.create_server('INVOKE_DEFAULT')
        return {'FINISHED'}

class MESHSYNC_OT_CreateServer(bpy.types.Operator):
    bl_idname = "meshsync.create_server"
    bl_label = "Create a Scene Server if there is none"

    def execute(self, context):
       #If there is no server in the scene, add one
        msb_context.sendEditorCommand(EDITOR_COMMAND_ADD_SERVER)
        bpy.ops.meshsync.finish_checks('INVOKE_DEFAULT')
        return {'FINISHED'}

class MESHSYNC_OT_FinishChecks(bpy.types.Operator):
    bl_idname ="meshsync.finish_checks"
    bl_label = "Finish Checks"

    def execute(self, context):
        global click_source
        if click_source == 'Manual':
            bpy.ops.meshsync.send_objects('INVOKE_DEFAULT')
        elif click_source == 'Auto':
            bpy.ops.meshsync.auto_sync('INVOKE_DEFAULT')
        return {'FINISHED'}

class MESHSYNC_OT_SendObjects(bpy.types.Operator):
    bl_idname = "meshsync.send_objects"
    bl_label = "Export Objects"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_OBJECTS)
        return{'FINISHED'}


class MESHSYNC_OT_SendAnimations(bpy.types.Operator):
    bl_idname = "meshsync.send_animations"
    bl_label = "Export Animations"
    def execute(self, context):
        msb_apply_scene_settings()
        msb_apply_animation_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_ANIMATIONS)
        return{'FINISHED'}
