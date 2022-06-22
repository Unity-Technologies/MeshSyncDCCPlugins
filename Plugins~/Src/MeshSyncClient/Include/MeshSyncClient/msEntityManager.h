#pragma once
#include "MeshSync/MeshSync.h" //msDeclClassPtr
#include "msTransformManager.h"
#include "MeshSync/SceneGraph/msEntity.h"

msDeclStructPtr(Identifier)
msDeclClassPtr(Transform)

#ifndef msRuntime
namespace ms {


struct EntityManagerRecord
{
    TransformPtr entity;
    int order = 0;
    uint64_t checksum_trans = 0;
    uint64_t checksum_geom = 0;
    bool dirty_trans = false;
    bool dirty_geom = false;
    bool updated = false;
    std::future<void> task;

    void waitTask();
};

class EntityManager : public TransformManager<EntityManagerRecord>
{
public:
    EntityManager();
    ~EntityManager();
    bool empty() const;

    // clear all states (both entity and delete record will be cleared)
    void clear() override;
    void clearEntityRecords();
    void clearDeleteRecords();

    // erase entity and add delete record
    bool erase(const std::string& path);
    bool erase(int id);
    bool erase(const Identifier& identifier);
    bool erase(TransformPtr v);
    bool eraseThreadSafe(TransformPtr v);

    // thread safe
    void add(TransformPtr v) override;

    void touch(const std::string& path) override;

    std::vector<TransformPtr> getAllEntities();
    std::vector<TransformPtr> getDirtyTransforms();
    std::vector<TransformPtr> getDirtyGeometries();
    std::vector<Identifier>& getDeleted();
    void makeDirtyAll();
    void clearDirtyFlags();

    std::vector<TransformPtr> getStaleEntities();
    void eraseStaleEntities() override;

    void updateChecksumGeom(Transform* obj);

private:

    void waitTasks();

    void addTransform(TransformPtr v);
    void addGeometry(TransformPtr v);

    using kvp = std::map<std::string, EntityManagerRecord>::value_type;

    int m_order = 0;
};

} // namespace ms
#endif // msRuntime
