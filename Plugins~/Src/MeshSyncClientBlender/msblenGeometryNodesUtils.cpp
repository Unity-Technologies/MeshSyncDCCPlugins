#include "msblenGeometryNodesUtils.h"
#include <sstream>
#include <BlenderPyObjects/BlenderPyContext.h>
#include <BlenderPyObjects/BlenderPyDepsgraphObjectInstance.h>
#include <BlenderPyObjects/BlenderPyDepsgraph.h>
#include <MeshSync/SceneGraph/msMesh.h>
#include "BlenderPyObjects/BlenderPyScene.h"
#include <msblenUtils.h>
#include <BLI_listbase.h>

#include "msblenEntityHandler.h"


using namespace std;
using namespace mu;

namespace blender {

#if BLENDER_VERSION >= 300
    GeometryNodesUtils::GeometryNodesUtils()
    {
        auto rotation = rotate_x(-90 * DegToRad);
        auto rotation180 = rotate_z(180 * DegToRad);
        auto scale_z = mu::float3::one();
        scale_z.z = -1;

        auto scale_x = mu::float3::one();
        scale_x.x = -1;

        m_blender_to_unity_world =
            to_mat4x4(rotation) *
            scale44(scale_x);

        m_blender_to_unity_local = 
            to_mat4x4(rotation) *
            to_mat4x4(rotation180) *
            scale44(scale_z);
        
        m_camera_light_correction = to_mat4x4(rotate_x(90 * DegToRad));
    }

    /// <summary>
    /// Converts the world matrix from blender to Unity coordinate systems
    /// </summary>
    /// <param name="blenderMatrix"></param>
    /// <returns></returns>
    float4x4 GeometryNodesUtils::blenderToUnityWorldMatrix(ms::TransformPtr transform, const float4x4& blenderMatrix) const
    {
        float4x4 result = blenderMatrix;

        auto type = transform->getType();
        auto is_camera = type == ms::Entity::Type::Camera;
        auto is_light = type == ms::Entity::Type::Light;
        msblenEntityHandler::applyCorrectionIfNeeded(result, is_camera, is_light);

        result = m_blender_to_unity_world *
            result *
            m_blender_to_unity_local;

        // Apply inverse of the correction because the original of the instanced light/camera
        // would have the correction applied and without this its inverse would not match
        // the inverse we use on blender's side:
        if (is_camera || is_light) {
            result = m_camera_light_correction * result;
        }
        
        return result;
    }
        
    void GeometryNodesUtils::setInstancesDirty(bool dirty)
    {
        m_instances_dirty = dirty;
    }
    bool GeometryNodesUtils::getInstancesDirty()
    {
        return m_instances_dirty;
    }

    std::string GeometryNodesUtils::get_data_path(Object* obj) {
        auto data = (ID*)obj->data;
        if (data) {
            return string(data->name) + string(obj->id.name);
        }
        return string(obj->id.name);
    };

    void GeometryNodesUtils::each_instanced_object(
        std::function<void(Record&)> obj_handler,
        std::function<void(Record&)> matrix_handler) {

        std::unordered_map<std::string, Record> records_by_session_id;
        std::unordered_map<std::string, Record> records_by_name;

        // Collect object names in the scene
        std::unordered_set<std::string> file_objects;

       /* BlenderPyScene scene = BlenderPyScene(BlenderPyContext::get().scene());
        scene.each_objects([&](Object* obj) {
            auto path = get_data_path(obj);
            file_objects.insert(path);
        });*/

        auto bpy_data = blender::BData(blender::BlenderPyContext::get().data());
        for (auto obj : bpy_data.objects()) {
            if (obj->id.override_library == nullptr) {
                auto path = get_data_path(obj);
                file_objects.insert(path);
            }
        }

        each_instance([&](Object* obj, Object* parent, float4x4 matrix) {
            ID* id;
            if (obj->data) {
                id = (ID*)obj->data;
            }
            else {
                id = &obj->id;
            }

            //Some objects, i.e. lights, do not use a session uuid.
            bool useName = id->session_uuid == 0;

            // An object might be sharing data with other objects, need to use the object name in keys                
            auto& rec = useName ? records_by_name[std::string(parent->id.name) + "_" + std::string(id->name + 2) + "_" + std::string(obj->id.name + 2)]
                : records_by_session_id[std::string(parent->id.name + 2) + "_" + std::to_string(id->session_uuid) + "_" + std::string(obj->id.name + 2)];

            if (!rec.handled_object) {
                rec.handled_object = true;

                // If there is no data on the object, the object can be uniquely identified by its name:
                if (obj->data) {
                    rec.name = std::string(id->name + 2) + "_" + std::string(obj->id.name + 2);
                }
                else {
                    rec.name = std::string(id->name + 2);
                }

                rec.obj = obj;
                rec.parent = parent;

                rec.from_file = file_objects.find(get_data_path(obj)) != file_objects.end();

                rec.id = std::string(parent->id.name + 2) + "_" + rec.name + "_" + std::to_string(id->session_uuid);
                obj_handler(rec);
            }

            rec.matrices.push_back(matrix);
        });


        // Export transforms
        for (auto& rec : records_by_session_id) {
            if (rec.second.handled_matrices)
                continue;

            rec.second.handled_matrices = true;
            matrix_handler(rec.second);
        }

        for (auto& rec : records_by_name) {
            if (rec.second.handled_matrices)
                continue;

            rec.second.handled_matrices = true;
            matrix_handler(rec.second);
        }
    }

    float4x4 getMatrix(float mat[4][4]) {
        auto object_parent_world_matrix = float4x4();

        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                object_parent_world_matrix[x][y] = mat[x][y];
            }
        }

        return object_parent_world_matrix;
    }
    
    void GeometryNodesUtils::each_instance(std::function<void(Object*, Object*, float4x4)> handler)
    {
        auto blender_ctx = BlenderPyContext::get();
        auto depsgraph_ctx = blender_ctx.evaluated_depsgraph_get();

        auto depsgraph = BlenderPyDepsgraph(depsgraph_ctx);
        
        // Build map of parents to their children:
        std::map<std::string, std::vector<std::string>> instanceParentsToChildren;
        auto bpy_data = blender::BData(blender::BlenderPyContext::get().data());
        
        for (auto obj : bpy_data.objects()) {
            if (obj->parent) 
            {
                auto parentName = msblenUtils::get_name(obj->parent);
                auto objectName = msblenUtils::get_name(obj);

                // Objects could appear twice if they have lib overrides:
                std::vector<std::string>& list = instanceParentsToChildren[parentName];
                if (find(list.begin(), list.end(), objectName) == list.end()) {
                    instanceParentsToChildren[parentName].push_back(objectName);
                }
            }
        }
        
        // Iterate over the object instances collection of depsgraph
        CollectionPropertyIterator it;
        depsgraph.object_instances_begin(&it);

        // Blender returns the instances in a flattened list, which causes duplicate instances when we have nested instances.
        // For example in an instance hierarchy like this:
        // A
        // |-B
        //   |-C
        //   |-D
        // Blender would return the instances in this order:
        // C on parent B
        // D on parent B
        // B on parent A
        // C on parent A (Duplicate!)
        // D on parent A (Duplicate!)
        // To get around this, build a map of child instances to their parents and if we find a parent, we can skip the children of that instance.
        
        // Parent we're currently under:
        std::string currentParent = "";
        
        // Iterator to the current position of child instances:
        vector<string>::iterator currentObjectChildIterator;

        // currentObjectChildIterator cannot be null, this keeps track whether we have an iterator or not:
        bool hasIterator = false;

        // Parent of the object for currentObjectChildIterator
        std::string currentObjectChildIteratorParent = "";

        // Keeps track of the parents we built a child hierarchy for. Once the parent changes, the previous parent's hierarchy is complete.
        std::vector<std::string> processedInstanceParents;
        

        for (; it.valid; depsgraph.object_instances_next(&it)) {

            // Get the instance as a Pointer RNA.
            auto instance_ctx = depsgraph.object_instances_get(&it);
            auto instance = BlenderPyDepsgraphInstance(instance_ctx);

            // Get the object that the instance refers to
            auto instance_object = instance.instance_object();

            // If the object is null, skip
            if (instance_object == nullptr)
                continue;

            // if the instance is not an instance, skip
            if (!instance.is_instance())
                continue;

            auto object = instance.object();

            auto world_matrix = float4x4();
            instance.world_matrix(&world_matrix);

            auto parent = instance.parent();

            auto parentName = msblenUtils::get_name(parent);
            auto objectName = msblenUtils::get_name(object);
            
            if (currentParent != parentName ||
                objectName == currentObjectChildIteratorParent) {
                currentParent = parentName;

                if (instanceParentsToChildren.find(objectName) != instanceParentsToChildren.end()) {
                    hasIterator = true;
                    currentObjectChildIterator = instanceParentsToChildren[objectName].begin();
                    currentObjectChildIteratorParent = objectName;
                }
                else {
                    hasIterator = false;
                }
            }
            
            // build parentToChildren mapping:
            if (std::find(processedInstanceParents.begin(), processedInstanceParents.end(), parentName) == processedInstanceParents.end()) {
                processedInstanceParents.push_back(parentName);
            }

            // If this is the current instance parent, add any instances to it as children
            if (processedInstanceParents[processedInstanceParents.size() - 1] == parentName) {
                instanceParentsToChildren[parentName].push_back(objectName);

                // If we modify the current iterator list, the iterator becomes invalid:
                if (currentObjectChildIteratorParent == parentName) {
                    hasIterator = false;
                }
            }

            // If we're currently iterating over the children of an instance parent, skip this child:
            if (objectName != currentObjectChildIteratorParent) {
                if (hasIterator &&
                    currentObjectChildIterator != instanceParentsToChildren[currentObjectChildIteratorParent].end()) {
                    if (*currentObjectChildIterator == objectName)
                    {
                        ++currentObjectChildIterator;

                        if(currentObjectChildIterator== instanceParentsToChildren[currentObjectChildIteratorParent].end())
                        {
                            hasIterator = false;
                        }

                        continue;
                    }

                    hasIterator = false;
                    currentObjectChildIteratorParent = "";
                }
                else {
                    if (instanceParentsToChildren.find(objectName) != instanceParentsToChildren.end()) {
                        hasIterator = true;
                        currentObjectChildIterator = instanceParentsToChildren[objectName].begin();
                        currentObjectChildIteratorParent = objectName;
                    }
                    else
                    {
                        hasIterator = false;
                        currentParent = "";
                    }
                }
            }


            // Instances can be hidden if they reference a mesh without vertices, even if the original mesh has verts.
            // If that happens, don't pass the instance on:
            if (object->type == OB_MESH) {
                auto mesh = (Mesh*)object->data;
                if (mesh == nullptr || mesh->totvert == 0) {
                    continue;
                }
            }
            
            handler(object, parent, world_matrix);
        }

        // Cleanup resources
        depsgraph.object_instances_end(&it);
    }
#endif
}
