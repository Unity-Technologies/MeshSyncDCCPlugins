#pragma once

#include "msblenMacros.h"
#include "MeshUtils/muMath.h" //mu::float3
#include "BlenderPyObjects/BlenderPyID.h" //BlenderPyID

struct Depsgraph;

namespace blender
{
    bool ready();
    void setup(py::object bpy_context);
    const void* CustomData_get(const CustomData& data, int type);
    int CustomData_get_offset(const CustomData& data, int type);
    mu::float3 BM_loop_calc_face_normal(const BMLoop& l);
    std::string abspath(const std::string& path);
    
    struct ListHeader { ListHeader *next, *prev; };

	template<typename T> inline T rna_sdata(py::object p) { 
		return reinterpret_cast<T>( reinterpret_cast<BPy_StructRNA*>(p.ptr())->ptr.data );
	}
	template<typename T> inline void rna_sdata(py::object p, T& v) { 
		v = reinterpret_cast<T>( reinterpret_cast<BPy_StructRNA*>(p.ptr())->ptr.data );
	}
	
	template<typename T>
    struct blist_iterator
    {
        T *m_ptr;

        blist_iterator(T *p) : m_ptr(p) {}
        T* operator*() const { return m_ptr; }
        T* operator++() { m_ptr = (T*)(((ListHeader*)m_ptr)->next); return m_ptr; }
        bool operator==(const blist_iterator& v) const { return m_ptr == v.m_ptr; }
        bool operator!=(const blist_iterator& v) const { return m_ptr != v.m_ptr; }
    };

    template<typename T>
    struct blist_range
    {
        using iterator = blist_iterator<T>;
        using const_iterator = blist_iterator<const T>;
        T *ptr;

        blist_range(T *p) : ptr(p) {}
        iterator begin() { return iterator(ptr); }
        iterator end() { return iterator(nullptr); }
        const_iterator begin() const { return const_iterator(ptr); }
        const_iterator end() const { return const_iterator(nullptr); }
    };
    template<typename T> blist_range<T> list_range(T *t) { return blist_range<T>(t); }

    template<typename T>
    struct barray_range
    {
        using iterator = T * ;
        using const_iterator = const T * ;
        using reference = T & ;
        using const_reference = const T & ;

        T *m_ptr;
        size_t m_size;

        barray_range(T *p, size_t s) : m_ptr(p), m_size(s) {}
        iterator begin() { return m_ptr; }
        iterator end() { return m_ptr + m_size; }
        const_iterator begin() const { return m_ptr; }
        const_iterator end() const { return m_ptr + m_size; }
        reference operator[](size_t i) { return m_ptr[i]; }
        const_reference operator[](size_t i) const { return m_ptr[i]; }
        size_t size() const { return m_size; }
        bool empty() const { return !m_ptr || m_size == 0; }
    };
    template<typename T> barray_range<T> array_range(T *t, size_t s) { return barray_range<T>(t, s); }


    class BObject
    {
    public:
        MSBLEN_BOILERPLATE(Object)
        MSBLEN_COMPATIBLE(BlenderPyID)

        blist_range<ModifierData> modifiers();
        blist_range<bDeformGroup> deform_groups();

        const char *name() const;
        void* data();
        mu::float4x4 matrix_local() const;
        mu::float4x4 matrix_world() const;
        bool is_selected() const;
        bool hide_viewport() const;
        bool hide_render() const;
        Mesh* to_mesh() const;
        void to_mesh_clear();
        void modifiers_clear();
    };

//----------------------------------------------------------------------------------------------------------------------

    class BMesh {
    public:
        MSBLEN_BOILERPLATE(Mesh)
        MSBLEN_COMPATIBLE(BlenderPyID)

        barray_range<MLoop> indices();
        barray_range<MEdge> edges();
        barray_range<MPoly> polygons();
        barray_range<MVert> vertices();
        barray_range<mu::float3> normals();
        barray_range<MLoopUV> uv();
        barray_range<MLoopCol> colors();
        MLoopUV* GetUV(const int index) const;
        inline uint32_t GetNumUVs() const;

        void calc_normals_split();
        void update();
        void clear_geometry();
        void addVertices(int count);
        void addPolygons(int count);
        void addLoops(int count);
        void addEdges(int count);
        void addNormals(int count);
    };

    uint32_t BMesh::GetNumUVs() const { return CustomData_number_of_layers(&m_ptr->ldata, CD_MLOOPUV); }


//----------------------------------------------------------------------------------------------------------------------

    using BMTriangle = BMLoop*[3];
    class BEditMesh
    {
    public:
        MSBLEN_BOILERPLATE2(BEditMesh, BMEditMesh)

        barray_range<BMFace*> polygons();
        barray_range<BMVert*> vertices();
        barray_range<BMTriangle> triangles();
        int uv_data_offset() const;

        MLoopUV* GetUV(const int index) const;
    };

    //----------------------------------------------------------------------------------------------------------------------
    
    class BNurb {
    public:
        MSBLEN_BOILERPLATE(Nurb)
        MSBLEN_COMPATIBLE(BlenderPyID)

        barray_range<MLoop> indices();
        barray_range<MEdge> edges();
        barray_range<MPoly> polygons();
        barray_range<MVert> bezier_points();

        void add_bezier_points(int count, Object* obj);
    };    

    class BCurve {
    public:
        MSBLEN_BOILERPLATE(Curve)
        MSBLEN_COMPATIBLE(BlenderPyID)

        void clearSplines();
        Nurb* newSpline();
    };

    //----------------------------------------------------------------------------------------------------------------------

    class BMaterial
    {
    public:
        MSBLEN_BOILERPLATE(Material)
        MSBLEN_COMPATIBLE(BlenderPyID)

        const char *name() const;
        const mu::float3& color() const;
        bool use_nodes() const;
        Material* active_node_material() const;
    };

    class BCamera
    {
    public:
        MSBLEN_BOILERPLATE(Camera)
        MSBLEN_COMPATIBLE(BlenderPyID)

        float clip_start() const;
        float clip_end() const;
        float angle_x() const;
        float angle_y() const;
        float lens() const;
        /*
            CAMERA_SENSOR_FIT_AUTO,
            CAMERA_SENSOR_FIT_HOR,
            CAMERA_SENSOR_FIT_VERT,
        */
        int sensor_fit() const;
        float sensor_width() const;  // in mm
        float sensor_height() const; // in mm
        float shift_x() const; // in percent
        float shift_y() const; // in percent
    };

    class BData
    {
    public:
        MSBLEN_BOILERPLATE2(BData, Main)

        blist_range<Object>     objects();
        blist_range<Mesh>       meshes();
        blist_range<Material>   materials();
        blist_range<Collection> collections();

        bool objects_is_updated();
        void remove(Mesh *v);
    };

} // namespace blender
