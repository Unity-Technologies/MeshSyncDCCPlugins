#include "pch.h"
#include "msblenBinder.h"
#include "msblenContext.h"
#include "msblenUtils.h"

#include "MeshSync/SceneCache/msSceneCacheOutputSettings.h"
#include "MeshSync/SceneGraph/msAnimation.h" //TransformAnimation
#include "MeshSync/SceneGraph/msCamera.h"
#include "MeshSync/SceneGraph/msSceneSettings.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/SceneGraph/msCurve.h"
#include "MeshSync/SceneGraph/msEntityConverter.h" //ScaleConverter
#include "MeshSync/SceneGraph/msScene.h"

#include "msblenEntityHandler.h"

#include "MeshSync/Utility/msMaterialExt.h" //AsStandardMaterial

#include "BlenderUtility.h" //ApplyMeshUV
#include "MeshSyncClient/SettingsUtility.h"
#include "MeshSyncClient/SceneCacheUtility.h"
#include "MeshSync/SceneCache/msSceneCacheOutputSettings.h"

#include "BlenderPyObjects/BlenderPyScene.h" //BlenderPyScene
#include "DNA_node_types.h"
#include <sstream>
#include "msblenBinder.h"
#include "MeshUtils/muLog.h"


#ifdef mscDebug
#define mscTrace(...) ::mu::Print("MeshSync trace: " __VA_ARGS__)
#define mscTraceW(...) ::mu::Print(L"MeshSync trace: " __VA_ARGS__)
#else
#define mscTrace(...)
#define mscTraceW(...)
#endif

namespace bl = blender;

using namespace msblenUtils;

//----------------------------------------------------------------------------------------------------------------------

void msblenContext::NodeRecord::clearState()
{
    dst_anim = nullptr;
    anim_extractor = nullptr;
}

void msblenContext::NodeRecord::recordAnimation(msblenContext *_this) const {
    (_this->*anim_extractor)(*dst_anim, obj);
}

//----------------------------------------------------------------------------------------------------------------------

static msblenContext* s_instance = nullptr;

msblenContext& msblenContext::getInstance() {
    if (s_instance == nullptr) {
        s_instance = new msblenContext();
    }

    return *s_instance;
}

void msblenContext::Destroy() {
    if (!s_instance) {
        return;
    }

    m_texture_manager.clear();
    m_material_manager.clear();

    m_entities_state->clear();
    m_instances_state->clear();

    delete s_instance;
    s_instance = nullptr;
}



msblenContext::msblenContext()
{
    m_settings.scene_settings.handedness = ms::Handedness::RightZUp;
}

msblenContext::~msblenContext()
{
    // no wait() because it can cause deadlock...
}

BlenderSyncSettings& msblenContext::getSettings() { return m_settings; }
const BlenderSyncSettings& msblenContext::getSettings() const { return m_settings; }
BlenderCacheSettings& msblenContext::getCacheSettings() { return m_cache_settings; }
const BlenderCacheSettings& msblenContext::getCacheSettings() const { return m_cache_settings; }

std::vector<Object*> msblenContext::getNodes(MeshSyncClient::ObjectScope scope)
{
    std::vector<Object*> ret;

    bl::BlenderPyScene scene = bl::BlenderPyScene(bl::BlenderPyContext::get().scene());
    if (scope == MeshSyncClient::ObjectScope::All) {
        scene.each_objects([&](Object *obj) {
            ret.push_back(obj);
        });
    } else if (scope == MeshSyncClient::ObjectScope::Selected) {
        scene.each_selection([&](Object *obj) {
            ret.push_back(obj);
        });
    } else if (scope == MeshSyncClient::ObjectScope::Updated) {
        bl::BData bpy_data = bl::BData(bl::BlenderPyContext::get().data());
        if (bpy_data.objects_is_updated()) {
            scene.each_objects([&](Object *obj) {
                const bl::BlenderPyID bid = bl::BlenderPyID(obj);
                if (bid.is_updated() || bid.is_updated_data())
                    ret.push_back(obj);
            });
        }
    }

    return ret;
}

int msblenContext::exportTexture(const std::string & path, ms::TextureType type)
{
    return m_texture_manager.addFile(path, type);
}

int msblenContext::getMaterialID(Material *m)
{
    if (m && m->id.orig_id)
        m = (Material*)m->id.orig_id;
    return m_material_ids.getID(m);
}

//----------------------------------------------------------------------------------------------------------------------

ms::MaterialPtr msblenContext::CreateDefaultMaterial(const uint32_t matIndex) {
    std::shared_ptr<ms::Material> dst = ms::Material::create();
    dst->id = m_material_ids.getID(nullptr);
    dst->index = matIndex;
    dst->name = "Default";
    return dst;
}


void msblenContext::RegisterSceneMaterials()
{
    int midx = 0;
    
    // Blender allows faces to have no material. add dummy material for them.
    m_material_manager.add(CreateDefaultMaterial(midx++));

    bl::BData bpy_data = bl::BData(bl::BlenderPyContext::get().data());
    for (struct Material* mat : bpy_data.materials()) {
        RegisterMaterial(mat, midx++);
    }
    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();
}

void msblenContext::RegisterObjectMaterials(const std::vector<Object*> objects) {
    int midx = 0;
    
    // Blender allows faces to have no material. add dummy material for them.
    m_material_manager.add(CreateDefaultMaterial(midx++));

    std::unordered_set<Material*> blMaterials;
    for (auto it = objects.begin();it!=objects.end();++it) {
        Object* curObject = *it;
        if (nullptr == curObject)
            continue;

        const short numMaterials = blender::BlenderUtility::GetNumMaterials(curObject);
        Material** matPtr = blender::BlenderUtility::GetMaterials(curObject);
        for (uint32_t i =0;i<numMaterials;++i){
            if (nullptr == matPtr[i])
                continue;
            blMaterials.emplace(matPtr[i]);
        }
    }

    //Register materials to material manager
    for (auto it = blMaterials.begin();it!=blMaterials.end();++it) {
        Material* mat = *it;
        RegisterMaterial(mat, midx++);

    }
    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();
}

void msblenContext::RegisterMaterial(Material* mat, const uint32_t matIndex) {
    std::shared_ptr<ms::Material> ret = ms::Material::create();
    ret->name = get_name(mat);
    ret->id = m_material_ids.getID(mat);
    ret->index = matIndex;

    ms::StandardMaterial& stdmat = ms::AsStandardMaterial(*ret);
    bl::BMaterial bm(mat);
    struct Material* color_src = mat;
    stdmat.setColor(mu::float4{ color_src->r, color_src->g, color_src->b, 1.0f });

    // todo: handle texture
#if 0
    if (m_settings.sync_textures) {
        auto export_texture = [this](MTex *mtex, ms::TextureType type) -> int {
            if (!mtex || !mtex->tex || !mtex->tex->ima)
                return -1;
            return exportTexture(bl::abspath(mtex->tex->ima->name), type);
        };
#if BLENDER_VERSION < 280
        stdmat.setColorMap(export_texture(mat->mtex[0], ms::TextureType::Default));
#endif
    }
#endif

    m_material_manager.add(ret);
    
}


//----------------------------------------------------------------------------------------------------------------------

void msblenContext::extractCameraData(const Object *src,
    bool& ortho, float& near_plane, float& far_plane, float& fov,
    float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift)
{
    bl::BCamera cam(src->data);

    // note: fbx exporter seems always export as perspective
    ortho = ((Camera*)src->data)->type == CAM_ORTHO;

    near_plane = cam.clip_start();
    far_plane = cam.clip_end();
    fov = cam.angle_y() * mu::RadToDeg;
    focal_length = cam.lens();
    sensor_size.x = cam.sensor_width();  // in mm
    sensor_size.y = cam.sensor_height(); // in mm

    int fit = cam.sensor_fit();
    if (fit == CAMERA_SENSOR_FIT_AUTO)
        fit = sensor_size.x > sensor_size.y ? CAMERA_SENSOR_FIT_HOR : CAMERA_SENSOR_FIT_VERT;
    const float aspx = sensor_size.x / sensor_size.y;
    const float aspy = sensor_size.y / sensor_size.x;
    lens_shift.x = cam.shift_x() * (fit == CAMERA_SENSOR_FIT_HOR ? 1.0f : aspy); // 0-1
    lens_shift.y = cam.shift_y() * (fit == CAMERA_SENSOR_FIT_HOR ? aspx : 1.0f); // 0-1
}

void msblenContext::extractLightData(const Object *src,
    ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& range, float& spot_angle)
{
    Light* data = (Light*)src->data;
    const float ENERGY_TO_INTENSITY = 0.001f; //Default light power in Blender: 1000, intensity in Unity: 1

    color = (mu::float4&)data->r;
    intensity = data->energy * ENERGY_TO_INTENSITY;
    range = data->dist;

    switch (data->type) {
    case LA_SUN:
        ltype = ms::Light::LightType::Directional;
        intensity = data->energy; //direct conversion for sun type
        break;
    case LA_SPOT:
        ltype = ms::Light::LightType::Spot;
        spot_angle = data->spotsize * mu::RadToDeg;
        break;
    case LA_AREA:
        ltype = ms::Light::LightType::Area;
        break;
    default:
        ltype = ms::Light::LightType::Point;
        break;
    }
    stype = (data->mode & 1) ? ms::Light::ShadowType::Soft : ms::Light::ShadowType::None;
}

ms::TransformPtr msblenContext::exportObject(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object* obj, bool parent, bool tip)
{
    if (!obj)
        return nullptr;

    msblenContextState::ObjectRecord& rec = state.touchRecord(paths, obj);
    if (rec.dst)
        return rec.dst; // already exported

    auto handle_parent = [&]() {
        if (parent)
            exportObject(state, paths, settings, obj->parent, parent, false);
    };
    auto handle_transform = [&]() {
        handle_parent();
        rec.dst = exportTransform(state, paths, settings, obj);
    };

    switch (obj->type) {
    case OB_ARMATURE:
    {
        if (!tip || (!settings.BakeModifiers && settings.sync_bones)) {
            handle_parent();
            rec.dst = exportArmature(state, paths, settings, obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_MESH:
    {
        if (!settings.BakeModifiers && settings.sync_bones) {
            if (auto* arm_mod = (const ArmatureModifierData*)FindModifier(obj, eModifierType_Armature))
                exportObject(state, paths, settings, arm_mod->object, parent);
        }
        if (settings.sync_meshes || (!settings.BakeModifiers && settings.sync_blendshapes)) {
            handle_parent();
            rec.dst = exportMesh(state, paths, settings, obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_CURVE: {
        if (settings.sync_meshes) {
            handle_parent();

            if (settings.curves_as_mesh) {
                rec.dst = exportMesh(state, paths, settings, obj);
            }
            else {
                rec.dst = m_curves_handler.exportCurve(state, paths, settings, obj, m_asyncTasksController);
            }
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_FONT:  //
    case OB_SURF:  //
    case OB_MBALL: // these can be converted to mesh
    {
        if (settings.sync_meshes && settings.curves_as_mesh) {
            handle_parent();
            rec.dst = exportMesh(state, paths, settings, obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_CAMERA:
    {
        if (settings.sync_cameras) {
            handle_parent();
            rec.dst = exportCamera(state, paths, settings, obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_LAMP:
    {
        if (settings.sync_lights) {
            handle_parent();
            rec.dst = exportLight(state, paths, settings, obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    default:
    {
        if (get_instance_collection(obj) || (!tip && parent)) {
            handle_parent();
            rec.dst = exportTransform(state, paths, settings, obj);
        }
        break;
    }
    }

    if (rec.dst) {
        if (get_instance_collection(obj)) {
            DupliGroupContext ctx;
            ctx.group_host = obj;
            ctx.dst = rec.dst;

            exportDupliGroup(state, paths, settings, obj, ctx);
        }
    }

#if BLENDER_VERSION >= 300
        blender::msblenModifiers::exportProperties(obj, &m_property_manager);
#endif

    return rec.dst;
}

ms::TransformPtr msblenContext::exportTransform(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object* src)
{
    std::shared_ptr<ms::Transform> ret = ms::Transform::create();
    ms::Transform& dst = *ret;
    dst.path = paths.get_path(src);
    msblenEntityHandler::extractTransformData(settings, src, dst);
    state.manager.add(ret);

    return ret;
}

ms::TransformPtr msblenContext::exportPose(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *armature, bPoseChannel *src)
{
    std::shared_ptr<ms::Transform> ret = ms::Transform::create();
    ms::Transform& dst = *ret;
    
    dst.path = paths.get_path(armature, src->bone);
    msblenEntityHandler::extractTransformData(settings, src, dst.position, dst.rotation, dst.scale);
    state.manager.add(ret);
    return ret;
}

ms::TransformPtr msblenContext::exportArmature(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *src)
{
    std::shared_ptr<ms::Transform> ret = ms::Transform::create();
    ms::Transform& dst = *ret;
    dst.path = paths.get_path(src);
    msblenEntityHandler::extractTransformData(settings, src, dst);
    state.manager.add(ret);

    for (struct bPoseChannel* pose : bl::list_range((bPoseChannel*)src->pose->chanbase.first)) {
        struct Bone* bone = pose->bone;
        std::map<struct Bone*, std::shared_ptr<ms::Transform>>::mapped_type& dst = state.bones[bone];
        dst = exportPose(state, paths, settings, src, pose);
    }
    return ret;
}

ms::TransformPtr msblenContext::exportReference(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, Object *src, const DupliGroupContext& ctx)
{
    msblenContextState::ObjectRecord& rec = state.touchRecord(paths, src);
    if (!rec.dst)
        return nullptr;

    const std::string local_path = paths.get_path(src);
    std::string path = ctx.dst->path + local_path;

    ms::TransformPtr dst;
    auto assign_base_params = [&]() {
        msblenEntityHandler::extractTransformData(settings, src, *dst);
        dst->path = path;
        dst->visibility = { visible_in_collection(src), visible_in_render(ctx.group_host), visible_in_viewport(ctx.group_host) };
        dst->world_matrix *= ctx.dst->world_matrix;
    };

    if (is_mesh(src)) {
        if (settings.BakeTransform) {
            dst = ms::Mesh::create();
            ms::Mesh& dst_mesh = static_cast<ms::Mesh&>(*dst);
            ms::Mesh& src_mesh = static_cast<ms::Mesh&>(*rec.dst);

            (ms::Transform&)dst_mesh = (ms::Transform&)src_mesh;
            assign_base_params();

            auto do_merge = [this, dst, &dst_mesh, &src_mesh, &settings, &state]() {
                dst_mesh.merge(src_mesh);
                if (settings.ExportSceneCache)
                    dst_mesh.detach();
                dst_mesh.refine_settings = src_mesh.refine_settings;
                dst_mesh.refine_settings.local2world = dst_mesh.world_matrix;
                dst_mesh.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_LOCAL2WORLD, true);
                state.manager.add(dst);
            };
            if (settings.multithreaded)
                // deferred to execute after extracting src mesh data is completed
                m_asyncTasksController.AddTask(std::launch::deferred, do_merge);
            else
                do_merge();
        }
        else {
            dst = ms::Transform::create();
            assign_base_params();
            dst->reference = local_path;
            state.manager.add(dst);
        }
    }
    else {
        dst = std::static_pointer_cast<ms::Transform>(rec.dst->clone());
        assign_base_params();
        state.manager.add(dst);
    }

    each_child(src, [&](Object *child) {
        exportReference(state, paths, settings, child, ctx);
    });

    if (get_instance_collection(src)) {
        DupliGroupContext ctx2;
        ctx2.group_host = src;
        ctx2.dst = dst;

        exportDupliGroup(state, paths, settings, src, ctx2);
    }
    return dst;
}

ms::TransformPtr msblenContext::exportDupliGroup(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *src, const DupliGroupContext& ctx)
{
    Collection* group = get_instance_collection(src);
    if (!group)
        return nullptr;

    const std::string local_path = std::string("/") + (group->id.name + 2);
    const std::string path = ctx.dst->path + local_path;

    std::shared_ptr<ms::Transform> dst = ms::Transform::create();
    dst->path = path;
    dst->visibility = { visible_in_collection(src), visible_in_render(ctx.group_host), visible_in_viewport(ctx.group_host) };

    const mu::tvec3<float> offset_pos = -get_instance_offset(group);
    dst->position = settings.BakeTransform ? mu::float3::zero() : offset_pos;
    dst->world_matrix = mu::translate(offset_pos) * ctx.dst->world_matrix;
    state.manager.add(dst);

    DupliGroupContext ctx2;
    ctx2.group_host = src;
    ctx2.dst = dst;
    auto gobjects = bl::list_range((CollectionObject*)group->gobject.first);
    for (auto go : gobjects) {
        auto obj = go->ob;
        if (auto t = exportObject(state, paths, settings, obj, true, false)) {
            const bool non_lib = obj->id.lib == nullptr;
            t->visibility = { visible_in_collection(obj), non_lib, non_lib };
        }
        exportReference(state, paths, settings, obj, ctx2);
    }

    return dst;
}

ms::CameraPtr msblenContext::exportCamera(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *src)
{
    std::shared_ptr<ms::Camera> ret = ms::Camera::create();
    ms::Camera& dst = *ret;
    dst.path = paths.get_path(src);
    msblenEntityHandler::extractTransformData(settings, src, dst);
    extractCameraData(src, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov, dst.focal_length, dst.sensor_size, dst.lens_shift);
    state.manager.add(ret);
    return ret;
}

ms::LightPtr msblenContext::exportLight(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *src)
{
    std::shared_ptr<ms::Light> ret = ms::Light::create();
    ms::Light& dst = *ret;
    dst.path = paths.get_path(src);
    msblenEntityHandler::extractTransformData(settings, src, dst);
    extractLightData(src, dst.light_type, dst.shadow_type, dst.color, dst.intensity, dst.range, dst.spot_angle);
    state.manager.add(ret);
    return ret;
}

void msblenContext::importEntities(std::vector<ms::EntityPtr> entities) {   
    for (auto& entity : entities) {
        switch (entity->getType())
        {
        case ms::EntityType::Curve:
            m_curves_handler.importCurve(dynamic_cast<ms::Curve*>(entity.get()));
            break;
        case ms::EntityType::Mesh:
            importMesh(dynamic_cast<ms::Mesh*>(entity.get()));
            break;
        default:
            break;
        }
    }
}

void set_object_mode() {
    try {
        py::eval<py::eval_mode::eval_statements>(
            "import bpy\n"
            "bpy.ops.object.mode_set()");
    }
    catch (py::error_already_set& e) {
        muLogError("%s\n", e.what());
    }
}

void msblenContext::importMesh(ms::Mesh* mesh) {
    auto obj = msblenUtils::get_object_from_path(mesh->path);

    // Make sure the object still exists:
    if (!obj) {
        return;
    }

    // Ensure we're in object mode, settiing data on the edit mesh is not supported:
    set_object_mode();

    // If we're baking modifiers, the mesh would contain the baked version so remove the modifiers now when the mesh is updated:
    if (getSettings().BakeModifiers) {
        blender::BObject bObj(obj);
        bObj.modifiers_clear();
    }

    auto data = (Mesh*)obj->data;

    auto blenMesh = (Mesh*)obj->data;
    std::vector<int> mid_table(blenMesh->totcol);
    for (int mi = 0; mi < blenMesh->totcol; ++mi)
        mid_table[mi] = getMaterialID(blenMesh->mat[mi]);
    
    // Make reverse lookup list:
    std::vector<int> rev_mid_table(mid_table.size());
    for (int i = 0; i < mid_table.size(); i++)
    {
        rev_mid_table[mid_table[i]] = i;
    }

    int num_indices = mesh->indices.size();
    int num_polygons = num_indices / 3;

    // The mesh for unity had split vertices, we need to undo that for blender so the faces are connected:
    // Merge verts in the same location:
    for (int i = 0; i < mesh->points.size(); i++)
    {
        for (int j = mesh->points.size() - 1; j > i; j--)
        {
            if (mesh->points[i] == mesh->points[j]) {
                // Remove vert
                mesh->points.erase(mesh->points.begin() + j);

                // Adjust indices to match:
                for (int ii = 0; ii < num_indices; ii++) {
                    if (mesh->indices[ii] == j) {
                        mesh->indices[ii] = i;
                    }
                    else if (mesh->indices[ii] > j) {
                        mesh->indices[ii]--;
                    }
                }
            }
        }
    }

    int num_vertices = mesh->points.size();

    bl::BMesh bmesh(data);

    bmesh.clear_geometry();
    bmesh.add_vertices(num_vertices);
    bmesh.add_polygons(num_polygons);
    bmesh.add_loops(num_indices);

    auto bmeshVerts = bmesh.vertices();
    auto bmeshIndices = bmesh.indices();
    auto bmeshPolygons = bmesh.polygons();
  
    // vertices
    for (size_t vi = 0; vi < num_vertices; ++vi) {
        copyFloatVector(bmeshVerts[vi].co, mesh->points[vi])
    }

    // faces
    int ii = 0;
    for (size_t pi = 0; pi < num_polygons; ++pi) {
        // int count = mesh->counts[pi];
        // always 3 for triangles from unity:
        const int count = 3;
        bmeshPolygons[pi].loopstart = ii;
        bmeshPolygons[pi].totloop = count;

        const int material_index = mesh->material_ids[pi];
        if (material_index != ms::InvalidID) {
            if (material_index < rev_mid_table.size()) {
                bmeshPolygons[pi].mat_nr = rev_mid_table[material_index];
            }
            else {
                bmeshPolygons[pi].mat_nr = 0;
            }
        }

        // Reverse triangle back because it was reversed in unity during refine step:
        bmeshIndices[bmeshPolygons[pi].loopstart + 0].v = mesh->indices[ii++];
        bmeshIndices[bmeshPolygons[pi].loopstart + 2].v = mesh->indices[ii++];
        bmeshIndices[bmeshPolygons[pi].loopstart + 1].v = mesh->indices[ii++];
    }

    // Calculate edges, normals, loops, etc:
    bmesh.update();
    bmesh.calc_normals_split();

    Depsgraph* depsgraph = bl::BlenderPyContext::get().evaluated_depsgraph_get();
    blender::BObject bObj(obj);
    bObj = (Object*)bl::BlenderPyID(bObj).evaluated_get(depsgraph);

    // Update the geometry hash so this is not sent back to unity, we just got it from there!
    mesh->indices.clear();
    mesh->points.clear();
    mesh->counts.clear();
    mesh->setupDataFlags();
    doExtractMeshData(*m_entities_state, m_settings, *mesh, obj, data, mesh->world_matrix);

    // Update the cached geometry checksum so this does not get sent back to unity:
    m_entity_manager.updateChecksumGeom(mesh);
}

ms::MeshPtr msblenContext::exportMesh(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *src)
{
    bl::BObject bobj(src);
    Mesh *data = nullptr;
    if (is_mesh(src))
        data = (Mesh*)src->data;
    bool is_editing = false;

    if (settings.sync_meshes && data) {
        // check if mesh is dirty
        if (BMEditMesh* edit_mesh = get_edit_mesh(data)) {
            is_editing = true;
            struct BMesh* bm = edit_mesh->bm;
            if (bm->elem_table_dirty) {
                // mesh is editing and dirty. just add to pending list
                state.pending.insert(src);
                return nullptr;
            }
        }
    }

    std::shared_ptr<ms::Mesh> ret = ms::Mesh::create();
    ms::Mesh& dst = *ret;
    dst.path = paths.get_path(src);

    // transform
    msblenEntityHandler::extractTransformData(settings, src, dst);

    if (settings.sync_meshes) {
        const bool need_convert = 
            (!is_editing && settings.BakeModifiers ) || !is_mesh(src);

        if (need_convert) {
            if (settings.BakeModifiers ) {
                Depsgraph* depsgraph = bl::BlenderPyContext::get().evaluated_depsgraph_get();
                bobj = (Object*)bl::BlenderPyID(bobj).evaluated_get(depsgraph);
            }
            if (Mesh *tmp = bobj.to_mesh()) {
                data = tmp;
                m_meshes_to_clear.push_back(src);
            }
        }

        // calculate per index normals
        // note: when bake_modifiers is enabled, it is done for baked meshes
        if (data && settings.sync_normals && settings.calc_per_index_normals) {
            // calc_normals_split() seems can't be multi-threaded. it will cause unpredictable crash...
            // todo: calculate normals by myself to be multi-threaded
            bl::BMesh(data).calc_normals_split();
        }
    }

    if (data) {
        auto task = [this, ret, src, data, &settings, &state]() {
            auto& dst = *ret;
            doExtractMeshData(state, settings, dst, src, data, dst.world_matrix);
            state.manager.add(ret);
        };

        if (settings.multithreaded)
            m_asyncTasksController.AddTask(std::launch::async, task);
        else
            task();
    }
    return ret;
}

void msblenContext::doExtractMeshData(msblenContextState& state, BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data, mu::float4x4 world)
{
    if (settings.sync_meshes) {
        bl::BObject bobj(obj);
        bl::BMesh bmesh(data);
        const bool is_editing = get_edit_mesh(bmesh.ptr()) != nullptr;

        // on edit mode, editing is applied to EditMesh and base Mesh is intact. so get data from EditMesh on edit mode.
        // todo: Blender 2.8 displays transparent final mesh on edit mode. extract data from it.
        if (is_editing) {
            doExtractEditMeshData(state, settings, dst, obj, data);
        }
        else {
            doExtractNonEditMeshData(state, settings, dst, obj, data);
        }

        if (!settings.BakeModifiers&& !is_editing) {
            // mirror
            if (const MirrorModifierData* mirror = (const MirrorModifierData*)FindModifier(obj, eModifierType_Mirror)) {
                if (mirror->flag & MOD_MIR_AXIS_X) dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_MIRROR_X, true);
                if (mirror->flag & MOD_MIR_AXIS_Y) dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_MIRROR_Y, true);
                if (mirror->flag & MOD_MIR_AXIS_Z) dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_MIRROR_Z, true);
                if (mirror->mirror_ob) {
                    dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_MIRROR_BASIS, true);
                    mu::float4x4 wm = bobj.matrix_world();
                    mu::float4x4 mm = bl::BObject(mirror->mirror_ob).matrix_world();
                    dst.refine_settings.mirror_basis = wm * mu::invert(mm);
                }
            }
        }
        if (settings.BakeTransform) {
            dst.refine_settings.local2world = world;
            dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_LOCAL2WORLD, true);
        }
    }
    else {
        if (!settings.BakeModifiers&& settings.sync_blendshapes) {
            doExtractBlendshapeWeights(state, settings, dst, obj, data);
        }
    }

    if (dst.normals.empty())
        dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
    if (dst.tangents.empty())
        dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);
    dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_FLIP_FACES, true);
    dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_MAKE_DOUBLE_SIDED, settings.make_double_sided);
}

void msblenContext::doExtractBlendshapeWeights(msblenContextState& state, BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data)
{
    struct Mesh& mesh = *data;
    if (!settings.BakeModifiers) {
        // blend shapes
        if (settings.sync_blendshapes && mesh.key) {
            RawVector<mu::float3> basis;
            int bi = 0;
            each_key(&mesh, [&](const KeyBlock *kb) {
                if (bi == 0) { // Basis
                }
                else {
                    ms::BlendShapeDataPtr bsd = dst.addBlendShape(kb->name);
                    bsd->weight = kb->curval * 100.0f;
                }
                ++bi;
            });
        }
    }
}

void msblenContext::doExtractNonEditMeshData(msblenContextState& state, BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data)
{
    bl::BObject bobj(obj);
    bl::BMesh bmesh(data);
    struct Mesh& mesh = *data;

    blender::barray_range<struct MLoop> indices = bmesh.indices();
    blender::barray_range<struct MPoly> polygons = bmesh.polygons();
    blender::barray_range<struct MVert> vertices = bmesh.vertices();

    const size_t num_indices = indices.size();
    const size_t num_polygons = polygons.size();
    size_t num_vertices = vertices.size();

    std::vector<int> mid_table(mesh.totcol);
    for (int mi = 0; mi < mesh.totcol; ++mi)
        mid_table[mi] = getMaterialID(mesh.mat[mi]);
    if (mid_table.empty())
        mid_table.push_back(ms::InvalidID);

    // vertices
    dst.points.resize_discard(num_vertices);
    for (size_t vi = 0; vi < num_vertices; ++vi) {
        dst.points[vi] = (mu::float3&)vertices[vi].co;
    }

    // faces
    dst.indices.reserve(num_indices);
    dst.counts.resize_discard(num_polygons);
    dst.material_ids.resize_discard(num_polygons);
    {
        int ii = 0;
        for (size_t pi = 0; pi < num_polygons; ++pi) {
            struct MPoly& polygon = polygons[pi];
            const int material_index = polygon.mat_nr;
            const int count = polygon.totloop;
            dst.counts[pi] = count;
            dst.material_ids[pi] = mid_table[material_index];
            dst.indices.resize(dst.indices.size() + count);

            struct MLoop* idx = &indices[polygon.loopstart];
            for (int li = 0; li < count; ++li) {
                dst.indices[ii++] = idx[li].v;
            }
        }
    }

    // normals
    if (settings.sync_normals) {
#if 0
        // per-vertex
        dst.normals.resize_discard(num_vertices);
        for (size_t vi = 0; vi < num_vertices; ++vi) {
            dst.normals[vi] = to_float3(vertices[vi].no);
        }
#endif
        // per-index
        blender::barray_range<mu::tvec3<float>> normals = bmesh.normals();
        if (!normals.empty()) {
            dst.normals.resize_discard(num_indices);
            for (size_t ii = 0; ii < num_indices; ++ii)
                dst.normals[ii] = ms::ceilToDecimals(normals[ii]);
        }
    }


    // uv
    if (settings.sync_uvs) {

        blender::BlenderUtility::ApplyBMeshUVToMesh(&bmesh, num_indices, &dst);
    }

    // colors
    if (settings.sync_colors) {
        blender::barray_range<struct MLoopCol> colors = bmesh.colors();
        if (!colors.empty()) {
            dst.colors.resize_discard(num_indices);
            for (size_t ii = 0; ii < num_indices; ++ii)
                dst.colors[ii] = to_float4(colors[ii]);
        }
    }

    if (!settings.BakeModifiers) {
        // bones

        auto extract_bindpose = [](auto *bone) {
            mu::tmat4x4<float> mat_bone = (mu::float4x4&)bone->arm_mat * g_arm_to_world;
            // armature-space to world-space
            return mu::invert(mu::swap_yz(mu::flip_z(mat_bone)));
        };

        if (settings.sync_bones) {
            const ArmatureModifierData* arm_mod = (const ArmatureModifierData*)FindModifier(
                obj, eModifierType_Armature);
            if (arm_mod) {
                // request bake TRS
                dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_LOCAL2WORLD, true);
                dst.refine_settings.local2world = mu::transform(dst.position, invert(dst.rotation), dst.scale);

                struct Object* arm_obj = arm_mod->object;
                int group_index = 0;
                each_deform_group(obj, [&](const bDeformGroup *g) {
                    bool found = false;
                    Bone* bone = find_bone(arm_obj, g->name);
                    if (bone) {
                        ms::TransformPtr trans = findBone(state, arm_obj, bone);
                        if (trans) {
                            found = true;
                            ms::BoneDataPtr b = dst.addBone(trans->path);
                            b->bindpose = extract_bindpose(bone);
                            b->weights.resize_zeroclear(num_vertices);

                            for (int vi = 0; vi < num_vertices; ++vi) {
                                int num_weights = mesh.dvert[vi].totweight;
                                struct MDeformVert& dvert = mesh.dvert[vi];
                                for (int wi = 0; wi < num_weights; ++wi) {
                                    if (dvert.dw[wi].def_nr == group_index) {
                                        b->weights[vi] = dvert.dw[wi].weight;
                                    }
                                }
                            }
                        }
                    }
                    if (!found) {
                        mscTrace("bone not found %s\n", g->name);
                    }
                    ++group_index;
                });
            }
        }

        // blend shapes
        if (settings.sync_blendshapes && mesh.key) {
            RawVector<mu::float3> basis;
            int bi = 0;
            each_key(&mesh, [&](const KeyBlock *kb) {
                if (bi == 0) { // Basis
                    basis.resize_discard(kb->totelem);
                    memcpy(basis.data(), kb->data, basis.size() * sizeof(mu::float3));
                }
                else {
                    ms::BlendShapeDataPtr bsd = dst.addBlendShape(kb->name);
                    bsd->weight = kb->curval * 100.0f;

                    bsd->frames.push_back(ms::BlendShapeFrameData::create());
                    ms::BlendShapeFrameData& frame = *bsd->frames.back();
                    frame.weight = 100.0f;
                    frame.points.resize_discard(kb->totelem);
                    memcpy(frame.points.data(), kb->data, basis.size() * sizeof(mu::float3));

                    const size_t len = frame.points.size();
                    for (size_t i = 0; i < len; ++i) {
                        frame.points[i] -= basis[i];
                    }
                }
                ++bi;
            });
        }
    }

#if 0
    // lines
    // (blender doesn't include lines & points in polygons - MPoly::totloop is always >= 3)
    {
        auto edges = bmesh.edges();

        std::vector<bool> point_shared(num_vertices);
        for (size_t pi = 0; pi < num_polygons; ++pi) {
            auto& polygon = polygons[pi];
            int count = polygon.totloop;
            auto *idx = &indices[polygon.loopstart];
            for (int li = 0; li < count; ++li) {
                point_shared[idx[li].v] = true;
            }
        }

        size_t lines_begin = dst.indices.size();
        size_t num_lines = 0;
        for (auto edge : edges) {
            if (!point_shared[edge.v1] || !point_shared[edge.v2]) {
                ++num_lines;
                dst.counts.push_back(2);
                dst.indices.push_back(edge.v1);
                dst.indices.push_back(edge.v2);
            }
        }

        if (num_lines > 0) {
            num_indices = dst.indices.size();

            if (!dst.normals.empty() && settings.sync_normals == msbNormalSyncMode::PerIndex) {
                dst.normals.resize(num_indices, mu::float3::zero());
            }
            if (!dst.uv0.empty()) {
                dst.uv0.resize(num_indices, mu::float2::zero());
            }
            if (!dst.colors.empty()) {
                auto colors = bmesh.colors();
                dst.colors.resize(num_indices, mu::float4::one());
                for (size_t ii = lines_begin; ii < num_indices; ++ii) {
                    int vi = dst.indices[ii];
                    dst.colors[ii] = to_float4(colors[vi]);
                }
            }
        }
    }
#endif
}

void msblenContext::doExtractEditMeshData(msblenContextState& state, BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data)
{
    bl::BObject bobj(obj);
    bl::BMesh bmesh(data);
    bl::BEditMesh emesh(get_edit_mesh(bmesh.ptr()));
    struct Mesh& mesh = *data;

    blender::barray_range<struct BMFace*> polygons = emesh.polygons();
    blender::barray_range<struct BMLoop*[3]> triangles = emesh.triangles();
    blender::barray_range<struct BMVert*> vertices = emesh.vertices();

    const size_t num_triangles = triangles.size();
    const size_t num_vertices = vertices.size();
    const size_t num_indices = triangles.size() * 3;

    std::vector<int> mid_table(mesh.totcol);
    for (int mi = 0; mi < mesh.totcol; ++mi)
        mid_table[mi] = getMaterialID(mesh.mat[mi]);
    if (mid_table.empty())
        mid_table.push_back(-1);

    // vertices
    dst.points.resize_discard(num_vertices);
    for (size_t vi = 0; vi < num_vertices; ++vi) {
        dst.points[vi] = (mu::float3&)vertices[vi]->co;
    }

    // faces
    {
        dst.indices.resize(num_indices);
        dst.counts.resize_discard(num_triangles);
        dst.material_ids.resize_discard(num_triangles);

        size_t ii = 0;
        for (size_t ti = 0; ti < num_triangles; ++ti) {
            struct BMLoop*(& triangle)[3] = triangles[ti];

            int material_index = 0;
            const int polygon_index = triangle[0]->f->head.index;
            if (polygon_index < polygons.size())
                material_index = mid_table[polygons[polygon_index]->mat_nr];
            dst.material_ids[ti] = material_index;

            dst.counts[ti] = 3;
            for (struct BMLoop* idx : triangle)
                dst.indices[ii++] = idx->v->head.index;
        }
    }

    // normals
    if (settings.sync_normals) {
#if 0
        // per-vertex
        dst.normals.resize_discard(num_vertices);
        for (size_t vi = 0; vi < num_vertices; ++vi)
            dst.normals[vi] = to_float3(vertices[vi]->no);
#endif
        // per-index
        dst.normals.resize_discard(num_indices);
        size_t ii = 0;
        for (size_t ti = 0; ti < num_triangles; ++ti) {
            struct BMLoop*(& triangle)[3] = triangles[ti];
            for (struct BMLoop* idx : triangle)
                dst.normals[ii++] = -bl::BM_loop_calc_face_normal(*idx);
        }
    }

    // uv
    if (settings.sync_uvs) {
        //const int offset = emesh.uv_data_offset();
        //if (offset != -1) {
        //    dst.m_uv[0].resize_discard(num_indices);
        //    size_t ii = 0;
        //    for (size_t ti = 0; ti < num_triangles; ++ti) {
        //        auto& triangle = triangles[ti];
        //        for (auto *idx : triangle)
        //            dst.m_uv[0][ii++] = *reinterpret_cast<mu::float2*>((char*)idx->head.data + offset);
        //    }
        //}
        //
        //
        //
        blender::BlenderUtility::ApplyBMeshUVToMesh(&bmesh, num_indices, &dst);

    }
}

ms::TransformPtr msblenContext::findBone(msblenContextState& state, Object *armature, Bone *bone)
{
    std::map<struct Bone*, std::shared_ptr<ms::Transform>>::iterator it = state.bones.find(bone);
    return it != state.bones.end() ? it->second : nullptr;
}

void msblenContext::exportAnimation(msblenContextPathProvider& paths, BlenderSyncSettings& settings, Object *obj, bool force, const std::string& base_path)
{
    if (!obj)
        return;

    std::string path = base_path + paths.get_path(obj);
    if (m_anim_records.find(path) != m_anim_records.end())
        return;

    std::shared_ptr<ms::AnimationClip>& clip = m_animations.front();
    ms::TransformAnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;
    Collection* group = get_instance_collection(obj);

    auto add_animation = [this, &clip](BlenderSyncSettings& settings, const std::string& path, void *obj, ms::TransformAnimationPtr dst, AnimationRecord::extractor_t extractor) {
        dst->path = path;
        std::map<std::basic_string<char>, AnimationRecord>::mapped_type& rec = m_anim_records[path];
        rec.extractor = extractor;
        rec.obj = obj;
        rec.dst = dst;
        rec.settings = settings;
        clip->addAnimation(dst);
    };

    switch (obj->type) {
    case OB_CAMERA:
    {
        // camera
        exportAnimation(paths, settings, obj->parent, true, base_path);
        add_animation(settings, path, obj, ms::CameraAnimation::create(), &msblenContext::extractCameraAnimationData);
        break;
    }
    case OB_LAMP:
    {
        // lights
        exportAnimation(paths, settings, obj->parent, true, base_path);
        add_animation(settings, path, obj, ms::LightAnimation::create(), &msblenContext::extractLightAnimationData);
        break;
    }
    case OB_MESH:
    {
        // meshes
        exportAnimation(paths, settings, obj->parent, true, base_path);
        add_animation(settings, path, obj, ms::MeshAnimation::create(), &msblenContext::extractMeshAnimationData);
        break;
    }
    default:
    if (force || obj->type == OB_ARMATURE || group) {
        exportAnimation(paths, settings, obj->parent, true, base_path);
        add_animation(settings, path, obj, ms::TransformAnimation::create(), &msblenContext::extractTransformAnimationData);

        if (obj->type == OB_ARMATURE && (!settings.BakeModifiers && settings.sync_bones)) {
            // bones
            blender::blist_range<struct bPoseChannel> poses = bl::list_range((bPoseChannel*)obj->pose->chanbase.first);
            for (struct bPoseChannel* pose : poses) {
                auto pose_path = base_path + paths.get_path(obj, pose->bone);
                add_animation(settings, pose_path, pose, ms::TransformAnimation::create(), &msblenContext::extractPoseAnimationData);
            }
        }
        break;
    }
    }

    // handle dupli group
    if (group) {
        std::string group_path = base_path;
        group_path += '/';
        group_path += get_name(obj);
        group_path += '/';
        group_path += (group->id.name + 2);

        auto gobjects = bl::list_range((CollectionObject*)group->gobject.first);
        for (auto go : gobjects) {
            exportAnimation(paths, settings, go->ob, false, group_path);
        }
    }
}

void msblenContext::extractTransformAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst_, void *obj)
{
    ms::TransformAnimation& dst = (ms::TransformAnimation&)dst_;

    mu::float3 pos;
    mu::quatf rot;
    mu::float3 scale;
    ms::VisibilityFlags vis;
    msblenEntityHandler::extractTransformData(settings, (Object*)obj, pos, rot, scale, vis);

    float t = m_anim_time;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    dst.visible.push_back({ t, (int)vis.visible_in_render });
}

void msblenContext::extractPoseAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst_, void *obj)
{
    ms::TransformAnimation& dst = (ms::TransformAnimation&)dst_;

    mu::float3 t;
    mu::quatf r;
    mu::float3 s;
    msblenEntityHandler::extractTransformData(settings, (bPoseChannel*)obj, t, r, s);

    const float time = m_anim_time;
    dst.translation.push_back({ time, t });
    dst.rotation.push_back({ time, r });
    dst.scale.push_back({ time, s });
}

void msblenContext::extractCameraAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst_, void *obj)
{
    extractTransformAnimationData(settings, dst_, obj);

    ms::CameraAnimation& dst = (ms::CameraAnimation&)dst_;

    bool ortho;
    float near_plane, far_plane, fov, focal_length;
    mu::float2 sensor_size, lens_shift;
    extractCameraData((Object*)obj, ortho, near_plane, far_plane, fov, focal_length, sensor_size, lens_shift);

    float t = m_anim_time;
    dst.near_plane.push_back({ t , near_plane });
    dst.far_plane.push_back({ t , far_plane });
    dst.fov.push_back({ t , fov });
    dst.focal_length.push_back({ t , focal_length });
    dst.sensor_size.push_back({ t , sensor_size });
    dst.lens_shift.push_back({ t , lens_shift });
}

void msblenContext::extractLightAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst_, void *obj)
{
    extractTransformAnimationData(settings, dst_, obj);

    ms::LightAnimation& dst = (ms::LightAnimation&)dst_;

    ms::Light::LightType ltype;
    ms::Light::ShadowType stype;
    mu::float4 color;
    float intensity, range, spot_angle;
    extractLightData((Object*)obj, ltype, stype, color, intensity, range, spot_angle);

    float t = m_anim_time;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    dst.range.push_back({ t, range });
    if (ltype == ms::Light::LightType::Spot) {
        dst.spot_angle.push_back({ t, spot_angle });
    }
}

void msblenContext::extractMeshAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation & dst_, void * obj)
{
    extractTransformAnimationData(settings, dst_, obj);

    ms::MeshAnimation& dst = (ms::MeshAnimation&)dst_;
    float t = m_anim_time;

    struct Mesh& mesh = *(Mesh*)((Object*)obj)->data;
    if (!get_edit_mesh(&mesh) && mesh.key) {
        // blendshape weight animation
        int bi = 0;
        each_key(&mesh, [&](const KeyBlock *kb) {
            if (bi == 0) { // Basis
            }
            else {
                dst.getBlendshapeCurve(kb->name).push_back({ t, kb->curval * 100.0f });
            }
            ++bi;
        });
    }
}


void msblenContext::logInfo(const char * format, ...)
{
    const int MaxBuf = 2048;
    char buf[MaxBuf];

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    puts(buf);
    va_end(args);
}

bool msblenContext::isServerAvailable()
{
    m_sender.client_settings = m_settings.client_settings;
    return m_sender.isServerAvaileble();
}

const std::string& msblenContext::getErrorMessage()
{
    return m_sender.getErrorMessage();
}

void msblenContext::wait()
{
    py::gil_scoped_release release;

    m_sender.wait();
}

void msblenContext::clear()
{
    m_material_ids.clear();
    m_texture_manager.clear();
    m_material_manager.clear();
    m_entity_manager.clear();
}

bool msblenContext::prepare()
{
    if (!bl::ready())
        return false;
    return true;
}

bool msblenContext::sendMaterials(bool dirty_all)
{
    if (!prepare() || m_sender.isExporting() || m_ignore_events)
        return false;

    m_settings.Validate();
    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(dirty_all);
    RegisterSceneMaterials();

    // send
    WaitAndKickAsyncExport();
    return true;
}

void msblenContext::requestLiveEditMessage()
{
    if (m_settings.ExportSceneCache) {
        return;
    }

    m_sender.on_live_edit_response_received = [this](auto properties, auto entities, std::string messageFromServer) {
        m_property_manager.updateFromServer(properties, entities);

        if (messageFromServer == ms::REQUEST_SYNC) {
            m_server_requested_sync = true;
        }
        else if (messageFromServer == ms::REQUEST_PYTHON_CALLBACK) {
            m_server_requested_python_callback = true;
        }
    };
    m_sender.requestLiveEditMessage();
}

bool msblenContext::sendObjectsAndRequestLiveEdit(MeshSyncClient::ObjectScope scope, bool dirty_all)
{
    bool result = sendObjects(scope, dirty_all);
    requestLiveEditMessage();

    return result;
}

bool msblenContext::sendObjects(MeshSyncClient::ObjectScope scope, bool dirty_all)
{
    if (!prepare() || m_sender.isExporting() || m_ignore_events)
        return false;

    this->m_intermediate_paths.mappedNames.clear();

    blender::msblenModifiers::importProperties(m_property_manager.getReceivedProperties());
    importEntities(m_property_manager.getReceivedEntities());

    m_property_manager.clearReceivedData();

    // If a python callback was requested, that may change the scene so run it and don't export until next update:
	if (m_server_requested_python_callback) {
        m_server_requested_python_callback = false;
        m_ignore_events = true;
        blender::callPythonMethod("meshsync_server_requested_callback");
        m_ignore_events = false;
        return false;
    }

    if (m_server_requested_sync) {
		scope = MeshSyncClient::ObjectScope::All;
        dirty_all = true;
        m_server_requested_sync = false;
    }

    m_settings.Validate();
    m_entity_manager.setAlwaysMarkDirty(dirty_all);
    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(false); // false because too heavy
    m_instances_state->manager.setAlwaysMarkDirty(dirty_all);
    m_property_manager.clear();

    if (m_settings.sync_meshes)
        RegisterSceneMaterials();

    if (scope == MeshSyncClient::ObjectScope::Updated) {
        bl::BData bpy_data = bl::BData(bl::BlenderPyContext::get().data());
        if (!bpy_data.objects_is_updated())
            return true; // nothing to send

        bl::BlenderPyScene scene = bl::BlenderPyScene(bl::BlenderPyContext::get().scene());
        scene.each_objects([this](Object *obj) {
            bl::BlenderPyID bid = bl::BlenderPyID(obj);
            if (bid.is_updated() || bid.is_updated_data())
                exportObject(*m_entities_state, m_default_paths, m_settings, obj, false);
            else
                m_entities_state->touchRecord(m_default_paths, obj); // this cannot be covered by getNodes()
        });
    }
    else {
        for(std::vector<Object*>::value_type obj : getNodes(scope))
            exportObject(*m_entities_state, m_default_paths, m_settings, obj, true);
    }

#if BLENDER_VERSION >= 300
    if (m_geometryNodeUtils.getInstancesDirty() || dirty_all) {
        exportInstances();
        m_asyncTasksController.Wait();
        m_instances_state->eraseStaleObjects();
    }
#endif

    m_asyncTasksController.Wait();
    m_entities_state->eraseStaleObjects();

    WaitAndKickAsyncExport();
    return true;
}

bool msblenContext::sendAnimations(MeshSyncClient::ObjectScope scope)
{
    if (!prepare() || m_sender.isExporting() || m_ignore_events)
        return false;

    m_settings.Validate();
    m_ignore_events = true;

    bl::BlenderPyScene scene = bl::BlenderPyScene(bl::BlenderPyContext::get().scene());
    const int frame_rate = scene.fps();
    const int frame_step = std::max(m_settings.frame_step, 1);

    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create()); // create default clip

    ms::AnimationClip& clip = *m_animations.back();
    clip.frame_rate = static_cast<float>(frame_rate);

    // list target objects
    for (std::vector<Object*>::value_type obj : getNodes(scope))
        exportAnimation(m_default_paths, m_settings, obj, false);

    // advance frame and record animations
    {
        const int frame_current = scene.GetCurrentFrame();
        const int frame_start = scene.frame_start();
        const int frame_end = scene.frame_end();
        const int interval = frame_step;
        const int reserve_size = (frame_end - frame_start) / interval + 1;

        for (auto& kvp : m_anim_records) {
            kvp.second.dst->reserve(reserve_size);
        };
        for (int f = frame_start;;) {
            scene.frame_set(f);
            m_anim_time = static_cast<float>(f - frame_start) / frame_rate;

            mu::parallel_for_each(m_anim_records.begin(), m_anim_records.end(), [this](auto& kvp) {
                kvp.second(this);
            });

            if (f >= frame_end)
                break;
            else
                f = std::min(f + interval, frame_end);
        }
        m_anim_records.clear();
        scene.frame_set(frame_current);
    }

    m_ignore_events = false;

    // send
    if (!m_animations.empty()) {
        WaitAndKickAsyncExport();
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool msblenContext::ExportCache(const std::string& path, const BlenderCacheSettings& cache_settings) {
    using namespace MeshSyncClient;

    bl::BlenderPyScene scene = bl::BlenderPyScene(bl::BlenderPyContext::get().scene());
    const float frameRate = static_cast<float>(scene.fps());

    const BlenderSyncSettings settings_old = m_settings;
    m_settings.curves_as_mesh = cache_settings.curves_as_mesh;
    SettingsUtility::ApplyCacheToSyncSettings(cache_settings, &m_settings);

    const ms::SceneCacheOutputSettings oscs = SettingsUtility::CreateSceneCacheOutputSettings(frameRate, cache_settings);
    const std::string destPath = SceneCacheUtility::BuildFilePath(path);
    if (!m_cache_writer.Open(destPath.c_str(), oscs)) {
        logInfo("MeshSync: Can't write scene cache to %s", destPath.c_str());
        m_settings = settings_old;
        return false;
    }

    m_material_manager.setAlwaysMarkDirty(true);
    m_entity_manager.setAlwaysMarkDirty(true);

    const MaterialFrameRange materialRange = cache_settings.material_frame_range;
    const std::vector<Object*> nodes = getNodes(cache_settings.object_scope);
    mu::ScopedTimer timer;

    const int prevFrame = scene.GetCurrentFrame();
    int frameStart = 0, frameEnd = 0;
    switch(cache_settings.frame_range){
        case MeshSyncClient::FrameRange::Current:{
            frameStart = frameEnd = prevFrame;
            break;
        }
        case MeshSyncClient::FrameRange::All:{
            frameStart = scene.frame_start();
            frameEnd = scene.frame_end();
            break;
        }
        case MeshSyncClient::FrameRange::Custom:{
            frameStart = cache_settings.frame_begin;
            frameEnd = cache_settings.frame_end;
            break;
        }
    }
    const int frameStep = std::max(static_cast<int>(cache_settings.frame_step), 1);

    // record
    bl::BlenderPyContext  pyContext = bl::BlenderPyContext::get();
    Depsgraph* depsGraph = pyContext.evaluated_depsgraph_get();

    int sceneIndex = 0;
    for (int f = frameStart; f <= frameEnd; f += frameStep) {

        //[Note-sin: 2021-3-29] use Depsgraph.update() to optimize for setting frame (scene.frame_set(f))
        scene.SetCurrentFrame(f, depsGraph);

        m_anim_time = static_cast<float>(f - frameStart) / frameRate;

        if (sceneIndex == 0) {
            RegisterObjectMaterials(nodes); //needed to export material IDs in meshes
            if (MeshSyncClient::MaterialFrameRange::None == materialRange)
                m_material_manager.clearDirtyFlags();
        } else if (MeshSyncClient::MaterialFrameRange::All == materialRange) {
            RegisterObjectMaterials(nodes);
        }

        DoExportSceneCache(nodes);
        ++sceneIndex;
    }
    scene.SetCurrentFrame(prevFrame, depsGraph);

    m_asyncTasksController.Wait();
    logInfo("MeshSync: Finished writing scene cache to %s (%f) ms", destPath.c_str(), timer.elapsed());

    m_settings = settings_old;
    m_cache_writer.Close();
    return true;
}


void msblenContext::DoExportSceneCache(const std::vector<Object*>& nodes)
{
    for (const std::vector<Object*>::value_type& n : nodes)
        exportObject(*m_entities_state, m_default_paths, m_settings, n, true);

    m_texture_manager.clearDirtyFlags();
    WaitAndKickAsyncExport();
}

//----------------------------------------------------------------------------------------------------------------------

void msblenContext::flushPendingList() {
    flushPendingList(*m_entities_state, m_default_paths, m_settings);
    flushPendingList(*m_instances_state, m_default_paths, m_settings);
}

void msblenContext::flushPendingList(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings) {
    if (!state.pending.empty() && !m_sender.isExporting()) {
        for (auto p : state.pending)
            exportObject(state, paths, settings, p, false);
        state.pending.clear();

        WaitAndKickAsyncExport();
    }
}

void msblenContext::WaitAndKickAsyncExport()
{
    m_asyncTasksController.Wait();

    // clear baked meshes
    if (!m_meshes_to_clear.empty()) {
        for (const struct Object* v : m_meshes_to_clear) {
            bl::BObject bobj(v);
            bobj.to_mesh_clear();
        }
        m_meshes_to_clear.clear();
    }

    m_entities_state->clearRecordsState();
    m_entities_state->bones.clear();

    m_instances_state->clearRecordsState();
    m_instances_state->bones.clear();

    using Exporter = ms::SceneExporter;
    Exporter *exporter = m_settings.ExportSceneCache ? (Exporter*)&m_cache_writer : (Exporter*)&m_sender;
    
    // kick async send
    exporter->on_prepare = [this, exporter]() {
        blender::callPythonMethod("meshsync_prepare");  

        if (ms::AsyncSceneSender* sender = dynamic_cast<ms::AsyncSceneSender*>(exporter)) {
            sender->client_settings = m_settings.client_settings;
        }
        else if (ms::SceneCacheWriter* writer = dynamic_cast<ms::SceneCacheWriter*>(exporter)) {
            writer->SetTime(m_anim_time);
        }

        ms::SceneExporter& t = *exporter;
        t.scene_settings = m_settings.scene_settings;
        const float scale_factor = 1.0f / m_settings.scene_settings.scale_factor;
        t.scene_settings.scale_factor = 1.0f;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.instanceInfos = m_instances_manager.getDirtyInstances();
        t.instanceMeshes = m_instances_manager.getDirtyMeshes();
    	t.propertyInfos = m_property_manager.getAllProperties();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();
        t.deleted_instances = m_instances_manager.getDeleted();

        if (scale_factor != 1.0f) {
            ms::ScaleConverter cv(scale_factor);
            for (std::vector<std::shared_ptr<ms::Transform>>::value_type& obj : t.transforms) { cv.convert(*obj); }
            for (std::vector<std::shared_ptr<ms::Transform>>::value_type& obj : t.geometries) { cv.convert(*obj); }
            for (std::vector<std::shared_ptr<ms::Transform>>::value_type& obj : t.instanceMeshes) { cv.convert(*obj); }
            for (std::vector<std::shared_ptr<ms::AnimationClip>>::value_type& obj : t.animations) { cv.convert(*obj); }
        }
    };
    exporter->on_success = [this]() {
        m_material_ids.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
        m_instances_manager.clearDirtyFlags();

        blender::callPythonMethod("meshsync_post_export");
    };

    exporter->on_before_send = [this] {
        blender::callPythonMethod("meshsync_pre_export");
    };

    exporter->kick();
}

/// Application Handler Events ///

/// <summary>
/// Event corresponding to the application handler
/// https://docs.blender.org/api/current/bpy.app.handlers.html
/// </summary>
void msblenContext::onDepsgraphUpdatedPost(Depsgraph* graph)
{
#if BLENDER_VERSION >= 300
    m_geometryNodeUtils.setInstancesDirty(true);
#endif

}
