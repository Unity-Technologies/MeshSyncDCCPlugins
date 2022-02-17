#include "pch.h"

#include "BlenderPyID.h"
#include "BlenderPyCommon.h" //call, etc

namespace blender
{

extern bContext *g_context;


StructRNA* BlenderPyID::s_type;
PropertyRNA* BlenderPyID_is_updated;
PropertyRNA* BlenderPyID_is_updated_data;
FunctionRNA* BlenderPyID_evaluated_get;


const char *BlenderPyID::name() const { return m_ptr->name + 2; }
bool BlenderPyID::is_updated() const {
    return true; //before 2.80: get_bool(m_ptr, BID_is_updated);
}

bool BlenderPyID::is_updated_data() const {
    return true; //before 2.80:  return get_bool(m_ptr, BID_is_updated_data);
}

ID* blender::BlenderPyID::evaluated_get(Depsgraph* depsgraph)
{
    return call<ID, ID*, Depsgraph*>(g_context, m_ptr, BlenderPyID_evaluated_get, depsgraph);
}



} // namespace blender
