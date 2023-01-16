import bpy
from . import MeshSyncClientBlender as ms
from .unity_mesh_sync_common import *

class MESHSYNC_PT_Cache(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Cache"
    bl_parent_id = "MESHSYNC_PT_Main"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.operator("meshsync.export_cache", text="Export Cache")

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
    export_instances: bpy.props.BoolProperty(name = "Export Instances", default = False)

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
        ctx.export_instances = self.export_instances

        ctx.export(self.filepath)
        MS_MessageBox("Finished writing scene cache to " + self.filepath)
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
        self.frame_step = round(ctx.frame_step);
        self.curves_as_mesh = ctx.curves_as_mesh;
        self.make_double_sided = ctx.make_double_sided;
        self.bake_modifiers = ctx.bake_modifiers;
        self.bake_transform = ctx.bake_transform;
        self.flatten_hierarchy = ctx.flatten_hierarchy;
        self.merge_meshes = ctx.merge_meshes;
        self.strip_normals = ctx.strip_normals;
        self.strip_tangents = ctx.strip_tangents;
        self.export_instances = ctx.export_instances;

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
        layout.prop(self, "strip_normals")
        layout.prop(self, "strip_tangents")
        layout.prop(self, "export_instances")