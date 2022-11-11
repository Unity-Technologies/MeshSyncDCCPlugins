import bpy
import os
import platform

from .unity_mesh_sync_installation import *
from . import MeshSyncClientBlender as ms
from bpy.types import AddonPreferences
from bpy.props import StringProperty, IntProperty, BoolProperty
from threading import Thread
from time import sleep

msb_context = ms.Context()


def msb_get_editor_path_prefix_default():
    os = platform.system()
    if os == 'Windows':
        path = "C:\\Program Files\\Unity\\Hub\\Editor"
    elif os == 'Darwin' or os == 'Linux':
        path = "/Applications/Unity/Hub/Editor"
    return path

class MESHSYNC_OT_OpenHub(bpy.types.Operator):
    bl_idname = "meshsync.open_hub"
    bl_label = "Select Project with Unity Hub"
    active = False
    logs_thread = None


    @classmethod
    def description(cls, context, properties):
        return "Open Unity Hub to select a Unity project to sync data to"

    def invoke(self, context, event):

        if MESHSYNC_OT_OpenHub.active:
            MESHSYNC_OT_OpenHub.active = False
            if self.logs_thread is not None:
                self.logs_thread.join()
        else:
            if self.logs_thread is not None:
                self.logs_thread.join()
            MESHSYNC_OT_OpenHub.active = True

            self.logs_thread = Thread(target = self.monitor_logs, args = (context,))
            self.logs_thread.start()

            hub_path = msb_preferences(context).hub_path
            subprocess.Popen([hub_path])

        return {'FINISHED'}

    def monitor_logs(self, context):
        logs_path = os.path.join(msb_get_hub_dir(), "logs", "info-log.json")
        with open(logs_path, "r+") as log_file:
            log_file.seek(0, os.SEEK_END)

            while MESHSYNC_OT_OpenHub.active:
                lines = log_file.readlines()
                for line in lines:
                    print(line)
                    MESHSYNC_OT_OpenHub.handle_log_entry(context, line)

                #if idle, sleep for 0.1 seconds
                sleep(0.1)

            # in case the state changed before the last set of line was read 
            lines = log_file.readlines()
            for line in lines:
                print(line)
                MESHSYNC_OT_OpenHub.handle_log_entry(context, line)

    def handle_log_entry(context, line):
        path = MESHSYNC_OT_OpenHub.extract_path_from_log_entry(line, 'openProject', 'openProject projectPath: (.*), current editor:')
        print("Checking for already open project:" + str(path))
        if path is not None:
            msb_preferences(context).project_path = path
            return

        path  = MESHSYNC_OT_OpenHub.extract_path_from_log_entry(line, 'ALREADY_OPEN', '\"projectPath\":\"(.*)\"')
        print("Checking for already open:" + str(path))
        if path is not None:
            msb_preferences(context).project_path = path
            return

    def extract_path_from_log_entry(line, token, regex):
        if token in line:
            message = json.loads(line)['message']
            result = re.search(regex, message)
            if result is not None:
                return os.path.normpath(result.group(1))


class MESHSYNC_OT_ResetPreferences(bpy.types.Operator):
    bl_idname = "meshsync.reset_preferences"
    bl_label = "Reset Preferences"

    @classmethod
    def description(cls, context, properties):
        return "Find where the Unity Hub and the Unity Editors are located in the system"

    def execute(self, context):
        msb_preferences(context).reset()
        return {'FINISHED'}

class MESHSYNC_Preferences(AddonPreferences):

    # this must match the add-on name, use '__package__'
    # when defining this in a submodule of a python package.
    bl_idname = __package__

    def hub_exists(self):
        return os.path.exists(self.hub_path)

    def editors_path_exists(self):
        return os.path.exists(self.editors_path)

    def is_hub_installed():
        path = msb_get_hub_path()
        return os.path.exists(path)

    def is_hub_supported():
        hub_version = msb_get_hub_version()
        if hub_version == "":
            return False

        return msb_get_most_recent_version(hub_version, "3.0.0") == hub_version

    def reset(self):
        self.hub_path = msb_get_hub_path()
        self.hub_version = msb_get_hub_version()
        self.hub_installed = MESHSYNC_Preferences.is_hub_installed()
        self.hub_supported = MESHSYNC_Preferences.is_hub_supported()

        self.editors_path = msb_get_editors_path()

    def redraw(self, context):
        regions = context.area
        if regions == None:
            return None

        for region in context.area.regions:
            if region.type == "UI":
                region.tag_redraw()
        return None

    def draw(self, context):
        layout = self.layout
        editor_layout = layout.box()
        editor_layout.label(text ="Editor Settings")
        editor_layout.prop(self, "hub_path")
        editor_layout.prop(self, "editors_path")

        if not self.hub_exists() and not self.hub_installed:
            row = editor_layout.row()
            row.label(text = "Could not detect Unity Hub installation.", icon = 'ERROR')
            row.operator("wm.url_open", text="Download Unity Hub", icon = 'URL').url = "https://unity3d.com/get-unity/download"
            row = editor_layout.row()
            row.operator("meshsync.reset_preferences", text="Auto Detect", icon="FILE_REFRESH")
        elif not self.hub_exists() or not self.editors_path_exists():
            row = editor_layout.row()
            row.label(text = "One of the paths above does not exist", icon = 'ERROR')
            row.operator("meshsync.reset_preferences", text="Auto Detect", icon="FILE_REFRESH")
        else:
            editor_layout.operator("meshsync.reset_preferences", text="Auto Detect", icon="FILE_REFRESH")

        project_layout = layout.box()
        project_layout.label(text ="Project Settings")
        project_layout.prop(self, "project_path")

        if self.hub_supported:
            row = project_layout.row()
            if MESHSYNC_OT_OpenHub.active:
                row.operator("meshsync.open_hub", text = "Stop using Unity Hub", icon = 'PAUSE')
            else:
                row.operator("meshsync.open_hub", text = "Select project via Unity Hub", icon = 'PLAY')


    def register():
        bpy.utils.register_class(MESHSYNC_OT_ResetPreferences)
        bpy.utils.register_class(MESHSYNC_OT_OpenHub)

    def unregister():
        bpy.utils.unregister_class(MESHSYNC_OT_ResetPreferences)
        bpy.utils.unregister_class(MESHSYNC_OT_OpenHub)

    project_path: StringProperty(name = "Unity Project", default= "C:/", subtype = 'DIR_PATH', update = redraw)
    editors_path: bpy.props.StringProperty(name = "Unity Editors Location", default= msb_get_editor_path_prefix_default(), subtype = 'DIR_PATH')
    hub_path: bpy.props.StringProperty(name = "Unity Hub Location", default = msb_get_hub_path(), subtype = 'FILE_PATH')
    hub_version = bpy.props.StringProperty(name = "Unity Hub Version", default = msb_get_hub_version())
    hub_installed : bpy.props.BoolProperty(name = "Hub Installed", default = is_hub_installed())
    hub_supported: bpy.props.BoolProperty(name = "Hub Supported", default = is_hub_supported())
