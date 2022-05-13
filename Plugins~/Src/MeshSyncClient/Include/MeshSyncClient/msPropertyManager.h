#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/SceneGraph/msIdentifier.h"
#include <map>
#include <unordered_map>

#ifndef msRuntime

msDeclClassPtr(PropertyInfo)
msDeclClassPtr(Entity)

namespace ms {
    class PropertyManager
    {
    public:
        std::vector<PropertyInfoPtr> getAllProperties();
        void add(PropertyInfoPtr propertyInfo);
        void clear();

        void updateFromServer(std::vector<PropertyInfo> properties, std::vector<EntityPtr> entities);
        std::vector<PropertyInfo> getReceivedProperties();
        std::vector<EntityPtr> getReceivedEntities();
        void clearReceivedProperties();
    private:
        struct Record
        {
            bool dirty = true;
            PropertyInfoPtr propertyInfo;
        };

        std::vector<Record> m_records;
        std::vector<PropertyInfo> m_receivedProperties;
        std::vector<EntityPtr> m_receivedEntities;
        std::mutex m_mutex;
        bool m_always_mark_dirty;
    };
} // namespace ms
#endif // msRuntime
