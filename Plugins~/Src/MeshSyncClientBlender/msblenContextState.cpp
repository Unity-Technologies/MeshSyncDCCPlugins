#include "msblenContextState.h"
#include <msblenUtils.h>

using namespace msblenUtils;

void msblenContextState::ObjectRecord::clearState()
{
    touched = renamed = false;
    dst = nullptr;
}

void msblenContextState::eraseObjectRecords()
{
    for (auto i = records.begin(); i != records.end(); /**/) {
        if (!i->second.touched)
            records.erase(i++);
        else
            ++i;
    }
}

void msblenContextState::eraseStaleObjects()
{
    eraseObjectRecords();
    manager.eraseStaleEntities();
}

void msblenContextState::clear() {
    manager.clear();
}

void msblenContextState::clearRecordsState() {
    for (std::map<const void*, msblenContextState::ObjectRecord>::value_type& kvp : records)
        kvp.second.clearState();
}

msblenContextState::ObjectRecord& msblenContextState::touchRecord(
    msblenContextPathProvider& paths,
    const Object* obj,
    const std::string& base_path,
    bool children)
{
    auto& rec = records[obj];
    if (rec.touched && base_path.empty())
        return rec; // already touched

    rec.touched = true;

    std::string local_path = paths.get_path(obj);
    if (local_path != rec.path) {
        rec.renamed = true;
        rec.path = local_path;
    }
    std::string path = base_path + local_path;
    manager.touch(path);

    // trace bones
    if (is_armature(obj)) {
        blender::blist_range<struct bPoseChannel> poses = blender::list_range((bPoseChannel*)obj->pose->chanbase.first);
        for (struct bPoseChannel* pose : poses) {
            records[pose->bone].touched = true;
            manager.touch(base_path + paths.get_path(obj, pose->bone));
        }
    }

    // care children
    if (children) {
        each_child(obj, [&](Object* child) {
            touchRecord(paths, child, base_path, true);
            });
    }

    // trace dupli group
    if (Collection* group = get_instance_collection(obj)) {
        const std::string group_path = path + '/' + (group->id.name + 2);
        manager.touch(group_path);

        auto gobjects = blender::list_range((CollectionObject*)group->gobject.first);
        for (auto go : gobjects)
            touchRecord(paths, go->ob, group_path, true);
    }
    return rec;
}
