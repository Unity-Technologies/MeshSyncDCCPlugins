#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/SceneGraph/msIdentifier.h"
#include<map>
#include <unordered_map>
#include "msTransformManager.h"

#ifndef msRuntime

msDeclClassPtr(InstanceInfo)

namespace ms {

    class InstancesManager : public TransformManager
    {
    public:
        std::vector<TransformPtr> getDirtyMeshes();
        std::vector<InstanceInfoPtr> getDirtyInstances();
        std::vector<Identifier>& getDeleted();
        void clearDirtyFlags();
        void add(InstanceInfoPtr instanceInfo);
        void add(TransformPtr entity);
        void clear();
        void touch(const std::string& path);

        void eraseStaleEntities();

        void setAlwaysMarkDirty(bool alwaysDirty);

    private:

        struct Record
        {
            bool dirtyInstances = false;
            bool dirtyMesh = false;
            InstanceInfoPtr instances = nullptr;
            TransformPtr entity = nullptr;
            bool updated = false;
        };

        std::map<std::string, Record> m_records;
        std::vector<Identifier> m_deleted;
        bool m_always_mark_dirty;

        std::mutex m_mutex;

        Record& lockAndGet(const std::string& path);
    };



} // namespace ms
#endif // msRuntime
