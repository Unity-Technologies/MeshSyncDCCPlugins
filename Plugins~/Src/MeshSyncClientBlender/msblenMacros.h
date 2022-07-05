#pragma once

//Data structure change in Blender 2.81: https://developer.blender.org/D5558
#define PointerRNA_OWNER_ID(ptr) (ptr.owner_id)
#define PointerRNA_OWNER_ID_CAST(obj) reinterpret_cast<ID*>(obj)

namespace blender {

template<typename T> inline T rna_data(py::object p) { 
    return reinterpret_cast<T>(PointerRNA_OWNER_ID( reinterpret_cast<BPy_StructRNA*>(p.ptr())->ptr) ); 
}
template<typename T> inline void rna_data(py::object p, T& v) { 
    v = reinterpret_cast<T>(PointerRNA_OWNER_ID( reinterpret_cast<BPy_StructRNA*>(p.ptr())->ptr) );
}
}

//----------------------------------------------------------------------------------------------------------------------

#define MSBLEN_BOILERPLATE2(Type, BType)\
    using btype = ::BType;\
    static StructRNA *s_type;\
    ::BType *m_ptr;\
    static StructRNA* type() { return s_type; }\
    Type(const void *p) : m_ptr((::BType*)p) {}\
    Type(py::object p) : m_ptr(rna_data<::BType*>(p)) {}\
    ::BType* ptr() {return m_ptr; }

#define MSBLEN_BOILERPLATE(Type) MSBLEN_BOILERPLATE2(B##Type, Type)

#define MSBLEN_COMPATIBLE(Type)\
    operator Type() { return *(Type*)this; }\
    operator Type() const { return *(const Type*)this; }

#if BLENDER_VERSION >= 302
    #define OB_CURVE OB_CURVES_LEGACY
#endif


