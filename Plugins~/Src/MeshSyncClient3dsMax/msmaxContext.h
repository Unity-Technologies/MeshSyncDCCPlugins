#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/SceneGraph/msLight.h"
#include "msmaxUtils.h"
#include "MeshSync/SceneGraph/msTexture.h" //ms::TextureType
#include "MeshSync/Utility/msAsyncSceneExporter.h" //AsyncSceneCacheWriter

#include "MeshSyncClient/ExportTarget.h"
#include "MeshSyncClient/msEntityManager.h"
#include "MeshSyncClient/msMaterialManager.h"
#include "MeshSyncClient/msTextureManager.h"
#include "MeshSyncClient/ObjectScope.h"
#include "MeshSyncClient/MeshSyncClientMacros.h"

#include "MaxCacheSettings.h"
#include "MaxSyncSettings.h"


#define msmaxAPI extern "C" __declspec(dllexport)




class msmaxContext {
public:
    static msmaxContext& getInstance();

    msmaxContext();
    ~msmaxContext();
    MaxSyncSettings& getSettings();
    MaxCacheSettings& getCacheSettings();

    void onStartup();
    void onShutdown();
    void onNewScene();
    void onSceneUpdated();
    void onTimeChanged();
    void onNodeAdded(INode *n);
    void onNodeDeleted(INode *n);
    void onNodeRenamed();
    void onNodeLinkChanged(INode *n);
    void onNodeUpdated(INode *n);
    void onGeometryUpdated(INode *n);
    void onRepaint();

    void logInfo(const char *format, ...);
    bool isServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void update();
    bool sendObjects(MeshSyncClient::ObjectScope scope, bool dirty_all);
    bool sendMaterials(bool dirty_all);
    bool sendAnimations(MeshSyncClient::ObjectScope scope);
    bool ExportCache(const std::string& path, const MaxCacheSettings& cache_settings);

    bool recvScene();

    // thread safe. c will be called from main thread
    void addDeferredCall(const std::function<void()>& c);
    void feedDeferredCalls();

    TimeValue getExportTime() const;

    // UI
    void registerMenu();
    void unregisterMenu();

    void openSettingWindow();
    void closeSettingWindow();
    bool isSettingWindowOpened() const;
    void updateSettingControls();

    void openCacheWindow();
    void closeCacheWindow();
    bool isCacheWindowOpened() const;
    void updateCacheControls();

private:

    NOCOPY_NOASSIGN(msmaxContext);

    struct TreeNode {
        DEFAULT_NOCOPY_NOASSIGN(TreeNode);
        int index = 0;
        INode *node = nullptr;
        Object *baseobj = nullptr;
        std::wstring name;
        std::string path;
        int id = ms::InvalidID;

        bool dirty_trans = true;
        bool dirty_geom = true;
        ms::TransformPtr dst;

        ms::Identifier getIdentifier() const;
        void clearDirty();
        void clearState();
    };

    struct AnimationRecord {
        DEFAULT_NOCOPY_NOASSIGN(AnimationRecord);
        using extractor_t = void (msmaxContext::*)(ms::TransformAnimation& dst, TreeNode *n);
        extractor_t extractor = nullptr;
        TreeNode *node = nullptr;
        ms::TransformAnimationPtr dst;

        void operator()(msmaxContext *_this);
    };

    struct MaterialRecord {
        DEFAULT_NOCOPY_NOASSIGN(MaterialRecord);
        int material_id = 0;
        std::vector<int> submaterial_ids;
    };

    void updateRecords(bool track_delete = true);
    TreeNode& getNodeRecord(INode *n);
    std::vector<TreeNode*> getNodes(MeshSyncClient::ObjectScope scope);

    void kickAsyncExport();

    int exportTexture(const std::string& path, ms::TextureType type = ms::TextureType::Default);
    void exportMaterials();

    ms::TransformPtr exportObject(INode *node, bool tip);
    template<class T> std::shared_ptr<T> createEntity(TreeNode& n);
    ms::TransformPtr exportTransform(TreeNode& node);
    ms::TransformPtr exportInstance(TreeNode& node, ms::TransformPtr base);
    ms::TransformPtr exportCamera(TreeNode& node);
    ms::TransformPtr exportLight(TreeNode& node);
    ms::TransformPtr exportMesh(TreeNode& node);

    mu::float4x4 getPivotMatrix(INode *n);
    mu::float4x4 getWorldMatrix(INode *n, TimeValue t, bool cancel_camera_correction = true);
    void extractTransform(
        TreeNode& node, TimeValue t, mu::float3& pos, mu::quatf& rot, mu::float3& scale, ms::VisibilityFlags& vis,
        mu::float4x4 *dst_world = nullptr, mu::float4x4 *dst_local = nullptr);
    void extractTransform(TreeNode& node, TimeValue t, ms::Transform& dst);
    void extractCameraData(TreeNode& node, TimeValue t,
        bool& ortho, float& fov, float& near_plane, float& far_plane,
        float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift,
        mu::float4x4 *view_mat = nullptr);
    void extractLightData(TreeNode& node, TimeValue t,
        ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& spot_angle);

    void doExtractMeshData(ms::Mesh& dst, INode *n, Mesh *mesh);

    bool exportAnimations(INode *node, bool force);
    void extractTransformAnimation(ms::TransformAnimation& dst, TreeNode *n);
    void extractCameraAnimation(ms::TransformAnimation& dst, TreeNode *n);
    void extractLightAnimation(ms::TransformAnimation& dst, TreeNode *n);
    void extractMeshAnimation(ms::TransformAnimation& dst, TreeNode *n);


    void DoExportSceneCache(const int sceneIndex, const MeshSyncClient::MaterialFrameRange materialFrameRange, 
                            const std::vector<msmaxContext::TreeNode*>& nodes);

private:
    MaxSyncSettings m_settings;
    MaxCacheSettings m_cache_settings;
    ISceneEventManager::CallbackKey m_cbkey = 0;

    std::map<INode*, TreeNode> m_node_records;
    std::map<Mtl*, MaterialRecord> m_material_records;
    std::vector<std::future<void>> m_async_tasks;
    std::vector<TriObject*> m_tmp_triobj;
    std::vector<Mesh*> m_tmp_meshes;
    RenderScope m_render_scope;
    time_t m_time_to_update_scene;

    bool m_dirty = true;
    bool m_scene_updated = true;
    MeshSyncClient::ObjectScope m_pending_request = MeshSyncClient::ObjectScope::None;

    std::map<INode*, AnimationRecord> m_anim_records;
    TimeValue m_current_time_tick;
    float m_anim_time = 0.0f;
    std::vector<ms::AnimationClipPtr> m_animations;

    ms::IDGenerator<Mtl*> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
    ms::AsyncSceneCacheWriter m_cache_writer;

    std::vector<std::function<void()>> m_deferred_calls;
    std::mutex m_mutex;
};

#define msmaxGetContext() msmaxContext::getInstance()
#define msmaxGetSettings() msmaxGetContext().getSettings()
bool msmaxSendScene(MeshSyncClient::ExportTarget target, MeshSyncClient::ObjectScope scope);
bool msmaxExportCache(const std::string& path, const MaxCacheSettings& cache_settings);
