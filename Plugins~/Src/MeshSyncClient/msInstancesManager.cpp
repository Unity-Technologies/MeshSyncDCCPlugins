#include "MeshSyncClient/msInstancesManager.h"
#include "pch.h"
#include "MeshSync/SceneGraph/msInstanceInfo.h"

using namespace std;
namespace ms {
    std::vector<TransformPtr> InstancesManager::getDirtyMeshes()
    {
        vector<TransformPtr> ret;
        for (auto& p : m_records) {
            Record& r = p.second;
            if (r.dirtyMesh) {
                ret.push_back(r.mesh);
            }
        }
        return ret;
    }
    vector<InstanceInfoPtr> InstancesManager::getDirtyInstances()
    {
        vector<InstanceInfoPtr> ret;
        for (auto& p : m_records) {
            Record& r = p.second;
            if (r.dirtyInstances) {
                ret.push_back(r.instances);
            }
        }
        return ret;
    }

    vector<Identifier>& InstancesManager::getDeletedInstanceInfos()
    {
        return m_deleted_instanceInfo;
    }

    vector<Identifier>& InstancesManager::getDeletedMeshes() {
        return m_deleted_meshes;
    }

    void InstancesManager::clearDirtyFlags()
    {
        for (auto& p : m_records) {
            Record& r = p.second;
            r.dirtyInstances = false;
            r.dirtyMesh = false;
        }

        m_deleted_instanceInfo.clear();
        m_deleted_meshes.clear();
    }

    void InstancesManager::add(TransformPtr mesh) {
        
        auto path = mesh->path;
        auto it = std::find_if(m_deleted_meshes.begin(), m_deleted_meshes.end(), [&path](Identifier& v) { return v.name == path; });
        if (it != m_deleted_meshes.end()) {
            m_deleted_meshes.erase(it);
        }

        auto& rec = m_records[path];

        if (m_always_mark_dirty  || rec.mesh == nullptr || rec.mesh->hash() != mesh->hash()) {
            rec.dirtyMesh = true;
        }

        rec.mesh = mesh;
    }

    void InstancesManager::add(InstanceInfoPtr info)
    {
        auto path = info->path;
        
        auto it = std::find_if(m_deleted_instanceInfo.begin(), m_deleted_instanceInfo.end(), [&path](Identifier& v) { return v.name == path; });
        if (it != m_deleted_instanceInfo.end()) {
            m_deleted_instanceInfo.erase(it);
        }

        auto& rec = m_records[path];

        //TODO implement hash for InstanceInfo and check
        if (m_always_mark_dirty || rec.instances == nullptr) {
            rec.dirtyInstances = true;
        }

        rec.instances = info;
    }

    void InstancesManager::clear()
    {
        m_records.clear();
        m_deleted_instanceInfo.clear();
    }

    void InstancesManager::deleteAll()
    {
        for (auto& record : m_records) {
            m_deleted_instanceInfo.push_back(record.second.instances->getIdentifier());
            if (record.second.mesh != nullptr) {
                m_deleted_meshes.push_back(record.second.mesh->getIdentifier());
            }
        }
    }

    void InstancesManager::setAlwaysMarkDirty(bool alwaysDirty) {
        m_always_mark_dirty = alwaysDirty;
    }
}
