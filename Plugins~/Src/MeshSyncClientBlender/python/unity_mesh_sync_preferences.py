import bpy
import os
import platform
import atexit

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

    thread = None
    cancel_thread = False

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
        self.is_meshsync_in_manifest = msb_meshsync_version_manifest(self.project_path) != ""
        self.is_meshsync_in_manifest_lock = msb_meshsync_version_package_lock(self.project_path) != ""
        self.is_project_running = msb_is_project_open(self.project_path)

    def on_project_path_updated(self, context):
        self.update_project_info()


    def monitor_package_lock(self, context):
        while not self.cancel_thread:
            sleep(0.5)
            if msb_meshsync_version_package_lock(self.project_path) != "":
                self.is_meshsync_in_manifest_lock = True
                self.redraw(context)
                return


    def on_in_package_lock_updated(self, context):
        if self.thread is not None:
            self.cancel_thread = True
            self.thead.join(2.0)

        if not self.is_meshsync_in_manifest_lock:
            self.thread = Thread(target = self.monitor_package_lock, args = (context,), daemon = True)
            self.cancel_thread = False
            self.thread.start()
            atexit.unregister(self.shutdown_thread)
            atexit.register(self.shutdown_thread)

    def shutdown_thread():
        MESHSYNC_Preferences.cancel_thread = True
        if MESHSYNC_Preferences.thread is not None:
            MESHSYNC_Preferences.thread.join(2.0)

    def redraw(self, context):
        regions = context.area
        if regions == None:
            return None

        for region in context.area.regions:
            if region.type == "UI":
                region.tag_redraw()
        return None

    project_path: StringProperty(name = "Unity Project", default= "C:/", subtype = 'DIR_PATH', update = on_project_path_updated)
    editors_path: bpy.props.StringProperty(name = "Unity Editors", default= msb_get_editor_path_prefix_default(), subtype = 'DIR_PATH')
    hub_path: bpy.props.StringProperty(name = "Unity Hub", default = msb_get_hub_path(), subtype = 'FILE_PATH')
    hub_installed: bpy.props.BoolProperty(name = "Hub Installed", default = is_hub_installed())
    is_unity_project: bpy.props.BoolProperty(name = "Is Unity project", default = False)
    is_meshsync_in_manifest: bpy.props.BoolProperty(name = "Is Meshsync added in the package manifest", default = False)
    is_meshsync_in_manifest_lock: bpy.props.BoolProperty(name= "Is Meshsync resolved by the unity editor", default = False, update = on_in_package_lock_updated)
    is_project_running: bpy.props.BoolProperty(name = "Is the project open", default = False)

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
            project_layout.label(text = "Not a Unity Project. Please select a Unity Project folder (parent of Assets folder).", icon = 'ERROR')
            return

        project_layout.label(text = "Valid project path.", icon = "CHECKMARK")

        if not self.is_meshsync_in_manifest:
            row = project_layout.row()
            row.label(text = "MeshSync is missing from project manifest.", icon = 'ERROR')
            row.operator("meshsync.install_meshsync", text = 'Add to manifest')
            return

        project_layout.label(text = "MeshSync found in project manifest.", icon = 'CHECKMARK')

        if not self.is_meshsync_in_manifest_lock:
            row = project_layout.row()
            if self.is_project_running:
                row.label(text = "Meshsync is not loaded from the manifest. Select the project window to allow the Unity Editor to load MeshSync.", icon = 'ERROR')
                return
            else:
                row.label(text = "Meshsync will be loaded from the manifest the next time you launch the project.", icon = 'CHECKMARK')
        else:
            row = project_layout.row()
            row.label(text = "MeshSync loaded from manifest", icon = 'CHECKMARK')

        project_layout.label(text = "All set up! Use the MeshSync panel in Active Tool and Workspace Settings to sync data to your project.")

    def register():
        bpy.utils.register_class(MESHSYNC_OT_ResetPreferences)
        bpy.utils.register_class(MESHSYNC_OT_InstallMeshSync)

    def unregister():
        bpy.utils.unregister_class(MESHSYNC_OT_ResetPreferences)
        bpy.utils.unregister_class(MESHSYNC_OT_InstallMeshSync)
