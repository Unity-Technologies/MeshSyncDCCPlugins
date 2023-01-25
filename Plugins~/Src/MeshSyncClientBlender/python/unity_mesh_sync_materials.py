import bpy
from . import MeshSyncClientBlender as ms
from .unity_mesh_sync_installation import *
from .unity_mesh_sync_common import *

msb_context = ms.Context()
msb_cache = ms.Cache()

class MESHSYNC_OT_SendMaterials(bpy.types.Operator):
    bl_idname = "meshsync.send_materials"
    bl_label = "Export materials"
    def execute(self, context):
        status = msb_try_setup_scene_server(context)
        if msb_error_messages_for_status(status, context) == False:
            return {'FINISHED'}
        msb_apply_scene_settings()
        msb_context.setup(bpy.context)
        msb_context.export(msb_context.TARGET_MATERIALS)
        return {'FINISHED'}

class MESHSYNC_PT_Materials(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Materials"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(scene, "meshsync_material_sync_mode", expand=True, text = "Data Mode")
        if scene.meshsync_material_sync_mode == '1':
            layout.label(text="Please bake materials to ensure this works.")
        
        layout.operator("meshsync.send_materials", text="Sync")