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

    void InstancesManager::clearDirtyFlags()
    {
        for (auto& p : m_records) {
            Record& r = p.second;
            r.dirty = false;
        }
    }

    /// TODO - Make this thread safe
    void InstancesManager::add(InstanceInfoPtr info)
    {
        auto& rec = m_records[info->path];
        rec.instances = info;
        rec.dirty = true;
    }

    void InstancesManager::clear()
    {
        m_records.clear();
    }

    void InstancesManager::setAlwaysMarkDirty(bool alwaysDirty) {
        // For the moment, we always mark it as dirty, do nothing
    }
}
