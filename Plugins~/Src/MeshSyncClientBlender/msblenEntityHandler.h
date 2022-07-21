#pragma once

#include "../MeshSyncClientBlender/msblenContextState.h"
#include "BlenderSyncSettings.h"

#include <MeshSyncClient/msEntityManager.h>

static const mu::float4x4 g_arm_to_world = mu::float4x4{
	1, 0, 0, 0,
	0, 0,-1, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};
static const mu::float4x4 g_world_to_arm = mu::float4x4{
	1, 0, 0, 0,
	0, 0, 1, 0,
	0,-1, 0, 0,
	0, 0, 0, 1
};

// Base class for exporting and importing entity data from and to blender.
class msblenEntityHandler {
public:
	// TODO: make all of these instance methods and have a handler for each entity type so:

	static mu::float4x4 getWorldMatrix(const Object* obj);
	static mu::float4x4 getLocalMatrix(const Object* obj);
	static mu::float4x4 getLocalMatrix(const Bone* bone);
	static mu::float4x4 getLocalMatrix(const bPoseChannel* pose);

	static void extractTransformData(BlenderSyncSettings& settings, const Object* src,
		mu::float3& t, mu::quatf& r, mu::float3& s, ms::VisibilityFlags& vis,
		mu::float4x4* dst_world = nullptr, mu::float4x4* dst_local = nullptr);
	static void extractTransformData(BlenderSyncSettings& settings, const Object* src, ms::Transform& dst);
	static void extractTransformData(BlenderSyncSettings& settings, const bPoseChannel* pose, mu::float3& t, mu::quatf& r, mu::float3& s);

	static void extract_bone_trs(const mu::float4x4& mat, mu::float3& t, mu::quatf& r, mu::float3& s);
};

