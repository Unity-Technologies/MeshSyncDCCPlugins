#pragma once

#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #define NOMINMAX
    #include <windows.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <iostream>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <thread>
#include <future>

#include "pybind11/pybind11.h"
#include "pybind11/operators.h"
#include "pybind11/eval.h"
#include "pybind11/stl.h"
namespace py = pybind11;

#ifndef NDEBUG
    #define NDEBUG
#endif
#pragma warning( push )
#pragma warning( disable : 4200 ) // zero length array
#include "BKE_blender_version.h"
#include "BKE_main.h"
#include "BKE_customdata.h"
#include "BKE_context.h"
#include "BKE_fcurve.h"
#include "BKE_editmesh.h"
#include "BKE_material.h"
#include "RNA_define.h"
#include "RNA_types.h"

#include "DNA_anim_types.h"
#include "DNA_armature_types.h"
#include "DNA_camera_types.h"
#include "DNA_collection_types.h"
#include "DNA_gpencil_types.h" //bGPdata
#if BLENDER_VERSION < 302
#include "DNA_hair_types.h" //Hair
#endif
#include "DNA_key_types.h"
#include "DNA_light_types.h"
#include "DNA_material_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_modifier_types.h"
#include "DNA_object_types.h"
#include "DNA_pointcloud_types.h" //PointCloud
#include "DNA_scene_types.h"
#include "DNA_volume_types.h" //Volume

#include "BLI_utildefines.h"
#include "BLI_math_base.h"
#include "BLI_math_vector.h"
#include "bmesh_class.h"
#include "intern/rna_internal_types.h"
#include "intern/bpy_rna.h"
#include "intern/bmesh_structure.h"
#pragma warning( pop ) 
