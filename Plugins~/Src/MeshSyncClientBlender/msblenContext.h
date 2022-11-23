#pragma once

#include "MeshUtils/MeshUtils.h"

#include "MeshSync/MeshSync.h"
#include "MeshSync/SceneGraph/msLight.h"

#include "MeshSync/msClient.h"
#include "MeshSync/SceneExporter.h"
#include "MeshSync/AsyncSceneSender.h"
#include "MeshSync/SceneCache/msSceneCacheWriter.h"
#include "MeshSync/MeshSyncMacros.h"

#include "MeshSyncClient/msEntityManager.h"
#include "MeshSyncClient/msMaterialManager.h"
#include "MeshSyncClient/msTextureManager.h"
#include "MeshSyncClient/ObjectScope.h"

#include "BlenderCacheSettings.h"
#include "BlenderSyncSettings.h"
#include "MeshSyncClient/AsyncTasksController.h"
#include "msblenModifiers.h"

#include "MeshSyncClient/msInstancesManager.h"
#include "MeshSyncClient/msPropertyManager.h"
#include "MeshSyncClient/msTransformManager.h"

#include "../MeshSyncClientBlender/msblenContextState.h"

#include "msblenCurveHandler.h"

#include "MeshSync/Utility/msMaterialExt.h" //AsStandardMaterial

#if BLENDER_VERSION >= 300
#include <msblenGeometryNodeUtils.h>
#endif

#include "msblenMaterialsExportHelper.h"

#include <msblenContextDefaultPathProvider.h>
#include <msblenContextIntermediatePathProvider.h>

#include <MeshSync/Utility/msIdUtility.h>

class msblenContext;

using exportCache = std::unordered_map<std::string, std::string>;

class msblenContext {
public:
    msblenContext();
    ~msblenContext();

    static msblenContext& getInstance();
    void Destroy();

    BlenderSyncSettings& getSettings();
    const BlenderSyncSettings& getSettings() const;
    BlenderCacheSettings& getCacheSettings();
    const BlenderCacheSettings& getCacheSettings() const;

    void logInfo(const char *format, ...);
    bool isServerAvailable();
    bool isEditorServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void clear();
    bool prepare();

    void resetMaterials();

    bool sendMaterials(bool dirty_all);
    bool sendObjects(MeshSyncClient::ObjectScope scope, bool dirty_all);
    bool sendObjectsAndRequestLiveEdit(MeshSyncClient::ObjectScope scope, bool dirty_all);
    bool sendAnimations(MeshSyncClient::ObjectScope scope);
    bool ExportCache(const std::string& path, const BlenderCacheSettings& cache_settings);

    void flushPendingList();
    void flushPendingList(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings);

    void onDepsgraphUpdatedPost(Depsgraph* graph);

    void requestLiveEditMessage();
    
    bool sendEditorCommand(ms::EditorCommandMessage::CommandType type, const char* input = nullptr);
    string& getEditorCommandReply();

private:
    // todo
    struct NodeRecord {
        MS_CLASS_DEFAULT_NOCOPY_NOASSIGN(NodeRecord);
        NodeRecord *parent = nullptr;

        std::string path;
        std::string name;
        Object *host = nullptr; // parent of dupli group
        Object *obj = nullptr;

        ms::TransformPtr dst;
        ms::TransformAnimationPtr dst_anim;
        using AnimationExtractor = void (msblenContext::*)(ms::TransformAnimation& dst, void *obj);
        AnimationExtractor anim_extractor = nullptr;

        void clearState();
        void recordAnimation(msblenContext *_this) const;
    };

    struct AnimationRecord  {
        MS_CLASS_DEFAULT_NOCOPY_NOASSIGN(AnimationRecord);
        using extractor_t = void (msblenContext::*)(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);

        void *obj = nullptr;
        ms::TransformAnimationPtr dst;
        extractor_t extractor = nullptr;
        BlenderSyncSettings settings;

        void operator()(msblenContext *_this) {
            (_this->*extractor)(settings,*dst, obj);
        }
    };

    struct DupliGroupContext {
        const Object *group_host;
        ms::TransformPtr dst;
    };

    static std::vector<Object*> getNodes(MeshSyncClient::ObjectScope scope);

    int getMaterialID(Material *m);
    ms::MaterialPtr CreateDefaultMaterial(const uint32_t matIndex);
    void RegisterSceneMaterials();
    void RegisterObjectMaterials(const std::vector<Object*> objects);
    void RegisterMaterial(Material* mat, const uint32_t matIndex);

    ms::TransformPtr exportObject(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *obj, bool parent, bool tip = true, exportCache* cache = nullptr);
    ms::TransformPtr exportTransform(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *obj);
    ms::TransformPtr exportPose(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *armature, bPoseChannel *obj);
    ms::TransformPtr exportArmature(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *obj);
    ms::TransformPtr exportReference(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, Object *obj, const DupliGroupContext& ctx);
    ms::TransformPtr exportDupliGroup(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *obj, const DupliGroupContext& ctx);
    ms::CameraPtr exportCamera(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *obj);
    ms::LightPtr exportLight(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *obj);
    ms::MeshPtr exportMesh(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object *obj);

    void extractCameraData(const Object *src, bool& ortho, float& near_plane, float& far_plane, float& fov,
        float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift);
    void extractLightData(const Object *src,
        ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& range, float& spot_angle);

    void doExtractMeshData(msblenContextState& state, BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data, mu::float4x4 world);
    void doExtractBlendshapeWeights(msblenContextState& state, BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data);
    void doExtractNonEditMeshData(msblenContextState& state, BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data);
    void doExtractEditMeshData(msblenContextState& state, BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data);
    void importMesh(ms::Mesh* mesh);
    void importEntities(std::vector<ms::EntityPtr> entities);

    ms::TransformPtr findBone(msblenContextState& state, Object *armature, Bone *bone);

    void exportAnimation(msblenContextPathProvider& paths, BlenderSyncSettings& settings, Object *obj, bool force, const std::string& base_path = "");
    void extractTransformAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);
    void extractPoseAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);
    void extractCameraAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);
    void extractLightAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);
    void extractMeshAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);

    void DoExportSceneCache(const std::vector<Object*>& nodes);
    void WaitAndKickAsyncExport();

#if BLENDER_VERSION >= 300
    void exportInstances();
    void exportInstances(Object* object, Object* parent, SharedVector<mu::float4x4>, mu::float4x4& inverse, msblenContextPathProvider& pathProvider);

    ms::InstanceInfoPtr exportInstanceInfo(
        msblenContextState& state,
        msblenContextPathProvider& paths,
        Object* instancedObject,
        Object* parent,
        SharedVector<mu::float4x4> mat);

#endif

private:
    std::shared_ptr<msblenContextState> m_entities_state =
        std::shared_ptr<msblenContextState>(new msblenContextState(m_entity_manager));

    std::shared_ptr<msblenContextState> m_instances_state =
        std::shared_ptr<msblenContextState>(new msblenContextState(m_instances_manager));

    msblenContextDefaultPathProvider m_default_paths;
    msblenContextIntermediatePathProvider m_intermediate_paths;

    BlenderSyncSettings m_settings;
    BlenderCacheSettings m_cache_settings;

    MeshSyncClient::AsyncTasksController m_asyncTasksController;

    std::vector<const Object*> m_meshes_to_clear;

    std::vector<ms::AnimationClipPtr> m_animations;
    ms::IDGenerator<Material*> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
    ms::SceneCacheWriter m_cache_writer;
    ms::InstancesManager m_instances_manager;

    ms::PropertyManager m_property_manager;

    msblenCurveHandler m_curves_handler;

    bool m_server_requested_sync;

    string m_editor_command_reply;

    ms::IdUtility id_utility;

#if BLENDER_VERSION >= 300
    blender::GeometryNodesUtils m_geometryNodeUtils;
#endif

    blender::msblenMaterialsExportHelper m_materialsHelper;

    // animation export
    std::map<std::string, AnimationRecord> m_anim_records;
    float m_anim_time = 0.0f;
    bool m_ignore_events = false;
};
using msblenContextPtr = std::shared_ptr<msblenContext>;
#define msblenGetContext() msblenContext::getInstance()
#define msblenGetSettings() msblenContext::getInstance().getSettings()
