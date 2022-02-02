#include "pch.h"
#include "msmayaContext.h"
#include "msmayaUtils.h"
#include "msmayaCommand.h"
#include <cstdarg>

#include "MeshSync/SceneCache/msSceneCacheSettings.h"
#include "MeshSync/SceneGraph/msAnimation.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/Utility/msMaterialExt.h" //AsStandardMaterial
#include "MeshSyncClient/SettingsUtility.h"
#include "MeshSyncClient/SceneCacheUtility.h"


bool DAGNode::isInstanced() const
{
    return branches.size() > 1;
}

ms::Identifier TreeNode::getIdentifier() const
{
    return { path, id };
}

void TreeNode::clearState()
{
    dst_obj = nullptr;
    dst_anim = nullptr;
}

bool TreeNode::isInstanced() const
{
    return shape->branches.size() > 1;
}

bool TreeNode::isPrimaryInstance() const
{
    return getPrimaryInstanceNode() == this;
}

TreeNode* TreeNode::getPrimaryInstanceNode() const
{
    if (!shape->branches.empty())
        return shape->branches.front();
    return nullptr;
}

MDagPath TreeNode::getDagPath(bool include_shape) const
{
    MDagPath ret;
    getDagPath_(ret);
    if (include_shape && shape)
        ret.push(shape->node);
    return ret;
}
void TreeNode::getDagPath_(MDagPath & dst) const
{
    if (parent) {
        parent->getDagPath_(dst);
        dst.push(trans->node);
    }
    else {
        MDagPath::getAPathTo(trans->node, dst);
    }
}

MObject TreeNode::getTrans() const { return trans ? trans->node : MObject(); }
MObject TreeNode::getShape() const { return shape ? shape->node : MObject(); }

bool TreeNode::isVisibleInHierarchy() const
{
    if (parent && !parent->isVisibleInHierarchy())
        return false;
    return IsVisible(trans->node);
}

static MDagPath GetDagPath(const TreeNode *branch, const MObject& node)
{
    MDagPath ret;
    branch->getDagPath_(ret);
    ret.push(node);
    return ret;
}

static TreeNode* FindBranch(const DAGNodeMap& dnmap, const MDagPath& dagpath)
{
    auto node = dagpath.node();
    auto it = dnmap.find(node);
    if (it != dnmap.end()) {
        const auto& dn = it->second;
        if (dn.branches.size() == 1) {
            return dn.branches.front();
        }
        else {
            for (auto *branch : dn.branches) {
                if (branch->getDagPath() == dagpath) {
                    return branch;
                }
            }
        }
    }

    return nullptr;
}

static void OnIdle(float elapsedTime, float lastTime, void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->update();
}

static void OnSceneLoadBegin(void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->onSceneLoadBegin();
}

static void OnSceneLoadEnd(void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->onSceneLoadEnd();
}

static void OnNodeRenamed(void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->onNodeRenamed();
}

static void OnDagChange(MDagMessage::DagMessage msg, MDagPath &child, MDagPath &parent, void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->onSceneUpdated();
}

static void OnTimeChange(MTime& time, void* _this)
{
    reinterpret_cast<msmayaContext*>(_this)->onTimeChange(time);
}

static void OnNodeRemoved(MObject& node, void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->onNodeRemoved(node);
}


static void OnTransformUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    if (msg == MNodeMessage::kAttributeEval) { return; }
    reinterpret_cast<msmayaContext*>(_this)->onNodeUpdated(plug.node());
}

static void OnCameraUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->onNodeUpdated(plug.node());
}

static void OnLightUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->onNodeUpdated(plug.node());
}

static void OnMeshUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    reinterpret_cast<msmayaContext*>(_this)->onNodeUpdated(plug.node());
}


void msmayaContext::onNodeUpdated(const MObject& node)
{
    m_dag_nodes[node].dirty = true;
}

void msmayaContext::onNodeRemoved(const MObject& node)
{
    {
        auto it = m_dag_nodes.find(node);
        if (it != m_dag_nodes.end()) {
            for (auto tn : it->second.branches)
                m_entity_manager.erase(tn->getIdentifier());
            m_dag_nodes.erase(it);
        }
    }
}

void msmayaContext::onNodeRenamed()
{
    m_scene_updated = true;
}

void msmayaContext::onSceneUpdated()
{
    m_scene_updated = true;
}

void msmayaContext::onSceneLoadBegin()
{
    m_ignore_update = true;
}

void msmayaContext::onSceneLoadEnd()
{
    m_ignore_update = false;
}

void msmayaContext::onTimeChange(const MTime & time)
{
    if (m_settings.auto_sync) {
        m_pending_scope = MeshSyncClient::ObjectScope::All;
        // timer callback won't be fired while scrubbing time slider. so call update() immediately
        update();

        // for timer callback
        m_pending_scope = MeshSyncClient::ObjectScope::All;
    }
}

void msmayaContext::logInfo(const char * format, ...)
{
    const int MaxBuf = 2048;
    char buf[MaxBuf];

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    MGlobal::displayInfo(buf);
    va_end(args);
}

void msmayaContext::logError(const char * format, ...)
{
    const int MaxBuf = 2048;
    char buf[MaxBuf];

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    MGlobal::displayError(buf);
    va_end(args);
}

bool msmayaContext::isServerAvailable()
{
    m_sender.client_settings = m_settings.client_settings;
    return m_sender.isServerAvaileble();
}

const std::string& msmayaContext::getErrorMessage()
{
    return m_sender.getErrorMessage();
}


void msmayaContext::TaskRecord::add(TreeNode *n, const task_t & task)
{
    tasks.push_back({ n, task });
}

void msmayaContext::TaskRecord::process()
{
    if (tasks.size() > 1) {
        auto primary = std::get<0>(tasks[0])->getPrimaryInstanceNode();

        // process primary node first
        for (auto& t : tasks) {
            if (std::get<0>(t) == primary) {
                std::get<1>(t)();
                break;
            }
        }
        // process others
        for (auto& t : tasks) {
            if (std::get<0>(t) != primary)
                std::get<1>(t)();
        }
    }
    else {
        for (auto& t : tasks)
            std::get<1>(t)();
    }

    tasks.clear();
}


static std::unique_ptr<msmayaContext> g_plugin;

void msmayaInitialize(MObject& obj)
{
    g_plugin.reset(new msmayaContext(obj));
}

void msmayaUninitialize()
{
    g_plugin.reset();
}

msmayaContext& msmayaContext::getInstance()
{
    return *g_plugin;
}

MayaSyncSettings& msmayaContext::getSettings()
{
    return m_settings;
}

MayaCacheSettings& msmayaContext::getCacheSettings()
{
    return m_cache_settings;
}

msmayaContext::msmayaContext(MObject obj)
    : m_obj(obj)
    , m_iplugin(obj, msVendor, MESHSYNC_DCC_PLUGIN_VER)
{
#define Body(CmdType) m_iplugin.registerCommand(CmdType::name(), CmdType::create, CmdType::createSyntax);
    EachCommand(Body)
#undef Body

        registerGlobalCallbacks();
}

msmayaContext::~msmayaContext()
{
    wait();
    removeNodeCallbacks();
    removeGlobalCallbacks();

#define Body(CmdType) m_iplugin.deregisterCommand(CmdType::name());
    EachCommand(Body)
#undef Body
}

void msmayaContext::registerGlobalCallbacks()
{
    MStatus stat;
    m_cids_global.push_back(MTimerMessage::addTimerCallback(0.03f, OnIdle, this, &stat));
    m_cids_global.push_back(MSceneMessage::addCallback(MSceneMessage::kBeforeNew, OnSceneLoadBegin, this, &stat));
    m_cids_global.push_back(MSceneMessage::addCallback(MSceneMessage::kBeforeOpen, OnSceneLoadBegin, this, &stat));
    m_cids_global.push_back(MSceneMessage::addCallback(MSceneMessage::kAfterNew, OnSceneLoadEnd, this, &stat));
    m_cids_global.push_back(MSceneMessage::addCallback(MSceneMessage::kAfterOpen, OnSceneLoadEnd, this, &stat));
    m_cids_global.push_back(MDagMessage::addAllDagChangesCallback(OnDagChange, this, &stat));
    m_cids_global.push_back(MEventMessage::addEventCallback("NameChanged", OnNodeRenamed, this, &stat));
    m_cids_global.push_back(MDGMessage::addForceUpdateCallback(OnTimeChange, this));
    m_cids_global.push_back(MDGMessage::addNodeRemovedCallback(OnNodeRemoved, kDefaultNodeType, this));

    // shut up warning about blendshape
    MGlobal::executeCommand("cycleCheck -e off");
}

void msmayaContext::registerNodeCallbacks()
{
    // joints
    EnumerateNode(MFn::kJoint, [this](MObject& node) {
        auto& rec = m_dag_nodes[node];
        if (!rec.cid)
            rec.cid = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
    });

    auto register_parent_transforms = [this](MObject& n) {
        EachParentRecursive(n, [this](MObject parent) {
            if (parent.hasFn(MFn::kTransform)) {
                auto& rec = m_dag_nodes[parent];
                if (!rec.cid)
                    rec.cid = MNodeMessage::addAttributeChangedCallback(parent, OnTransformUpdated, this);
            }
        });
    };

    // cameras
    EnumerateNode(MFn::kCamera, [&](MObject& node) {
        Pad<MFnDagNode> fn(node);
        if (!fn.isIntermediateObject()) {
            register_parent_transforms(node);
            auto& rec = m_dag_nodes[node];
            if (!rec.cid)
                rec.cid = MNodeMessage::addAttributeChangedCallback(node, OnCameraUpdated, this);
        }
    });

    // lights
    EnumerateNode(MFn::kLight, [&](MObject& node) {
        Pad<MFnDagNode> fn(node);
        if (!fn.isIntermediateObject()) {
            register_parent_transforms(node);
            auto& rec = m_dag_nodes[node];
            if (!rec.cid)
                rec.cid = MNodeMessage::addAttributeChangedCallback(node, OnLightUpdated, this);
        }
    });

    //  meshes
    EnumerateNode(MFn::kMesh, [&](MObject& node) {
        Pad<MFnDagNode> fn(node);
        if (!fn.isIntermediateObject()) {
            register_parent_transforms(node);
            auto& rec = m_dag_nodes[node];
            if (!rec.cid)
                m_dag_nodes[node].cid = MNodeMessage::addAttributeChangedCallback(node, OnMeshUpdated, this);
        }
    });
}

void msmayaContext::removeGlobalCallbacks()
{
    for (auto& cid : m_cids_global) {
        MMessage::removeCallback(cid);
    }
    m_cids_global.clear();
}

void msmayaContext::removeNodeCallbacks()
{
    for (auto& rec : m_dag_nodes) {
        if (rec.second.cid) {
            MMessage::removeCallback(rec.second.cid);
            rec.second.cid = 0;
        }
    }
}

std::vector<TreeNode*> msmayaContext::getNodes(MeshSyncClient::ObjectScope scope, bool include_children)
{
    std::vector<TreeNode*> ret;
    if (scope == MeshSyncClient::ObjectScope::All) {
        size_t n = m_tree_nodes.size();
        ret.resize(n);
        for (size_t i = 0; i < n; ++i)
            ret[i] = m_tree_nodes[i].get();
    }
    else if (scope == MeshSyncClient::ObjectScope::Selected) {
        MSelectionList selected;
        MGlobal::getActiveSelectionList(selected);
        uint32_t n = selected.length();
        if (include_children) {
            std::set<TreeNode*> history;
            // note: this must be std::function because auto lambda can not recurse
            std::function<void (TreeNode *n)> gather_children = [&](TreeNode *n) {
                if (!history.insert(n).second)
                    return;
                ret.push_back(n);
                for (auto c : n->children)
                    gather_children(c);
            };

            for (uint32_t i = 0; i < n; ++i) {
                MObject obj;
                selected.getDependNode(i, obj);

                auto it = m_dag_nodes.find(obj);
                if (it != m_dag_nodes.end()) {
                    for (auto n : it->second.branches)
                        gather_children(n);
                }
            }
        }
        else {
            for (uint32_t i = 0; i < n; ++i) {
                MObject obj;
                selected.getDependNode(i, obj);
                auto it = m_dag_nodes.find(obj);
                if (it != m_dag_nodes.end())
                    ret.insert(ret.end(), it->second.branches.begin(), it->second.branches.end());
            }
        }
    }
    else if (scope == MeshSyncClient::ObjectScope::Updated) {
        for (auto& kvp : m_dag_nodes) {
            auto& rec = kvp.second;
            if (rec.dirty) {
                ret.insert(ret.end(), rec.branches.begin(), rec.branches.end());
            }
        }
    }
    return ret;
}

void msmayaContext::constructTree()
{
    struct ExistRecord
    {
        std::string path;
        bool exists;

        bool operator<(const ExistRecord& v) const { return path < v.path; }
        bool operator<(const std::string& v) const { return path < v; }
    };
    // create path list to detect rename / re-parent 
    std::vector<ExistRecord> old_records;
    old_records.reserve(m_tree_nodes.size());
    for (auto& n : m_tree_nodes)
        old_records.push_back({ std::move(n->path), false });
    std::sort(old_records.begin(), old_records.end());

    // clear old tree
    m_tree_roots.clear();
    m_tree_nodes.clear();
    for (auto& kvp : m_dag_nodes)
        kvp.second.branches.clear();
    m_index_seed = 0;

    // build new tree
    EnumerateNode(MFn::kTransform, [&](MObject& node) {
        if (Pad<MFnDagNode>(node).parent(0).hasFn(MFn::kWorld)) // root transforms
            constructTree(node, nullptr, "");
    });

    // erase renamed / re-parented objects
    for (auto& n : m_tree_nodes) {
        auto it = std::lower_bound(old_records.begin(), old_records.end(), n->path);
        if (it != old_records.end() && it->path == n->path)
            it->exists = true;
    }
    for (auto& r : old_records) {
        if (!r.exists)
            m_entity_manager.erase(r.path);
    }
}

void msmayaContext::constructTree(const MObject& node, TreeNode *parent, const std::string& base)
{
    MObject shape;
    {
        MDagPath dpath;
        MDagPath::getAPathTo(node, dpath);
        dpath.extendToShape();
        shape = dpath.node();
    }

    std::string name = GetName(node);
    std::string path = base;
    path += '/';
    path += name;

    auto& rec_node = m_dag_nodes[node];
    auto& rec_shape = m_dag_nodes[shape];
    if (rec_node.node.isNull()) {
        rec_node.node = node;
        rec_shape.node = shape;
    }

    auto *n = new TreeNode();
    m_tree_nodes.emplace_back(n);
    n->trans = &rec_node;
    n->shape = &rec_shape;
    n->name = name;
    n->path = path;
    n->index = ++m_index_seed;
    n->parent = parent;

    if (parent)
        parent->children.push_back(n);
    else
        m_tree_roots.push_back(n);

    rec_node.branches.push_back(n);
    if (shape != node)
        rec_shape.branches.push_back(n);

    EachChild(node, [&](const MObject& c) {
        if (c.hasFn(MFn::kTransform))
            constructTree(c, n, path);
    });
}


bool msmayaContext::sendMaterials(bool dirty_all)
{
    if (m_sender.isExporting()) {
        return false;
    }

    m_settings.Validate();
    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(dirty_all);
    RegisterSceneMaterials();

    // send
    WaitAndKickAsyncExport();
    return true;
}

bool msmayaContext::sendObjects(MeshSyncClient::ObjectScope scope, bool dirty_all)
{
    if (m_sender.isExporting()) {
        m_pending_scope = scope;
        return false;
    }
    m_pending_scope = MeshSyncClient::ObjectScope::None;

    if (scope != MeshSyncClient::ObjectScope::Updated)
        m_entity_manager.clearEntityRecords();

    m_settings.Validate();
    m_entity_manager.setAlwaysMarkDirty(dirty_all);
    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(false); // false because too heavy

    if (m_settings.sync_meshes)
        RegisterSceneMaterials();

    int num_exported = 0;
    bool handle_parents = scope != MeshSyncClient::ObjectScope::Updated;
    for (auto n : getNodes(scope)) {
        if (exportObject(n, handle_parents))
            ++num_exported;
        n->trans->dirty = false;
    }

    if (num_exported > 0 || !m_entity_manager.getDeleted().empty()) {
        WaitAndKickAsyncExport();
        return true;
    }
    else {
        return false;
    }
}

bool msmayaContext::sendAnimations(MeshSyncClient::ObjectScope scope)
{
    if (m_sender.isExporting())
        return false;

    m_settings.Validate();
    if (exportAnimations(scope) > 0)
        WaitAndKickAsyncExport();
    return true;
}

bool msmayaContext::ExportCache(const std::string& path, const MayaCacheSettings& cache_settings)
{
    using namespace MeshSyncClient;
    const float frameRate = (float)MTime(1.0, MTime::kSeconds).as(MTime::uiUnit());
    const float frameStep = std::max(cache_settings.frame_step, 0.1f);

    const MayaSyncSettings settings_old = m_settings;
    m_settings.remove_namespace = cache_settings.remove_namespace;
    SettingsUtility::ApplyCacheToSyncSettings(cache_settings, &m_settings);

    const float sampleRate = frameRate * std::max(1.0f / frameStep, 1.0f);
    const ms::OSceneCacheSettings oscs = SettingsUtility::CreateOSceneCacheSettings(sampleRate, cache_settings);

    const std::string destPath = SceneCacheUtility::BuildFilePath(path);
    if (!m_cache_writer.open(destPath.c_str(), oscs)) {
        logInfo("MeshSync: Can't write scene cache to %s", destPath.c_str());
        m_settings = settings_old;
        return false;
    }

    m_material_manager.setAlwaysMarkDirty(true);
    m_entity_manager.setAlwaysMarkDirty(true);

    const MaterialFrameRange material_range = cache_settings.material_frame_range;
    const std::vector<TreeNode*> nodes = getNodes(cache_settings.object_scope, true);

    if (cache_settings.frame_range == MeshSyncClient::FrameRange::Current) {
        m_anim_time = 0.0f;
        // RegisterSceneMaterials() is needed to export material IDs in meshes
        RegisterSceneMaterials();
        if ( MeshSyncClient::MaterialFrameRange::None == material_range)
            m_material_manager.clearDirtyFlags();
        DoExportSceneCache(nodes);
    } else {
        const MTime prevTime = MAnimControl::currentTime();

        const MTime interval = MTime(frameStep, MTime::uiUnit());
        MTime timeStart = MTime(cache_settings.frame_begin, MTime::uiUnit());
        MTime timeEnd = MTime(cache_settings.frame_end, MTime::uiUnit());
        if (MeshSyncClient::FrameRange::Custom != cache_settings.frame_range ) {
            timeStart = MAnimControl::minTime();
            timeEnd = MAnimControl::maxTime();
        } 
        timeEnd = std::max(timeEnd, timeStart); // sanitize

        // advance frame and record
        int sceneIndex = 0;
        m_ignore_update = true;
        for (MTime t = timeStart; t <= timeEnd; t += interval) {
            m_anim_time = static_cast<float>((t - timeStart).as(MTime::kSeconds));
            MGlobal::viewFrame(t);

            if (sceneIndex == 0) {
                // RegisterSceneMaterials() is needed to export material IDs in meshes
                RegisterSceneMaterials();
                if (MeshSyncClient::MaterialFrameRange::None == material_range)
                    m_material_manager.clearDirtyFlags();
            } else {
                if (MeshSyncClient::MaterialFrameRange::All == material_range)
                    RegisterSceneMaterials();
            }

            DoExportSceneCache(nodes);
            ++sceneIndex;
        }
        MGlobal::viewFrame(prevTime);
        m_ignore_update = false;
    }

    logInfo("MeshSync: Finished writing scene cache to %s", destPath.c_str());

    m_settings = settings_old;
    m_cache_writer.close();
    return true;
}

void msmayaContext::DoExportSceneCache(const std::vector<TreeNode*>& nodes)
{
    for (const std::vector<TreeNode*>::value_type& n : nodes)
        exportObject(n, true);

    m_texture_manager.clearDirtyFlags();
    WaitAndKickAsyncExport();
}

//----------------------------------------------------------------------------------------------------------------------

void msmayaContext::wait()
{
    m_sender.wait();
}

void msmayaContext::update()
{
    if (m_ignore_update)
        return;

    if (m_scene_updated) {
        m_scene_updated = false;

        constructTree();
        registerNodeCallbacks();
        if (m_settings.auto_sync) {
            m_pending_scope = MeshSyncClient::ObjectScope::All;
        }
    }

    if (m_pending_scope != MeshSyncClient::ObjectScope::None) {
        sendObjects(m_pending_scope, false);
    }
    else if (m_settings.auto_sync) {
        sendObjects(MeshSyncClient::ObjectScope::Updated, false);
    }
}


void msmayaContext::WaitAndKickAsyncExport()
{
    // process parallel extract tasks
    if (!m_extract_tasks.empty()) {
        if (m_settings.multithreaded) {
            mu::parallel_for_each(m_extract_tasks.begin(), m_extract_tasks.end(), [](TaskRecords::value_type& kvp) {
                kvp.second.process();
            });
        }
        else {
            for (std::map<TreeNode*, TaskRecord>::value_type& kvp : m_extract_tasks) {
                kvp.second.process();
            }
        }
        m_extract_tasks.clear();
    }

    // cleanup
    for (std::vector<std::unique_ptr<TreeNode>>::value_type& n : m_tree_nodes)
        n->clearState();

    MDistance dist;
    dist.setValue(1.0f);
    const float to_meter = static_cast<float>(dist.asMeters());

    using Exporter = ms::SceneExporter;
    Exporter *exporter = m_settings.ExportSceneCache ? (Exporter*)&m_cache_writer : (Exporter*)&m_sender;

    exporter->on_prepare = [this, to_meter, exporter]() {
        if (ms::AsyncSceneSender* sender = dynamic_cast<ms::AsyncSceneSender*>(exporter)) {
            sender->client_settings = m_settings.client_settings;
        }
        else if (ms::SceneCacheWriter* writer = dynamic_cast<ms::SceneCacheWriter*>(exporter)) {
            writer->time = m_anim_time;
        }

        ms::SceneExporter& t = *exporter;
        t.scene_settings.handedness = ms::Handedness::Right;
        t.scene_settings.scale_factor = m_settings.scale_factor / to_meter;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();
    };
    exporter->on_success = [this]() {
        m_material_ids.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
    };
    exporter->kick();
}

bool msmayaContext::recvObjects()
{
    return false;
}




static bool GetColorAndTexture(MFnDependencyNode& fn, const char *plug_name, mu::float4& color, std::string& texpath)
{
    color = mu::float4::zero();
    texpath.clear();

    MPlug plug = fn.findPlug(plug_name, true);
    if (!plug)
        return false;

    MPlugArray connected;
    plug.connectedTo(connected, true, false);
    for (int i = 0; i != connected.length(); ++i) {
        auto node = connected[i].node();
        auto node_type = node.apiType();
        if (node_type == MFn::kFileTexture) {
            MFnDependencyNode fn_tex(node);
            MPlug ftn = fn_tex.findPlug("ftn", true);
            MString path;
            ftn.getValue(path);

            texpath = path.asChar();
            break;
        }
        else if (node_type == MFn::kBump) {
            MFnDependencyNode fn_bump(node);
            MPlug bump_interp = fn_bump.findPlug("bumpInterp", true);
            if (!bump_interp.isNull()) {
                int t = bump_interp.asInt();
                // 0: bump
                // 1: tangent space normals
                // 2: object space normals
                // send texture file only when tangent space normals
                if (t == 1) {
                    return GetColorAndTexture(fn_bump, "bumpValue", color, texpath);
                }
            }
        }
    }

    auto get_color_element = [](MFnDependencyNode& fn, const char *plug_name, const char *color_name, double def = 0.0) -> double
    {
        MString name = plug_name;
        name += color_name;
        auto plug = fn.findPlug(name, true);
        if (!plug.isNull()) {
            double ret = 0.0;
            plug.getValue(ret);
            return ret;
        }
        else {
            return def;
        }
    };
    color = {
        (float)get_color_element(fn, plug_name, "R"),
        (float)get_color_element(fn, plug_name, "G"),
        (float)get_color_element(fn, plug_name, "B"),
        1.0f,
    };
    return true;
}

std::string msmayaContext::handleNamespace(const std::string& path)
{
    return m_settings.remove_namespace ? RemoveNamespace(path) : path;
}

int msmayaContext::exportTexture(const std::string& path, ms::TextureType type)
{
    return m_texture_manager.addFile(path, type);
}

int msmayaContext::findTexture(const std::string& path)
{
    return m_texture_manager.find(path);
}

void msmayaContext::RegisterSceneMaterials()
{
    int midx = 0;
    MItDependencyNodes it(MFn::kLambert);
    while (!it.isDone()) {
        MObject mo = it.thisNode();
        MFnLambertShader fn(mo);

        auto tmp = ms::Material::create();
        tmp->name = handleNamespace(fn.name().asChar());
        tmp->id = m_material_ids.getID(mo);
        tmp->index = midx++;
        {
            mu::float4 color;
            std::string texpath;
            auto& stdmat = ms::AsStandardMaterial(*tmp);
            if (GetColorAndTexture(fn, "color", color, texpath)) {
                if (!texpath.empty()) {
                    stdmat.setColor(mu::float4::one());
                    stdmat.setColorMap(m_settings.sync_textures ? exportTexture(texpath) : findTexture(texpath));
                }
                else {
                    stdmat.setColor(color);
                }
            }
            if (GetColorAndTexture(fn, "normalCamera", color, texpath)) {
                if (!texpath.empty())
                    stdmat.setBumpMap(m_settings.sync_textures ? exportTexture(texpath) : findTexture(texpath));
            }
        }
        m_material_manager.add(tmp);
        it.next();
    }

    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();
}

ms::TransformPtr msmayaContext::exportObject(TreeNode *n, bool parent, bool tip)
{
    if (!n || n->dst_obj)
        return nullptr; // null or already exported

    auto& trans = n->trans->node;
    auto& shape = n->shape->node;

    auto handle_parent = [&]() {
        if (parent)
            exportObject(n->parent, parent, false);
    };
    auto handle_transform = [&]() {
        handle_parent();
        n->dst_obj = exportTransform(n);
    };

    // Maya can instantiate any nodes (include its children), but we only care about Meshes (shape).
    auto handle_instance = [&]() -> bool {
        if (!m_settings.BakeTransform && n->isInstanced() && !n->isPrimaryInstance()) {
            n->dst_obj = exportInstance(n);
            return true;
        }
        return false;
    };

    if (shape.hasFn(MFn::kMesh)) {
        if (m_settings.sync_meshes || m_settings.sync_blendshapes) {
            handle_parent();
            if (!handle_instance())
                n->dst_obj = exportMesh(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (shape.hasFn(MFn::kCamera)) {
        if (m_settings.sync_cameras) {
            handle_parent();
            n->dst_obj = exportCamera(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (shape.hasFn(MFn::kLight)) {
        if (m_settings.sync_lights) {
            handle_parent();
            n->dst_obj = exportLight(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (shape.hasFn(MFn::kJoint)) {
        if (m_settings.sync_bones && !m_settings.BakeModifiers) {
            handle_parent();
            n->dst_obj = exportTransform(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (!tip) {
        // intermediate node
        handle_transform();
    }

    return n->dst_obj;
}

mu::float4x4 msmayaContext::getWorldMatrix(TreeNode *n)
{
    auto& trans = n->trans->node;
    MObject obj_wmat;
    MFnDependencyNode(trans).findPlug("worldMatrix", true).elementByLogicalIndex(0).getValue(obj_wmat);
    return to_float4x4(MFnMatrixData(obj_wmat).matrix());
}

mu::float4x4 msmayaContext::getLocalMatrix(TreeNode *n, mu::float4x4 *dst_world)
{
    // get local matrix by world matrix * inverse parent world matrix.
    // using parentInverseMatrix plug seems more appropriate, but it seems sometimes have incorrect value on Maya2016...
    auto r = getWorldMatrix(n);
    if (dst_world)
        *dst_world = r;

    if (n->parent) {
        auto pw = getWorldMatrix(n->parent);
        r *= mu::invert(pw);
    }
    return r;
}

void msmayaContext::extractTransformData(TreeNode *n,
    mu::float3& t, mu::quatf& r, mu::float3& s, ms::VisibilityFlags& vis,
    mu::float4x4 *dst_world, mu::float4x4 *dst_local)
{
    if (n->trans->isInstanced()) {
        n = n->getPrimaryInstanceNode();
    }

    mu::float4x4 world, local;
    local = getLocalMatrix(n, &world);
    if (dst_world)
        *dst_world = world;
    if (dst_local)
        *dst_local = local;

    // maya-compatible transform extraction
    auto maya_compatible_transform_extraction = [&]() {
        auto& td = n->transform_data;
        td.pivot = mu::float3::zero();
        td.pivot_offset = mu::float3::zero();
        mu::extract_trs(local, td.translation, td.rotation, td.scale);

        n->model_transform = mu::float4x4::identity();
        n->maya_transform = local;
    };

    // fbx-compatible transform extraction
    // - scale pivot is ignored
    // - rotation orientation is ignored
    auto fbx_compatible_transform_extraction = [&]() {
        MFnTransform fn_trans(n->getDagPath(false));

        MQuaternion r;
        double s[3];
        fn_trans.getRotation(r);
        fn_trans.getScale(s);

        auto& td = n->transform_data;
        td.translation = to_float3(fn_trans.getTranslation(MSpace::kTransform));
        td.pivot = to_float3(fn_trans.rotatePivot(MSpace::kTransform));
        td.pivot_offset = to_float3(fn_trans.rotatePivotTranslation(MSpace::kTransform));
        td.rotation = to_quatf(r);
        td.scale = to_float3(s);

        n->model_transform = mu::float4x4::identity();
        (mu::float3&)n->model_transform[3] = -td.pivot;
        n->maya_transform = local;
    };

    auto camera_correction = [](mu::quatf v) {
        return mu::quatf{ -v.z, v.w, v.x, -v.y };
    };

    if (m_settings.BakeTransform) {
        if (n->shape->node.hasFn(MFn::kCamera) || n->shape->node.hasFn(MFn::kLight)) {
            mu::extract_trs(world, t, r, s);
            r = camera_correction(r);
        }
        else {
            t = mu::float3::zero();
            r = mu::quatf::identity();
            s = mu::float3::one();
        }
    }
    else {
        if (!m_settings.fbx_compatible_transform || n->shape->node.hasFn(MFn::kJoint))
            maya_compatible_transform_extraction();
        else
            fbx_compatible_transform_extraction();

        TransformData& td = n->transform_data;
        t = td.translation + td.pivot + td.pivot_offset;
        r = td.rotation;
        s = td.scale;

        if (n->parent)
            t -= n->parent->transform_data.pivot;
        if (n->shape->node.hasFn(MFn::kCamera) || n->shape->node.hasFn(MFn::kLight))
            r = camera_correction(r);
    }

    vis = { n->isVisibleInHierarchy(), true, true };
}

void msmayaContext::extractTransformData(TreeNode *n, ms::Transform& dst)
{
    extractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visibility, &dst.world_matrix, &dst.local_matrix);
}

void msmayaContext::extractCameraData(TreeNode *n,
    bool& ortho, float& near_plane, float& far_plane, float& fov,
    float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift)
{
    auto& shape = n->shape->node;
    MFnCamera mcam(shape);

    ortho = mcam.isOrtho();
    near_plane = (float)mcam.nearClippingPlane();
    far_plane = (float)mcam.farClippingPlane();
    fov = (float)mcam.verticalFieldOfView() * mu::RadToDeg;

    focal_length = (float)mcam.focalLength();
    sensor_size.x = (float)(mcam.horizontalFilmAperture() * mu::InchToMillimeter_d);
    sensor_size.y = (float)(mcam.verticalFilmAperture() * mu::InchToMillimeter_d);

    mu::float2 shift = {
        (float)(mcam.horizontalFilmOffset() * mu::InchToMillimeter_d),
        (float)(mcam.verticalFilmOffset() * mu::InchToMillimeter_d),
    };
    lens_shift = shift / sensor_size; // to 0-1
}

void msmayaContext::extractLightData(TreeNode *n,
    ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& spot_angle)
{
    auto& shape = n->shape->node;
    if (shape.hasFn(MFn::kLight)) {
        MFnLight mlight(shape);
        auto mcol = mlight.color();
        color = { mcol.r, mcol.g, mcol.b, mcol.a };
        intensity = mlight.intensity();

        if (shape.hasFn(MFn::kNonExtendedLight)) {
            MFnNonExtendedLight melight(shape);
            stype = (melight.useRayTraceShadows() || melight.useDepthMapShadows()) ? ms::Light::ShadowType::Soft : ms::Light::ShadowType::None;
        }

        if (shape.hasFn(MFn::kSpotLight)) {
            MFnSpotLight mlight(shape);
            ltype = ms::Light::LightType::Spot;
            spot_angle = (float)mlight.coneAngle() * mu::RadToDeg;
        }
        else if (shape.hasFn(MFn::kDirectionalLight)) {
            //MFnDirectionalLight mlight(shape);
            ltype = ms::Light::LightType::Directional;
        }
        else if (shape.hasFn(MFn::kPointLight)) {
            //MFnPointLight mlight(shape);
            ltype = ms::Light::LightType::Point;
        }
        else if (shape.hasFn(MFn::kAreaLight)) {
            //MFnAreaLight mlight(shape);
            ltype = ms::Light::LightType::Area;
        }
    }
}

template<class T>
std::shared_ptr<T> msmayaContext::createEntity(TreeNode& n)
{
    auto ret = T::create();
    auto& dst = *ret;
    dst.path = handleNamespace(n.path);
    dst.index = n.index;
    n.dst_obj = ret;
    return ret;
}

ms::TransformPtr msmayaContext::exportTransform(TreeNode *n)
{
    auto ret = createEntity<ms::Transform>(*n);
    auto& dst = *ret;

    extractTransformData(n, dst);

    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr msmayaContext::exportInstance(TreeNode *n)
{
    auto ret = createEntity<ms::Transform>(*n);
    auto& dst = *ret;

    extractTransformData(n, dst);

    auto primary = n->getPrimaryInstanceNode();
    if (primary && n != primary)
        dst.reference = handleNamespace(primary->path);

    m_entity_manager.add(ret);
    return ret;
}

ms::CameraPtr msmayaContext::exportCamera(TreeNode *n)
{
    auto ret = createEntity<ms::Camera>(*n);
    auto& dst = *ret;

    extractTransformData(n, dst);
    extractCameraData(n, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov,
        dst.focal_length, dst.sensor_size, dst.lens_shift);

    m_entity_manager.add(ret);
    return ret;
}

ms::LightPtr msmayaContext::exportLight(TreeNode *n)
{
    auto ret = createEntity<ms::Light>(*n);
    auto& dst = *ret;

    extractTransformData(n, dst);
    extractLightData(n, dst.light_type, dst.shadow_type, dst.color, dst.intensity, dst.spot_angle);

    m_entity_manager.add(ret);
    return ret;
}

ms::MeshPtr msmayaContext::exportMesh(TreeNode *n)
{
    auto ret = createEntity<ms::Mesh>(*n);
    auto& dst = *ret;

    extractTransformData(n, dst);

    // send mesh contents even if the node is hidden.

    auto task = [this, ret, &dst, n]() {
        if (m_settings.sync_meshes) {
            if (m_settings.BakeModifiers)
                doExtractMeshDataBaked(dst, n);
            else
                doExtractMeshData(dst, n);

            if (m_settings.BakeTransform) {
                dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_LOCAL2WORLD, true);
                dst.refine_settings.local2world = dst.world_matrix;
            }

            if (dst.normals.empty()) {
                dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
            }
            if (dst.tangents.empty()) {
                dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);
            }
            dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_MAKE_DOUBLE_SIDED, m_settings.make_double_sided);
            dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_FLIP_FACES, true);
        }
        else {
            if (!m_settings.BakeModifiers && m_settings.sync_blendshapes)
                doExtractBlendshapeWeights(dst, n);
        }
        m_entity_manager.add(ret);
    };
    m_extract_tasks[n->shape->branches.front()].add(n, task);
    return ret;
}

void msmayaContext::doExtractBlendshapeWeights(ms::Mesh& dst, TreeNode *n)
{
    auto& shape = n->shape->node;
    if (!shape.hasFn(MFn::kMesh))
        return;

    dst.visibility = { IsVisible(shape), true, true };
    if (!dst.visibility.active)
        return;

    MFnMesh mmesh(shape);
    MFnBlendShapeDeformer fn_blendshape(FindBlendShape(mmesh.object()));

    if (m_settings.sync_blendshapes && !fn_blendshape.object().isNull()) {
        MPlug plug_weight = fn_blendshape.findPlug("weight", true);
        MPlug plug_it = fn_blendshape.findPlug("inputTarget", true);
        uint32_t num_it = plug_it.evaluateNumElements();
        for (uint32_t idx_it = 0; idx_it < num_it; ++idx_it) {
            MPlug plug_itp(plug_it.elementByPhysicalIndex(idx_it));
            if (plug_itp.logicalIndex() == 0) {
                MPlug plug_itg(plug_itp.child(0)); // .inputTarget[idx_it].inputTargetGroup
                uint32_t num_itg = plug_itg.evaluateNumElements();

                for (uint32_t idx_itg = 0; idx_itg < num_itg; ++idx_itg) {
                    auto dst_bs = ms::BlendShapeData::create();
                    dst.blendshapes.push_back(dst_bs);
                    MPlug plug_wc = plug_weight.elementByPhysicalIndex(idx_itg);
                    dst_bs->name = handleNamespace(plug_wc.name().asChar());
                    plug_wc.getValue(dst_bs->weight);
                    dst_bs->weight *= 100.0f; // 0.0f-1.0f -> 0.0f-100.0f
                }
            }
        }
    }
}

void msmayaContext::doExtractMeshDataImpl(ms::Mesh& dst, MFnMesh &mmesh, MFnMesh &mshape)
{
    // get points
    {
        int num_vertices = mmesh.numVertices();
        if (num_vertices == 0) {
            // return empty mesh
            return;
        }

        dst.points.resize_discard(num_vertices);
        auto *points = (const mu::float3*)mmesh.getRawPoints(nullptr);
        for (int i = 0; i < num_vertices; ++i) {
            dst.points[i] = points[i];
        }
    }

    // get faces
    {
        MItMeshPolygon it_poly(mmesh.object());
        dst.counts.reserve(it_poly.count());
        dst.indices.reserve(it_poly.count() * 4);

        while (!it_poly.isDone()) {
            int count = it_poly.polygonVertexCount();
            dst.counts.push_back(count);
            for (int i = 0; i < count; ++i) {
                dst.indices.push_back(it_poly.vertexIndex(i));
            }
            it_poly.next();
        }
    }

    uint32_t vertex_count = (uint32_t)dst.points.size();
    uint32_t index_count = (uint32_t)dst.indices.size();
    uint32_t face_count = (uint32_t)dst.counts.size();

    // get normals
    if (m_settings.sync_normals) {
        int num_normals = mmesh.numNormals();
        if (num_normals > 0) {
            dst.normals.resize_discard(index_count);
            auto *normals = (const mu::float3*)mmesh.getRawNormals(nullptr);

            size_t ii = 0;
            MItMeshPolygon it_poly(mmesh.object());
            while (!it_poly.isDone()) {
                int count = it_poly.polygonVertexCount();
                for (int i = 0; i < count; ++i) {
                    dst.normals[ii] = normals[it_poly.normalIndex(i)];
                    ++ii;
                }
                it_poly.next();
            }
        }
    }

    // get uv
    if (m_settings.sync_uvs) {
        MStringArray uvsets;
        mmesh.getUVSetNames(uvsets);

        if (uvsets.length() > 0 && mmesh.numUVs(uvsets[0]) > 0) {
            dst.m_uv[0].resize_zeroclear(index_count);

            MFloatArray u;
            MFloatArray v;
            mmesh.getUVs(u, v, &uvsets[0]);
            const auto *u_ptr = &u[0];
            const auto *v_ptr = &v[0];

            size_t ii = 0;
            MItMeshPolygon it_poly(mmesh.object());
            while (!it_poly.isDone()) {
                int count = it_poly.polygonVertexCount();
                for (int i = 0; i < count; ++i) {
                    int iu;
                    if (it_poly.getUVIndex(i, iu, &uvsets[0]) == MStatus::kSuccess && iu >= 0)
                        dst.m_uv[0][ii] = mu::float2{ u_ptr[iu], v_ptr[iu] };
                    ++ii;
                }
                it_poly.next();
            }
        }
    }

    // get vertex colors
    if (m_settings.sync_colors) {
        MStringArray color_sets;
        mmesh.getColorSetNames(color_sets);

        if (color_sets.length() > 0 && mmesh.numColors(color_sets[0]) > 0) {
            dst.colors.resize(index_count, mu::float4::one());

            MColorArray colors;
            mmesh.getColors(colors, &color_sets[0]);
            const auto *colors_ptr = &colors[0];

            size_t ii = 0;
            MItMeshPolygon it_poly(mmesh.object());
            while (!it_poly.isDone()) {
                int count = it_poly.polygonVertexCount();
                for (int i = 0; i < count; ++i) {
                    int ic;
                    if (it_poly.getColorIndex(i, ic, &color_sets[0]) == MStatus::kSuccess && ic >= 0)
                        dst.colors[ii] = (const mu::float4&)colors_ptr[ic];
                    ++ii;
                }
                it_poly.next();
            }
        }
    }


    // get face material id
    {
        RawVector<int> mids;
        MObjectArray shaders;
        MIntArray shader_indices;
        mshape.getConnectedShaders(0, shaders, shader_indices);
        mids.resize(shaders.length(), 0);
        for (uint32_t si = 0; si < shaders.length(); si++) {
            MItDependencyGraph it(shaders[si], MFn::kLambert, MItDependencyGraph::kUpstream);
            if (!it.isDone())
                mids[si] = m_material_ids.getID(it.currentItem());
        }

        if (mids.size() > 0) {
            // MIntArray::operator[] is dllimported function. get raw pointer for faster access.
            const int* shader_indices_ = &shader_indices[0];
            dst.material_ids.resize(face_count, 0);
            uint32_t len = std::min(face_count, shader_indices.length());
            for (uint32_t i = 0; i < len; ++i)
                dst.material_ids[i] = mids[shader_indices_[i]];
        }
    }
}

void msmayaContext::doExtractMeshData(ms::Mesh& dst, TreeNode *n)
{
    auto& trans = n->trans->node;
    auto& shape = n->shape->node;

    if (!shape.hasFn(MFn::kMesh))
        return;

    MStatus mstat;
    MFnMesh mmesh(shape);
    MFnSkinCluster fn_skin(FindSkinCluster(shape));
    bool is_skinned = !fn_skin.object().isNull();
    mu::float4x4 trans_geom = mu::float4x4::identity();
    GetTransformGeometryMatrix(shape, trans_geom);

    if (!mmesh.object().hasFn(MFn::kMesh)) {
        // return empty mesh
        return;
    }

    MFnMesh fn_src_mesh(FindOrigMesh(trans));
    MFnBlendShapeDeformer fn_blendshape(FindBlendShape(mmesh.object()));
    int skin_index = 0;
    if (m_settings.sync_bones && is_skinned)
        skin_index = fn_skin.indexForOutputShape(mmesh.object());

    // this code assumes blendshape is applied always after skinning, and there is no multiple blendshapes or skinnings.

    doExtractMeshDataImpl(dst, fn_src_mesh, mmesh);

    uint32_t vertex_count = (uint32_t)dst.points.size();

    auto apply_tweak = [&dst](MObject deformer, int obj_index) {
        MItDependencyGraph it(deformer, MFn::kTweak, MItDependencyGraph::kUpstream);
        if (!it.isDone()) {
            MObject tweak = it.currentItem();
            if (!tweak.isNull()) {
                MFnDependencyNode fn_tweak(tweak);
                auto plug_vlist = fn_tweak.findPlug("vlist", true);
                if (plug_vlist.isArray() && (int)plug_vlist.numElements() > obj_index) {
                    auto plug_vertex = plug_vlist.elementByPhysicalIndex(obj_index).child(0);
                    if (plug_vertex.isArray()) {
                        auto vertices_len = plug_vertex.numElements();
                        for (uint32_t vi = 0; vi < vertices_len; ++vi) {
                            MPlug p3 = plug_vertex.elementByPhysicalIndex(vi);
                            int li = p3.logicalIndex();
                            mu::float3 v;
                            p3.child(0).getValue(v.x);
                            p3.child(1).getValue(v.y);
                            p3.child(2).getValue(v.z);
                            dst.points[li] += v;
                        }
                    }
                }
            }
        }
    };

    auto apply_uv_tweak = [&dst](MObject deformer, int obj_index) {
        MItDependencyGraph it(deformer, MFn::kPolyTweakUV, MItDependencyGraph::kDownstream);
        if (!it.isDone()) {
            MObject tweak = it.currentItem();
            if (!tweak.isNull()) {
                MFnDependencyNode fn_tweak(tweak);
                auto plug_uvsetname = fn_tweak.findPlug("uvSetName", true);
                auto plug_uv = fn_tweak.findPlug("uvTweak", true);
                if (plug_uv.isArray() && (int)plug_uv.numElements() > obj_index) {
                    auto plug_vertex = plug_uv.elementByPhysicalIndex(obj_index).child(0);
                    if (plug_vertex.isArray()) {
                        auto vertices_len = plug_vertex.numElements();
                        for (uint32_t vi = 0; vi < vertices_len; ++vi) {
                            MPlug p2 = plug_vertex.elementByPhysicalIndex(vi);
                            int li = p2.logicalIndex();
                            mu::float2 v;
                            p2.child(0).getValue(v.x);
                            p2.child(1).getValue(v.y);
                            dst.m_uv[0][li] += v;
                        }
                    }
                }
            }
        }
    };


    // get blendshape data
    if (m_settings.sync_blendshapes && !fn_blendshape.object().isNull()) {
        // https://knowledge.autodesk.com/search-result/caas/CloudHelp/cloudhelp/2018/ENU/Maya-Tech-Docs/Nodes/blendShape-html.html

        auto gen_delta = [&dst](ms::BlendShapeFrameData& dst_frame, MPlug plug_geom) {
            MObject obj_geom;
            plug_geom.getValue(obj_geom);
            if (!obj_geom.isNull() && obj_geom.hasFn(MFn::kMesh)) {

                MFnMesh fn_geom(obj_geom);
                auto *points = (const mu::float3*)fn_geom.getRawPoints(nullptr);

                int len = std::min(fn_geom.numVertices(), (int)dst.points.size());
                for (int pi = 0; pi < len; ++pi) {
                    dst_frame.points[pi] = points[pi] - dst.points[pi];
                }
            }
        };

        auto retrieve_delta = [&dst](ms::BlendShapeFrameData& dst_frame, MPlug plug_ipt, MPlug plug_ict) {
            MObject obj_cld;
            plug_ict.getValue(obj_cld);

            MObject obj_points;
            {
                MObject tmp;
                plug_ipt.getValue(tmp);
                if (!tmp.isNull() && tmp.hasFn(MFn::kPointArrayData)) {
                    obj_points = tmp;
                }
            }

            if (obj_cld.hasFn(MFn::kComponentListData) && !obj_points.isNull()) {
                uint32_t base_point = 0;

                MFnPointArrayData fn_points(obj_points);
                // get raw pointer for faster access.
                const auto *points_ = &fn_points[0];

                MFnComponentListData fn_cld(obj_cld);
                uint32_t num_clists = fn_cld.length();
                for (uint32_t cli = 0; cli < num_clists; ++cli) {
                    MObject clist = fn_cld[cli];
                    if (clist.apiType() != MFn::kMeshVertComponent)
                        continue;

                    MIntArray indices;
                    MFnSingleIndexedComponent fn_indices(clist);
                    fn_indices.getElements(indices);
                    const int *indices_ = &indices[0];

                    auto len = indices.length();
                    for (uint32_t pi = 0; pi < len; ++pi)
                        dst_frame.points[indices_[pi]] = to_float3(points_[base_point + pi]);

                    base_point += len;
                }
            }
        };

        MPlug plug_weight = fn_blendshape.findPlug("weight", true);
        MPlug plug_it = fn_blendshape.findPlug("inputTarget", true);
        uint32_t num_it = plug_it.evaluateNumElements();
        for (uint32_t idx_it = 0; idx_it < num_it; ++idx_it) {
            MPlug plug_itp(plug_it.elementByPhysicalIndex(idx_it));
            if (plug_itp.logicalIndex() == 0) {
                MPlug plug_itg(plug_itp.child(0)); // .inputTarget[idx_it].inputTargetGroup
                uint32_t num_itg = plug_itg.evaluateNumElements();

                for (uint32_t idx_itg = 0; idx_itg < num_itg; ++idx_itg) {
                    auto dst_bs = ms::BlendShapeData::create();
                    dst.blendshapes.push_back(dst_bs);
                    MPlug plug_wc = plug_weight.elementByPhysicalIndex(idx_itg);
                    dst_bs->name = handleNamespace(plug_wc.name().asChar());
                    plug_wc.getValue(dst_bs->weight);
                    dst_bs->weight *= 100.0f; // 0.0f-1.0f -> 0.0f-100.0f

                    MPlug plug_itgp(plug_itg.elementByPhysicalIndex(idx_itg));
                    MPlug plug_iti(plug_itgp.child(0)); // .inputTarget[idx_it].inputTargetGroup[idx_itg].inputTargetItem
                    uint32_t num_iti(plug_iti.evaluateNumElements());
                    for (uint32_t idx_iti = 0; idx_iti != num_iti; ++idx_iti) {
                        MPlug plug_itip(plug_iti.elementByPhysicalIndex(idx_iti));

                        dst_bs->frames.push_back(ms::BlendShapeFrameData::create());
                        auto& dst_frame = *dst_bs->frames.back();
                        dst_frame.weight = float(plug_itip.logicalIndex() - 5000) / 10.0f; // index 5000-6000 -> weight 0.0f-100.0f
                        dst_frame.points.resize_zeroclear(dst.points.size());

                        MPlug plug_geom(plug_itip.child(0)); // .inputGeomTarget
                        if (plug_geom.isConnected()) {
                            // in this case target is geometry
                            gen_delta(dst_frame, plug_geom);
                        }
                        else {
                            // in this case there is no geometry target. try to retrieves deltas
                            MPlug plug_ipt(plug_itip.child(3)); // .inputPointsTarget
                            MPlug plug_ict(plug_itip.child(4)); // .inputComponentsTarget
                            retrieve_delta(dst_frame, plug_ipt, plug_ict);
                        }
                    }
                }
            }
        }
    }

    // get skinning data
    if (m_settings.sync_bones && !fn_skin.object().isNull()) {
        // request bake TRS
        dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_LOCAL2WORLD, true);
        dst.refine_settings.local2world = n->maya_transform;

        // get bone data
        MPlug plug_bindprematrix = fn_skin.findPlug("bindPreMatrix", true, &mstat);
        if (mstat == MStatus::kSuccess) {
            MDagPathArray joint_paths;
            auto num_joints = fn_skin.influenceObjects(joint_paths);

            for (uint32_t ij = 0; ij < num_joints; ij++) {
                auto bone = ms::BoneData::create();
                bone->weights.resize_zeroclear(dst.points.size());
                dst.bones.push_back(bone);

                const auto& joint_path = joint_paths[ij];
                auto joint_branch = FindBranch(m_dag_nodes, joint_path);
                if (!joint_branch || !joint_branch->trans || joint_branch->trans->node.isNull())
                    continue;

                auto joint_node = joint_branch->trans->node;
                bone->path = handleNamespace(ToString(joint_path));
                if (dst.bones.empty()) {
                    // get root bone path
                    auto dpath = joint_branch->getDagPath();
                    for (;;) {
                        auto tmp = dpath;
                        tmp.pop();
                        auto t = tmp.node();
                        if (!t.hasFn(MFn::kJoint))
                            break;
                        dpath = tmp;
                    }
                    dst.root_bone = ToString(dpath);
                }

                // get bindpose
                MObject matrix_obj;
                auto ijoint = fn_skin.indexForInfluenceObject(joint_paths[ij], &mstat);
                if (mstat == MStatus::kSuccess) {
                    auto matrix_plug = plug_bindprematrix.elementByLogicalIndex(ijoint, &mstat);
                    if (mstat == MStatus::kSuccess) {
                        matrix_plug.getValue(matrix_obj);
                        bone->bindpose = to_float4x4(MFnMatrixData(matrix_obj).matrix());
                    }
                }
            }

            // get weights
            MDagPath mesh_path = GetDagPath(n, mmesh.object());
            MFloatArray weights;
            MItGeometry gi(mesh_path);
            uint32_t vi = 0;
            while (!gi.isDone() && vi < vertex_count) {
                uint32_t influence_count = 0;
                if (fn_skin.getWeights(mesh_path, gi.currentItem(), weights, influence_count) == MStatus::kSuccess) {
                    for (uint32_t ij = 0; ij < influence_count; ij++) {
                        dst.bones[ij]->weights[vi] = weights[ij];
                    }
                }
                gi.next();
                ++vi;
            }
        }
    }
    else {
        // apply pivot
        dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_LOCAL2WORLD, true);
        dst.refine_settings.local2world = n->model_transform;
    }

    // apply tweaks
    if (m_settings.apply_tweak) {
        apply_tweak(shape, skin_index);
        apply_uv_tweak(shape, skin_index);
    }

    dst.refine_settings.local2world = dst.refine_settings.local2world * trans_geom;
}

void msmayaContext::doExtractMeshDataBaked(ms::Mesh& dst, TreeNode *n)
{
    auto& trans = n->trans->node;
    auto& shape = n->shape->node;
    if (!shape.hasFn(MFn::kMesh)) {
        // return empty mesh
        return;
    }

    MStatus mstat;
    MFnMesh mmesh(shape);
    doExtractMeshDataImpl(dst, mmesh, mmesh);

    // apply pivot
    dst.refine_settings.flags.Set(ms::MESH_REFINE_FLAG_LOCAL2WORLD, true);
    dst.refine_settings.local2world = n->model_transform;
}


void msmayaContext::AnimationRecord::operator()(msmayaContext *_this)
{
    (_this->*extractor)(*dst, tn);
}


int msmayaContext::exportAnimations(MeshSyncClient::ObjectScope scope)
{
    const float frame_rate = (float)MTime(1.0, MTime::kSeconds).as(MTime::uiUnit());
    const float frame_step = std::max(m_settings.frame_step, 0.1f);

    // create default clip
    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create());

    auto& clip = *m_animations.back();
    clip.frame_rate = frame_rate * std::max(1.0f / frame_step, 1.0f);


    int num_exported = 0;
    for (auto n : getNodes(scope)) {
        if (exportAnimation(n, false))
            ++num_exported;
    }

    // extract
    auto time_current = MAnimControl::currentTime();
    auto time_start = MAnimControl::minTime();
    auto time_end = MAnimControl::maxTime();
    auto interval = MTime(frame_step, MTime::uiUnit());

    int reserve_size = int((time_end.as(MTime::kSeconds) - time_start.as(MTime::kSeconds)) / interval.as(MTime::kSeconds)) + 1;
    for (auto& kvp : m_anim_records) {
        kvp.second.dst->reserve(reserve_size);
    }

    // advance frame and record
    m_ignore_update = true;
    for (MTime t = time_start;;) {
        m_anim_time = (float)(t - time_start).as(MTime::kSeconds);
        MGlobal::viewFrame(t);
        for (auto& kvp : m_anim_records)
            kvp.second(this);

        if (t >= time_end)
            break;
        else
            t += interval;
    }
    MGlobal::viewFrame(time_current);
    m_ignore_update = false;

    // cleanup
    m_anim_records.clear();

    if (num_exported == 0)
        m_animations.clear();

    return num_exported;
}

bool msmayaContext::exportAnimation(TreeNode *n, bool force)
{
    if (!n || n->dst_anim)
        return false; // null or already exported

    auto& trans = n->trans->node;
    auto& shape = n->shape->node;

    ms::TransformAnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;

    if (shape.hasFn(MFn::kCamera)) {
        exportAnimation(n->parent, true);
        dst = ms::CameraAnimation::create();
        extractor = &msmayaContext::extractCameraAnimationData;
    }
    else if (shape.hasFn(MFn::kLight)) {
        exportAnimation(n->parent, true);
        dst = ms::LightAnimation::create();
        extractor = &msmayaContext::extractLightAnimationData;
    }
    else if (shape.hasFn(MFn::kMesh)) {
        exportAnimation(n->parent, true);
        dst = ms::MeshAnimation::create();
        extractor = &msmayaContext::extractMeshAnimationData;
    }
    else if (shape.hasFn(MFn::kJoint) || force) {
        exportAnimation(n->parent, true);
        dst = ms::TransformAnimation::create();
        extractor = &msmayaContext::extractTransformAnimationData;
    }

    if (dst) {
        dst->path = handleNamespace(n->path);
        auto& rec = m_anim_records[n];
        rec.tn = n;
        rec.dst = dst;
        rec.extractor = extractor;
        n->dst_anim = dst;
        m_animations.front()->addAnimation(dst);
        return true;
    }
    else {
        return false;
    }
}

void msmayaContext::extractTransformAnimationData(ms::TransformAnimation& dst_, TreeNode *n)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    auto pos = mu::float3::zero();
    auto rot = mu::quatf::identity();
    auto scale = mu::float3::one();
    ms::VisibilityFlags vis;
    extractTransformData(n, pos, rot, scale, vis);

    float t = m_anim_time;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    dst.visible.push_back({ t, (int)vis.active });
}

void msmayaContext::extractCameraAnimationData(ms::TransformAnimation& dst_, TreeNode *n)
{
    extractTransformAnimationData(dst_, n);

    auto& dst = (ms::CameraAnimation&)dst_;

    bool ortho;
    float near_plane, far_plane, fov, focal_length;
    mu::float2 sensor_size, lens_shift;
    extractCameraData(n, ortho, near_plane, far_plane, fov, focal_length, sensor_size, lens_shift);

    float t = m_anim_time;
    dst.near_plane.push_back({ t, near_plane });
    dst.far_plane.push_back({ t, far_plane });
    dst.fov.push_back({ t, fov });

    dst.focal_length.push_back({ t, focal_length });
    dst.sensor_size.push_back({ t, sensor_size });
    dst.lens_shift.push_back({ t, lens_shift });
}

void msmayaContext::extractLightAnimationData(ms::TransformAnimation& dst_, TreeNode *n)
{
    extractTransformAnimationData(dst_, n);

    auto& dst = (ms::LightAnimation&)dst_;

    ms::Light::LightType ltype;
    ms::Light::ShadowType stype;
    mu::float4 color;
    float intensity;
    float spot_angle;
    extractLightData(n, ltype, stype, color, intensity, spot_angle);

    float t = m_anim_time;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    if (ltype == ms::Light::LightType::Spot)
        dst.spot_angle.push_back({ t, spot_angle });
}

void msmayaContext::extractMeshAnimationData(ms::TransformAnimation & dst_, TreeNode *n)
{
    extractTransformAnimationData(dst_, n);

    auto& dst = (ms::MeshAnimation&)dst_;
    auto& shape = n->shape->node;

    float t = m_anim_time;

    // get blendshape weights
    MFnBlendShapeDeformer fn_blendshape(FindBlendShape(shape));
    if (!fn_blendshape.object().isNull()) {
        MPlug plug_weight = fn_blendshape.findPlug("weight", true);
        MPlug plug_it = fn_blendshape.findPlug("inputTarget", true);
        uint32_t num_it = plug_it.evaluateNumElements();
        for (uint32_t idx_it = 0; idx_it < num_it; ++idx_it) {
            MPlug plug_itp(plug_it.elementByPhysicalIndex(idx_it));
            if (plug_itp.logicalIndex() == 0) {
                MPlug plug_itg(plug_itp.child(0)); // .inputTarget[idx_it].inputTargetGroup
                uint32_t num_itg = plug_itg.evaluateNumElements();

                for (uint32_t idx_itg = 0; idx_itg < num_itg; ++idx_itg) {
                    MPlug plug_wc = plug_weight.elementByPhysicalIndex(idx_itg);
                    std::string name = handleNamespace(plug_wc.name().asChar());

                    auto bsa = dst.getBlendshapeCurve(name);
                    float weight = 0.0f;
                    plug_wc.getValue(weight);
                    bsa.push_back({ t, weight * 100.0f });
                }
            }
        }
    }
}
