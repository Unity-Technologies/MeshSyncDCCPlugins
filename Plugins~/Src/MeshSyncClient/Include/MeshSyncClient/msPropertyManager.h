#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshSync/SceneGraph/msIdentifier.h"
#include <map>
#include <unordered_map>

#ifndef msRuntime

msDeclClassPtr(PropertyInfo)

namespace ms {
    class PropertyManager
    {
    public:
        std::vector<PropertyInfoPtr> getAllProperties();
        void add(PropertyInfoPtr propertyInfo);
        void clear();

    private:
        struct Record
        {
            bool dirty = true;
            PropertyInfoPtr propertyInfo;
        };

        std::vector<Record> m_records;
        std::vector<Identifier> m_deleted;
        std::mutex m_mutex;
        bool m_always_mark_dirty;
    };
} // namespace ms
#endif // msRuntime
