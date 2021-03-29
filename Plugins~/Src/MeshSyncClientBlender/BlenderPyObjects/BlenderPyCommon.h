#pragma once
#include <RNA_define.h> //PropertyRNA
#include "msblenMacros.h" //PointerRNA_OWNER_ID
#include "MeshSync/msFoundation.h" //msPacked

namespace blender {

template<typename Self>
static inline void* get_pointer(Self *self, PropertyRNA *prop)
{
    PointerRNA ptr;
    ptr.data = self;
    PointerRNA_OWNER_ID(ptr) = PointerRNA_OWNER_ID_CAST(self);

    PointerRNA ret = ((PointerPropertyRNA*)prop)->get(&ptr);
    return ret.data;
}

//----------------------------------------------------------------------------------------------------------------------

template<typename T> inline T rna_data(py::object p) { 
    return reinterpret_cast<T>(PointerRNA_OWNER_ID( reinterpret_cast<BPy_StructRNA*>(p.ptr())->ptr) ); 
}
template<typename T> inline void rna_data(py::object p, T& v) { 
    v = reinterpret_cast<T>(PointerRNA_OWNER_ID( reinterpret_cast<BPy_StructRNA*>(p.ptr())->ptr) );
}


//----------------------------------------------------------------------------------------------------------------------

template<class R>
struct ret_holder
{
    using ret_t = R & ;
    R r;
    R& get() { return r; }
};
template<>
struct ret_holder<void>
{
    using ret_t = void;
    void get() {}
};

//----------------------------------------------------------------------------------------------------------------------

#pragma pack(push, 1)
template<typename R>
struct param_holder0
{
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
} msPacked;
template<typename R, typename A1>
struct param_holder1
{
    A1 a1;
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
} msPacked;
template<typename R, typename A1, typename A2>
struct param_holder2
{
    A1 a1; A2 a2;
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
} msPacked;
template<typename R, typename A1, typename A2, typename A3>
struct param_holder3
{
    A1 a1; A2 a2; A3 a3;
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
} msPacked;
template<typename R, typename A1, typename A2, typename A3, typename A4>
struct param_holder4
{
    A1 a1; A2 a2; A3 a3; A4 a4;
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
} msPacked;
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
struct param_holder5
{
    A1 a1; A2 a2; A3 a3; A4 a4; A5 a5;
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
} msPacked;
#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------

template<typename T, typename R>
R call(T *self, FunctionRNA *f)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder0<R> params;
    ParameterList param_list;
    param_list.data = &params;

    f->call(blender::g_context, nullptr, &ptr, &param_list);
    return params.get();
}
template<typename T, typename R, typename A1>
R call(T *self, FunctionRNA *f, const A1& a1)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder1<R, A1> params = { a1 };
    ParameterList param_list;
    param_list.data = &params;

    f->call(blender::g_context, nullptr, &ptr, &param_list);
    return params.get();
}
template<typename T, typename R, typename A1, typename A2>
R call(T *self, FunctionRNA *f, const A1& a1, const A2& a2)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder2<R, A1, A2> params = { a1, a2 };
    ParameterList param_list;
    param_list.data = &params;

    f->call(blender::g_context, nullptr, &ptr, &param_list);
    return params.get();
}
template<typename T, typename R, typename A1, typename A2, typename A3>
R call(T *self, FunctionRNA *f, const A1& a1, const A2& a2, const A3& a3)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder3<R, A1, A2, A3> params = { a1, a2, a3 };
    ParameterList param_list;
    param_list.data = &params;

    f->call(blender::g_context, nullptr, &ptr, &param_list);
    return params.get();
}
template<typename T, typename R, typename A1, typename A2, typename A3, typename A4>
R call(T *self, FunctionRNA *f, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder4<R, A1, A2, A3, A4> params = { a1, a2, a3, a4 };
    ParameterList param_list;
    param_list.data = &params;

    f->call(blender::g_context, nullptr, &ptr, &param_list);
    return params.get();
}
template<typename T, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
R call(T *self, FunctionRNA *f, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder5<R, A1, A2, A3, A4, A5> params = { a1, a2, a3, a4, a5 };
    ParameterList param_list;
    param_list.data = &params;

    f->call(blender::g_context, nullptr, &ptr, &param_list);
    return params.get();
}

} //end namespace blender 
