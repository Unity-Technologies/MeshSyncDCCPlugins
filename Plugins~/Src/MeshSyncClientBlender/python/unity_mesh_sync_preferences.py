import bpy
import os
import platform
import subprocess
import re
import json

from threading import Thread
from time import sleep
from . import MeshSyncClientBlender as ms
from bpy.types import AddonPreferences
from bpy.props import StringProperty, IntProperty, BoolProperty

msb_context = ms.Context()

def msb_get_hub_dir():
    system = platform.system()
    if system == "Windows":
        return os.path.join(os.getenv('APPDATA'),"UnityHub")
    elif system == "Darwin":
        return os.path.join(os.getenv("HOME"),"Library","Application Support","UnityHub")

def msb_get_hub_path():
    config_path = os.path.join(msb_get_hub_dir(), "hubInfo.json")
    with open(config_path, "r+") as file:
        data = json.load(file)
        return os.path.normpath(data['executablePath'])

def msb_get_editors_path():
        path = msb_get_hub_path()
        p = subprocess.Popen([path, "--", "--headless","ip", "-g" ], stdout = subprocess.PIPE)
        path = None
        for line in iter(p.stdout.readline, b''):
            path = line.rstrip()
        return path.decode('utf-8')

def msb_validate_project_path(directory):
    project_version_path = os.path.join(directory,"ProjectSettings","ProjectVersion.txt")
    if os.path.exists(project_version_path):
        return True
    return False

def msb_validate_manifest(directory):
     with open(directory, "r+") as file:
        data = json.load(file)

        for package in data['dependencies']:
            if package == 'com.unity.meshsync':
                return True

        return False

def msb_validate_installed(directory):
    packages = os.path.join(directory,"Packages","manifest.json")
    return msb_validate_manifest(packages)

def msb_validate_resolved(directory):
    packages_lock = os.path.join(directory, "Packages", "packages-lock.json")
    return msb_validate_manifest(packages_lock)

def msb_preferences(context):
    return context.preferences.addons[__package__].preferences

def msb_is_project_open(directory):
    full_path = os.path.join(directory, "Temp", "UnityLockfile")
    if not os.path.exists(full_path):
        return False
    try:
        with open(full_path, 'wb') as file:
            system = platform.system()
            if system == "Darwin" or system == "Linux":
                import fcntl
                fcntl.lockf(file.fileno(), fcntl.LOCK_EX|fcntl.LOCK_NB)
                fcntl.lockf(file.fileno(), fcntl.LOCK_UN)
            elif system == "Windows":
                import msvcrt
                msvcrt.locking(file.fileno(), msvcrt.LK_NBLCK, 0)
                msvcrt.locking(file.fileno(), msvcrt.LK_UNLCK, 0)
    except:
        return True

    return False

class MESHSYNC_OT_OpenHub(bpy.types.Operator):
    bl_idname = "meshsync.open_hub"
    bl_label = "Create or Select Project with Unity Hub"
    log_file = None
    state = None
    logs_thread = None
    create_thread = None

    preview_collections = {}

    @classmethod
    def description(cls, context, properties):
        return "Open the Unity Hub to Create or Select a Unity project to sync data to"

    def preferences(self, context):
        return context.preferences.addons[__package__].preferences

    def open_hub(self, context):
        hub_path = self.preferences(context).hub_path
        subprocess.Popen([hub_path])

    def monitor_create(self, context):
        path = msb_preferences(context).project_path
        while True:
            created = msb_validate_project_path(path)
            if created:
                msb_preferences(context).is_project_being_created = False
                break
            sleep(0.1)

    def handle_log_entry(self, line, context):
        line = line.replace('"', '\"')
        if "openProject" in line:
            result = re.search('openProject projectPath: (.*), current editor:', line)
            if result is not None:
                path = os.path.normpath(result.group(1))
                print(path)
                self.preferences(context).project_path = path
                return

        if "createProject" in line:
            result = re.search('createProject projectPath: (.*), editor version:', line)
            if result is not None:
                path = os.path.normpath(result.group(1))
                print(path)
                self.preferences(context).project_path = path
                self.preferences(context).is_project_being_created = True
                create_thread = Thread(target = self.monitor_create, args = (context,))
                create_thread.start()
                return

        if "ALREADY_OPEN" in line:
            line = json.loads(line)['message']
            print(line) 
            result = re.search('\"projectPath\":\"(.*)\"', line)
            if result is not None:
                path = os.path.normpath(result.group(1))
                print(path)
                self.preferences(context).project_path = path
                return

    def parse_lines(self, context):
        line = self.log_file.readline()
        while len(line) > 0 :
            self.handle_log_entry(line, context)
            line = self.log_file.readline()

    def modal(self, context, event):
        event_type = event.type
        if event_type == 'WINDOW_DEACTIVATE':
            self.state = 'FOCUSED_HUB'
        if event_type == 'RIGHTMOUSE' or event_type == 'LEFTMOUSE':
            if self.state == 'FOCUSED_HUB':
                self.state = 'FINISHED'

                self.logs_thread.join()

                if msb_preferences(context).is_project_being_created:
                    return {'RUNNING_MODAL'}

                if self.create_thread is not None:
                    self.create_thread.join()

                return {'FINISHED'}

        if self.state == 'FINISHED' and not msb_preferences(context).is_project_being_created:
            if self.create_thread is not None:
                self.create_thread.join()
                return {'FINISHED'}

        # When the user returns the focus on Blender, we assume they have finished with the hub
        return {'RUNNING_MODAL'}

    def open_logs(self, context):
        logs_path = os.path.join(msb_get_hub_dir(), "logs", "info-log.json")
        log_file = open(logs_path, "r+")
        log_file.seek(0, os.SEEK_END)
        return log_file

    def monitor_logs(self, context):
        while self.state != 'FINISHED':
            self.parse_lines(context)
            sleep(0.1)

        self.parse_lines(context)
        self.log_file.close()

    def invoke(self, context, event):
        self.state = 'STARTED'
        self.log_file = self.open_logs(context)
        self.logs_thread = Thread(target = self.monitor_logs, args = (context,))
        self.logs_thread.start()

        self.open_hub(context)

        context.window_manager.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def get_icon():
        pcoll = MESHSYNC_OT_OpenHub.preview_collections["main"]
        return pcoll["hub_logo"]

    def register():
        import bpy.utils.previews
        pcoll = bpy.utils.previews.new()

        # path to the folder where the icon is
        # the path is calculated relative to this py file inside the addon folder
        my_icons_dir = os.path.join(os.path.dirname(__file__), "resources")

        # load a preview thumbnail of a file and store in the previews collection
        pcoll.load("hub_logo", os.path.join(my_icons_dir, "hub.png"), 'IMAGE')

        MESHSYNC_OT_OpenHub.preview_collections["main"] = pcoll

    def unregister():
        for pcoll in MESHSYNC_OT_OpenHub.preview_collections.values():
            bpy.utils.previews.remove(pcoll)
        MESHSYNC_OT_OpenHub.preview_collections.clear()

class MESHSYNC_OT_ResetPreferences(bpy.types.Operator):
    bl_idname = "meshsync.reset_preferences"
    bl_label = "Reset Preferences"

    @classmethod
    def description(cls, context, properties):
        return "Find where the Unity Hub and the Unity Editors are located in the system"

    def execute(self, context):
        preferences = context.preferences.addons[__package__].preferences
        preferences.reset()
        return {'FINISHED'}

class MESHSYNC_OT_InstallMeshSync(bpy.types.Operator):
    bl_idname = "meshsync.install_meshsync"
    bl_label = "Install MeshSync"

    thread = None


    def monitor(self, context):
        directory = msb_preferences(context).project_path
        while True:
            resolved = msb_validate_resolved(directory)
            msb_preferences(context).is_meshsync_resolved = resolved
            if resolved:
                break
            sleep(0.1)


    def install(self, context):
        directory = msb_preferences(context).project_path
        manifest_path = os.path.join(directory,"Packages","manifest.json")
        entry = "0.15.1-preview"

        with open(manifest_path, "r+") as file:
            data = json.load(file);
                
            dependencies = data["dependencies"];
            dependencies["com.unity.meshsync"] = entry
            file.seek(0)
            file.truncate(0)
            json.dump(data, file)

    def execute(self, context):
        self.install(context)
        msb_preferences(context).is_meshsync_installed = True

        project_path = msb_preferences(context).project_path
        if not msb_is_project_open(project_path):
            return {'FINISHED'}

        self.thread = Thread(target = self.monitor, args = (context,))
        self.thread.start()

        context.window_manager.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def modal(self, context, event):
        if msb_preferences(context).is_meshsync_resolved:
            self.thread.join()
            return {'FINISHED'}

        return {'RUNNING_MODAL'}

    @classmethod
    def description(cls, context, properties):
        return "Install MeshSync for the Unity Project"

class MESHSYNC_Preferences(AddonPreferences):

    # this must match the add-on name, use '__package__'
    # when defining this in a submodule of a python package.
    bl_idname = __package__

    def hub_exists(self):
        return os.path.exists(self.hub_path)

    def editors_path_exists(self):
        return os.path.exists(self.editors_path)

    def on_project_updated(self, context):
        self.is_project_being_created = False
        self.is_unity_project = msb_validate_project_path(self.project_path)
        if self.is_unity_project:
            self.is_meshsync_installed = msb_validate_installed(self.project_path)
            self.is_meshsync_resolved = msb_validate_resolved(self.project_path)
            self.is_project_running = msb_is_project_open(self.project_path)
        else:
            self.is_meshsync_installed = False
            self.is_meshsync_resolved = False
            self.is_project_running = False
        self.redraw(context)

    def on_is_unity_project_updated(self, context):
        self.is_meshsync_installed = msb_validate_installed(self.project_path)
        self.redraw(context)

    def on_is_meshsync_installed_updated(self, context):
        if self.is_meshsync_installed:
            self.is_project_running = msb_is_project_open(self.project_path)
        else:
            self.is_project_running = False
        self.redraw(context)

    def on_is_project_running_updated(self, context):
        self.redraw(context)

    def on_is_project_being_created_updated(self, context):
        self.is_unity_project = msb_validate_project_path(self.project_path)
        self.redraw(context)

    def reset(self):
        self.hub_path = msb_get_hub_path()
        self.editors_path = msb_get_editors_path()

    def redraw(self, context):
        regions = context.area
        if regions == None:
            return None

        for region in context.area.regions:
            if region.type == "UI":
                region.tag_redraw()
        return None

    hub_path: bpy.props.StringProperty(name = "Unity Hub", subtype = 'FILE_PATH')
    editors_path: bpy.props.StringProperty(name = "Unity Editors", subtype = 'DIR_PATH')

    project_path: StringProperty(name = "Unity Project", default= "C://Path//To//Unity//Project", subtype = 'DIR_PATH', update = on_project_updated)
    is_unity_project : bpy.props.BoolProperty(default = False, update = on_is_meshsync_installed_updated)
    is_meshsync_installed : bpy.props.BoolProperty(default = False, update = on_is_meshsync_installed_updated)
    is_project_running : bpy.props.BoolProperty(default = False, update = on_is_project_running_updated)
    is_meshsync_resolved : bpy.props.BoolProperty(default = False, update = redraw)
    is_project_being_created : bpy.props.BoolProperty(default= False, update = on_is_project_being_created_updated)

    def draw(self, context):
        layout = self.layout

        editor_layout = layout.box()
        editor_layout.label(text ="Editor Settings")
        editor_layout.prop(self, "hub_path")
        editor_layout.prop(self, "editors_path")
        if not self.hub_exists() or not self.editors_path_exists():
            row = editor_layout.row()
            row.label(text = "One of the paths above does not exist", icon = 'ERROR')
            row.operator("meshsync.reset_preferences", text="Auto Detect", icon="FILE_REFRESH")
        else:
            editor_layout.operator("meshsync.reset_preferences", text="Auto Detect", icon="FILE_REFRESH")

        project_layout = layout.box()
        project_layout.label(text ="Project Settings")
        project_layout.prop(self, "project_path")
        
        if self.hub_exists():
            project_layout.operator("meshsync.open_hub", icon_value = MESHSYNC_OT_OpenHub.get_icon().icon_id)

        if not self.is_unity_project:
            if self.is_project_being_created:
                project_layout.label(text = "Project is being created, please wait..")
            else:
                if self.hub_exists():
                    project_layout.label(text = "Not a Unity Project. Click on the button above to select or create a Unity Project, or provide the path to the Unity project (parent of Assets folder).",
                    icon = 'ERROR')
                else: 
                    project_layout.label(text = "Not a Unity Project. Please select a Unity Project folder (parent of Assets folder).", icon = 'ERROR')
            return
        
        project_layout.label(text = "Valid Unity Project path.", icon = "CHECKMARK")

        if not self.is_meshsync_installed:
            row = project_layout.row()
            row.label(text = "MeshSync is not installed", icon = 'ERROR')
            row.operator("meshsync.install_meshsync")
            return

        project_layout.label(text = "MeshSync installed in Unity Project.", icon = 'CHECKMARK')

        if not msb_is_project_open(self.project_path):
            project_layout.label(text = "MeshSync will be resolved in Unity Project at project lauch.", icon = 'CHECKMARK')
        else:
            if not self.is_meshsync_resolved:
                row = project_layout.row()
                row.label(text = "MeshSync is not loaded. Please click on the Unity Project window to allow the Unity Editor to load the MeshSync package.", icon = 'ERROR')
                return

            project_layout.label(text = "MeshSync resolved in Unity Project.", icon = 'CHECKMARK')

        project_layout.label(text = "All set up! Use the MeshSync panel in Active Tool and Workspace Settings to sync data to your project.")