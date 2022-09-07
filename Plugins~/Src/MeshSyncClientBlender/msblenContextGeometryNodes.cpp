#include "msblenGeometryNodeUtils.h"
#include "msblenContext.h"
#include "BlenderPyObjects/BlenderPyScene.h" //BlenderPyScene

/// Geometry Nodes Blender Context Functionality ///
#if BLENDER_VERSION >= 300

ms::InstanceInfoPtr msblenContext::exportInstanceInfo(
    msblenContextState& state,
    msblenContextPathProvider& paths,
    Object* instancedObject,
    Object* parent,
    SharedVector<mu::float4x4> mat) {

    auto info = ms::InstanceInfo::create();
    info->path = paths.get_path(instancedObject);
    info->parent_path = m_default_paths.get_path(parent); // parent will always be part of the scene

    info->transforms = std::move(mat);

    m_instances_manager.add(info);

    return info;
}

void msblenContext::exportInstances() {

    blender::BlenderPyScene scene = blender::BlenderPyScene(blender::BlenderPyContext::get().scene());

    std::unordered_set<Object*> scene_objects;
    scene.each_objects([this, &scene_objects](Object* obj)
        {
            scene_objects.insert(obj);
        });

    // Assume everything is now dirty
    m_instances_state->manager.setAlwaysMarkDirty(true);

    m_geometryNodeUtils.each_instanced_object([this, &scene_objects](Object* instanced, Object* parent, SharedVector<mu::float4x4> matrices, bool fromFile) {

        auto settings = m_settings;
        settings.BakeTransform = false;

        // There is some race condition that is causing rendering glitches on Unity. Seems related to UVs or triangle indices.
        // Not using threads seems to fix it but should be investigated more.
        settings.multithreaded = false;

        auto world_matrix = msblenEntityHandler::getWorldMatrix(parent);
        auto inverse = mu::invert(world_matrix);

        // If the instanced object is not present in the file
        if (!fromFile) {
            settings.BakeModifiers = false;
            auto transform = exportObject(*m_instances_state, m_intermediate_paths, settings, instanced, false);
            transform->reset();
            return exportInstancesWithPathProvider(instanced, parent, std::move(matrices), inverse, m_intermediate_paths);
        }
        
        // check if the object has been already exported as part of the scene
        auto scene_object = scene_objects.find(instanced);
        if (scene_object == scene_objects.end()) {
            exportObject(*m_instances_state, m_default_paths, settings, instanced, false);
        }

        return exportInstancesWithPathProvider(instanced, parent, std::move(matrices), inverse, m_default_paths);
        });

    m_geometryNodeUtils.setInstancesDirty(false);

    scene_objects.clear();
}

void msblenContext::exportInstancesWithPathProvider(Object* instancedObject, Object* parent, SharedVector<mu::float4x4> mat, mu::float4x4& inverse, msblenContextPathProvider& pathProvider)
{
    mu::parallel_for(0, mat.size(), 10, [&](int i)
        {
            mat[i] = m_geometryNodeUtils.blenderToUnityWorldMatrix(instancedObject, mat[i] * inverse);
        });

    exportInstanceInfo(*m_instances_state, pathProvider, instancedObject, parent, std::move(mat));
}

#endif
