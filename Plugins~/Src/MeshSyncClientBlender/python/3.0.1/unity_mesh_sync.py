bl_info = {
    "name": "Unity Mesh Sync",
    "author": "Unity Technologies",
    "blender": (3, 0, 1),
    "description": "Sync Meshes with Unity",
    "location": "View3D > Mesh Sync",
    "tracker_url": "https://github.com/Unity-Technologies/MeshSyncDCCPlugins",
    "support": "COMMUNITY",
    "category": "Import-Export",
}

import bpy
from bpy.app.handlers import persistent
import MeshSyncClientBlender as ms
from unity_mesh_sync_common import *

class MESHSYNC_PT:
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tool"


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
        layout.prop(scene, "meshsync_server_address")
        layout.prop(scene, "meshsync_server_port")


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
        layout.separator()
        if MESHSYNC_OT_AutoSync._timer:
            layout.operator("meshsync.auto_sync", text="Auto Sync", icon="PAUSE")
        else:
            layout.operator("meshsync.auto_sync", text="Auto Sync", icon="PLAY")
        layout.operator("meshsync.send_objects", text="Manual Sync")

class MESHSYNC_PT_MaterialBake(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Bake Materials"
    bl_parent_id = "MESHSYNC_PT_Main"
    
    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        
        box = layout.box()
        col = box.column(align=True)

        row = col.row(align = True)
        row.label(text="Width:")
        # row.operator("brm.bakeuiincrement", text="", icon="REMOVE").target = "width/2"
        row.prop(context.scene, "bakeWidth", text="")
        # row.operator("brm.bakeuiincrement", text="", icon="ADD").target = "width*2"
        
        row = col.row(align = True)
        row.label(text="Height:")
        # row.operator("brm.bakeuiincrement", text="", icon="REMOVE").target = "height/2"
        row.prop(context.scene, "bakeHeight", text="")
        # row.operator("brm.bakeuiincrement", text="", icon="ADD").target = "height*2"
        
        layout.label(text="Export Dir:")
        layout.prop(context.scene, 'bakeFolder', text="")
        layout.label(text="Filename:")
        layout.prop(context.scene, "bakePrefix", text="")
        
        box = layout.box()
        col = box.column(align=True)

        row = col.row(align = True)
        row.label(text="Samples:")        
        row.prop(context.scene, "samples", text="")
        row = col.row(align = True)
        row.label(text="Smart UV Project:")        
        row.prop(context.scene, "smartUV", text="")
        
        layout.operator("meshsync.dothing", text="Do Thing")
        

class MESHSYNC_PT_DoThing(bpy.types.Operator):
    bl_idname = "meshsync.dothing"        
    bl_label = "Do the thing" 
    
    def execute(self, context): 
        scene = context.scene
        
        hasfolder = os.access(scene.bakeFolder, os.W_OK)
        if hasfolder is False:
            self.report({'WARNING'}, "Select a valid export folder!")
            return {'FINISHED'}
            
        if scene.smartUV :
            if bpy.context.object.mode == 'OBJECT':
                bpy.ops.object.mode_set(mode='EDIT')
                 
            bpy.ops.mesh.select_mode(use_extend=False, use_expand=False, type='VERT')
            bpy.ops.mesh.select_all(action='SELECT')
            bpy.ops.mesh.select_linked(delimit={'SEAM'})
            bpy.ops.uv.smart_project(island_margin=0.01, # angle_limit=math.radians(settings.uvAngleLimit), 
                                    scale_to_bounds=True)
            bpy.ops.uv.pack_islands(rotate=True, margin=0.001)
            
        if bpy.context.object.mode == 'EDIT':
                bpy.ops.object.mode_set(mode='OBJECT')
                
        scene.render.engine = "CYCLES"
        scene.cycles.device = "GPU"
        scene.cycles.samples = scene.samples

        diffuseBakeImage = bpy.data.images.new("Bake"+scene.affixColor, width=scene.bakeWidth, height=scene.bakeHeight)
        #bakemat = bpy.data.materials.new(name="bakemat")
        #bakemat.use_nodes = true

        for mat in bpy.context.active_object.data.materials:
            node_tree = mat.node_tree
            node = node_tree.nodes.new("ShaderNodeTexImage")
            node.select = True
            node_tree.nodes.active = node
            node.image = diffuseBakeImage
        
        scene.render.bake.use_pass_direct = False
        scene.render.bake.use_pass_indirect = False
        scene.render.bake.use_pass_color = True

        bpy.ops.object.bake(type='DIFFUSE', use_clear=True, use_selected_to_active=False)
        diffuseBakeImage.filepath_raw = scene.bakeFolder+scene.bakePrefix+scene.affixColor+".png"
        diffuseBakeImage.file_format = 'PNG'
        diffuseBakeImage.save()
        
        normalBakeImage = bpy.data.images.new("Bake"+scene.affixNormal, width=scene.bakeWidth, height=scene.bakeHeight)

        for mat in bpy.context.active_object.data.materials:
            node_tree = mat.node_tree
            node = node_tree.nodes.active
            node.image = normalBakeImage
        
        bpy.ops.object.bake(type='NORMAL', use_clear=True, use_selected_to_active=False, normal_space='TANGENT')
        normalBakeImage.filepath_raw = scene.bakeFolder+scene.bakePrefix+scene.affixNormal+".png"
        normalBakeImage.file_format = 'PNG'
        normalBakeImage.save()
        
        aoBakeImage = bpy.data.images.new("Bake"+scene.affixAO, width=scene.bakeWidth, height=scene.bakeHeight)

        for mat in bpy.context.active_object.data.materials:
            node_tree = mat.node_tree
            node = node_tree.nodes.active
            node.image = aoBakeImage
        
        bpy.ops.object.bake(type='AO', use_clear=True, use_selected_to_active=False)
        aoBakeImage.filepath_raw = scene.bakeFolder+scene.bakePrefix+scene.affixAO+".png"
        aoBakeImage.file_format = 'PNG'
        aoBakeImage.save()
        
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.object.mode_set(mode='OBJECT')
        
        original_type = bpy.context.area.type
        bpy.context.area.type = "IMAGE_EDITOR"
        uvfilepath = scene.bakeFolder+scene.bakePrefix+scene.affixUV+".png"
        bpy.ops.uv.export_layout(filepath=uvfilepath, size=(context.scene.bakeWidth, context.scene.bakeHeight))
        bpy.context.area.type = original_type

        return {'FINISHED'} 
        

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


class MESHSYNC_PT_Cache(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Cache"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.operator("meshsync.export_cache", text="Export Cache")


class MESHSYNC_PT_Version(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Plugin Version"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.label(text = msb_context.PLUGIN_VERSION)


class MESHSYNC_OT_AutoSync(bpy.types.Operator):
    bl_idname = "meshsync.auto_sync"
    bl_label = "Auto Sync"
    _timer = None

    def invoke(self, context, event):
        scene = bpy.context.scene
        if not MESHSYNC_OT_AutoSync._timer:
            scene.meshsync_auto_sync = True
            if not scene.meshsync_auto_sync:
                # server not available
                return {'FINISHED'}
            MESHSYNC_OT_AutoSync._timer = context.window_manager.event_timer_add(1.0 / 3.0, window=context.window)
            context.window_manager.modal_handler_add(self)
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
        msb_context.flushPendingList();
        msb_apply_scene_settings()
        msb_context.setup(bpy.context);
        msb_context.exportUpdatedObjects()


class MESHSYNC_OT_ExportCache(bpy.types.Operator):
    bl_idname = "meshsync.export_cache"
    bl_label = "Export Cache"
    bl_description = "Export Cache"

    def on_bake_modifiers_updated(self = None, context = None):
        if not self.bake_modifiers:
            self.bake_transform = False

    def on_bake_transform_updated(self = None, context = None):
        if self.bake_transform:
            self.bake_modifiers = True

    filepath: bpy.props.StringProperty(subtype = "FILE_PATH")
    filename: bpy.props.StringProperty()
    directory: bpy.props.StringProperty(subtype = "FILE_PATH")

    # cache properties
    object_scope: bpy.props.EnumProperty(
        name = "Object Scope",
        default = "0",
        items = {
            ("0", "All", "Export all objects"),
            ("1", "Selected", "Export selected objects"),
        })
    frame_range: bpy.props.EnumProperty(
        name = "Frame Range",
        default = "1",
        items = {
            ("0", "Current", "Export current frame"),
            ("1", "All", "Export all frames"),
            ("2", "Custom", "Export speficied frames"),
        })
    frame_begin: bpy.props.IntProperty(name = "Frame Begin", default = 1)
    frame_end: bpy.props.IntProperty(name = "Frame End", default = 100)
    frame_step: bpy.props.IntProperty(name = "Frame Step", default = 1, min = 1)
    material_frame_range: bpy.props.EnumProperty(
        name = "Material Range",
        default = "1",
        items = {
            ("0", "None", "Export no materials"),
            ("1", "One", "Export one frame of materials"),
            ("2", "All", "Export all frames of materials"),
        })
    zstd_compression_level: bpy.props.IntProperty(name = "ZSTD Compression", default = 3)
    curves_as_mesh: bpy.props.BoolProperty(name = "Curves as Mesh", default = True)
    make_double_sided: bpy.props.BoolProperty(name = "Make Double Sided", default = False)
    bake_modifiers: bpy.props.BoolProperty(name = "Bake Modifiers", default = True, update = on_bake_modifiers_updated)
    bake_transform: bpy.props.BoolProperty(name = "Bake Transform", default = False, update = on_bake_transform_updated)
    flatten_hierarchy: bpy.props.BoolProperty(name = "Flatten Hierarchy", default = False)
    merge_meshes: bpy.props.BoolProperty(name = "Merge Meshes", default = False)
    strip_normals: bpy.props.BoolProperty(name = "Strip Normals", default = False)
    strip_tangents: bpy.props.BoolProperty(name = "Strip Tangents", default = False)

    def execute(self, context):
        ctx = msb_cache
        ctx.object_scope = int(self.object_scope)
        ctx.frame_range = int(self.frame_range)
        ctx.frame_begin = self.frame_begin
        ctx.frame_end = self.frame_end
        ctx.material_frame_range = int(self.material_frame_range)
        ctx.zstd_compression_level = self.zstd_compression_level
        ctx.frame_step = self.frame_step
        ctx.curves_as_mesh = self.curves_as_mesh
        ctx.make_double_sided = self.make_double_sided
        ctx.bake_modifiers = self.bake_modifiers
        ctx.bake_transform = self.bake_transform
        ctx.flatten_hierarchy = self.flatten_hierarchy
        ctx.merge_meshes = self.merge_meshes
        ctx.strip_normals = self.strip_normals
        ctx.strip_tangents = self.strip_tangents
        ctx.export(self.filepath)
        return {'FINISHED'}

    def invoke(self, context, event):
        msb_context.setup(bpy.context)
        ctx = msb_cache
        self.object_scope = str(ctx.object_scope);
        self.frame_range = str(ctx.frame_range);
        self.frame_begin = ctx.frame_begin;
        self.frame_end = ctx.frame_end;
        self.material_frame_range = str(ctx.material_frame_range);
        self.frame_end = ctx.frame_end;
        self.zstd_compression_level = ctx.zstd_compression_level;
        self.frame_step = ctx.frame_step;
        self.curves_as_mesh = ctx.curves_as_mesh;
        self.make_double_sided = ctx.make_double_sided;
        self.bake_modifiers = ctx.bake_modifiers;
        self.bake_transform = ctx.bake_transform;
        self.flatten_hierarchy = ctx.flatten_hierarchy;
        self.merge_meshes = ctx.merge_meshes;
        self.strip_normals = ctx.strip_normals;
        self.strip_tangents = ctx.strip_tangents;

        path = bpy.data.filepath
        if len(path) != 0:
            tmp = os.path.split(path)
            self.directory = tmp[0]
            self.filename = re.sub(r"\.[^.]+$", ".sc", tmp[1])
        else:
            self.directory = ""
            self.filename = "Untitled.sc";
        wm = bpy.context.window_manager
        wm.fileselect_add(self)
        return {'RUNNING_MODAL'}

    def draw(self, context):
        layout = self.layout
        if hasattr(layout, "use_property_split"): # false on 2.79
            layout.use_property_split = True
        layout.prop(self, "object_scope")
        layout.prop(self, "frame_range")
        if self.frame_range == "2":
            b = layout.box()
            b.prop(self, "frame_begin")
            b.prop(self, "frame_end")
        layout.prop(self, "material_frame_range")
        layout.prop(self, "zstd_compression_level")
        layout.prop(self, "frame_step")
        layout.prop(self, "curves_as_mesh")
        layout.prop(self, "make_double_sided")
        layout.prop(self, "bake_modifiers")
        layout.prop(self, "bake_transform")
        layout.prop(self, "flatten_hierarchy")
        #layout.prop(self, "merge_meshes")
        layout.prop(self, "strip_normals")
        layout.prop(self, "strip_tangents")

# ---------------------------------------------------------------------------------------------------------------------

classes = (
    MESHSYNC_PT_Main,
    MESHSYNC_PT_Server,
    MESHSYNC_PT_Scene,
    MESHSYNC_PT_MaterialBake,
    MESHSYNC_PT_DoThing,
    MESHSYNC_PT_Animation,
    MESHSYNC_PT_Cache,
    MESHSYNC_PT_Version,
    MESHSYNC_OT_SendObjects,
    MESHSYNC_OT_SendAnimations,
    MESHSYNC_OT_AutoSync,
    MESHSYNC_OT_ExportCache,
)

def register():
    msb_initialize_properties()
    for c in classes:
        bpy.utils.register_class(c)
        
    bpy.types.Scene.bakeWidth = bpy.props.IntProperty (name = "bakeWidth",default = 512,description = "Export Texture Width")  
    bpy.types.Scene.bakeHeight = bpy.props.IntProperty (name = "bakeHeight",default = 512,description = "Export Texture Height")
    bpy.types.Scene.bakePrefix = bpy.props.StringProperty (name = "bakePrefix",default = "testname", description = "Export filename")
    bpy.types.Scene.bakeFolder = bpy.props.StringProperty (name = "bakeFolder",default = "C:\\temp\\", description = "Destination folder", subtype = 'DIR_PATH')
    
    bpy.types.Scene.smartUV = bpy.props.BoolProperty (name = "smartUV",default = False,description = "Do a Smart UV Project on object")
    
    bpy.types.Scene.samples = bpy.props.IntProperty (name = "samples",default = 10,description = "Sample Count")
    
    bpy.types.Scene.affixNormal = bpy.props.StringProperty (name = "affixNormal",default = "_normal",description = "normal map affix")
    bpy.types.Scene.affixObject = bpy.props.StringProperty (name = "affixObject",default = "_object",description = "object normal map affix")
    bpy.types.Scene.affixAO = bpy.props.StringProperty (name = "affixAO",default = "_ao",description = "AO map affix")
    bpy.types.Scene.affixColor = bpy.props.StringProperty (name = "affixColor",default = "_color",description = "color map affix")
    bpy.types.Scene.affixRoughness = bpy.props.StringProperty (name = "affixRoughness",default = "_rough",description = "Roughness map affix")
    bpy.types.Scene.affixEmission = bpy.props.StringProperty (name = "affixEmission",default = "_emit",description = "Emission map affix")
    bpy.types.Scene.affixUV = bpy.props.StringProperty (name = "affixUV",default = "_uv",description = "UV map affix")

def unregister():
    msb_context.Destroy()
    for c in classes:
        bpy.utils.unregister_class(c)

def DestroyMeshSyncContext():
    msb_context.Destroy()

import atexit
atexit.register(DestroyMeshSyncContext)
    
# ---------------------------------------------------------------------------------------------------------------------

if __name__ == "__main__":
    register()
