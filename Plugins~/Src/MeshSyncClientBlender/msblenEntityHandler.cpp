#include "pch.h"
#include "msblenBinder.h"
#include "msblenEntityHandler.h"
#include "msblenUtils.h"

#include "msblenBinder.h"

#include "MeshSync/SceneGraph/msCurve.h"

namespace bl = blender;
using namespace msblenUtils;

static inline mu::float4x4 camera_correction(const mu::float4x4& v)
{
    return mu::float4x4{ {
        {-v[0][0],-v[0][1],-v[0][2],-v[0][3]},
        { v[1][0], v[1][1], v[1][2], v[1][3]},
        {-v[2][0],-v[2][1],-v[2][2],-v[2][3]},
        { v[3][0], v[3][1], v[3][2], v[3][3]},
    } };
}

mu::float4x4 msblenEntityHandler::getWorldMatrix(const Object* obj)
{
    mu::float4x4 r = bl::BObject(obj).matrix_world();
    if (is_camera(obj) || is_light(obj)) {
        // camera/light correction
        r = camera_correction(r);
    }
    return r;
}

void msblenEntityHandler::extract_bone_trs(const mu::float4x4& mat, mu::float3& t, mu::quatf& r, mu::float3& s)
{
    mu::extract_trs(mat, t, r, s);
    // armature-space to world-space
    t = mu::swap_yz(mu::flip_z(t));
    r = mu::swap_yz(mu::flip_z(r));
    s = mu::swap_yz(s);
}

mu::float4x4 msblenEntityHandler::getLocalMatrix(const Object* obj)
{
    mu::float4x4 r = bl::BObject(obj).matrix_local();
    if (obj->parent && obj->partype == PARBONE) {
        if (Bone* bone = find_bone(obj->parent, obj->parsubstr)) {
            r *= mu::translate(mu::float3{ 0.0f, mu::length((mu::float3&)bone->tail - (mu::float3&)bone->head), 0.0f });
            r *= g_world_to_arm;
        }
    }

    if (is_camera(obj) || is_light(obj)) {
        // camera/light correction
        r = camera_correction(r);
    }
    return r;
}

mu::float4x4 msblenEntityHandler::getLocalMatrix(const Bone* bone)
{
    mu::float4x4 r = (mu::float4x4&)bone->arm_mat;
    if (struct Bone* parent = bone->parent)
        r *= mu::invert((mu::float4x4&)parent->arm_mat);
    else
        r *= g_arm_to_world;
    // todo: armature to world here
    return r;
}

mu::float4x4 msblenEntityHandler::getLocalMatrix(const bPoseChannel* pose)
{
    mu::float4x4 r = (mu::float4x4&)pose->pose_mat;
    if (struct bPoseChannel* parent = pose->parent)
        r *= mu::invert((mu::float4x4&)parent->pose_mat);
    else
        r *= g_arm_to_world;
    // todo: armature to world here
    return r;
}

void msblenEntityHandler::extractTransformData(BlenderSyncSettings& settings, const Object* src, ms::Transform& dst)
{
    extractTransformData(settings, src, dst.position, dst.rotation, dst.scale, dst.visibility, &dst.world_matrix, &dst.local_matrix);
}

void msblenEntityHandler::extractTransformData(BlenderSyncSettings& settings, const bPoseChannel* src, mu::float3& t, mu::quatf& r, mu::float3& s)
{
    if (settings.BakeTransform) {
        t = mu::float3::zero();
        r = mu::quatf::identity();
        s = mu::float3::one();
    }
    else {
        extract_bone_trs(msblenEntityHandler::getLocalMatrix(src), t, r, s);
    }
}

void msblenEntityHandler::extractTransformData(BlenderSyncSettings& settings, const Object* obj,
    mu::float3& t, mu::quatf& r, mu::float3& s, ms::VisibilityFlags& vis,
    mu::float4x4* dst_world, mu::float4x4* dst_local)
{
    vis = { visible_in_collection(obj), visible_in_render(obj), visible_in_viewport(obj) };

    const mu::float4x4 local = getLocalMatrix(obj);
    const mu::float4x4 world = getWorldMatrix(obj);
    if (dst_world)
        *dst_world = world;
    if (dst_local)
        *dst_local = local;

    if (settings.BakeTransform) {
        if (is_camera(obj) || is_light(obj)) {
            mu::extract_trs(world, t, r, s);
        }
        else {
            t = mu::float3::zero();
            r = mu::quatf::identity();
            s = mu::float3::one();
        }
    }
    else {
        mu::extract_trs(local, t, r, s);
    }
}