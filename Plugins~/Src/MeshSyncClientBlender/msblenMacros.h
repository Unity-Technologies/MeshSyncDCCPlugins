#pragma once

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


