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
                ret.push_back(r.entity);
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

    vector<Identifier>& InstancesManager::getDeleted() {
        return m_deleted;
    }

    void InstancesManager::clearDirtyFlags()
    {
        for (auto& p : m_records) {
            Record& r = p.second;
            r.dirtyInstances = false;
            r.dirtyMesh = false;
            r.updated = false;
        }

        m_deleted.clear();
    }

    void InstancesManager::add(TransformPtr mesh) {

        auto& rec = lockAndGet(mesh->path);

        if (m_always_mark_dirty  || rec.entity == nullptr || rec.entity->hash() != mesh->hash()) {
            rec.dirtyMesh = true;
        }

        rec.updated = true;
        rec.entity = mesh;
    }

    

    void InstancesManager::add(InstanceInfoPtr info)
    {
        auto& rec = lockAndGet(info->path);

        //TODO implement hash for InstanceInfo and check
        if (m_always_mark_dirty || rec.instances == nullptr) {
            rec.dirtyInstances = true;
        }

        rec.updated = true;
        rec.instances = info;
    }

    void InstancesManager::clear()
    {
        m_records.clear();
    }

    void InstancesManager::touch(const std::string& path)
    {
        auto it = m_records.find(path);
        if (it != m_records.end())
            it->second.updated = true;
    }

    void InstancesManager::eraseStaleEntities()
    {
        for (auto it = m_records.begin(); it != m_records.end(); ) {
            if (!it->second.updated) {
                m_deleted.push_back(it->second.instances->getIdentifier());
                m_records.erase(it++);
            }
            else
                ++it;
        }
    }

    void InstancesManager::setAlwaysMarkDirty(bool alwaysDirty) {
        m_always_mark_dirty = alwaysDirty;
    }

    InstancesManager::Record& InstancesManager::lockAndGet(const std::string& path)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_deleted.empty()) {
            auto it = std::find_if(m_deleted.begin(), m_deleted.end(), [&path](Identifier& v) { return v.name == path; });
            if (it != m_deleted.end())
                m_deleted.erase(it);
        }

        return m_records[path];
    }
}
