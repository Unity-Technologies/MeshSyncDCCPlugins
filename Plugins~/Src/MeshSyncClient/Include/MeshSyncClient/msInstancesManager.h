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
    TransformPtr entity = nullptr;
    bool updated = false;
    std::map<std::string, bool> updatedParents;
    std::map<std::string, std::vector<InstanceInfoPtr>> instancesPerParent;
};

/// <summary>
/// Manager for transforms and transform instances.
/// </summary>
class InstancesManager : public TransformManager<InstancesManagerRecord>
{
public:
    /// <summary>
    /// Returns meshes that have changed since the last export.
    /// </summary>
    std::vector<TransformPtr> getDirtyMeshes();

    /// <summary>
    /// Returns instaces that have changed since the last export.
    /// </summary>
    std::vector<InstanceInfoPtr> getDirtyInstances();

    /// <summary>
    /// Returns identifiers of records that have been deleted since the last export.
    /// </summary>
    std::vector<Identifier>& getDeleted();

    /// <summary>
    /// Clears the dirty flags on the records.
    /// </summary>
    void clearDirtyFlags();

    /// <summary>
    /// Adds a record about instancing information.
    /// </summary>
    void add(InstanceInfoPtr instanceInfo);

    /// <summary>
    /// Adds or updates a record about the transform.
    /// </summary>
    void add (TransformPtr entity) override;

    /// <summary>
    /// Clears the records collection.
    /// </summary>
    void clear() override;

    /// <summary>
    /// Touches the record at given path. 
    /// The record will not be considered stale
    /// at the end of exportation.
    /// </summary>
    void touch(const std::string& path) override;

    /// <summary>
    /// Erases records that have not 
    /// been added or touched in the last exportation.
    /// </summary>
    void eraseStaleEntities() override;

    bool erase(const std::string& path);

    bool needsMirrorBaking() override { return false; }
};



} // namespace ms
#endif // msRuntime
