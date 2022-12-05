import os
import re
import bpy
import platform

from bpy.app.handlers import persistent
from . import MeshSyncClientBlender as ms

from .unity_mesh_sync_installation import *

msb_context = ms.Context()
msb_cache = ms.Cache()


def msb_apply_scene_settings(self=None, context=None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.server_address = scene.meshsync_server_address
    ctx.server_port = scene.meshsync_server_port
    ctx.editor_server_port = scene.meshsync_editor_server_port
    ctx.scale_factor = scene.meshsync_scale_factor
    ctx.sync_meshes = scene.meshsync_sync_meshes
    ctx.curves_as_mesh = scene.meshsync_curves_as_mesh
    ctx.make_double_sided = scene.meshsync_make_double_sided
    ctx.bake_modifiers = scene.meshsync_bake_modifiers
    ctx.bake_transform = scene.meshsync_bake_transform
    ctx.sync_bones = scene.meshsync_sync_bones
    ctx.sync_blendshapes = scene.meshsync_sync_blendshapes
    ctx.sync_textures = scene.meshsync_sync_textures
    ctx.sync_cameras = scene.meshsync_sync_cameras
    ctx.sync_lights = scene.meshsync_sync_lights
    ctx.material_sync_mode = int(scene.meshsync_material_sync_mode)
    return None


def msb_apply_animation_settings(self=None, context=None):
    ctx = msb_context
    scene = bpy.context.scene
    ctx.frame_step = scene.meshsync_frame_step
    return None


def msb_on_material_sync_updated(self=None, context=None):
    msb_context.resetMaterials()
    return None


def msb_on_scene_settings_updated(self=None, context=None):
    msb_apply_scene_settings()
    if bpy.context.scene.meshsync_auto_sync:
        msb_context.setup(bpy.context)
        msb_context.export(msb_context.TARGET_OBJECTS)
    return None


def msb_on_bake_modifiers_updated(self=None, context=None):
    scene = bpy.context.scene
    if not scene.meshsync_bake_modifiers:
        scene.meshsync_bake_transform = False
    return msb_on_scene_settings_updated(self, context)


def msb_on_bake_transform_updated(self=None, context=None):
    scene = bpy.context.scene
    if scene.meshsync_bake_transform:
        scene.meshsync_bake_modifiers = True
    return msb_on_scene_settings_updated(self, context)


def msb_on_toggle_auto_sync(self=None, context=None):
    msb_apply_scene_settings()
    if bpy.context.scene.meshsync_auto_sync:
        if not msb_context.is_server_available:
            print("MeshSync: " + msb_context.error_message)
            bpy.context.scene.meshsync_auto_sync = False
    if bpy.context.scene.meshsync_auto_sync:
        msb_context.setup(bpy.context)
        msb_context.export(msb_context.TARGET_OBJECTS)
    return None


def msb_on_animation_settings_updated(self=None, context=None):
    # nothing to do for now
    return None


def msb_on_unity_project_path_updated(self=None, context=None):
    # TODO invoke callback for installing meshsync
    return None


def msb_initialize_properties():
    # sync settings
    bpy.types.Scene.meshsync_auto_config_server = bpy.props.BoolProperty(name="Auto Config (Local server)",
                                                                         default=True,
                                                                         update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_server_address = bpy.props.StringProperty(name="Address", default="127.0.0.1",
                                                                       update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_server_port = bpy.props.IntProperty(name="Port", default=8080, min=0, max=65535,
                                                                 update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_editor_server_port = bpy.props.IntProperty(name="Unity Editor Port", default=8081, min=0,
                                                                        max=65535, update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_scale_factor = bpy.props.FloatProperty(name="Scale Factor", default=1.0,
                                                                    update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_meshes = bpy.props.BoolProperty(name="Sync Meshes", default=True,
                                                                  update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_curves_as_mesh = bpy.props.BoolProperty(name="Curves as Mesh", default=True,
                                                                     update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_make_double_sided = bpy.props.BoolProperty(name="Make Double Sided", default=False,
                                                                        update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_bake_modifiers = bpy.props.BoolProperty(name="Bake Modifiers", default=False,
                                                                     update=msb_on_bake_modifiers_updated)
    bpy.types.Scene.meshsync_bake_transform = bpy.props.BoolProperty(name="Bake Transform", default=False,
                                                                     update=msb_on_bake_transform_updated)
    bpy.types.Scene.meshsync_sync_bones = bpy.props.BoolProperty(name="Sync Bones", default=True,
                                                                 update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_blendshapes = bpy.props.BoolProperty(name="Sync Blend Shapes", default=True,
                                                                       update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_textures = bpy.props.BoolProperty(name="Sync Textures", default=True,
                                                                    update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_cameras = bpy.props.BoolProperty(name="Sync Cameras", default=True,
                                                                   update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_sync_lights = bpy.props.BoolProperty(name="Sync Lights", default=True,
                                                                  update=msb_on_scene_settings_updated)
    bpy.types.Scene.meshsync_auto_sync = bpy.props.BoolProperty(name="Auto Sync", default=False,
                                                                update=msb_on_toggle_auto_sync)
    bpy.types.Scene.meshsync_frame_step = bpy.props.IntProperty(name="Frame Step", default=1, min=1,
                                                                update=msb_on_animation_settings_updated)
    bpy.types.Scene.meshsync_material_sync_mode = bpy.props.EnumProperty(name="Material sync mode",
                                                                         items=(('0', 'None',
                                                                                 'Sync material IDs but no material data'),
                                                                                ('1', 'Basic',
                                                                                 'Sync colors and textures assigned to the BSDF'),
                                                                                ('2', 'Baked',
                                                                                 'Sync colors and textures assigned to the BSDF and bake them if needed')
                                                                                ),
                                                                         default='0',
                                                                         update=msb_on_material_sync_updated)
    bpy.types.Scene.meshsync_bakedTexturesPath = bpy.props.StringProperty(name="Baked texture path", default='')
    bpy.types.Scene.meshsync_bakedTextureSize = bpy.props.IntVectorProperty(name="Baked texture size", size=2,
                                                                            default=(512, 512))
    bpy.types.Scene.meshsync_bake_selection = bpy.props.EnumProperty(name="Bake",
                                                                         items=(('ALL', 'All',
                                                                                 'Bake all objects in the scene (including hidden)'),
                                                                                ('SELECTED', 'Selected',
                                                                                 'Bake only the selected object')),
                                                                         default='ALL')



@persistent
def on_scene_load(context):
    msb_context.clear()


@persistent
def on_scene_update(context):
    msb_context.flushPendingList()
    if (bpy.context.scene.meshsync_auto_sync):
        msb_apply_scene_settings()
        msb_context.setup(bpy.context)
        msb_context.exportUpdatedObjects()


class MESHSYNC_OT_SendObjects(bpy.types.Operator):
    bl_idname = "meshsync.send_objects"
    bl_label = "Export Objects"

    def execute(self, context):
        # Try to ensure there is a scene server running
        status = msb_try_setup_scene_server(context)
        if msb_error_messages_for_status(status, context) == False:
            return {'FINISHED'}

        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_OBJECTS)
        return {'FINISHED'}


class MESHSYNC_OT_SendAnimations(bpy.types.Operator):
    bl_idname = "meshsync.send_animations"
    bl_label = "Export Animations"

    def execute(self, context):
        # Try to ensure there is a scene server running
        status = msb_try_setup_scene_server(context)
        if msb_error_messages_for_status(status, context) == False:
            return {'FINISHED'}

        msb_apply_scene_settings()
        msb_apply_animation_settings()
        msb_context.setup(bpy.context);
        msb_context.export(msb_context.TARGET_ANIMATIONS)
        return {'FINISHED'}

class MESHSYNC_PT:
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tool"

from .unity_mesh_sync_baking import MESHSYNC_OT_select_bake_folder, MESHSYNC_OT_Bake, MESHSYNC_OT_RevertBake, MESHSYNC_PT_Baking

sharedClasses = [MESHSYNC_OT_select_bake_folder,
                 MESHSYNC_OT_Bake,
                 MESHSYNC_OT_RevertBake]