#include "MeshSyncClient/msInstancesManager.h"
#include "pch.h"
#include "MeshSync/SceneGraph/msInstanceInfo.h"

using namespace std;
namespace ms {
    std::vector<TransformPtr> InstancesManager::getDirtyMeshes()
    {
        vector<TransformPtr> ret;
        for (auto& p : m_records) {
            InstancesManagerRecord& r = p.second;
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
            InstancesManagerRecord& r = p.second;
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
            InstancesManagerRecord& r = p.second;
            r.dirtyInstances = false;
            r.dirtyMesh = false;
            r.updated = false;
        }

        m_deleted.clear();
    }

    void InstancesManager::add(TransformPtr entity) {

        auto& rec = lockAndGet(entity->path);

        if (m_always_mark_dirty  || rec.entity == nullptr || rec.entity->hash() != entity->hash()) {
            rec.dirtyMesh = true;
        }

        rec.updated = true;
        rec.entity = entity;
    }

    

    void InstancesManager::add(InstanceInfoPtr info)
    {
        auto& rec = lockAndGet(info->path);

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
                if (it->second.instances) {
                    m_deleted.push_back(it->second.instances->getIdentifier());
                }
                m_records.erase(it++);
            }
            else
                ++it;
        }
    }
}
