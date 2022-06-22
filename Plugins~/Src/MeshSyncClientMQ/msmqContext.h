#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/msClient.h"
#include "MeshSync/AsyncSceneSender.h" //AsyncSceneSender
#include "MeshSync/SceneCache/msSceneCacheWriter.h" //SceneCacheWriter
#include "MeshSync/MeshSyncMacros.h"

#include "MeshSyncClient/msEntityManager.h"
#include "MeshSyncClient/msMaterialManager.h"
#include "MeshSyncClient/msTextureManager.h"

#include "MQCacheSettings.h"

struct SyncSettings
{
    ms::ClientSettings client_settings;

    float scale_factor = 100.0f;
    std::string host_camera_path = "/Main Camera";
    bool auto_sync = false;
    bool sync_normals = true;
    bool sync_vertex_color = true;
    bool make_double_sided = false;
    bool sync_camera = true;
    bool sync_morphs = true;
    bool sync_bones = true;
    bool sync_poses = true;
    bool sync_textures = true;
    bool recording = false;

    bool bake_skin = false;
    bool bake_cloth = false;

    void validate();
};


class msmqContext
{
public:
    msmqContext(MQBasePlugin *plugin);
    ~msmqContext();

    SyncSettings& getSettings();

    void logInfo(const char *format, ...);
    bool isServerAvailable();
    const std::string& getErrorMessage();

    bool isSending();
    void wait();
    void clear();
    void update(MQDocument doc);

    bool startRecording(std::string& path);
    void stopRecording();

    bool sendMaterials(MQDocument doc, bool dirty_all);
    bool sendMeshes(MQDocument doc, bool dirty_all);
    bool sendCamera(MQDocument doc, bool dirty_all);
    bool importMeshes(MQDocument doc);

private:
    struct MorphRecord {
        MS_CLASS_DEFAULT_NOCOPY_NOASSIGN(MorphRecord);
        MQObject base_obj = nullptr;
        MQObject target_obj = nullptr;
        ms::MeshPtr base;
        ms::BlendShapeDataPtr dst;

    };

    struct ObjectRecord {
        MS_CLASS_DEFAULT_NOASSIGN(ObjectRecord);
        MQObject obj = nullptr;
        ms::MeshPtr dst;
    };

    struct BoneRecord {
        MS_CLASS_DEFAULT_NOCOPY_NOASSIGN(BoneRecord);
        UINT bone_id = -1;
        UINT parent_id = -1;
        std::string name;
        mu::float3 pose_pos = mu::float3::zero();
        mu::quatf pose_rot = mu::quatf::identity();
        mu::float4x4 bindpose = mu::float4x4::identity();
        ms::TransformPtr dst = ms::Transform::create();
    };

    using HostMeshes = std::map<int, ms::MeshPtr>;

    void WaitAndKickAsyncExport();
    int getMaterialID(int material_index) const;
    int exportTexture(const std::string& path, ms::TextureType type);
    int RegisterSceneMaterials(MQDocument doc);

    MQObject findMesh(MQDocument doc, const char *name);
    MQObject createMesh(MQDocument doc, const ms::Mesh& data, const char *name);
    void extractMeshData(MQDocument doc, MQObject src, ms::Mesh& dst);
    void extractCameraData(MQDocument doc, MQScene src, ms::Camera& dst); // true if anything changed
    void buildBonePath(std::string& dst, BoneRecord& bd);


    MQBasePlugin *m_plugin = nullptr;

    SyncSettings m_settings;
    MQCacheSettings m_cache_settings;

    HostMeshes m_host_meshes;

    std::vector<ObjectRecord> m_obj_records;
    std::map<UINT, MorphRecord> m_morph_records;
    std::map<UINT, BoneRecord> m_bone_records;

    ms::IDGenerator<MQCMaterial*> m_material_ids;
    std::vector<int> m_material_index_to_id;

    ms::CameraPtr m_camera;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;

    ms::AsyncSceneSender m_send_meshes;
    ms::AsyncSceneSender m_send_camera;
    ms::SceneCacheWriter m_cache_writer;
    bool m_pending_send_meshes = false;

    mu::nanosec m_time = 0;
};
