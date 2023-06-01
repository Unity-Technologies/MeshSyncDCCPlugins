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

            auto id = static_cast<ID*>(obj->data);
            scene_objects.insert(id->name + 2);
        });
    
    std::unordered_map<std::string, ms::TransformPtr> exportedTransforms;

    m_geometryNodeUtils.each_instanced_object(
        [this, &scene_objects, &exportedTransforms](blender::GeometryNodesUtils::Record& rec) {
            auto obj = rec.obj;
            
            if (!rec.from_file) {
                auto settings = m_settings;
                settings.BakeModifiers = false;
                settings.multithreaded = false;

                auto transform = exportObject(*m_instances_state, m_intermediate_paths, settings, rec.obj, false, true);

                // Objects that aren't in the file should always be hidden:
                transform->visibility = { false, false, false };

                transform->reset();
                exportedTransforms[rec.id] = transform;
            }
            else if (scene_objects.find(static_cast<ID*>(obj->data)->name + 2) == scene_objects.end()) {
                auto settings = m_settings;
                settings.multithreaded = false;
                settings.BakeModifiers = false;
                exportedTransforms[rec.id] = exportObject(*m_instances_state, m_default_paths, settings, rec.obj, false, true);
            }
            else {
                // The object was already synced as part of the scene
                auto& sceneTransform = exportObject(*m_entities_state, m_default_paths, m_settings, rec.obj, true, true);

                if (sceneTransform) {
                    m_instances_state->manager.add(sceneTransform);
                    exportedTransforms[rec.id] = sceneTransform;
                }
            }
        },
        [this, &exportedTransforms](blender::GeometryNodesUtils::Record& rec) {
            auto& transform = exportedTransforms[rec.id];

            // If this transform was not exported (happens when the sync setting for lights, etc. is not enabled), skip it:
            if(!transform)
                return;

            auto world_matrix = msblenEntityHandler::getWorldMatrix(rec.parent);
            auto inverse = mu::invert(world_matrix);

            //parent is always part of the scene
            const auto& parent = exportObject(*m_entities_state, m_default_paths, m_settings, rec.parent, false);

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
