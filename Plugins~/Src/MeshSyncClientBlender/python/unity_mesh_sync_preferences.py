import bpy
import os
import platform

from .unity_mesh_sync_installation import *
from . import MeshSyncClientBlender as ms
from bpy.types import AddonPreferences
from bpy.props import StringProperty, IntProperty, BoolProperty

msb_context = ms.Context()


def msb_get_editor_path_prefix_default():
    os = platform.system()
    if os == 'Windows':
        path = "C:\\Program Files\\Unity\\Hub\\Editor"
    elif os == 'Darwin' or os == 'Linux':
        path = "/Applications/Unity/Hub/Editor"
    return path

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

    def hub_exists(self):
        return os.path.exists(self.hub_path)

    def editors_path_exists(self):
        return os.path.exists(self.editors_path)

    def is_hub_installed():
        path = msb_get_hub_path()
        return dir is not None and os.path.exists(path)

    def reset(self):
        self.hub_path = msb_get_hub_path()
        self.editors_path = msb_get_editors_path()
        self.hub_installed = MESHSYNC_Preferences.is_hub_installed()

    project_path: StringProperty(name = "Unity Project", default= "C:/", subtype = 'DIR_PATH')
    editors_path: bpy.props.StringProperty(name = "Unity Editors", default= msb_get_editor_path_prefix_default(), subtype = 'DIR_PATH')
    hub_path: bpy.props.StringProperty(name = "Unity Hub", default = msb_get_hub_path(), subtype = 'FILE_PATH')
    hub_installed : bpy.props.BoolProperty(name = "Hub Installed", default = is_hub_installed())

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

    def register():
        bpy.utils.register_class(MESHSYNC_OT_ResetPreferences)

    def unregister():
        bpy.utils.unregister_class(MESHSYNC_OT_ResetPreferences)
