#include "msblenGeometryNodeUtils.h"
#include "msblenContext.h"
#include "BlenderPyObjects/BlenderPyScene.h" //BlenderPyScene

/// Geometry Nodes Blender Context Functionality ///
#if BLENDER_VERSION >= 300

ms::InstanceInfoPtr msblenContext::exportInstanceInfo(
    msblenContextState& state,
    msblenContextPathProvider& paths,
    ms::TransformPtr transform,
    ms::TransformPtr parent,
    SharedVector<mu::float4x4> mat) {

    auto info = ms::InstanceInfo::create();
    info->path = transform->path;
    info->parent_path = parent->path; // parent will always be part of the scene

    info->transforms = std::move(mat);

    m_instances_manager.add(info);

    return info;
}

void msblenContext::exportInstances() {

    blender::BlenderPyScene scene = blender::BlenderPyScene(blender::BlenderPyContext::get().scene());

    std::unordered_set<std::string> scene_objects;
    scene.each_objects([this, &scene_objects](Object* obj)
        {
            if (obj == nullptr || obj->data == nullptr)
                return;

            auto id = (ID*)obj->data;
            scene_objects.insert(id->name + 2);
        });

    // Assume everything is now dirty
    m_instances_state->manager.setAlwaysMarkDirty(true);

    std::unordered_map<unsigned int, ms::TransformPtr> exportedTransforms;

    m_geometryNodeUtils.each_instanced_object(
        [this, &scene_objects, &exportedTransforms](blender::GeometryNodesUtils::Record& rec) {
            auto obj = rec.obj;
            if (!rec.from_file) {
                auto settings = m_settings;
                settings.BakeModifiers = false;
                settings.multithreaded = false;

                auto transform = exportObject(*m_instances_state, m_intermediate_paths, settings, rec.obj, false, true);
                transform->reset();
                exportedTransforms[rec.id] = transform;
            }
            else if (scene_objects.find(rec.name) == scene_objects.end()) {
                exportedTransforms[rec.id] = exportObject(*m_instances_state, m_default_paths, m_settings, rec.obj, false);
            }
            else {
                exportedTransforms[rec.id] = exportObject(*m_entities_state, m_default_paths, m_settings, rec.obj, true, true);
            }
        },
        [this, &exportedTransforms](blender::GeometryNodesUtils::Record& rec) {
            auto world_matrix = msblenEntityHandler::getWorldMatrix(rec.parent);
            auto inverse = mu::invert(world_matrix);
            auto& transform = exportedTransforms[rec.id];

            //parent is always part of the scene
            auto& parent = exportObject(*m_entities_state, m_default_paths, m_settings, rec.parent, false);

            if (rec.from_file) {
                exportInstances(transform, parent, std::move(rec.matrices), inverse, m_default_paths);
            }
            else {
                exportInstances(transform, parent, std::move(rec.matrices), inverse, m_intermediate_paths);
            }
            
        });

    m_geometryNodeUtils.setInstancesDirty(false);

    scene_objects.clear();
}

void msblenContext::exportInstances(ms::TransformPtr transform, ms::TransformPtr parent, SharedVector<mu::float4x4> mat, mu::float4x4& inverse, msblenContextPathProvider& pathProvider)
{
    mu::parallel_for(0, mat.size(), 10, [&](int i)
        {
            mat[i] = m_geometryNodeUtils.blenderToUnityWorldMatrix(transform, mat[i] * inverse);
        });

    exportInstanceInfo(*m_instances_state, pathProvider, transform, parent, std::move(mat));
}

#endif
