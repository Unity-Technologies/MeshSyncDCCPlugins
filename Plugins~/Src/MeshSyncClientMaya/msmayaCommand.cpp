#include "pch.h"
#include "msmayaContext.h"
#include "msmayaCommand.h"

#include "MeshSyncClient/ExportTarget.h"
#include "MeshSyncClient/MaterialFrameRange.h"

template<class T> static bool get_arg(T& dst, const char *name, MArgParser& args);

template<> bool get_arg(std::string& dst, const char *name, MArgParser& args)
{
    MString tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
        dst = tmp.asChar();
    return stat == MStatus::kSuccess;
}
template<> bool get_arg(bool& dst, const char *name, MArgParser& args)
{
    bool tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
        dst = tmp;
    return stat == MStatus::kSuccess;
}
template<> bool get_arg(int& dst, const char *name, MArgParser& args)
{
    int tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
        dst = tmp;
    return stat == MStatus::kSuccess;
}
template<> bool get_arg(uint16_t& dst, const char *name, MArgParser& args)
{
    int tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
        dst = (uint16_t)tmp;
    return stat == MStatus::kSuccess;
}
template<> bool get_arg(float& dst, const char *name, MArgParser& args)
{
    double tmp;
    auto stat = args.getFlagArgument(name, 0, tmp);
    if (stat == MStatus::kSuccess)
        dst = (float)tmp;
    return stat == MStatus::kSuccess;
}


template<class T> static void to_MString(MString& dst, const T& value);

template<> void to_MString(MString& dst, const std::string& value)
{
    dst += value.c_str();
}
template<> void to_MString(MString& dst, const bool& value)
{
    dst += (int)value;
}
template<> void to_MString(MString& dst, const int& value)
{
    dst += value;
}
template<> void to_MString(MString& dst, const uint16_t& value)
{
    dst += (int)value;
}
template<> void to_MString(MString& dst, const float& value)
{
    dst += value;
}

template<class T> static void set_result(const T& value);

template<> void set_result(const std::string& value)
{
    MPxCommand::setResult(value.c_str());
}
template<> void set_result(const bool& value)
{
    MPxCommand::setResult(value);
}
template<> void set_result(const int& value)
{
    MPxCommand::setResult(value);
}
template<> void set_result(const uint16_t& value)
{
    MPxCommand::setResult(value);
}
template<> void set_result(const float& value)
{
    MPxCommand::setResult(value);
}


void* CmdServerStatus::create()
{
    return new CmdServerStatus();
}

const char* CmdServerStatus::name()
{
    return "UnityMeshSync_ServerStatus";
}

MSyntax CmdServerStatus::createSyntax()
{
    MSyntax syntax;
    syntax.enableQuery(true);
    syntax.enableEdit(false);
    syntax.addFlag("-ia", "-isAvailable", MSyntax::kBoolean);
    syntax.addFlag("-em", "-errorMessage", MSyntax::kString);
    return syntax;
}

MStatus CmdServerStatus::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    if (!args.isQuery())
        return MStatus::kFailure;

    auto& ctx = msmayaGetContext();
    if (args.isFlagSet("isAvailable"))
        set_result(ctx.isServerAvailable());
    else if (args.isFlagSet("errorMessage"))
        set_result(std::string("MeshSync: ") + ctx.getErrorMessage() + "\n");
    return MStatus::kSuccess;
}


void* CmdSettings::create()
{
    return new CmdSettings();
}

const char* CmdSettings::name()
{
    return "UnityMeshSync_Settings";
}


MSyntax CmdSettings::createSyntax()
{
    MSyntax syntax;
    syntax.enableQuery(true);
    syntax.enableEdit(false);
    syntax.addFlag("-plv", "-pluginVersion", MSyntax::kString);
    syntax.addFlag("-prv", "-protocolVersion", MSyntax::kString);
    syntax.addFlag("-a", "-address", MSyntax::kString);
    syntax.addFlag("-p", "-port", MSyntax::kLong);
    syntax.addFlag("-sf", "-scaleFactor", MSyntax::kDouble);
    syntax.addFlag("-as", "-autosync", MSyntax::kBoolean);
    syntax.addFlag("-sm", "-syncMeshes", MSyntax::kBoolean);
    syntax.addFlag("-smn", "-syncNormals", MSyntax::kBoolean);
    syntax.addFlag("-smu", "-syncUVs", MSyntax::kBoolean);
    syntax.addFlag("-smc", "-syncColors", MSyntax::kBoolean);
    syntax.addFlag("-ds", "-makeDoubleSided", MSyntax::kBoolean);
    syntax.addFlag("-bd", "-bakeDeformers", MSyntax::kBoolean);
    syntax.addFlag("-bt", "-bakeTransform", MSyntax::kBoolean);
    syntax.addFlag("-tw", "-applyTweak", MSyntax::kBoolean);
    syntax.addFlag("-sms", "-syncBlendShapes", MSyntax::kBoolean);
    syntax.addFlag("-smb", "-syncBones", MSyntax::kBoolean);
    syntax.addFlag("-stx", "-syncTextures", MSyntax::kBoolean);
    syntax.addFlag("-sc", "-syncCameras", MSyntax::kBoolean);
    syntax.addFlag("-sl", "-syncLights", MSyntax::kBoolean);
    syntax.addFlag("-sco", "-syncConstraints", MSyntax::kBoolean);
    syntax.addFlag("-fs", "-frameStep", MSyntax::kDouble);
    syntax.addFlag("-kfr", "-keyframeReduction", MSyntax::kBoolean);
    syntax.addFlag("-kfc", "-keepFlatCurves", MSyntax::kBoolean);
    syntax.addFlag("-rn", "-removeNamespace", MSyntax::kBoolean);
    syntax.addFlag("-mt", "-multithreaded", MSyntax::kBoolean);
    syntax.addFlag("-fct", "-fbxCompatibleTransform", MSyntax::kBoolean);
    return syntax;
}

MStatus CmdSettings::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    auto& settings = msmayaGetSettings();

    if (args.isFlagSet("pluginVersion")) {
        if (args.isQuery())
            set_result(std::string(MESHSYNC_DCC_PLUGIN_VER));
    }
    else if (args.isFlagSet("protocolVersion")) {
        if (args.isQuery())
            set_result(std::to_string(msProtocolVersion));
    }

#define Handle(Name, Value, Sync)\
    if (args.isFlagSet(Name)) {\
        if(args.isQuery()) {\
            set_result(settings.Value);\
        }\
        else {\
            get_arg(settings.Value, Name, args);\
            if (settings.auto_sync && Sync)\
                msmayaGetContext().sendObjects(MeshSyncClient::ObjectScope::All, false);\
        }\
    }

    Handle("address", client_settings.server, false);
    Handle("port", client_settings.port, false);
    Handle("scaleFactor", scale_factor, true);
    Handle("autosync", auto_sync, true);
    Handle("syncMeshes", sync_meshes, true);
    Handle("syncNormals", sync_normals, true);
    Handle("syncUVs", sync_uvs, true);
    Handle("syncColors", sync_colors, true);
    Handle("makeDoubleSided", make_double_sided, true);
    Handle("bakeDeformers", BakeModifiers, true);
    Handle("bakeTransform", BakeTransform, true);
    Handle("applyTweak", apply_tweak, true);
    Handle("syncBlendShapes", sync_blendshapes, true);
    Handle("syncBones", sync_bones, true);
    Handle("syncTextures", sync_textures, true);
    Handle("syncCameras", sync_cameras, true);
    Handle("syncLights", sync_lights, true);
    Handle("syncConstraints", sync_constraints, true);
    Handle("frameStep", frame_step, false);
    Handle("removeNamespace", remove_namespace, true);
    Handle("multithreaded", multithreaded, false);
    Handle("fbxCompatibleTransform", fbx_compatible_transform, true);
#undef Handle

    return MStatus::kSuccess;
}


void* CmdSend::create()
{
    return new CmdSend();
}
const char* CmdSend::name()
{
    return "UnityMeshSync_Export";
}

MSyntax CmdSend::createSyntax()
{
    MSyntax syntax;
    syntax.enableQuery(false);
    syntax.enableEdit(false);
    syntax.addFlag("-s", "-scope", MSyntax::kString);
    syntax.addFlag("-t", "-target", MSyntax::kString);
    return syntax;
}

MStatus CmdSend::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);

    MeshSyncClient::ExportTarget target = MeshSyncClient::ExportTarget::Objects;
    MeshSyncClient::ObjectScope scope = MeshSyncClient::ObjectScope::All;

    // parse args
    if (args.isFlagSet("target")) {
        std::string t;
        get_arg(t, "target", args);
        if (t == "objects")
            target = MeshSyncClient::ExportTarget::Objects;
        else if (t == "materials")
            target = MeshSyncClient::ExportTarget::Materials;
        else if (t == "animations")
            target = MeshSyncClient::ExportTarget::Animations;
        else if (t == "everything")
            target = MeshSyncClient::ExportTarget::Everything;
    }
    if (args.isFlagSet("scope")) {
        std::string s;
        get_arg(s, "scope", args);
        if (s == "all")
            scope = MeshSyncClient::ObjectScope::All;
        else if (s == "selection")
            scope = MeshSyncClient::ObjectScope::Selected;
        else if (s == "updated")
            scope = MeshSyncClient::ObjectScope::Updated;
    }

    // do send
    auto& inst = msmayaGetContext();
    if (!inst.isServerAvailable()) {
        inst.logError("MeshSync: Server not available. %s", inst.getErrorMessage().c_str());
        return MStatus::kFailure;
    }

    if (target == MeshSyncClient::ExportTarget::Objects) {
        inst.wait();
        inst.sendObjects(scope, true);
    }
    else if (target == MeshSyncClient::ExportTarget::Materials) {
        inst.wait();
        inst.sendMaterials(true);
    }
    else if (target == MeshSyncClient::ExportTarget::Animations) {
        inst.wait();
        inst.sendAnimations(scope);
    }
    else if (target == MeshSyncClient::ExportTarget::Everything) {
        inst.wait();
        inst.sendMaterials(true);
        inst.wait();
        inst.sendObjects(scope, true);
        inst.wait();
        inst.sendAnimations(scope);
    }
    return MStatus::kSuccess;
}



void* CmdImport::create()
{
    return new CmdImport();
}

const char* CmdImport::name()
{
    return "UnityMeshSync_Import";
}

MSyntax CmdImport::createSyntax()
{
    MSyntax syntax;
    return syntax;
}

MStatus CmdImport::doIt(const MArgList&)
{
    msmayaGetContext().recvObjects();
    return MStatus::kSuccess;
}



void* CmdExportCache::create()
{
    return new CmdExportCache();
}

const char* CmdExportCache::name()
{
    return "UnityMeshSync_Cache";
}

MSyntax CmdExportCache::createSyntax()
{
    MSyntax syntax;
    syntax.enableQuery(true);
    syntax.enableEdit(false);
    syntax.addFlag("-p", "-path", MSyntax::kString);
    syntax.addFlag("-os", "-objectScope", MSyntax::kLong);
    syntax.addFlag("-fr", "-frameRange", MSyntax::kLong);
    syntax.addFlag("-frb", "-frameBegin", MSyntax::kLong);
    syntax.addFlag("-fre", "-frameEnd", MSyntax::kLong);
    syntax.addFlag("-mfr", "-materialFrameRange", MSyntax::kLong);
    syntax.addFlag("-fs", "-frameStep", MSyntax::kDouble);

    syntax.addFlag("-z", "-zstdCompressionLevel", MSyntax::kLong);
    syntax.addFlag("-rn", "-removeNamespace", MSyntax::kBoolean);
    syntax.addFlag("-ds", "-makeDoubleSided", MSyntax::kBoolean);
    syntax.addFlag("-bd", "-bakeDeformers", MSyntax::kBoolean);
    syntax.addFlag("-bt", "-bakeTransform", MSyntax::kBoolean);
    syntax.addFlag("-fh", "-flattenHierarchy", MSyntax::kBoolean);
    syntax.addFlag("-mm", "-mergeMeshes", MSyntax::kBoolean);
    syntax.addFlag("-sn", "-stripNormals", MSyntax::kBoolean);
    syntax.addFlag("-st", "-stripTangents", MSyntax::kBoolean);
    return syntax;
}

//----------------------------------------------------------------------------------------------------------------------

template<class T> void GetOrSetArg(MArgParser& args, const char* argName, T& argValue) {
    if (!args.isFlagSet(argName))
        return;

    if (args.isQuery())
        //puts the given value into the return value area for a command.
        set_result(argValue);
    else
        get_arg(argValue, argName, args);
}


//----------------------------------------------------------------------------------------------------------------------

MStatus CmdExportCache::doIt(const MArgList& args_)
{
    MStatus status;
    MArgParser args(syntax(), args_, &status);
    MayaCacheSettings& settings = msmayaGetContext().getCacheSettings();

    std::string outputPath;
    GetOrSetArg(args, "path", outputPath);

#define Handle(Name, Value, ...)\
    if (args.isFlagSet(Name)) {\
            if (args.isQuery())\
                    set_result(__VA_ARGS__ settings.Value);\
            else\
                    get_arg(__VA_ARGS__ settings.Value, Name, args);\
    }

    Handle("objectScope", object_scope, (int&));
    Handle("frameRange", frame_range, (int&));
    Handle("frameBegin", frame_begin);
    Handle("frameEnd", frame_end);
    Handle("materialFrameRange", material_frame_range, (int&));
    Handle("frameStep", frame_step);

    Handle("zstdCompressionLevel", zstd_compression_level);
    Handle("removeNamespace", remove_namespace);
    Handle("makeDoubleSided", make_double_sided);
    Handle("bakeDeformers", bake_modifiers);
    Handle("bakeTransform", bake_transform);
    Handle("flattenHierarchy", flatten_hierarchy);
    Handle("mergeMeshes", merge_meshes);
    Handle("stripNormals", strip_normals);
    Handle("stripTangents", strip_tangents);

#undef Handle

    if (!args.isQuery()) {
        msmayaContext& ctx = msmayaContext::getInstance();
        if (ctx.ExportCache(outputPath, settings))
            return MStatus::kSuccess;
        else
            return MStatus::kFailure;
    }
    return MStatus::kSuccess;
}
