bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "blender": (3, 2, 0),
    "description": "Sync Meshes with Unity",
    "location": "View3D > Mesh Sync",
    "tracker_url": "https://github.com/Unity-Technologies/MeshSyncDCCPlugins",
    "support": "COMMUNITY",
    "category": "Import-Export",
}

import bpy
from bpy.app.handlers import persistent
from . import MeshSyncClientBlender as ms
from .unity_mesh_sync_common import *
from .unity_mesh_sync_preferences import *
from .unity_mesh_sync_scene_cache import *


class MESHSYNC_PT_Main(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "MeshSync"

    def draw(self, context):
        pass


class MESHSYNC_PT_Server(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Server"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False

        layout.prop(scene, "meshsync_auto_config_server")
        
        row = layout.row()
        row.prop(scene, "meshsync_server_address")
        row.enabled = not context.scene.meshsync_auto_config_server
        
        row = layout.row()
        row.prop(scene, "meshsync_server_port")
        row.enabled = not context.scene.meshsync_auto_config_server

        row = layout.row()
        row.prop(scene, "meshsync_editor_server_port")
        row.enabled = not context.scene.meshsync_auto_config_server


class MESHSYNC_PT_Scene(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Scene"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(scene, "meshsync_scale_factor")
        layout.prop(scene, "meshsync_sync_meshes")
        if scene.meshsync_sync_meshes:
            b = layout.box()
            b.prop(scene, "meshsync_curves_as_mesh")
            b.prop(scene, "meshsync_make_double_sided")
            b.prop(scene, "meshsync_bake_modifiers")
            b.prop(scene, "meshsync_bake_transform")
        layout.prop(scene, "meshsync_sync_bones")
        layout.prop(scene, "meshsync_sync_blendshapes")
        #layout.prop(scene, "meshsync_sync_textures")
        layout.prop(scene, "meshsync_sync_cameras")
        layout.prop(scene, "meshsync_sync_lights")
        layout.prop(scene, "meshsync_material_sync_mode")
        
        layout.separator()
        if MESHSYNC_OT_AutoSync._timer:
            layout.operator("meshsync.auto_sync", text="Auto Sync", icon="PAUSE")
        else:
            layout.operator("meshsync.auto_sync", text="Auto Sync", icon="PLAY")
        layout.operator("meshsync.send_objects", text="Manual Sync")


class MESHSYNC_PT_Animation(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Animation"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(scene, "meshsync_frame_step")
        layout.operator("meshsync.send_animations", text="Sync")


class MESHSYNC_PT_Version(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Plugin Version"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.label(text = msb_context.PLUGIN_VERSION)

class MESHSYNC_PT_UnityProject(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Unity Project"
    bl_parent_id = "MESHSYNC_PT_Main"

    initialized = False

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        preferences = context.preferences
        addon_prefs = preferences.addons[__package__].preferences

        layout.prop(addon_prefs, "project_path")

class MESHSYNC_OT_AutoSync(bpy.types.Operator):
    bl_idname = "meshsync.auto_sync"
    bl_label = "Auto Sync"
    _timer = None
    _registered = False

    def __del__(self):
        MESHSYNC_OT_AutoSync._timer = None

    def execute(self, context):
        return self.invoke(context, None)

    # When a file is loaded, the modal registration is reset:
    @persistent
    def load_handler(dummy):
        MESHSYNC_OT_AutoSync._registered = False

    def invoke(self, context, event):
        scene = bpy.context.scene
        if not MESHSYNC_OT_AutoSync._timer:

            setup = msb_try_setup_scene_server(context)
            if msb_error_messages_for_status(setup, context) == False:
                return {'FINISHED'}

            scene.meshsync_auto_sync = True
            if not scene.meshsync_auto_sync:
                # server not available
                return {'FINISHED'}
            update_step = 0.01 # 1.0/3.0
            MESHSYNC_OT_AutoSync._timer = context.window_manager.event_timer_add(update_step, window=context.window)

            # There is no way to unregister modal callbacks!
            # To ensure this does not get repeatedly registered, keep track of it and only do it once:
            if not MESHSYNC_OT_AutoSync._registered:
                context.window_manager.modal_handler_add(self)
                MESHSYNC_OT_AutoSync._registered = True

            if bpy.app.background:
                import time
                while True:
                    time.sleep(update_step)
                    self.update()

            return {'RUNNING_MODAL'}
        else:
            scene.meshsync_auto_sync = False
            context.window_manager.event_timer_remove(MESHSYNC_OT_AutoSync._timer)
            MESHSYNC_OT_AutoSync._timer = None
            return {'FINISHED'}

    def modal(self, context, event):
        if event.type == "TIMER":
            self.update()
        return {'PASS_THROUGH'}

    def update(self):
        if not bpy.context.scene.meshsync_auto_sync:
            return
        msb_context.flushPendingList()
        msb_apply_scene_settings()
        msb_context.setup(bpy.context)
        msb_context.exportUpdatedObjects()

# ---------------------------------------------------------------------------------------------------------------------

classes = [
    MESHSYNC_PT_Main,
    MESHSYNC_PT_Server,
    MESHSYNC_PT_Scene,
    MESHSYNC_PT_UnityProject,
    MESHSYNC_PT_Animation,
    MESHSYNC_PT_Cache,
    MESHSYNC_PT_Version,
    MESHSYNC_OT_SendObjects,
    MESHSYNC_OT_SendAnimations,
    MESHSYNC_OT_AutoSync,
    MESHSYNC_OT_ExportCache,
    MESHSYNC_Preferences
] + sharedClasses

def register():
    bpy.app.handlers.load_post.append(MESHSYNC_OT_AutoSync.load_handler)
    for c in classes:
        bpy.utils.register_class(c)
    msb_initialize_properties()

def unregister():
    msb_context.Destroy()
    for c in classes:
        bpy.utils.unregister_class(c)

def DestroyMeshSyncContext():
    msb_context.Destroy()

import atexit
atexit.register(DestroyMeshSyncContext)

@persistent
def on_depsgraph_update_post(scene):
    graph = bpy.context.evaluated_depsgraph_get()
    msb_context.setup(bpy.context)
    msb_context.OnDepsgraphUpdatePost(graph)

bpy.app.handlers.depsgraph_update_post.append(on_depsgraph_update_post)
bpy.app.handlers.load_post.append(on_depsgraph_update_post)
    
# ---------------------------------------------------------------------------------------------------------------------

if __name__ == "__main__":
    register()
