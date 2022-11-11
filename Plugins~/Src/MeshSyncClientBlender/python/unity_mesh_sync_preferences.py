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

class MESHSYNC_OT_InstallMeshSync(bpy.types.Operator):
    bl_idname = "meshsync.install_meshsync"
    bl_label = "Install MeshSync"

    @classmethod
    def description(cls, context, properties):
        return "Install MeshSync for the selected project"

    def execute(self, context):
        entry = msb_get_meshsync_entry()
        msb_add_meshsync_to_unity_manifest(msb_preferences(context).project_path, entry)
        msb_preferences(context).update_project_info()

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

    def update_project_info(self):
        self.is_unity_project = msb_validate_project_path(self.project_path)
        self.is_meshsync_installed = msb_meshsync_version_manifest(self.project_path) != ""

    def on_project_path_updated(self, context):
        self.update_project_info()


    project_path: StringProperty(name = "Unity Project", default= "C:/", subtype = 'DIR_PATH', update = on_project_path_updated)
    editors_path: bpy.props.StringProperty(name = "Unity Editors", default= msb_get_editor_path_prefix_default(), subtype = 'DIR_PATH')
    hub_path: bpy.props.StringProperty(name = "Unity Hub", default = msb_get_hub_path(), subtype = 'FILE_PATH')
    hub_installed: bpy.props.BoolProperty(name = "Hub Installed", default = is_hub_installed())
    is_unity_project: bpy.props.BoolProperty(name = "Is Unity project", default = False)
    is_meshsync_installed: bpy.props.BoolProperty(name = "Is Meshsync installed", default = False)

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

        if not self.is_unity_project:
            project_layout.label(text = "Not a Unity Project. Please select a Unity Project folder.", icon = 'ERROR')
            return

        project_layout.label(text = "Valid Unity Project path.", icon = "CHECKMARK")

        if not self.is_meshsync_installed:
            row = project_layout.row()
            row.label(text = "MeshSync is not installed", icon = 'ERROR')
            row.operator("meshsync.install_meshsync")
            return

        project_layout.label(text = "MeshSync installed in Unity Project.", icon = 'CHECKMARK')
        project_layout.label(text = "All set up! Use the MeshSync panel in Active Tool and Workspace Settings to sync data to your project.")

    def register():
        bpy.utils.register_class(MESHSYNC_OT_ResetPreferences)
        bpy.utils.register_class(MESHSYNC_OT_InstallMeshSync)

    def unregister():
        bpy.utils.unregister_class(MESHSYNC_OT_ResetPreferences)
        bpy.utils.unregister_class(MESHSYNC_OT_InstallMeshSync)
