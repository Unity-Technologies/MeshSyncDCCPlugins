#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/SceneGraph/msIdentifier.h"
#include<map>
#include <unordered_map>

#ifndef msRuntime

msDeclClassPtr(InstanceInfo)

namespace ms {

    class InstancesManager
    {
    public:
        std::vector<TransformPtr> getDirtyMeshes();
        std::vector<InstanceInfoPtr> getDirtyInstances();
        std::vector<Identifier>& getDeletedInstanceInfos();
        std::vector<Identifier>& getDeletedMeshes();
        void clearDirtyFlags();
        void add(InstanceInfoPtr instanceInfo);
        void add(TransformPtr mesh);
        void clear();
        void deleteAll();

        void setAlwaysMarkDirty(bool alwaysDirty);

    private:
        struct Record
        {
            bool dirtyInstances = false;
            bool dirtyMesh = false;
            InstanceInfoPtr instances;
            TransformPtr mesh;
        };

        std::map<std::string, Record> m_records;
        std::vector<Identifier> m_deleted_instanceInfo;
        std::vector<Identifier> m_deleted_meshes;
        bool m_always_mark_dirty;
    };



} // namespace ms
#endif // msRuntime
