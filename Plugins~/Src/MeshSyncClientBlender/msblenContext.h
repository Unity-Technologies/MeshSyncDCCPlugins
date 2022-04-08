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

class msblenContext;

class msblenContext {
public:
    static msblenContext& getInstance();
    void Destroy();

    BlenderSyncSettings& getSettings();
    const BlenderSyncSettings& getSettings() const;
    BlenderCacheSettings& getCacheSettings();
    const BlenderCacheSettings& getCacheSettings() const;

    void logInfo(const char *format, ...);
    bool isServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void clear();
    bool prepare();

    bool sendMaterials(bool dirty_all);
    bool sendObjects(MeshSyncClient::ObjectScope scope, bool dirty_all);
    bool sendAnimations(MeshSyncClient::ObjectScope scope);
    bool ExportCache(const std::string& path, const BlenderCacheSettings& cache_settings);

    void flushPendingList();

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

    // note:
    // ObjectRecord and Blender's Object is *NOT* 1 on 1 because there is 'dupli group' in Blender.
    // dupli group is a collection of nodes that will be instanced.
    // so, only the path is unique. Object maybe shared by multiple ObjectRecord.
    struct ObjectRecord {
        MS_CLASS_DEFAULT_NOCOPY_NOASSIGN(ObjectRecord);
        //std::vector<NodeRecord*> branches; // todo

        std::string path;
        std::string name;
        Object *host = nullptr; // parent of dupli group
        Object *obj = nullptr;
        Bone *bone = nullptr;

        ms::TransformPtr dst;

        bool touched = false;
        bool renamed = false;

        void clearState();
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

    msblenContext();
    ~msblenContext();

    static std::vector<Object*> getNodes(MeshSyncClient::ObjectScope scope);

    int exportTexture(const std::string & path, ms::TextureType type);
    int getMaterialID(Material *m);
    ms::MaterialPtr CreateDefaultMaterial(const uint32_t matIndex);
    void RegisterSceneMaterials();
    void RegisterObjectMaterials(const std::vector<Object*> objects);
    void RegisterMaterial(Material* mat, const uint32_t matIndex);


    ms::TransformPtr exportObject(BlenderSyncSettings& settings, const Object *obj, bool parent, bool tip = true);
    ms::TransformPtr exportTransform(BlenderSyncSettings& settings, const Object *obj);
    ms::TransformPtr exportPose(BlenderSyncSettings& settings, const Object *armature, bPoseChannel *obj);
    ms::TransformPtr exportArmature(BlenderSyncSettings& settings, const Object *obj);
    ms::TransformPtr exportReference(BlenderSyncSettings& settings, Object *obj, const DupliGroupContext& ctx);
    ms::TransformPtr exportDupliGroup(BlenderSyncSettings& settings, const Object *obj, const DupliGroupContext& ctx);
    ms::CameraPtr exportCamera(BlenderSyncSettings& settings, const Object *obj);
    ms::LightPtr exportLight(BlenderSyncSettings& settings, const Object *obj);
    ms::MeshPtr exportMesh(BlenderSyncSettings& settings, const Object *obj);

    mu::float4x4 getWorldMatrix(const Object *obj);
    mu::float4x4 getLocalMatrix(const Object *obj);
    mu::float4x4 getLocalMatrix(const Bone *bone);
    mu::float4x4 getLocalMatrix(const bPoseChannel *pose);
    void extractTransformData(BlenderSyncSettings& settings, const Object *src,
        mu::float3& t, mu::quatf& r, mu::float3& s, ms::VisibilityFlags& vis,
        mu::float4x4 *dst_world = nullptr, mu::float4x4 *dst_local = nullptr);
    void extractTransformData(BlenderSyncSettings& settings, const Object *src, ms::Transform& dst);
    void extractTransformData(BlenderSyncSettings& settings, const bPoseChannel *pose, mu::float3& t, mu::quatf& r, mu::float3& s);

    void extractCameraData(const Object *src, bool& ortho, float& near_plane, float& far_plane, float& fov,
        float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift);
    void extractLightData(const Object *src,
        ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& range, float& spot_angle);

    void doExtractMeshData(BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data, mu::float4x4 world);
    void doExtractBlendshapeWeights(BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data);
    void doExtractNonEditMeshData(BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data);
    void doExtractEditMeshData(BlenderSyncSettings& settings, ms::Mesh& dst, const Object *obj, Mesh *data);

    ms::TransformPtr findBone(Object *armature, Bone *bone);
    ObjectRecord& touchRecord(const Object *obj, const std::string& base_path = "", bool children = false);
    void eraseStaleObjects();

    void exportAnimation(BlenderSyncSettings& settings, Object *obj, bool force, const std::string& base_path = "");
    void extractTransformAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);
    void extractPoseAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);
    void extractCameraAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);
    void extractLightAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);
    void extractMeshAnimationData(BlenderSyncSettings& settings, ms::TransformAnimation& dst, void *obj);

    void DoExportSceneCache(const std::vector<Object*>& nodes);
    void WaitAndKickAsyncExport();

private:
    BlenderSyncSettings m_settings;
    BlenderCacheSettings m_cache_settings;
    std::set<const Object*> m_pending;
    std::map<Bone*, ms::TransformPtr> m_bones;
    std::map<const void*, ObjectRecord> m_obj_records; // key can be object or bone

    MeshSyncClient::AsyncTasksController m_asyncTasksController;

    std::vector<const Object*> m_meshes_to_clear;

    std::vector<ms::AnimationClipPtr> m_animations;
    ms::IDGenerator<Material*> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
    ms::SceneCacheWriter m_cache_writer;

    // animation export
    std::map<std::string, AnimationRecord> m_anim_records;
    float m_anim_time = 0.0f;
    bool m_ignore_events = false;
};
using msblenContextPtr = std::shared_ptr<msblenContext>;
#define msblenGetContext() msblenContext::getInstance()
#define msblenGetSettings() msblenContext::getInstance().getSettings()
