#include "pch.h"

#include "BlenderPyID.h"
#include "BlenderPyCommon.h" //call, etc

namespace blender
{

extern bContext *g_context;


StructRNA* BID::s_type;
PropertyRNA* BID_is_updated;
PropertyRNA* BID_is_updated_data;
FunctionRNA* BID_evaluated_get;


const char *BID::name() const { return m_ptr->name + 2; }
bool BID::is_updated() const
{
#if BLENDER_VERSION < 280
    return get_bool(m_ptr, BID_is_updated);
#else
    return true;
#endif

}
bool BID::is_updated_data() const
{
#if BLENDER_VERSION < 280
    return get_bool(m_ptr, BID_is_updated_data);
#else
    return true;
#endif
}

ID* blender::BID::evaluated_get(Depsgraph* depsgraph)
{
    return call<ID, ID*, Depsgraph*>(m_ptr, BID_evaluated_get, depsgraph);
}



} // namespace blender
