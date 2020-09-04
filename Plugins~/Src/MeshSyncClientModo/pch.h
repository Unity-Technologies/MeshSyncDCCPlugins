#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <codecvt>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <regex>
#include <future>
#include <mutex>
#include <memory>
#include <cassert>

#ifdef _WIN32
    #pragma warning(push, 0)
#endif
#include <lxversion.h>
#include <lxvector.h>
#include <lxidef.h>
#include <lx_value.hpp>
#include <lx_plugin.hpp>
#include <lx_item.hpp>
#include <lx_locator.hpp>
#include <lx_mesh.hpp>
#include <lx_deform.hpp>
#include <lx_layer.hpp>
#include <lx_package.hpp>
#include <lx_particle.hpp>
#include <lx_image.hpp>
#include <lx_log.hpp>
#include <lx_listener.hpp>
#include <lx_scripts.hpp>
#include <lxu_command.hpp>
#include <lxu_package.hpp>
#include <lxu_prefvalue.hpp>
#include <lxu_log.hpp>
#include <lxw_anim.hpp>
#include <lxw_customview.hpp>
#ifdef _WIN32
    #pragma warning(pop)
#define NOMAXMIN
#define NOMINMAX
#include <windows.h>
#endif

//[TODO-sin:2020-9-4] Temporary hack
#include "MeshUtils/muRawVector.h" //SharedVector
#include "MeshUtils/muMath.h" //mu::float4x4
#include "MeshUtils/muSIMD.h" //SumInt32
#include "MeshSync/msMisc.h" //nanosec

#include "MeshSync/MeshSync.h"
#ifdef GetObject
    #undef GetObject
#endif
