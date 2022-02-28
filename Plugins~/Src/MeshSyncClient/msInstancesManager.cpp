#include "MeshSyncClient/msInstancesManager.h"
#include "pch.h"
#include "MeshSync/SceneGraph/msInstanceInfo.h"

using namespace std;
namespace ms {
    vector<InstanceInfoPtr> ms::InstancesManager::getAllInstances()
    {
        
        vector<InstanceInfoPtr> ret;
        for (auto& v : m_records)
            ret.push_back(v.second.instances);

        return ret;
    }

    vector<InstanceInfoPtr> InstancesManager::getDirtyInstances()
    {
        vector<InstanceInfoPtr> ret;
        for (auto& p : m_records) {
            Record& r = p.second;
            if (r.dirty) {
                ret.push_back(r.instances);
            }
        }
        return ret;
    }

    vector<Identifier>& InstancesManager::getDeleted()
    {
        return m_deleted;
    }

    void InstancesManager::clearDirtyFlags()
    {
        for (auto& p : m_records) {
            Record& r = p.second;
            r.dirty = false;
        }

        m_deleted.clear();
    }

    void InstancesManager::add(InstanceInfoPtr info)
    {
        auto path = info->path;
        auto it = std::find_if(m_deleted.begin(), m_deleted.end(), [&path](Identifier& v) { return v.name == path; });
        if (it != m_deleted.end()) {
            m_deleted.erase(it);
        }

        auto& rec = m_records[path];
        rec.instances = info;
        
        if (m_always_mark_dirty) {
            rec.dirty = true;
        }
    }

    void InstancesManager::clear()
    {
        m_records.clear();
        m_deleted.clear();
    }

    void InstancesManager::deleteAll()
    {
        for (auto& record : m_records) {
            m_deleted.push_back(record.second.instances->getIdentifier());
        }
    }

    void InstancesManager::setAlwaysMarkDirty(bool alwaysDirty) {
        m_always_mark_dirty = alwaysDirty;
    }
}
