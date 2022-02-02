#pragma once


#include "MeshSync/MeshSync.h"
#include "MeshSync/msClient.h"
#include "MeshSync/SceneGraph/msLight.h"
#include "MeshSync/SceneGraph/msCamera.h"
#include "MeshSync/AsyncSceneSender.h" //AsyncSceneSender
#include "MeshSync/MeshSyncMacros.h"

#include "MeshSyncClient/msEntityManager.h"
#include "MeshSyncClient/msMaterialManager.h"
#include "MeshSyncClient/msTextureManager.h"

struct SyncSettings
{
    ms::ClientSettings client_settings;
    int  timeout_ms = 5000;
    float scale_factor = 100.0f;
    float frame_step = 1.0f;

    bool auto_sync = false;
    bool sync_cameras = true;
    bool sync_lights = true;
    bool sync_bones = true;
    bool sync_blendshapes = true;
    bool sync_meshes = true;
    bool sync_textures = true;
    bool make_double_sided = false;
    bool bake_deformars = false;
    bool parallel_extraction = true;

    void validate();
};

class msmobuDevice : public FBDevice
{
FBDeviceDeclare(msmobuDevice, FBDevice);
public:
    bool FBCreate() override;
    void FBDestroy() override;
    bool DeviceOperation(kDeviceOperations pOperation) override;
    void DeviceTransportNotify(kTransportMode pMode, FBTime pTime, FBTime pSystem) override;

    SyncSettings& getSettings();

    void logInfo(const char *format, ...);
    void logError(const char *format, ...);
    bool isServerAvailable();
    const std::string& getErrorMessage();

    void wait();
    void update();

    bool sendMaterials(bool dirty_all);
    bool sendObjects(bool dirty_all);
    bool sendAnimations();

private:
    struct NodeRecord
    {
        std::string name;
        std::string path;
        int id = ms::InvalidID;
        int index = 0;
        FBModel *src = nullptr;
        ms::TransformPtr dst;
        bool exist = false;

        ms::Identifier getIdentifier() const;
    };
    using NodeRecords = std::map<FBModel*, NodeRecord>;
    using ExtractTasks = std::vector<std::function<void()>>;

    struct TextureRecord {
        MS_CLASS_DEFAULT_NOCOPY_NOASSIGN(TextureRecord);
        int id = ms::InvalidID;
        ms::Texture *dst = nullptr;
    };
    using TextureRecords = std::map<FBTexture*, TextureRecord>;

    struct MaterialRecord  {
        MS_CLASS_DEFAULT_NOCOPY_NOASSIGN(MaterialRecord);
        int id = 0;
        ms::Material *dst = nullptr;
    };
    using MaterialRecords = std::map<FBMaterial*, MaterialRecord>;

    struct AnimationRecord  {
        MS_CLASS_DEFAULT_NOCOPY_NOASSIGN(AnimationRecord);
        using extractor_t = void (msmobuDevice::*)(ms::TransformAnimation& dst, FBModel *src);

        ms::TransformAnimationPtr dst;
        FBModel *src = nullptr;
        extractor_t extractor = nullptr;

        void operator()(msmobuDevice *_this);
    };
    using AnimationRecords = std::map<FBModel*, AnimationRecord>;

private:
    void onSceneChange(HIRegister pCaller, HKEventBase pEvent);
    void onDataUpdate(HIRegister pCaller, HKEventBase pEvent);
    void onRender(HIRegister pCaller, HKEventBase pEvent);
    void onSynchronization(HIRegister pCaller, HKEventBase pEvent);

    void kickAsyncSend();

    ms::TransformPtr exportObject(FBModel* src, bool parent, bool tip = true);
    template<class T> std::shared_ptr<T> createEntity(NodeRecord& n);
    ms::TransformPtr exportTransform(NodeRecord& n);
    ms::CameraPtr exportCamera(NodeRecord& n);
    ms::LightPtr exportLight(NodeRecord& n);
    ms::MeshPtr exportBlendshapeWeights(NodeRecord& n);
    ms::MeshPtr exportMesh(NodeRecord& n);
    void doExtractMesh(ms::Mesh& dst, FBModel* src);

    void extractTransformData(FBModel *src, mu::float3& pos, mu::quatf& rot, mu::float3& scale, ms::VisibilityFlags& vis);
    void extractCameraData(FBCamera* src, bool& ortho, float& near_plane, float& far_plane, float& fov,
        float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift);
    void extractLightData(FBLight* src, ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& spot_angle);

    int exportTexture(FBTexture* src, FBMaterialTextureType type);
    bool exportMaterial(FBMaterial* src, int index);
    bool RegisterSceneMaterials();

    bool exportAnimations();
    bool exportAnimation(FBModel* src, bool force);
    void extractTransformAnimation(ms::TransformAnimation& dst, FBModel* src);
    void extractCameraAnimation(ms::TransformAnimation& dst, FBModel* src);
    void extractLightAnimation(ms::TransformAnimation& dst, FBModel* src);
    void extractMeshAnimation(ms::TransformAnimation& dst, FBModel* src);

private:
    SyncSettings m_settings;
    bool m_data_updated = false;
    bool m_dirty_meshes = true;
    bool m_dirty_textures = true;
    bool m_pending = false;

    float m_anim_time = 0.0f;
    int m_texture_id_seed = 0;
    int m_node_index_seed = 0;

    NodeRecords m_node_records;
    TextureRecords m_texture_records;
    MaterialRecords m_material_records;
    ExtractTasks m_extract_tasks;
    AnimationRecords m_anim_records;

    std::vector<ms::AnimationClipPtr> m_animations;

    ms::IDGenerator<FBMaterial*> m_material_ids;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;
    ms::AsyncSceneSender m_sender;
};
