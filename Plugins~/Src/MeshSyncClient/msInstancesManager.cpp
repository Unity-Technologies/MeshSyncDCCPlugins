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
                for (auto& instances : r.instancesPerParent)
                {
                    ret.insert(ret.end(), instances.second.begin(), instances.second.end());
                }
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
            r.updatedParents.clear();
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
        
        for (int i = 0; i < rec.instancesPerParent[info->parent_path].size(); ++i)
        {
            const auto& instance = rec.instancesPerParent[info->parent_path][i];

            if (instance->parent_path == info->parent_path)
            {
                // Remove instance from the list, the new one will be added below:
                rec.instancesPerParent[info->parent_path].erase(rec.instancesPerParent[info->parent_path].begin() + i);

                break;
            }
        }

        // instanceInfos need to be sent every update because it's how the server knows which instances are still alive:
        rec.dirtyInstances = true;

        rec.instancesPerParent[info->parent_path].push_back(info);
        rec.updatedParents[info->parent_path] = true;
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
                m_records.erase(it++);
            }
            else
            {
                // Remove records to parents that were not updated:
                auto& parentMap = it->second.instancesPerParent;
                auto& updatedParents = it->second.updatedParents;

                for (auto parentMap_it = parentMap.cbegin(), nextParentMap_it = parentMap_it; 
                    parentMap_it != parentMap.cend();
                    parentMap_it = nextParentMap_it)
                {
                    ++nextParentMap_it;

                    auto& instances = parentMap_it->second;
                     
                    for (auto& instance : instances)
                    {
                        auto& parentPath = instance->parent_path;
                        if (!updatedParents[parentPath])
                        {
                            parentMap.erase(parentPath);
                        }
                    }
                }

                ++it;
            }
        }
    }
}
