#include "pch.h"
#include "msmqUtils.h"

#include "MeshUtils/muDebugTimer.h"

std::string GetName(MQObject obj)
{
    char name[MaxNameBuffer];
    obj->GetName(name, sizeof(name));
    mu::SanitizeNodeName(name);
    return mu::ToUTF8(name);
}

std::string GetName(MQMaterial obj)
{
    char name[MaxNameBuffer];
    obj->GetName(name, sizeof(name));
    mu::SanitizeNodeName(name);
    return mu::ToUTF8(name);
}

static std::string GetPathImpl(MQDocument doc, MQObject obj)
{
    std::string ret;
    if (auto parent = doc->GetParentObject(obj))
        ret += GetPathImpl(doc, parent);

    char name[MaxNameBuffer];
    obj->GetName(name, sizeof(name));
    mu::SanitizeNodeName(name);

    ret += "/";
    ret += name;
    return ret;
}
std::string GetPath(MQDocument doc, MQObject obj)
{
    return mu::ToUTF8(GetPathImpl(doc, obj));
}

bool ExtractID(const char *name, int& id)
{
    if (auto p = std::strstr(name, "[id:")) {
        if (sscanf(p, "[id:%08x]", &id) == 1) {
            return true;
        }
    }
    return false;
}

mu::float3 ToEular(const MQAngle& ang, bool flip_head)
{
    if (flip_head) {
        return mu::float3{
            ang.pitch,
            -ang.head + 180.0f, // I can't explain why this modification is needed...
            ang.bank
        } *mu::DegToRad;
    }
    else {
        return mu::float3{
            ang.pitch,
            ang.head,
            ang.bank
        } *mu::DegToRad;
    }
}

mu::quatf ToQuaternion(const MQAngle& ang)
{
    return rotate_zxy(ToEular(ang));
}

void ExtractLocalTransform(MQObject obj, mu::float3& pos, mu::quatf& rot, mu::float3& scale)
{
    pos = to_float3(obj->GetTranslation());
    rot = ToQuaternion(obj->GetRotation());
    scale = to_float3(obj->GetScaling());
}

mu::float4x4 ExtractGlobalMatrix(MQDocument doc, MQObject obj)
{
    auto mat = to_float4x4(obj->GetLocalMatrix());
    if (auto parent = doc->GetParentObject(obj)) {
        auto pmat = ExtractGlobalMatrix(doc, parent);
        mat = mat * pmat;
    }
    return mat;
}
