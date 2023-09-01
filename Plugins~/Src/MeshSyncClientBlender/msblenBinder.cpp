#include "pch.h"
#include "msblenContext.h"
#include "msblenUtils.h"
#include "msblenBinder.h"

#include "MeshUtils/muLog.h"

#include "BlenderPyObjects/BlenderPyDepsgraph.h"
#include "BlenderPyObjects/BlenderPyDepsgraphObjectInstance.h"
#include "BlenderPyObjects/BlenderPyContext.h"
#include "BlenderPyObjects/BlenderPyScene.h"
#include "BlenderPyObjects/BlenderPyCommon.h" //call, etc

namespace blender {

bContext *g_context;

extern PropertyRNA* BlenderPyID_is_updated;
extern PropertyRNA* BlenderPyID_is_updated_data;
extern FunctionRNA* BlenderPyID_evaluated_get;
extern FunctionRNA* BlenderPyID_update_tag;

StructRNA* BObject::s_type;
static PropertyRNA* BObject_matrix_local;
static PropertyRNA* BObject_matrix_world;
static PropertyRNA* BObject_hide;
static PropertyRNA* BObject_hide_viewport;
static PropertyRNA* BObject_hide_render;
static PropertyRNA* BObject_select;
static FunctionRNA* BObject_select_get;
static FunctionRNA* BObject_to_mesh;
static FunctionRNA* BObject_to_mesh_clear;
static FunctionRNA* BObject_modifiers_clear;

StructRNA* BMesh::s_type;
static FunctionRNA* BMesh_calc_normals_split;
static FunctionRNA* BMesh_update;
static FunctionRNA* BMesh_clear_geometry;
static FunctionRNA* BMesh_vertices_add;
static FunctionRNA* BMesh_polygons_add;
static FunctionRNA* BMesh_loops_add;
static FunctionRNA* BMesh_edges_add;
static FunctionRNA* BMesh_normals_add;
static PropertyRNA* BMesh_polygons;
static PropertyRNA* UVLoopLayers_active;
static PropertyRNA* LoopColors_active;

StructRNA* BCurve::s_type;
static PropertyRNA* BCurve_splines;
static FunctionRNA* BCurve_splines_clear;
static FunctionRNA* BCurve_splines_new;

StructRNA* BNurb::s_type;
static FunctionRNA* BNurb_splines_bezier_add;

StructRNA* BMaterial::s_type;
static PropertyRNA* BMaterial_use_nodes;
static PropertyRNA* BMaterial_active_node_material;

StructRNA* BCamera::s_type;
static PropertyRNA* BCamera_clip_start;
static PropertyRNA* BCamera_clip_end;
static PropertyRNA* BCamera_angle_x;
static PropertyRNA* BCamera_angle_y;
static PropertyRNA* BCamera_lens;
static PropertyRNA* BCamera_sensor_fit;
static PropertyRNA* BCamera_sensor_width;
static PropertyRNA* BCamera_sensor_height;
static PropertyRNA* BCamera_shift_x;
static PropertyRNA* BCamera_shift_y;

extern PropertyRNA* BlenderPyScene_frame_start;
extern PropertyRNA* BlenderPyScene_frame_end;
extern PropertyRNA* BlenderPyScene_frame_current;
extern FunctionRNA* BlenderPyScene_frame_set;

StructRNA* BData::s_type;
static PropertyRNA* BlendDataObjects_is_updated;
static FunctionRNA* BlendDataMeshes_remove;

extern PropertyRNA* BlenderPyContext_blend_data;
extern PropertyRNA* BlenderPyContext_scene;
extern FunctionRNA* BlenderPyContext_evaluated_depsgraph_get;
extern FunctionRNA* BlenderPyContext_depsgraph_update;
extern PropertyRNA* BlenderPyContext_view_layer;

extern PropertyRNA* BlenderPyDepsgraphObjectInstance_instance_object;
extern PropertyRNA* BlenderPyDepsgraphObjectInstance_is_instance;
extern PropertyRNA* BlenderPyDepsgraphObjectInstance_world_matrix;
extern PropertyRNA* BlenderPyDepsgraphObjectInstance_parent;
extern PropertyRNA* BlenderPyDepsgraphObjectInstance_object;

extern PropertyRNA* BlenderPyDepsgraph_object_instances;

static FunctionRNA* BlenderPyAttribute_new;

bool ready()
{
    return g_context != nullptr;
}

// context: bpi.context in python
void setup(py::object bpy_context)
{
    if (g_context)
        return;

    BPy_StructRNA* rna = (BPy_StructRNA*)bpy_context.ptr();
    if (strcmp(rna->ob_base.ob_type->tp_name, "Context") != 0) {
        return;
    }

    auto first_type = (StructRNA*)&rna->ptr.type->cont;
    while (first_type->cont.prev) {
        first_type = (StructRNA*)first_type->cont.prev;
    }
    rna_sdata(bpy_context, g_context);

    // resolve blender types and functions
#define match_type(N) strcmp(type->identifier, N) == 0
#define each_func for (auto *f : list_range((FunctionRNA*)type->functions.first))
#define each_prop for (auto *f : list_range((PropertyRNA*)type->cont.properties.first))
#define assign(NAME, FUNC_OR_PROP) if (strcmp(f->identifier, (NAME)) == 0) (FUNC_OR_PROP) = f;

    for (auto *type : list_range((StructRNA*)first_type)) {
        if(match_type("AttributeGroup")) {
            each_func{
                assign("new", BlenderPyAttribute_new)
            }
        }
        if (match_type("ID")) {
            BlenderPyID::s_type = type;
            each_prop{
                assign("is_updated", BlenderPyID_is_updated)
                assign("is_updated_data", BlenderPyID_is_updated_data)
            }
            each_func {
               assign("evaluated_get", BlenderPyID_evaluated_get)
               assign("update_tag", BlenderPyID_update_tag)
            }
        }
        else if (match_type("Object")) {
            BObject::s_type = type;
            each_prop{
                assign("matrix_local", BObject_matrix_local)
                assign("matrix_world", BObject_matrix_world)
                assign("hide", BObject_hide)
                assign("hide_viewport", BObject_hide_viewport)
                assign("hide_render", BObject_hide_render)
                assign("select", BObject_select)
            }
            each_func {
                assign("select_get", BObject_select_get)
                assign("to_mesh", BObject_to_mesh)
                assign("to_mesh_clear", BObject_to_mesh_clear)
            }
        }
        else if (match_type("ObjectModifiers")) {
            each_func{
                assign("clear", BObject_modifiers_clear)
            }
        }        
        else if (match_type("Mesh")) {
            BMesh::s_type = type;
            each_func {
                assign("calc_normals_split", BMesh_calc_normals_split)
                assign("update", BMesh_update)
                assign("clear_geometry", BMesh_clear_geometry)          
            }
            each_prop{
                assign("polygons", BMesh_polygons)
            }
        }
        else if (match_type("MeshVertices")) {
            each_func{
                assign("add", BMesh_vertices_add)
            }
        }     
        else if (match_type("MeshPolygons")) {
            each_func{
                assign("add", BMesh_polygons_add)
            }
        }        
        else if (match_type("MeshLoops")) {
            each_func{
                assign("add", BMesh_loops_add)
            }
        }
        else if (match_type("MeshEdges")) {
            each_func{
                assign("add", BMesh_edges_add)
            }
        }        
        else if (match_type("Curve")) {
            BCurve::s_type = type;
            each_prop{
                assign("splines", BCurve_splines)
            }
        }
        else if (match_type("CurveSplines")) {
            each_func{
                assign("clear", BCurve_splines_clear)
                assign("new", BCurve_splines_new)            
            }
        }     
        else if (match_type("SplineBezierPoints")) {
            BNurb::s_type = type;
            each_func{
                assign("add", BNurb_splines_bezier_add)
            }
        }        
        else if (match_type("UVLoopLayers")) {
            each_prop{
                assign("active", UVLoopLayers_active)
            }
        }
        else if (match_type("LoopColors")) {
            each_prop{
                assign("active", LoopColors_active)
            }
        }
        else if (match_type("Camera")) {
            BCamera::s_type = type;
            each_prop{
                assign("clip_start", BCamera_clip_start)
                assign("clip_end", BCamera_clip_end)
                assign("angle_x", BCamera_angle_x)
                assign("angle_y", BCamera_angle_y)
                assign("lens", BCamera_lens)
                assign("sensor_fit", BCamera_sensor_fit)
                assign("sensor_width", BCamera_sensor_width)
                assign("sensor_height", BCamera_sensor_height)
                assign("shift_x", BCamera_shift_x)
                assign("shift_y", BCamera_shift_y)
            }
        }
        else if (match_type("Material")) {
            BMaterial::s_type = type;
            each_prop{
                assign("use_nodes", BMaterial_use_nodes)
                assign("active_node_material", BMaterial_active_node_material)
            }
        }
        else if (match_type("Scene")) {
            BlenderPyScene::s_type = type;
            each_prop{
                assign("frame_start", BlenderPyScene_frame_start)
                assign("frame_end", BlenderPyScene_frame_end)
                assign("frame_current", BlenderPyScene_frame_current)
            }
            each_func{
                assign("frame_set", BlenderPyScene_frame_set)
            }
        }
        else if (match_type("BlendData")) {
            BData::s_type = type;
        }
        else if (match_type("BlendDataObjects")) {
            each_prop{
                assign("is_updated", BlendDataObjects_is_updated)
            }
        }
        else if (match_type("BlendDataMeshes")) {
            each_func{
                assign("remove", BlendDataMeshes_remove)
            }
        }
        else if (match_type("Context")) {
            BlenderPyContext::s_type = type;
            each_prop{
                assign("blend_data", BlenderPyContext_blend_data)
                assign("scene", BlenderPyContext_scene)
                assign("view_layer", BlenderPyContext_view_layer)
            }
            each_func{
                assign("evaluated_depsgraph_get", BlenderPyContext_evaluated_depsgraph_get)
            }
        }
        else if (match_type("Depsgraph")) {
            each_prop{
                assign("object_instances", BlenderPyDepsgraph_object_instances)
                }
            each_func{
                assign("update", BlenderPyContext_depsgraph_update)
                }
        }
        else if (match_type("DepsgraphObjectInstance")) {
            each_prop{
                assign("instance_object", BlenderPyDepsgraphObjectInstance_instance_object)
                assign("is_instance", BlenderPyDepsgraphObjectInstance_is_instance)
                assign("matrix_world", BlenderPyDepsgraphObjectInstance_world_matrix)
                assign("parent", BlenderPyDepsgraphObjectInstance_parent)
                assign("object", BlenderPyDepsgraphObjectInstance_object)
            }
        }
    }
#undef each_func
#undef assign
#undef match_type

    // test
    //auto scene = BlenderPyContext::get().scene();
}




template<typename Self>
static inline bool get_bool(Self *self, PropertyRNA *prop)
{
    PointerRNA ptr;
	ptr.data = self;
	PointerRNA_OWNER_ID(ptr) = PointerRNA_OWNER_ID_CAST(self);
	
    return ((BoolPropertyRNA*)prop)->get(&ptr) != 0;
}
template<typename Self>
static inline float get_float(Self *self, PropertyRNA *prop)
{
    PointerRNA ptr;
	ptr.data = self;
	PointerRNA_OWNER_ID(ptr) = PointerRNA_OWNER_ID_CAST(self);
	
    return ((FloatPropertyRNA*)prop)->get(&ptr);
}
template<typename Self>
static inline void get_float_array(Self *self, float *dst, PropertyRNA *prop)
{
    PointerRNA ptr;
	ptr.data = self;
	PointerRNA_OWNER_ID(ptr) = PointerRNA_OWNER_ID_CAST(self);

    ((FloatPropertyRNA*)prop)->getarray(&ptr, dst);
}


const char *BObject::name() const { return ((BlenderPyID)*this).name(); }
void* BObject::data() { return m_ptr->data; }

mu::float4x4 BObject::matrix_local() const
{
    mu::float4x4 ret;
    get_float_array(m_ptr, (float*)&ret, BObject_matrix_local);
    return ret;
}

mu::float4x4 BObject::matrix_world() const
{
    mu::float4x4 ret;
    get_float_array(m_ptr, (float*)&ret, BObject_matrix_world);
    return ret;
}
bool BObject::hide_viewport() const
{
    return get_bool(m_ptr, BObject_hide_viewport);
}
bool BObject::hide_render() const
{
    return get_bool(m_ptr, BObject_hide_render);
}


bool blender::BObject::is_selected() const
{
#if BLENDER_VERSION >= 304
    PointerRNA ptr;
    ptr.data = nullptr;
    ptr.owner_id = nullptr;
    ptr.type = nullptr;
    return call<Object, bool, PointerRNA>(g_context, m_ptr, BObject_select_get, ptr);
#else
    return call<Object, bool, ViewLayer*>(g_context, m_ptr, BObject_select_get, nullptr);
#endif
}

Mesh* BObject::to_mesh() const
{
#if BLENDER_VERSION >= 303
    // In blender 3.3 the pointer to interpret the bool moves by 8 instead of 1 for some reason so this needs to be a type of that size:
    return call<Object, Mesh*, bool*, Depsgraph*>(g_context, m_ptr, BObject_to_mesh, nullptr, nullptr);
#else
    return call<Object, Mesh*, bool, Depsgraph*>(g_context, m_ptr, BObject_to_mesh, false, nullptr);
#endif
}

void BObject::to_mesh_clear()
{
    call<Object, Mesh*>(g_context, m_ptr, BObject_to_mesh_clear);
}

void BObject::modifiers_clear() 
{
    call<Object, void>(g_context, m_ptr, BObject_modifiers_clear);
}

blist_range<ModifierData> BObject::modifiers()
{
    return list_range((ModifierData*)m_ptr->modifiers.first);
}
blist_range<bDeformGroup> BObject::deform_groups()
{
    return list_range((bDeformGroup*)m_ptr->defbase.first);
}


barray_range<MLoop> BMesh::indices()
{
#if BLENDER_VERSION >= 306
    return { (MLoop*)CustomData_get_layer_named(&m_ptr->ldata, CD_PROP_INT32, ".corner_vert"), (size_t)m_ptr->totloop };
#elif BLENDER_VERSION >= 304
    return{ (MLoop*)CustomData_get(m_ptr->ldata, CD_MLOOP), (size_t)m_ptr->totloop };
#else
    return { m_ptr->mloop, (size_t)m_ptr->totloop };
#endif
}
barray_range<MEdge> BMesh::edges()
{
    return { m_ptr->medge, (size_t)m_ptr->totedge };
}

#if BLENDER_VERSION < 306
barray_range<MPoly> BMesh::polygons() {
#if BLENDER_VERSION >= 304
    return { (MPoly*)CustomData_get(m_ptr->pdata, CD_MPOLY), (size_t)m_ptr->totpoly };
#else
    return { m_ptr->mpoly, (size_t)m_ptr->totpoly };
#endif
}
#else
OffsetIndices<int> BMesh::polygons()
{
    return Span(m_ptr->poly_offset_indices, m_ptr->totpoly + 1);
}
MutableSpan<int> BMesh::polygonsForWrite()
{
    return MutableSpan(m_ptr->poly_offset_indices, m_ptr->totpoly + 1);
}
#endif // #if BLENDER_VERSION < 306

barray_range<MVert> BMesh::vertices()
{
#if BLENDER_VERSION >= 305
    auto positions = (const float(*)[3])CustomData_get_layer_named(&m_ptr->vdata, CD_PROP_FLOAT3, "position");    
    return { (MVert*)positions, (size_t)m_ptr->totvert };
#elif BLENDER_VERSION >= 304
    return { (MVert*)CustomData_get(m_ptr->vdata, CD_MVERT),(size_t) m_ptr->totvert};
#else
    return { m_ptr->mvert, (size_t)m_ptr->totvert };
#endif
}

blender::barray_range<mu::float3> BMesh::normals()
{
    if (CustomData_number_of_layers(&m_ptr->ldata, CD_NORMAL) > 0) {
        auto data = (mu::float3*)CustomData_get(m_ptr->ldata, CD_NORMAL);
        if (data != nullptr)
            return { data, (size_t)m_ptr->totloop };
    }
    return { nullptr, (size_t)0 };
}

#if BLENDER_VERSION >= 304
barray_range<int> BMesh::material_indices()
{
    auto layer = (int*)CustomData_get_layer_named(&m_ptr->pdata, CD_PROP_INT32, "material_index");
    if (layer)
        return { layer, (size_t)m_ptr->totpoly };

    return { nullptr, (size_t)0 };
}
#endif

//----------------------------------------------------------------------------------------------------------------------


MLoopUV* BMesh::GetUV(const int index) const {
#if BLENDER_VERSION < 305
    return static_cast<MLoopUV *>(CustomData_get_layer_n(&m_ptr->ldata, CD_MLOOPUV, index));
#else
    auto uvs = CustomData_get_layer_n(&m_ptr->ldata, CD_PROP_FLOAT2, index);
    return (MLoopUV*)uvs;
#endif
}

//----------------------------------------------------------------------------------------------------------------------


barray_range<MLoopCol> BMesh::colors()
{
    auto layer_data = (CustomDataLayer*)get_pointer(m_ptr, LoopColors_active);
    if (layer_data && layer_data->data)
        return { (MLoopCol*)layer_data->data, (size_t)m_ptr->totloop };
    else
        return { nullptr, (size_t)0 };
}

void BMesh::calc_normals_split()
{
    call<Mesh, void>(g_context, m_ptr, BMesh_calc_normals_split);
}

void BMesh::update() 
{
    call<Mesh, void>(g_context, m_ptr, BMesh_update);    
}

void BMesh::clear_geometry()
{
    call<Mesh, void>(g_context, m_ptr, BMesh_clear_geometry);
}

void BMesh::add_vertices(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_vertices_add, count);
}

void BMesh::add_polygons(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_polygons_add, count);
}

void BMesh::add_loops(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_loops_add, count);
}

void BMesh::add_edges(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_edges_add, count);
}

void BMesh::add_normals(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_normals_add, count);
}

#if BLENDER_VERSION >= 306
void BMesh::shade_flat() {

    bool* sharp_faces = (bool*)CustomData_get_layer_named(&m_ptr->pdata, CD_PROP_BOOL, "sharp_face");
    if (!sharp_faces) {
        call<ID, PointerRNA, const char*, long long, long long>(g_context, &m_ptr->id, BlenderPyAttribute_new, "sharp_face", CD_PROP_BOOL, 2, &m_ptr->id);   // ATTR_DOMAIN_FACE = 2
        sharp_faces = (bool*)CustomData_get_layer_named(&m_ptr->pdata, CD_PROP_BOOL, "sharp_face");
    }

    if (sharp_faces) {
        std::fill(sharp_faces, sharp_faces + m_ptr->totpoly, true);
    }
}
#endif

barray_range<BMFace*> BEditMesh::polygons()
{
    return { m_ptr->bm->ftable, (size_t)m_ptr->bm->ftable_tot };
}

barray_range<BMVert*> BEditMesh::vertices()
{
    return { m_ptr->bm->vtable, (size_t)m_ptr->bm->vtable_tot };
}

barray_range<BMTriangle> BEditMesh::triangles()
{
    return barray_range<BMTriangle> { m_ptr->looptris, (size_t)m_ptr->tottri };
}

int BEditMesh::uv_data_offset(int index) const
{
#if BLENDER_VERSION < 306
    int layer_index = CustomData_get_layer_index_n(&m_ptr->bm->ldata, CD_MLOOPUV, index);
#else
    int layer_index = CustomData_get_layer_index_n(&m_ptr->bm->ldata, CD_PROP_FLOAT2, index);
#endif
    if (layer_index == -1) {
        return NULL;
    }

    auto layer = m_ptr->bm->ldata.layers[layer_index];
    return layer.offset;
}

void BNurb::add_bezier_points(int count, Object* obj) {
    call<Nurb, void, int>(g_context, m_ptr, BNurb_splines_bezier_add, count, obj->id);
}

void BCurve::clear_splines() {
    call<Curve, void>(g_context, m_ptr, BCurve_splines_clear);
}

Nurb* BCurve::new_spline() {
// In blender 3.3, the pointer moves 8 instead of 4 bytes:
#if BLENDER_VERSION >= 303
    return call<Curve, Nurb*, long long>(g_context, m_ptr, BCurve_splines_new, CU_BEZIER);
#else
    return call<Curve, Nurb*, int>(g_context, m_ptr, BCurve_splines_new, CU_BEZIER);
#endif
}

const char *BMaterial::name() const
{
    return m_ptr->id.name + 2;
}
const mu::float3& BMaterial::color() const
{
    return (mu::float3&)m_ptr->r;
}
bool BMaterial::use_nodes() const
{
    return get_bool(m_ptr, BMaterial_use_nodes);
}
Material * BMaterial::active_node_material() const
{
    return (Material*)get_pointer(m_ptr, BMaterial_active_node_material);
}

float BCamera::clip_start() const { return get_float(m_ptr, BCamera_clip_start); }
float BCamera::clip_end() const { return get_float(m_ptr, BCamera_clip_end); }
float BCamera::angle_y() const { return get_float(m_ptr, BCamera_angle_y); }
float BCamera::angle_x() const { return get_float(m_ptr, BCamera_angle_x); }
float BCamera::lens() const { return get_float(m_ptr, BCamera_lens); }
int   BCamera::sensor_fit() const { return GetInt(m_ptr, BCamera_sensor_fit); }
float BCamera::sensor_width() const { return get_float(m_ptr, BCamera_sensor_width); }
float BCamera::sensor_height() const { return get_float(m_ptr, BCamera_sensor_height); }
float BCamera::shift_x() const { return get_float(m_ptr, BCamera_shift_x); }
float BCamera::shift_y() const { return get_float(m_ptr, BCamera_shift_y); }

blist_range<Object> BData::objects() {
    return list_range((Object*)m_ptr->objects.first);
}

blist_range<Mesh> BData::meshes(){
    return list_range((Mesh*)m_ptr->meshes.first);
}

blist_range<Material> BData::materials(){
    return list_range((Material*)m_ptr->materials.first);
}

blist_range<Collection> BData::collections() {
    return list_range((Collection*)m_ptr->collections.first);
}

bool BData::objects_is_updated() {
    return true; //before 2.80: get_bool(m_ptr, BlendDataObjects_is_updated);
}

void BData::remove(Mesh * v)
{
    PointerRNA t = {};
    t.data = v;
    call<Main, void, PointerRNA*>(g_context, m_ptr, BlendDataMeshes_remove, &t);
}

const void* CustomData_get(const CustomData& data, int type)
{
    int layer_index = data.typemap[type];
    if (layer_index == -1)
        return nullptr;
    layer_index = layer_index + data.layers[layer_index].active;
    return data.layers[layer_index].data;
}

int CustomData_get_offset(const CustomData& data, int type)
{
    int layer_index = data.typemap[type];
    if (layer_index == -1)
        return -1;

    return data.layers[layer_index].offset;
}


mu::float3 BM_loop_calc_face_normal(const BMLoop& l)
{
    float r_normal[3];
    float v1[3], v2[3];
    sub_v3_v3v3(v1, l.prev->v->co, l.v->co);
    sub_v3_v3v3(v2, l.next->v->co, l.v->co);

    cross_v3_v3v3(r_normal, v1, v2);
    const float len = normalize_v3(r_normal);
    if (UNLIKELY(len == 0.0f)) {
        copy_v3_v3(r_normal, l.f->no);
    }
    return (mu::float3&)r_normal;
}

std::string abspath(const std::string& path, const std::string& libName)
{
    try {
        auto global = py::dict();
        auto local = py::dict();
        local["path"] = py::str(path);
        local["libName"] = py::str(libName);
        py::eval<py::eval_mode::eval_statements>(
            "import bpy.path\n"
            "lib = bpy.data.libraries[libName] if libName else None\n"
            "ret = bpy.path.abspath(path, library=lib)\n"
            , global, local);
        return (py::str)local["ret"];
    }
    catch (py::error_already_set& e) {
        muLogError("%s\n", e.what());
        return path;
    }
}

std::string getBlenderVersion()
{
    try {
        auto global = py::dict();
        auto local = py::dict();
        py::eval<py::eval_mode::eval_statements>(
            "import bpy\n"
            "ret = bpy.app.version_string"
            , global, local);
        return (py::str)local["ret"];
    }
    catch (py::error_already_set& e) {
        muLogError("%s\n", e.what());
        return "";
    }
}

/**
 * Calls a python method that takes no arguments.
 */
void callPythonMethod(const char* name) {
    py::gil_scoped_acquire acquire;

    try {
        auto statement = Format("import MeshSyncClientBlender\n" 
            "from MeshSyncClientBlender.unity_mesh_sync_common import *\n"
            "try: %s()\n"
            "except Exception as e: print(e)", name);
        
        py::eval<py::eval_mode::eval_statements>(
            statement.c_str());
    }
    catch (py::error_already_set& e) {
        muLogError("%s\n", e.what());
    }

    py::gil_scoped_release release;
}

} // namespace blender
