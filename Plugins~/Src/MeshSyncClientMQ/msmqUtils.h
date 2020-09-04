#pragma once

#include "MeshSync/MeshSync.h"
#include "MeshUtils/muMath.h"

#define MaxNameBuffer 128


inline mu::float4 to_float4(const MQColor& v)
{
    return { v.r, v.g, v.b, 1.0f };
}
inline mu::float3 to_float3(const MQPoint& v)
{
    return (const mu::float3&)v;
}
inline mu::float4x4 to_float4x4(const MQMatrix& v)
{
    return (const mu::float4x4&)v;
}
inline MQColor to_MQColor(const mu::float4& v)
{
    return MQColor(v[0], v[1], v[2]);
}

std::string GetName(MQObject obj);
std::string GetName(MQMaterial obj);
std::string GetPath(MQDocument doc, MQObject obj);
bool ExtractID(const char *name, int& id);

mu::float3 ToEular(const MQAngle& ang, bool flip_head = false);
mu::quatf ToQuaternion(const MQAngle& ang);
void ExtractLocalTransform(MQObject obj, mu::float3& pos, mu::quatf& rot, mu::float3& scale);
mu::float4x4 ExtractGlobalMatrix(MQDocument doc, MQObject obj);
