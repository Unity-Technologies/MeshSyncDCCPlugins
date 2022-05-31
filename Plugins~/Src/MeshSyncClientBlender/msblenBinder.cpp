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
static FunctionRNA* BMesh_add_vertices;
static FunctionRNA* BMesh_add_polygons;
static FunctionRNA* BMesh_add_loops;
static FunctionRNA* BMesh_add_edges;
static FunctionRNA* BMesh_add_normals;
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

extern PropertyRNA* BlenderPyContext_depsgraph_object_instances;

extern PropertyRNA* BlenderPyNodeTree_inputs;
extern PropertyRNA* BlenderPyNodeTree_nodes;
extern PropertyRNA* BlenderPyNodeTree_outputs;

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
#define match_func(N) strcmp(func->identifier, N) == 0
#define match_prop(N) strcmp(prop->identifier, N) == 0
#define each_func for (auto *func : list_range((FunctionRNA*)type->functions.first))
#define each_prop for (auto *prop : list_range((PropertyRNA*)type->cont.properties.first))

    for (auto *type : list_range((StructRNA*)first_type)) {
        if (match_type("ID")) {
            BlenderPyID::s_type = type;
            each_prop{
                if (match_prop("is_updated")) BlenderPyID_is_updated = prop;
                if (match_prop("is_updated_data")) BlenderPyID_is_updated_data = prop;
            }
            each_func {
                if (match_func("evaluated_get")) BlenderPyID_evaluated_get = func;
                if (match_func("update_tag")) BlenderPyID_update_tag = func;
            }
        }
        else if (match_type("Object")) {
            BObject::s_type = type;
            each_prop{
                if (match_prop("matrix_local")) BObject_matrix_local = prop;
                if (match_prop("matrix_world")) BObject_matrix_world = prop;
                if (match_prop("hide")) BObject_hide = prop;
                if (match_prop("hide_viewport")) BObject_hide_viewport = prop;
                if (match_prop("hide_render")) BObject_hide_render = prop;
                if (match_prop("select")) BObject_select = prop;
            }
            each_func {
                if (match_func("select_get")) BObject_select_get = func;
                if (match_func("to_mesh")) BObject_to_mesh = func;
                if (match_func("to_mesh_clear")) BObject_to_mesh_clear = func;
            }
        }
        else if (match_type("ObjectModifiers")) {
            each_func{
                if (match_func("clear")) BObject_modifiers_clear = func;
            }
        }        
        else if (match_type("Mesh")) {
            BMesh::s_type = type;
            each_func {
                if (match_func("calc_normals_split")) BMesh_calc_normals_split = func;
                if (match_func("update")) BMesh_update = func;
                if (match_func("clear_geometry")) BMesh_clear_geometry = func;
            }
        }
        else if (match_type("MeshVertices")) {
            each_func{
                if (match_func("add")) BMesh_add_vertices = func;
            }
        }     
        else if (match_type("MeshPolygons")) {
            each_func{
                if (match_func("add")) BMesh_add_polygons = func;
            }
        }        
        else if (match_type("MeshLoops")) {
            each_func{
                if (match_func("add")) BMesh_add_loops = func;
            }
        }
        else if (match_type("MeshEdges")) {
            each_func{
                if (match_func("add")) BMesh_add_edges = func;
            }
        }        
        else if (match_type("Curve")) {
            BCurve::s_type = type;
            each_prop{
                if (match_prop("splines")) BCurve_splines = prop;
            }
        }
        else if (match_type("CurveSplines")) {
            each_func{
                if (match_func("clear")) BCurve_splines_clear = func;
                if (match_func("new")) BCurve_splines_new = func;            
            }
        }     
        else if (match_type("SplineBezierPoints")) {
            BNurb::s_type = type;
            each_func{
                if (match_func("add")) BNurb_splines_bezier_add = func;
            }
        }        
        else if (match_type("UVLoopLayers")) {
            each_prop{
                if (match_prop("active")) UVLoopLayers_active = prop;
            }
        }
        else if (match_type("LoopColors")) {
            each_prop{
                if (match_prop("active")) LoopColors_active = prop;
            }
        }
        else if (match_type("Camera")) {
            BCamera::s_type = type;
            each_prop{
                if (match_prop("clip_start")) BCamera_clip_start = prop;
                if (match_prop("clip_end")) BCamera_clip_end = prop;
                if (match_prop("angle_x")) BCamera_angle_x = prop;
                if (match_prop("angle_y")) BCamera_angle_y = prop;
                if (match_prop("lens")) BCamera_lens = prop;
                if (match_prop("sensor_fit")) BCamera_sensor_fit = prop;
                if (match_prop("sensor_width")) BCamera_sensor_width = prop;
                if (match_prop("sensor_height")) BCamera_sensor_height = prop;
                if (match_prop("shift_x")) BCamera_shift_x = prop;
                if (match_prop("shift_y")) BCamera_shift_y = prop;
            }
        }
        else if (match_type("Material")) {
            BMaterial::s_type = type;
            each_prop{
                if (match_prop("use_nodes")) BMaterial_use_nodes = prop;
                if (match_prop("active_node_material")) BMaterial_active_node_material = prop;
            }
        }      
        else if (match_type("Scene")) {
            BlenderPyScene::s_type = type;
            each_prop{
                if (match_prop("frame_start")) BlenderPyScene_frame_start = prop;
                if (match_prop("frame_end")) BlenderPyScene_frame_end = prop;
                if (match_prop("frame_current")) BlenderPyScene_frame_current = prop;
            }
            each_func{
                if (match_func("frame_set")) BlenderPyScene_frame_set = func;
            }
        }
        else if (match_type("BlendData")) {
            BData::s_type = type;
        }
        else if (match_type("BlendDataObjects")) {
            each_prop{
                if (match_prop("is_updated")) BlendDataObjects_is_updated = prop;
            }
        }
        else if (match_type("BlendDataMeshes")) {
            each_func{
                if (match_func("remove")) BlendDataMeshes_remove = func;
            }
        }
        else if (match_type("Context")) {
            BlenderPyContext::s_type = type;
            each_prop{
                if (match_prop("blend_data")) BlenderPyContext_blend_data = prop;
                if (match_prop("scene")) BlenderPyContext_scene = prop;
                if (match_prop("view_layer")) BlenderPyContext_view_layer = prop;
            }
            each_func{
                if (match_func("evaluated_depsgraph_get")) BlenderPyContext_evaluated_depsgraph_get = func;
            }
        }
        else if (match_type("Depsgraph")) {
            each_prop{
                if (match_prop("object_instances")) {
                    BlenderPyDepsgraph_object_instances = prop;
                }
            }
            each_func{
                if (match_func("update")) {
                    BlenderPyContext_depsgraph_update = func;
                }
            }
        }
        else if (match_type("DepsgraphObjectInstance")) {
            each_prop{
                if (match_prop("instance_object")) {
                    BlenderPyDepsgraphObjectInstance_instance_object = prop;
                }

                if (match_prop("is_instance")) {
                    BlenderPyDepsgraphObjectInstance_is_instance = prop;
                }
                if (match_prop("matrix_world")) {
                    BlenderPyDepsgraphObjectInstance_world_matrix = prop;
                }
                if (match_prop("parent")) {
                    BlenderPyDepsgraphObjectInstance_parent = prop;
                }
                if (match_prop("object")) {
                    BlenderPyDepsgraphObjectInstance_object = prop;
                }
            }
        }
        else if (match_type("NodeTree")) {
            each_prop{
                if (match_prop("inputs")) {
                    BlenderPyNodeTree_inputs = prop;
                }
                if (match_prop("nodes")) {
                    BlenderPyNodeTree_nodes = prop;
                }
                if (match_prop("outputs")) {
                    BlenderPyNodeTree_outputs = prop;
                }
            }
        }
    }
#undef each_iprop
#undef each_nprop
#undef each_func
#undef match_prop
#undef match_func
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
    return call<Object, bool, ViewLayer*>(g_context, m_ptr, BObject_select_get, nullptr);
}

Mesh* BObject::to_mesh() const
{
    return call<Object, Mesh*, bool, Depsgraph*>(g_context, m_ptr, BObject_to_mesh, false, nullptr);
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
    return { m_ptr->mloop, (size_t)m_ptr->totloop };
}
barray_range<MEdge> BMesh::edges()
{
    return { m_ptr->medge, (size_t)m_ptr->totedge };
}
barray_range<MPoly> BMesh::polygons()
{
    return { m_ptr->mpoly, (size_t)m_ptr->totpoly };
}

barray_range<MVert> BMesh::vertices()
{
    return { m_ptr->mvert, (size_t)m_ptr->totvert };
}
barray_range<mu::float3> BMesh::normals()
{
    if (CustomData_number_of_layers(&m_ptr->ldata, CD_NORMAL) > 0) {
        auto data = (mu::float3*)CustomData_get(m_ptr->ldata, CD_NORMAL);
        if (data != nullptr)
            return { data, (size_t)m_ptr->totloop };
    }
    return { nullptr, (size_t)0 };
}

//----------------------------------------------------------------------------------------------------------------------

barray_range<MLoopUV> BMesh::uv()
{
    CustomDataLayer* layer_data = static_cast<CustomDataLayer*>(get_pointer(m_ptr, UVLoopLayers_active));
    if (layer_data && layer_data->data)
        return { static_cast<MLoopUV*>(layer_data->data), static_cast<size_t>(m_ptr->totloop) };
    else
        return { nullptr, (size_t)0 };
}

MLoopUV* BMesh::GetUV(const int index) const {
    return static_cast<MLoopUV *>(CustomData_get_layer_n(&m_ptr->ldata, CD_MLOOPUV, index));
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

void BMesh::addVertices(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_add_vertices, count);
}

void BMesh::addPolygons(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_add_polygons, count);
}

void BMesh::addLoops(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_add_loops, count);
}

void BMesh::addEdges(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_add_edges, count);
}

void BMesh::addNormals(int count) {
    call<Mesh, void, int>(g_context, m_ptr, BMesh_add_normals, count);
}

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

int BEditMesh::uv_data_offset() const
{
    return CustomData_get_offset(m_ptr->bm->ldata, CD_MLOOPUV);
}

MLoopUV* BEditMesh::GetUV(const int index) const {
    return static_cast<MLoopUV *>(CustomData_get_layer_n(&m_ptr->bm->ldata, CD_MLOOPUV, index));
}

void BNurb::add_bezier_points(int count, Object* obj) {
    call<Nurb, void, int>(g_context, m_ptr, BNurb_splines_bezier_add, count, obj->id);
}

void BCurve::clearSplines() {
    call<Curve, void>(g_context, m_ptr, BCurve_splines_clear);
}

Nurb* BCurve::newSpline() {
    return call<Curve, Nurb*, int>(g_context, m_ptr, BCurve_splines_new, CU_BEZIER);
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

std::string abspath(const std::string& path)
{
    try {
        auto global = py::dict();
        auto local = py::dict();
        local["path"] = py::str(path);
        py::eval<py::eval_mode::eval_statements>(
            "import bpy.path\n"
            "ret = bpy.path.abspath(path)\n"
            , global, local);
        return (py::str)local["ret"];
    }
    catch (py::error_already_set& e) {
        muLogError("%s\n", e.what());
        return path;
    }
}

} // namespace blender
