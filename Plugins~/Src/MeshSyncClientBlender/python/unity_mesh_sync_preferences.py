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
    try: #To avoid blocking the user from launching blender in case something goes wrong
        path = msb_get_hub_path()
        p = subprocess.Popen([path, "--", "--headless","ip", "-g" ], stdout = subprocess.PIPE)
        path = None
        for line in iter(p.stdout.readline, b''):
            path = line.rstrip()
        return path.decode('utf-8')
    except:
        return ""

class MESHSYNC_OT_OpenHub(bpy.types.Operator):
    bl_idname = "meshsync.open_hub"
    bl_label = "Create or Select Project with Unity Hub"
    log_file = None
    state = None
    thread = None

    preview_collections = {}

    @classmethod
    def description(cls, context, properties):
        return "Open the Unity Hub to Create or Select a Unity project to sync data to"

    def preferences(self, context):
        return context.preferences.addons[__package__].preferences

    def open_hub(self, context):
        hub_path = self.preferences(context).hub_path
        subprocess.Popen([hub_path])


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
            result = re.search('createProject projectPath: (.*), current editor:', line)
            if result is not None:
                path = os.path.normpath(result.group(1))
                print(path)
                self.preferences(context).project_path = path
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

                self.thread.join()

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
        self.thread = Thread(target = self.monitor_logs, args = (context,))
        self.thread.start()

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


class MESHSYNC_Preferences(AddonPreferences):

    # this must match the add-on name, use '__package__'
    # when defining this in a submodule of a python package.
    bl_idname = __package__

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

    project_path: StringProperty(name = "Unity Project", default= "C://Path//To//Unity//Project", subtype = 'DIR_PATH', update = redraw)
    hub_path: bpy.props.StringProperty(name = "Unity Hub", subtype = 'FILE_PATH')
    editors_path: bpy.props.StringProperty(name = "Unity Editors", subtype = 'DIR_PATH')

    def draw(self, context):
        layout = self.layout

        editor_layout = layout.box()
        editor_layout.label(text ="Editor Settings")
        editor_layout.prop(self, "hub_path")
        editor_layout.prop(self, "editors_path")
        editor_layout.operator("meshsync.reset_preferences", text="Auto Detect", icon="FILE_REFRESH")

        project_layout = layout.box()
        project_layout.label(text ="Project Settings")
        project_layout.prop(self, "project_path")
        project_layout.operator("meshsync.open_hub", icon_value = MESHSYNC_OT_OpenHub.get_icon().icon_id)