import bpy
import os
import platform
import subprocess
import re
import json

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

class MESHSYNC_OT_OpenHub(bpy.types.Operator):
    bl_idname = "meshsync.open_hub"
    bl_label = "Open Unity Hub"
    log_file = None
    state = None

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
            result = re.search('openProject projectPath: (.*),', line)
            if result is not None:
                path = os.path.normpath(result.group(1))
                print(path)
                self.preferences(context).project_path = path
                return

        if "createProject" in line:
            result = re.search('createProject projectPath: (.*),', line)
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
                self.parse_lines(context)
                return {'FINISHED'}

        # When the user returns the focus on Blender, we assume they have finished with the hub
        return {'RUNNING_MODAL'}

    def open_logs(self, context):
        logs_path = os.path.join(msb_get_hub_dir(), "logs", "info-log.json")
        log_file = open(logs_path, "r+")
        log_file.seek(0, os.SEEK_END)
        return log_file

    def invoke(self, context, event):
        self.log_file = self.open_logs(context)
        self.open_hub(context)
        self.state = 'STARTED'
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

class MESHSYNC_Preferences(AddonPreferences):

    # this must match the add-on name, use '__package__'
    # when defining this in a submodule of a python package.
    bl_idname = __package__

    project_path: StringProperty(name = "Unity Project", default= "C:/", subtype = 'DIR_PATH')

    hub_path: bpy.props.StringProperty(name = "Unity Hub", default= msb_get_hub_path(), subtype = 'DIR_PATH')

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "project_path")
        layout.prop(self, "hub_path")