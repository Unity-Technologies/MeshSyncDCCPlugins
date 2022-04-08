#pragma once

#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "msITransformManager.h"
#include "MeshSync/SceneGraph/msIdentifier.h"

namespace ms{

/// <summary>
/// Abstraction for managers that handle records with TransformPtr data.
/// </summary>
/// <typeparam name="T">The type of record that the manager uses</typeparam>
template <class T>
class TransformManager : public ITransformManager {
public:

    void setAlwaysMarkDirty(bool v) override {
        m_always_mark_dirty = v;
    }

	virtual void add(TransformPtr transform) = 0;
	virtual void touch(const std::string& path) = 0;
	virtual void eraseStaleEntities() = 0;
	virtual void clear() = 0;

    TransformManager() {}
    ~TransformManager() {}

    TransformManager& operator=(const TransformManager& v)
    {
        return *this;
    }


    TransformManager(TransformManager&& v)
    {
        *this = std::move(v);
    }

    TransformManager& operator=(TransformManager&& v)
    {
          
        return *this;
    }

    TransformManager(const TransformManager& v){ *this = v; }

protected:
    bool m_always_mark_dirty = false;
    std::mutex m_mutex;
    std::vector<Identifier> m_deleted;

    std::map<std::string, T> m_records;

    T& lockAndGet(const std::string& path) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_deleted.empty()) {
            auto it = std::find_if(m_deleted.begin(), m_deleted.end(), [&path](Identifier& v) { return v.name == path; });
            if (it != m_deleted.end())
                m_deleted.erase(it);
        }
        return m_records[path];
    }
};
}