#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/SceneGraph/msIdentifier.h"
#include<map>

#ifndef msRuntime

msDeclClassPtr(InstanceInfo)

namespace ms {

    class InstancesManager
    {
    public:
        std::vector<InstanceInfoPtr> getAllInstances();
        std::vector<InstanceInfoPtr> getDirtyInstances();
        void clearDirtyFlags();
        void add(InstanceInfoPtr instanceInfo);
        void clear();

        void setAlwaysMarkDirty(bool alwaysDirty);

    private:
        struct Record
        {
            bool dirty;
            InstanceInfoPtr instances;
        };

        std::map<std::string, Record> m_records;
        std::mutex m_mutex;
    };



} // namespace ms
#endif // msRuntime
