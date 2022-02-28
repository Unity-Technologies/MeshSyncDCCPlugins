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
        std::vector<InstanceInfoPtr> getAllInstances();
        std::vector<InstanceInfoPtr> getDirtyInstances();
        std::vector<Identifier>& getDeleted();
        void clearDirtyFlags();
        void add(InstanceInfoPtr instanceInfo);
        void clear();
        void deleteAll();

        void setAlwaysMarkDirty(bool alwaysDirty);

    private:
        struct Record
        {
            bool dirty = true;
            InstanceInfoPtr instances;
        };

        std::map<std::string, Record> m_records;
        std::vector<Identifier> m_deleted;
        std::mutex m_mutex;
        bool m_always_mark_dirty;
    };



} // namespace ms
#endif // msRuntime
