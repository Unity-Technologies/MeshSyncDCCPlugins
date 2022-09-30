import bpy
import os
import platform

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
    return path;

class MESHSYNC_Preferences(AddonPreferences):

    # this must match the add-on name, use '__package__'
    # when defining this in a submodule of a python package.
    bl_idname = __package__

    project_path: StringProperty(name = "Unity Project", default= "C:/", subtype = 'DIR_PATH')
    editors_path: bpy.props.StringProperty(name = "Unity Editors", default= msb_get_editor_path_prefix_default(), subtype = 'DIR_PATH')

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "project_path")
        layout.prop(self, "editors_path")
