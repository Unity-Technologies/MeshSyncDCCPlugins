#pragma once

#include "MeshSync/MeshSync.h"
#include<map>
#include <unordered_map>
#include "msTransformManager.h"

#ifndef msRuntime

msDeclClassPtr(InstanceInfo)

namespace ms {

struct InstancesManagerRecord
{
    bool dirtyInstances = false;
    bool dirtyMesh = false;
    InstanceInfoPtr instances = nullptr;
    TransformPtr entity = nullptr;
    bool updated = false;
};

class InstancesManager : public TransformManager<InstancesManagerRecord>
{
public:
    std::vector<TransformPtr> getDirtyMeshes();
    std::vector<InstanceInfoPtr> getDirtyInstances();
    std::vector<Identifier>& getDeleted();
    void clearDirtyFlags();
    void add(InstanceInfoPtr instanceInfo);
    void add (TransformPtr entity) override;
    void clear() override;
    void touch(const std::string& path) override;

    void eraseStaleEntities() override;
};



} // namespace ms
#endif // msRuntime
