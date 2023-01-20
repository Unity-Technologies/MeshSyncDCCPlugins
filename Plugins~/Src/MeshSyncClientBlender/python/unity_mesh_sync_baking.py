import bpy, os, datetime, time, math
from bpy_extras.io_utils import ExportHelper
from bpy.app.handlers import persistent
import functools
import tempfile

from .unity_mesh_sync_common import MESHSYNC_PT

# Constants:
ORIGINAL_MATERIAL = 'ORIGINAL_MATERIAL'
BAKED_MATERIAL_SHADER = 'BAKED_MATERIAL_SHADER'
UV_OVERRIDE = 'UV_OVERRIDE'


class LogLevel:
    VERBOSE = 0
    NORMAL = 1
    ERROR = 2


# For debugging and getting a callstack on error:
throwExceptions = False
showLogLevel = LogLevel.ERROR

AO_CHANNEL_NAME = "Ambient Occlusion"

BAKED_CHANNELS = ["Base Color",
                  "Metallic",
                  "Roughness",
                  "Clearcoat",
                  "Emission",
                  "Normal",
                  AO_CHANNEL_NAME]

# The above channels may not exist in all BSDF types, mapping of alternative names:
synonymMap = {"Base Color": ["Color"]}

channelNameToBakeName = {
    'Base Color': 'DIFFUSE',
    'Color': 'DIFFUSE',
    'Roughness': 'ROUGHNESS',
    'Normal': 'NORMAL',
    AO_CHANNEL_NAME: 'AO'
}


def msb_log(text, level=LogLevel.VERBOSE):
    if level >= showLogLevel:
        print(text)


def msb_canObjectMaterialsBeBaked(obj: bpy.types.Object) -> bool:
    hasMaterials = obj.data is not None and obj.type == 'MESH'
    if not hasMaterials:
        return False
    # If it's a mesh, make sure it actually has vertices, or we'll get errors baking it later:
    if obj.type == 'MESH':
        if len(obj.data.vertices) == 0:
            return False

    return True


# Methods to help getting and setting nested attributes:
def msb_rsetattr(obj, attr, val):
    pre, _, post = attr.rpartition('.')
    return setattr(msb_rgetattr(obj, pre) if pre else obj, post, val)


def msb_rgetattr(obj, attr, *args):
    def _getattr(obj, attr):
        return getattr(obj, attr, *args)

    return functools.reduce(_getattr, [obj] + attr.split('.'))


# Setting classes:
class MESHSYNC_BakeChannelSetting(bpy.types.PropertyGroup):
    name: bpy.props.StringProperty(name="")
    bakeChannelEnabled: bpy.props.BoolProperty(name="", description="Whether to bake this channel or not", default=True)


def msb_bakeAllGet(meshsync_bake_settings):
    for channelSetting in meshsync_bake_settings.bake_channel_settings:
        if not channelSetting.bakeChannelEnabled:
            return False
    return True


def msb_bakeAllSet(meshsync_bake_settings, newValue):
    for channelSetting in meshsync_bake_settings.bake_channel_settings:
        channelSetting.bakeChannelEnabled = newValue


class MESHSYNC_BakeSettings(bpy.types.PropertyGroup):
    '''
    Groups all bake settings in a single class.
    '''
    bakedTexturesPath: bpy.props.StringProperty(name="Baked texture path", default=tempfile.gettempdir())
    baked_texture_dimensions: bpy.props.EnumProperty(name="Texture dimensions",
                                           items=(('PIXELS', 'Pixels',
                                                   'Custom texture size'),
                                                  ('TEXEL_DENSITY', 'Texel density',
                                                   'Use polygon size in the UV map to determine texture dimensions')),
                                           default='PIXELS')
    texel_density: bpy.props.FloatProperty(name="Texels / World Unit",
                                         description="How many texture pixels for 1 blender world unit",
                                         default=2048)
    texel_density_limit: bpy.props.IntProperty(name="Max texture size",
                                             description="Maximum texture size when calculating dimensions from texel density",
                                             min=1,
                                             max=65536,
                                             default=2048)
    texel_density_pot: bpy.props.BoolProperty(name="Power of 2",
                                              description="Whether to increase the texture size to the next power of 2",
                                              default=True)
    bakedTextureSize: bpy.props.IntVectorProperty(name="Baked texture size", size=2,
                                                  default=(512, 512))
    bake_selection: bpy.props.EnumProperty(name="Objects to bake",
                                           items=(('ALL', 'All',
                                                   'Bake all objects in the scene (including hidden)'),
                                                  ('SELECTED', 'Selected',
                                                   'Bake only the selected object')),
                                           default='ALL')
    bake_channel_settings: bpy.props.CollectionProperty(type=MESHSYNC_BakeChannelSetting)
    bake_all_channels: bpy.props.BoolProperty(name="All", description="Toggle all",
                                              get=msb_bakeAllGet,
                                              set=msb_bakeAllSet)
    generate_uvs: bpy.props.EnumProperty(name="Generate UVs",
                                         description="Whether to auto-generate UVs",
                                         items=(('OFF', 'Off',
                                                 'Bake all objects in the scene (including hidden)'),
                                                ('IF_NEEDED', 'If needed',
                                                 'Automatically UV unwraps objects if there are no UVs or existing UVs are not in the 0..1 range. WARNING: This will delete existing UVs on the object!'),
                                                ('ALWAYS', 'Always',
                                                 'Always automatically UV unwraps objects. WARNING: This will delete existing UVs on the object!')),
                                         default='IF_NEEDED')
    apply_modifiers: bpy.props.BoolProperty(name="Apply modifiers",
                                            description="In order to bake and get correct UVs, all modifiers need to be applied. WARNING: This will apply and remove existing modifiers on the object!",
                                            default=True)
    realize_instances: bpy.props.BoolProperty(name="Realize instances",
                                            description = "Realize geometry node instances to include them in the bake", default = True)
    run_modal: bpy.props.BoolProperty(name="Run Modal",
                                            description="If this is enabled blender stays more interactive but baking is slower.",
                                            default=False)
    bake_progress: bpy.props.FloatProperty(
        name="Progress",
        subtype="PERCENTAGE",
        min=0,
        max=100,
        precision=0)
    bake_message: bpy.props.StringProperty()
    bake_maps_remaining: bpy.props.StringProperty(name="Maps baked")
    bake_time_remaining: bpy.props.StringProperty(name="Estimated time left")


def msb_bakeAllChanged(self, context):
    bakeAll = context.scene.meshsync_bake_settings.bake_all_channels
    for channelSetting in context.scene.meshsync_bake_settings.bake_channel_settings:
        channelSetting.bakeChannelEnabled = bakeAll


@persistent
def msb_setBakingDefaults(dummy):
    context = bpy.context
    bakeSettings = context.scene.meshsync_bake_settings
    bakeSettings.bake_progress = 0
    if len(bakeSettings.bake_channel_settings) != len(BAKED_CHANNELS):
        bakeSettings.bake_channel_settings.clear()
        for channel in BAKED_CHANNELS:
            channelSettings = bakeSettings.bake_channel_settings.add()
            channelSettings.name = channel


class MESHSYNC_PT_Baking(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Baking"
    bl_parent_id = "MESHSYNC_PT_Main"
    bl_order = -10

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False

        bakeSettings = context.scene.meshsync_bake_settings

        layout.prop(bakeSettings, "bake_selection", expand=True)

        box = layout.box()
        box.alignment = 'LEFT'
        box.label(text="Material channels to bake")
        box.prop(bakeSettings, "bake_all_channels")
        for channelSetting in bakeSettings.bake_channel_settings:
            row = box.row()
            row.prop(channelSetting, "bakeChannelEnabled", text=channelSetting.name)

        layout.prop(bakeSettings, "generate_uvs", expand=True)
        layout.prop(bakeSettings, "apply_modifiers")
        if bakeSettings.apply_modifiers:
            layout.prop(bakeSettings, "realize_instances")
        layout.prop(bakeSettings, "run_modal")

        layout.prop(bakeSettings, "baked_texture_dimensions", expand=True)
        if bakeSettings.baked_texture_dimensions == 'PIXELS':
            layout.prop(bakeSettings, "bakedTextureSize")
        else:
            layout.prop(bakeSettings, "texel_density")
            layout.prop(bakeSettings, "texel_density_limit")
            layout.prop(bakeSettings, "texel_density_pot")

        layout.operator("meshsync.bake_materials")

        if bakeSettings.bake_progress > 0.0:
            box = layout.box()
            box.prop(bakeSettings, "bake_progress")
            box.label(text=bakeSettings.bake_message, icon='INFO')
            box.label(text=bakeSettings.bake_maps_remaining)
            if len(bakeSettings.bake_time_remaining) > 0:
                box.label(text=bakeSettings.bake_time_remaining)
            if bakeSettings.bake_progress < 100:
                box.label(text="Hold ESC to cancel")

        row = layout.row()
        row.prop(bakeSettings, "bakedTexturesPath")
        row.operator("meshsync.choose_material_bake_folder", icon="FILE_FOLDER", text="")

        layout.operator("meshsync.revert_bake_materials")


class MESHSYNC_OT_Bake(bpy.types.Operator):
    bl_idname = "meshsync.bake_materials"
    bl_label = "Bake to individual materials"
    bl_description = "Bakes textures, creates material copies and assigns the baked materials for all materials that " \
                     "cannot be exported without baking them to textures"

    maxBakeProgress = 0
    mapsToBake = 0
    currentBakeProgress = 0

    def isMaterialCopy(self, mat):
        return ORIGINAL_MATERIAL in mat

    def canMaterialBeBaked(self, mat):
        return mat is not None and \
            not self.isMaterialCopy(mat) and \
            mat.use_nodes

    def findMaterialOutputNode(self, node_tree):
        outputNode = None
        for node in node_tree.nodes:
            if node.type == 'GROUP':
                outputNodeInGroup = self.findMaterialOutputNode(node.node_tree)
                if outputNodeInGroup is not None:
                    outputNode = outputNodeInGroup
            elif node.type == 'OUTPUT_MATERIAL' and len(node.inputs[0].links) == 1 and node.is_active_output:
                outputNode = node
                break

        return outputNode

    def findMaterialOutputNodeAndInput(self, mat):
        outputNode = self.findMaterialOutputNode(mat.node_tree)

        if outputNode is None:
            msb_log(f"Cannot find material output node with a surface input. Cannot bake {mat.name}!", LogLevel.ERROR)
            return None, None

        # Get used shader or whatever is connected to the material output node:
        input = self.traverseReroutes(outputNode.inputs[0].links[0].from_node)
        if input is None:
            msb_log(f"Cannot find material output node with a valid surface input. Cannot bake {mat.name}!",
                    LogLevel.ERROR)
            return outputNode, None

        if input.mute:
            msb_log(f"Input to material output is muted. Cannot bake {mat.name}!", LogLevel.ERROR)
            return outputNode, None

        if input.type in ['HOLDOUT']:
            msb_log(f"Input to material output is an unsupported shader type: {input.type}!", LogLevel.ERROR)
            return outputNode, None

        return outputNode, input

    def deselectAllMaterialNodes(self, mat):
        for node in mat.node_tree.nodes:
            node.select = False

    def isConnectedUpstream(self, nodeA, nodeB):
        '''
        True if nodeB is connected anywhere upstream of nodeA.
        '''
        for input in nodeA.inputs:
            for link in input.links:
                if link.from_node == nodeB or self.isConnectedUpstream(link.from_node, nodeB):
                    return True
        return False

    def cleanUpNodeTreeAndConnectBakedBSDF(self, bakedMat, matOutput):
        node_tree = bakedMat.node_tree
        bakedBSDF = node_tree.nodes[bakedMat[BAKED_MATERIAL_SHADER]]
        matOutput = node_tree.nodes[matOutput.name]
        node_tree.links.new(bakedBSDF.outputs[0], matOutput.inputs[0])

        # The currently selected nodes are used by baked version:
        nodesToDelete = []
        for node in node_tree.nodes:
            if not node.select and node != matOutput and not self.isConnectedUpstream(matOutput, node):
                nodesToDelete.append(node)

        for node in nodesToDelete:
            node_tree.nodes.remove(node)

    def isChannelBakeEnabled(self, context, channel):
        bakeSettings = context.scene.meshsync_bake_settings
        for channelSetting in bakeSettings.bake_channel_settings:
            if channelSetting.name == channel:
                return channelSetting.bakeChannelEnabled
        return False

    def incrementProgress(self, context, message, obj=None, reset=False):
        bakeSettings = context.scene.meshsync_bake_settings
        if reset:
            bakeSettings.bake_progress = 100
            bakeSettings.bake_time_remaining = ""
        else:
            bakeSettings.bake_progress += 100.0 / self.maxBakeProgress * self.getObjectProgressWeight(obj)
            elapsedSeconds = datetime.timedelta(seconds=(time.time() - self.startTime)).total_seconds()

            self.currentBakeProgress += 1

            bakeSettings.bake_maps_remaining = f"Baking map {self.currentBakeProgress}/{self.mapsToBake}"

            # The remaining time is not going to be very precise because
            # it's hard to predict how complex each baking task is.
            # Show approximate times only:
            if elapsedSeconds > 3 and bakeSettings.bake_progress > 0:
                remainingTotalSeconds = int(elapsedSeconds / (bakeSettings.bake_progress / 100) - elapsedSeconds)

                if remainingTotalSeconds <= 60:
                    bakeSettings.bake_time_remaining = "Estimated time left: Less than a minute."
                else:
                    remainingMinutes = remainingTotalSeconds / 60
                    if remainingMinutes > 60:
                        remainingHours = remainingMinutes / 60
                        bakeSettings.bake_time_remaining = f"Estimated time left: {int(remainingHours+1)} hours"
                    else:
                        bakeSettings.bake_time_remaining = f"Estimated time left: {int(remainingMinutes+1)} minutes"
            else:
                bakeSettings.bake_time_remaining = "Calculating remaining time..."

        bakeSettings.bake_message = message

    def addRealizeInstances(self, mod):
        nodes = mod.node_group.nodes
        outputs = [x for x in nodes if x.type == "GROUP_OUTPUT"]

        for output in outputs:
            if len(output.inputs) == 0 or len(output.inputs[0].links) == 0:
                continue

            link = output.inputs[0].links[0]
            realize = nodes.new("GeometryNodeRealizeInstances")
            mod.node_group.links.new(link.from_socket, realize.inputs[0])
            mod.node_group.links.new(realize.outputs[0], link.to_socket)

    def preBakeObject(self, obj):
        '''
        Counts how many textures need to be baked so progress can be calculated.
        :return:
        '''
        if obj.data is None or obj.type != 'MESH':
            return

        context = self.context

        self.selectObject(obj, context)

        if context.object is not None and context.object.mode != 'OBJECT':
            bpy.ops.object.mode_set(mode='OBJECT')

        # Apply modifiers before baking, this is needed because the modifiers can have an impact on material
        # slots and mesh data:
        if len(obj.modifiers) > 0:
            bakeSettings = context.scene.meshsync_bake_settings
            if bakeSettings.apply_modifiers:
                # Can't apply modifiers with shared data:
                bpy.ops.object.make_single_user(type='SELECTED_OBJECTS', obdata=True)
                for mod in obj.modifiers[:]:
                    if bakeSettings.realize_instances and mod.type == "NODES":
                        self.addRealizeInstances(mod)
                    try:
                        bpy.ops.object.modifier_apply(modifier=mod.name)
                    except Exception as e:
                        print(f"Error applying modifier: {e}")
            else:
                msb_log(
                    f"WARNING: Object '{obj.name}' has modifiers but the option to apply modifiers is disabled. The baked material will probably not be correct.",
                    LogLevel.ERROR)

        # Make sure previous bake is undone:
        msb_revertBakedMaterials(obj)

        if not msb_canObjectMaterialsBeBaked(obj):
            return

        for matIndex, matSlot in enumerate(obj.material_slots):
            mat = matSlot.material
            if not self.canMaterialBeBaked(mat):
                continue

            obj.active_material_index = matIndex

            matOutput, bsdf = self.findMaterialOutputNodeAndInput(mat)

            if matOutput is None or bsdf is None:
                continue

            context = self.context
            for channel in BAKED_CHANNELS:
                if not self.isChannelBakeEnabled(context, channel):
                    continue

                if not self.doesBSDFChannelNeedBaking(obj, bsdf, channel)[0]:
                    continue

                self.mapsToBake += 1
                self.maxBakeProgress += self.getObjectProgressWeight(obj)

            obj.material_slots[matIndex].material = mat

    def getObjectProgressWeight(self, obj):
        # The polygon count has some impact on baking duration but not a lot, so scale it down:
        return max(1, int(len(obj.data.polygons) / 10000))

    def bakeObjectMaterials(self, obj, materials):
        context = self.context
        bakeSettings = context.scene.meshsync_bake_settings
        self.finalMaterials = []

        for matIndex, mat in enumerate(materials):
            obj.material_slots[matIndex].material = mat

            if not self.canMaterialBeBaked(mat):
                self.finalMaterials.append(mat)
                continue

            obj.active_material_index = matIndex

            matOutput, bsdf = self.findMaterialOutputNodeAndInput(mat)

            if matOutput is None or bsdf is None:
                self.finalMaterials.append(mat)
                continue

            self.bakedImageNodeYOffset = 0  # To keep track of image node location for this object
            self.objectBakeInfo = {}  # To keep track of baked channels for this object to check if image nodes can be reused

            # Ensure object is not hidden, otherwise baking will fail:
            wasHiddenViewport = obj.hide_get()
            wasHiddenRender = obj.hide_render
            obj.hide_set(False)
            obj.hide_render = False
            context.view_layer.objects.active = obj

            self.deselectAllMaterialNodes(mat)

            msb_log(f"********** Checking if '{mat.name}' on '{obj.name}' needs baked materials. **********")

            # If any channel was baked, it will be on a new material,
            # store that to frame all new nodes after everything is baked:
            bakedMat = mat
            for channel in BAKED_CHANNELS:
                if not self.isChannelBakeEnabled(context, channel):
                    continue

                if bakeSettings.run_modal:
                    yield
                    context = self.context

                didBake, newMat = self.bakeBSDFChannelIfNeeded(context, obj, mat, bsdf, matOutput, channel)
                if didBake:
                    bakedMat = newMat

                if bakeSettings.run_modal:
                    yield
                    context = self.context

            if bakedMat != mat:
                self.cleanUpNodeTreeAndConnectBakedBSDF(bakedMat, matOutput)

            # Needed for restore afterwards:
            self.finalMaterials.append(bakedMat)

            obj.material_slots[matIndex].material = None

            # Restore state:
            obj.select_set(False)
            obj.hide_set(wasHiddenViewport)
            obj.hide_render = wasHiddenRender

            if bakeSettings.run_modal:
                yield
                context = self.context

        if UV_OVERRIDE in obj.data and len(obj.data.uv_layers) > 1:
            msb_log(
                f"New UVs were generated for '{obj.name}' for baking. Old UVs need to be deleted so the baked textures work correctly.")
            bakedUVLayer = obj.data[UV_OVERRIDE]
            for uvLayerIndex in range(len(obj.data.uv_layers) - 1, -1, -1):
                uvLayer = obj.data.uv_layers[uvLayerIndex]
                if uvLayer.name != bakedUVLayer:
                    msb_log(f"Deleting uv layer: {uvLayer.name}")
                    obj.data.uv_layers.remove(uvLayer)
            del obj.data[UV_OVERRIDE]

    def bakeObject(self, obj):
        if not msb_canObjectMaterialsBeBaked(obj):
            return

        context = self.context

        self.selectObject(obj, context)

        msb_log(f"********** Processing object '{obj.name}' **********")

        # We might want to support baking all materials into one:
        bakeIndividualMats = True

        # In order to bake individual materials, we need to remove any other materials.
        # To do this we need to:
        # - Create a list of all materials that are on the object
        # - Clear all material slots on the object but don't delete them
        # - Iterate over the materials and assign each one and bake it
        # - Assign the baked materials to the slots in the order of the original materials
        materials = []
        for matSlot in obj.material_slots:
            materials.append(matSlot.material)

        if bakeIndividualMats:
            for matSlot in obj.material_slots:
                matSlot.material = None

        try:
            for _ in self.bakeObjectMaterials(obj, materials):
                yield
        except Exception as e:
            if throwExceptions:
                raise e
            self.finalMaterials = materials
            msb_log(f"Error: {e}", LogLevel.ERROR)

        if bakeIndividualMats:
            # Restore material slots:
            for matIndex, mat in enumerate(self.finalMaterials):
                obj.material_slots[matIndex].material = mat

    def enableAllCollectionsRecursively(self, col):
        '''
        Enables the view layer and all its children recursively.
        '''
        if col.exclude:
            col.exclude = False
            self.excludedCollections.append(col.name)

        for child in col.children:
            self.enableAllCollectionsRecursively(child)

    def restoreAllCollectionsRecursively(self, col):
        '''
        Disables the view layer and all its children recursively if
        they have been enabled by 'enableAllCollectionsRecursively'.
        '''
        if col.name in self.excludedCollections:
            col.exclude = True
            self.excludedCollections.remove(col.name)

        for child in col.children:
            self.restoreAllCollectionsRecursively(child)

    def children_recursive(self, col):
        for child in col.children:
            yield child
            if len(child.children) > 0:
                yield self.children_recursive(col)

    def bake(self):
        context = self.context

        # Make sure meshsync is finished and ready:
        from .unity_mesh_sync_common import msb_apply_scene_settings, msb_context

        msb_context.flushPendingList()
        msb_apply_scene_settings()
        msb_context.setup(context)

        activeObject = context.object
        selectedObjects = context.selected_objects

        self.objectsProcessedForBaking = []
        bakeSettings = context.scene.meshsync_bake_settings
        bakeSettings.bake_progress = 0

        bakeSelection = bakeSettings.bake_selection

        if bakeSelection == 'ALL':
            objectsToBake = context.scene.objects
        elif bakeSelection == 'SELECTED':
            if len(context.selected_objects) > 0:
                objectsToBake = selectedObjects
            else:
                msb_log("No objects selected, nothing to bake!", LogLevel.ERROR)
                return {'CANCELLED'}

        self.setupRenderSettings(context)

        # Make sure all collections are visible, baking won't work for objects in hidden collections:
        hiddenCollectionsViewport = []
        hiddenCollectionsRender = []
        self.excludedCollections = []

        for col in self.children_recursive(context.scene.collection):
            if col.hide_viewport:
                col.hide_viewport = False
                hiddenCollectionsViewport.append(col.name)
            if col.hide_render:
                col.hide_render = False
                hiddenCollectionsRender.append(col.name)

        for col in context.view_layer.layer_collection.children:
            self.enableAllCollectionsRecursively(col)

        self.maxBakeProgress = 0
        self.currentBakeProgress = 0
        for obj in objectsToBake:
            self.preBakeObject(obj)

        if bakeSettings.run_modal:
            yield

        for obj in objectsToBake:
            for _ in self.bakeObject(obj):
                yield

        # Restore state:
        self.restoreOriginalSettings(context)
        bakeSettings = context.scene.meshsync_bake_settings
        bakeSettings.bake_progress = 0
        context.view_layer.objects.active = activeObject
        for o in context.selected_objects:
            o.select_set(False)
        for o in selectedObjects:
            o.select_set(True)
        for hiddenCollection in hiddenCollectionsViewport:
            for col in self.children_recursive(context.scene.collection):
                if col.name == hiddenCollection:
                    col.hide_viewport = True
        for hiddenCollection in hiddenCollectionsRender:
            for col in self.children_recursive(context.scene.collection):
                if col.name == hiddenCollection:
                    col.hide_render = True
        for col in context.view_layer.layer_collection.children:
            self.restoreAllCollectionsRecursively(col)

    def checkIfUVMapIsNotUV0(self, obj, uvMapName, channel):
        '''
        :param obj: Object the material is on
        :param uvMapName: Name of the UV map
        :param channel: Input channel to process
        :return: [True, Reason] if the given UV map name is not UV0 of the given object. Otherwise [False].
        '''
        if len(uvMapName) == 0:
            return [False]
        else:
            # If there is a UV map set, we need to bake if the object has other UV maps we could use instead:
            if len(obj.data.uv_layers) == 0:
                msb_log(
                    f"Cannot bake '{obj.name}' because it does not have any UV channels but the input for '{channel}' needs a '{uvMapName}' UV map.")
                return [False]

            # If this is UV0 of the object, don't bake:
            if obj.data.uv_layers.find(uvMapName) == -1:
                msb_log(
                    f"Cannot bake '{obj.name}' because it does not have the UV map {uvMapName} that the input for '{channel}' needs.")
                return [False]

            if obj.data.uv_layers[0].name == uvMapName:
                return [False]

            return [True, "Image UV input is not UV0."]

    def doesImageNodeNotUseUv0(self, obj, link, imageNode, channel):
        if link.from_socket.name != 'Color':
            return [True, f"Not using Color output of image node."]

        uvInputSocket = imageNode.inputs['Vector']

        if len(uvInputSocket.links) == 0:
            # It's an image connected to the socket with default UVs, don't bake that:
            return [False]
        else:
            uvCoordNode = uvInputSocket.links[0].from_node
            if uvCoordNode.type != 'UVMAP':
                return [True, "Image input is not a UV map."]

            uvMapName = uvCoordNode.uv_map

            return self.checkIfUVMapIsNotUV0(obj, uvMapName, channel)

    def handleImageNode(self, obj, link, channel, imageNode):
        if imageNode.image.source == 'TILED':
            return [True, "Image is a UDIM tile that needs to be baked."]

        if channel == 'Normal':
            return [True, "Normals require a normal map node as input."]

        return self.doesImageNodeNotUseUv0(obj, link, imageNode, channel)

    def handleNormalNode(self, obj, link, channel, normalMapNode):
        if channel != 'Normal':
            return [True, "The 'normal map' node is only supported as input for normals."]

        uvmapCheck = self.checkIfUVMapIsNotUV0(obj, normalMapNode.uv_map, channel)
        if uvmapCheck[0]:
            return uvmapCheck

        strengthInput = normalMapNode.inputs['Strength']
        if len(strengthInput.links) > 0:
            strengthInputNode = strengthInput.links[0].from_node
            if strengthInputNode.type != 'VALUE':
                return [True, "Normal map strength input is not a constant."]

        colorInput = normalMapNode.inputs['Color']
        if len(colorInput.links) > 0:
            colorInputNode = colorInput.links[0].from_node
            if colorInputNode.type != 'TEX_IMAGE':
                return [True, "Normal map input is not an image."]

            if colorInput.links[0].from_socket.name != 'Color':
                return [True, "Non-color channel of texture is used as normal map input."]

            return self.doesImageNodeNotUseUv0(obj, colorInput.links[0], colorInputNode, channel)

        return [False]

    def doesBSDFChannelNeedBaking(self, obj, bsdf,
                                  channel: str) -> list:
        if channel == AO_CHANNEL_NAME:
            return [True, "Ambient occlusion requires baking."]

        if bsdf is None:
            return [True, "Material output is not connected to a shader."]

        if not self.canBsdfBeBaked(bsdf):
            return [True, f"Material output is not connected to a supported shader but a node of type: {bsdf.type}."]

        bsdfInputName = self.getBSDFChannelInputName(bsdf, channel)
        if bsdfInputName is None:
            return [False]

        inputSocket = bsdf.inputs[bsdfInputName]

        # If there's nothing connected to the socket, we can use the socket's default value.
        if len(inputSocket.links) == 0:
            return [False]

        link = inputSocket.links[0]
        nodeConnectedToChannelSocket = link.from_node

        type = nodeConnectedToChannelSocket.type
        
        # These can be exported by meshsync from their default value:
        if type in ['RGB', 'VALUE']:
            return [False]

        if type == 'TEX_IMAGE':
            return self.handleImageNode(obj, link, channel, nodeConnectedToChannelSocket)
        elif type == 'NORMAL_MAP':
            return self.handleNormalNode(obj, link, channel, nodeConnectedToChannelSocket)

        return [True, "Node input is procedural."]

    def bakeBSDFChannelIfNeeded(self, context, obj, mat, bsdf, matOutput, channel):
        result = self.doesBSDFChannelNeedBaking(obj, bsdf, channel)
        if result[0]:
            msb_log(f"Baking {channel} for '{obj.name}'. Reason: {result[1]}")
            bakedMat = self.bakeChannel(context, obj, mat, bsdf, matOutput, channel)
            return True, bakedMat

        return False, mat

    def getNextPowerOf2(self, number):
        return 2**(number - 1).bit_length()

    def getTextureDimensions(self, context, obj):
        bakeSettings = context.scene.meshsync_bake_settings

        if bakeSettings.baked_texture_dimensions == 'PIXELS':
            return bakeSettings.bakedTextureSize
        else:
            # For each face, get the area in space and UV space
            mesh = obj.data

            import numpy as np
            uv_layer = mesh.uv_layers.active.data

            def polyArea(x, y):
                return 0.5 * np.abs(np.dot(x, np.roll(y, 1)) - np.dot(y, np.roll(x, 1)))

            # Add up uvArea / polygon area and divide by number of polygons to get the average ratio
            # and calculate the texture dimensions based on the texel density from that:
            ratioSum = 0

            for poly in mesh.polygons:
                # calculate UV area:
                x = []
                y = []

                # Put vertices of the polygon into lists to calculate the UV area:
                for loop_index in range(poly.loop_start, poly.loop_start + poly.loop_total):
                    uv = uv_layer[loop_index].uv
                    x.append(uv[0])
                    y.append(uv[1])

                uvArea = polyArea(x, y)
                pArea = poly.area

                # Avoid division by 0:
                if pArea <= 0.000001:
                    pArea = 0.001

                ratioSum += math.sqrt(uvArea / pArea)

            # Calculate average texel density for each face:
            avgDensity = ratioSum / len(mesh.polygons)

            # Calculate how large the texture needs to be to get the desired texel density:
            dims = max(1, int(bakeSettings.texel_density / avgDensity))

            if bakeSettings.texel_density_pot:
                dims = self.getNextPowerOf2(dims)

            dims = min(dims, bakeSettings.texel_density_limit)
            dims = (dims, dims)
            msb_log(f"Calculated texture size: {dims}", LogLevel.VERBOSE)

            return dims

    def createImage(self, context, obj, name, colorSpace, alpha=False):
        imageName = name.replace(" ", "_")

        # Delete any existing image with this name to ensure the dimensions and alpha settings are correct:
        existingImageIndex = bpy.data.images.find(imageName)
        if existingImageIndex >= 0:
            bpy.data.images.remove(bpy.data.images[existingImageIndex])

        texDims = self.getTextureDimensions(context, obj)

        result = bpy.data.images.new(imageName,
                                     width=texDims[0],
                                     height=texDims[1],
                                     alpha=alpha)
        result.colorspace_settings.name = colorSpace

        return result

    def prepareObjectForBaking(self, context, obj):
        if obj in self.objectsProcessedForBaking:
            return

        self.objectsProcessedForBaking.append(obj)

        bakeSettings = context.scene.meshsync_bake_settings

        # UVs:
        generateUVs = bakeSettings.generate_uvs == 'ALWAYS' or len(obj.data.uv_layers) == 0

        if generateUVs:
            if bakeSettings.generate_uvs == 'OFF':
                raise Exception(
                    f"Object: '{obj.name}' has no UVs. Automatically generating UVs is disabled, so this object cannot be baked!")
        else:
            # Even though there are UVs, they might not be useful for baking.
            # Make sure they're not all in the same spot and in the 0..1 range:
            # Note: This does not prevent overlapping UVs or faces with no area but that's up to the user to fix:
            import numpy as np
            uvOutOfBounds = False
            for i in range(len(obj.data.uv_layers)):
                uv_map = obj.data.uv_layers[i]
                if uv_map.active:
                    nl = len(obj.data.loops)
                    uv_verts = np.zeros(nl * 2)
                    uv_map.data.foreach_get("uv", uv_verts)
                    uniqueUVs = np.unique(uv_verts.round(decimals=4))
                    greater = np.any(np.greater(uniqueUVs, 1))
                    if greater:
                        uvOutOfBounds = True
                        break
                    less = np.any(np.less(uniqueUVs, 0))
                    if less:
                        uvOutOfBounds = True
                        break
                    break

            if uvOutOfBounds:
                if bakeSettings.generate_uvs == 'OFF':
                    raise Exception(
                        f"Object: '{obj.name}' has no usable UVs. Automatically generating UVs is disabled, so this object cannot be baked!")

                msb_log("UVs are not in 0..1 range for baking, generating new UVs.")
                generateUVs = True

        if generateUVs:
            msb_log(f"Auto generating UVs for object: '{obj.name}'.")

            bakeUVLayer = obj.data.uv_layers.new(name="Baked")
            obj.data.uv_layers.active = bakeUVLayer
            # Store uv override on the mesh, we'll need this later to delete all other UV layers,
            # otherwise the baked images will use the wrong UVs!
            obj.data[UV_OVERRIDE] = bakeUVLayer.name

            if context.object.mode == 'OBJECT':
                bpy.ops.object.mode_set(mode='EDIT')

            bpy.ops.mesh.select_mode(use_extend=False, use_expand=False, type='VERT')
            bpy.ops.mesh.select_all(action='SELECT')
            bpy.ops.mesh.select_linked(delimit={'SEAM'})
            bpy.ops.uv.smart_project(island_margin=0.01, scale_to_bounds=True)
            bpy.ops.uv.pack_islands(rotate=True, margin=0.001)

        if UV_OVERRIDE in obj.data:
            obj.data.uv_layers.active = obj.data.uv_layers[obj.data[UV_OVERRIDE]]
        else:
            obj.data.uv_layers.active = obj.data.uv_layers[0]

        if obj.mode == 'EDIT':
            bpy.ops.object.mode_set(mode='OBJECT')

    def getNodeYLocation(self, node):
        location = node.location[1]
        if node.parent is not None:
            location += self.getNodeYLocation(node.parent)
        return location

    def prepareMaterial(self, context, obj, bsdf, mat, canBakeBSDF):
        '''
        Creates a material copy for baking if necessary.

        Keeps the original material so the bake can be reverted.
        The material copy is modified to bake the individual channels by connecting them
        directly to the material output and baking it as emission.
        After all the baking is complete, all nodes in the material copy,
        except the BSDF and material output and baked images are deleted so the material only uses the baked images.
        
        :return: Material copy for baking or the material if it's already a baking copy.
        '''
        # If this is a material copy, do not copy it again:
        if ORIGINAL_MATERIAL in mat:
            return mat

        # Use existing copy if there is one:
        matCopyName = f"{mat.name}_{obj.name}_baked"

        matCopyIndex = bpy.data.materials.find(matCopyName)
        if matCopyIndex >= 0:
            matCopy = bpy.data.materials[matCopyIndex]

            # Replace material with its baking copy:
            matIndex = obj.material_slots.find(mat.name)
            obj.material_slots[matIndex].material = matCopy

            return matCopy

        # Make material copy for baking:
        mat.use_fake_user = True  # Make sure this does not get deleted when it's not referenced anymore
        matCopy = mat.copy()
        matCopy[ORIGINAL_MATERIAL] = mat.name
        matCopy.name = matCopyName

        # Replace material with its baking copy:
        matIndex = obj.material_slots.find(mat.name)
        obj.material_slots[matIndex].material = matCopy

        # Ungroup all node groups for easy, error-free access:
        for node in matCopy.node_tree.nodes:
            node.select = False

        # Need to set the context area type for the group_ungroup operator to work:
        area = self.area
        old_type = area.type
        area.ui_type = 'ShaderNodeTree'

        space = area.spaces.active
        space.node_tree = matCopy.node_tree

        for node in matCopy.node_tree.nodes:
            if node.type == 'GROUP':
                node.select = True
                with context.temp_override(area=area):
                    bpy.ops.node.group_ungroup()
                node.select = False

        for node in matCopy.node_tree.nodes:
            node.select = False

        # Restore context area type:
        area.type = old_type

        msb_log(f"Creating material copy '{mat.name}'->'{matCopy.name}'")

        # Use same BSDF type if we can bake its inputs,
        # otherwise connect the fallback baked maps to principled bsdf:
        if canBakeBSDF:
            bakedBSDF = matCopy.node_tree.nodes.new(type=bsdf.bl_idname)
        else:
            bakedBSDF = matCopy.node_tree.nodes.new(type='ShaderNodeBsdfPrincipled')

        # Copy the input settings from the original bsdf so anything that's not baked still matches:
        for input in bsdf.inputs:
            bakedBSDFInputName = self.getBSDFChannelInputName(bakedBSDF, input.name)
            if bakedBSDFInputName is None:
                continue

            bakedBSDF.inputs[input.name].default_value = input.default_value

        # Find the lowest node in the tree and put the baked bsdf under that:
        minYLocation = bsdf.location[1]
        for node in matCopy.node_tree.nodes:
            minYLocation = min(minYLocation, self.getNodeYLocation(node))

        bakedBSDF.location = bsdf.location  # (bsdf.location[0], minYLocation - 1000)
        # Give bsdf a name and set its name on the material, so we can find it again:
        bakedBSDF.name = BAKED_MATERIAL_SHADER
        matCopy[BAKED_MATERIAL_SHADER] = bakedBSDF.name

        # Copy inputs from original BSDF if they match:
        for input in bsdf.inputs:
            if len(input.links) > 0 and input.name in bakedBSDF.inputs:
                inputLink = input.links[0]
                inputNode = matCopy.node_tree.nodes[inputLink.from_node.name]
                inputSocket = inputNode.outputs[inputLink.from_socket.name]
                matCopy.node_tree.links.new(inputSocket, bakedBSDF.inputs[input.name])

        return matCopy

    def selectObject(self, obj, context):
        for ob in context.selected_objects:
            ob.select_set(False)
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj

    def prepareBake(self, context, obj, bsdf, mat, canBakeBSDF):
        if context.object is not None:
            bpy.ops.object.mode_set(mode='OBJECT')

        self.selectObject(obj, context)

        self.prepareObjectForBaking(context, obj)
        mat = self.prepareMaterial(context, obj, bsdf, mat, canBakeBSDF)

        return mat

    def setupRenderSettings(self, context):
        self.originalSceneSettings = []

        self.setRestorableContextSetting(context, "scene.render.engine", "CYCLES")
        if context.preferences.addons['cycles'].preferences.has_active_device():
            self.setRestorableContextSetting(context, "scene.cycles.device", "GPU")

        if context.preferences.addons["cycles"].preferences.compute_device_type == "None":
            msb_log("The cycles render device is not set. Baking would be faster if this is set to CUDA, OptiX or HIP.")

        self.setRestorableContextSetting(context, "scene.cycles.samples",
                                         10)  # TODO: This could be a setting for the user to change

        self.setRestorableContextSetting(context, "scene.render.bake.use_pass_direct", False)
        self.setRestorableContextSetting(context, "scene.render.bake.use_pass_indirect", False)
        self.setRestorableContextSetting(context, "scene.render.bake.use_pass_color", True)
        self.setRestorableContextSetting(context, "scene.render.bake.image_settings.file_format", 'PNG')
        self.setRestorableContextSetting(context, "scene.render.bake.use_selected_to_active", False)
        self.setRestorableContextSetting(context, "scene.render.bake.use_cage", False)

        # Set Correct Colour space for bake
        self.setRestorableContextSetting(context, "scene.display_settings.display_device", 'sRGB')
        self.setRestorableContextSetting(context, "scene.view_settings.view_transform", 'Standard')
        self.setRestorableContextSetting(context, "scene.view_settings.look", 'None')
        self.setRestorableContextSetting(context, "scene.view_settings.exposure", 0)
        self.setRestorableContextSetting(context, "scene.view_settings.gamma", 1)
        self.setRestorableContextSetting(context, "scene.sequencer_colorspace_settings.name", 'sRGB')
        self.setRestorableContextSetting(context, "scene.cycles.time_limit", 0)
        self.setRestorableContextSetting(context, "scene.cycles.use_auto_tile", True)

        self.setRestorableContextSetting(context, "scene.cycles.max_bounces", 12)
        self.setRestorableContextSetting(context, "scene.cycles.diffuse_bounces", 8)
        self.setRestorableContextSetting(context, "scene.cycles.glossy_bounces", 2)
        self.setRestorableContextSetting(context, "scene.cycles.transparent_max_bounces", 0)
        self.setRestorableContextSetting(context, "scene.cycles.transmission_bounces", 2)
        self.setRestorableContextSetting(context, "scene.cycles.volume_bounces", 0)

    def setRestorableContextSetting(self, context, settingName, value):
        '''
        Allows to set nested settings and record their original value to restore them after baking.
        '''
        self.originalSceneSettings.append((settingName, msb_rgetattr(context, settingName)))

        msb_rsetattr(context, settingName, value)
        pass

    def restoreOriginalSettings(self, context):
        '''
        Restores original settings that were set using 'setRestorableContextSetting'.
        '''
        for originalSettingName, originalSettingValue in self.originalSceneSettings:
            msb_rsetattr(context, originalSettingName, originalSettingValue)

    def traverseReroutes(self, nodeOrSocket):
        '''
        Goes upstream until it finds a node or node socket that is not a reroute node.
        :param nodeOrSocket: node or node socket
        :return: upstream input the node or node socket resolves to after reroutes
        '''

        if isinstance(nodeOrSocket, bpy.types.NodeSocket):
            isSocket = True
            node = nodeOrSocket.node
        else:
            isSocket = False
            node = nodeOrSocket

        if node.mute:
            return None

        if node is None or node.type != 'REROUTE':
            return nodeOrSocket

        if len(node.inputs[0].links) == 0:
            return None

        if isSocket:
            result = self.traverseReroutes(node.inputs[0].links[0].from_socket)
        else:
            result = self.traverseReroutes(node.inputs[0].links[0].from_node)

        return result

    def canBsdfBeBaked(self, bsdf):
        if bsdf.type in ['EMISSION', 'SUBSURFACE_SCATTERING']:
            return True

        return 'bsdf' in bsdf.type.lower()

    def bakeToImage(self, context, obj, mat, bsdf, bakeType, channel):
        colorSpace = self.getChannelColourSpace(channel)

        bakeImage = self.createImage(context, obj, f"{mat.name}_{channel.lower()}", colorSpace,
                                     alpha=(colorSpace == 'sRGB'))

        node_tree = mat.node_tree

        # Set up image node
        bakedImageNode = node_tree.nodes.new("ShaderNodeTexImage")
        bakedImageNode.select = True
        node_tree.nodes.active = bakedImageNode
        bakedImageNode.image = bakeImage
        bakedImageNode.location = (bsdf.location[0] - 500, bsdf.location[1] - self.bakedImageNodeYOffset)
        self.bakedImageNodeYOffset += 300

        # Bake
        msb_log("Baking in progress...")
        bpy.ops.object.bake(type=bakeType, use_clear=True, use_selected_to_active=False, use_split_materials=True)
        bakeImage.filepath_raw = os.path.join(context.scene.meshsync_bake_settings.bakedTexturesPath,
                                              bakeImage.name + ".png")
        bakeImage.file_format = "PNG"
        bakeImage.save()
        bakeImage.colorspace_settings.name = colorSpace

        return bakedImageNode

    def getChannelColourSpace(self, channel):
        if channel in ["Base Color", "Color"]:
            return 'sRGB'
        else:
            return 'Non-Color'

    def bakeChannelInputsDirectly(self, context, obj, mat, bsdf, matOutput, channel) -> bpy.types.ShaderNodeTexImage:
        '''
        Connects BSDF channel input directly to the material output and bakes it.
        :return: Image node that contains the baked image
        '''
        bsdfChannelSocketName = self.getBSDFChannelInputName(bsdf, channel)

        node_tree = mat.node_tree
        link = mat.node_tree.links.new

        # Make sure bsdf points to material copy:
        bsdf = node_tree.nodes[bsdf.name]
        bakedBSDF = node_tree.nodes[mat[BAKED_MATERIAL_SHADER]]
        matOutput = node_tree.nodes[matOutput.name]

        # Normals cannot be baked this way, use what the material output receives instead:
        if channel == 'Normal':
            link(bsdf.outputs[0], matOutput.inputs[0])
            return self.bakeWithFallback(context, obj, mat, channel)

        bsdfChannelSocket = bsdf.inputs[bsdfChannelSocketName]

        channelInput = self.traverseReroutes(bsdfChannelSocket.links[0].from_socket)

        if channelInput in self.objectBakeInfo:
            msb_log(f"Input for channel {bsdfChannelSocketName} was already baked, reusing the same image.")
            return self.objectBakeInfo[channelInput]

        msb_log(f"Baking inputs of channel: '{bsdfChannelSocketName}'.")

        link(channelInput, matOutput.inputs[0])

        bakedImageNode = self.bakeToImage(context, obj, mat, bakedBSDF, 'EMIT', channel)

        self.objectBakeInfo[channelInput] = bakedImageNode

        return bakedImageNode

    def bakeWithFallback(self, context, obj, mat, channel):
        '''
        Does default blender bake of the given channel.
        :return: Image node that contains the baked image
        '''
        if channel in channelNameToBakeName:
            bakeType = channelNameToBakeName[channel]
        else:
            msb_log(
                f"Unable to bake {channel} for {mat.name} on {obj.name}. The channel is not supported in fallback mode.\n")
            return None

        msb_log(f"Baking {channel} in fallback mode as {bakeType}")

        node_tree = mat.node_tree
        bsdf = node_tree.nodes[mat[BAKED_MATERIAL_SHADER]]

        return self.bakeToImage(context, obj, mat, bsdf, bakeType, channel)

    def getChannelNameSynonyms(self, channel):
        '''
        Returns possible other names for the given channel because some BSDFs use other names for the same channel.
        '''
        if channel in synonymMap:
            return synonymMap[channel]
        return []

    def getBSDFChannelInputName(self, bsdf, channel):
        '''
        Returns the name of the input for the given channel on the bsdf.
        None if the BSDF does not support the input.
        '''
        if channel in bsdf.inputs:
            return channel

        for c in self.getChannelNameSynonyms(channel):
            if c in bsdf.inputs:
                return c

        return None

    def bakeChannel(self, context, obj, mat, bsdf, matOutput, channel):
        self.incrementProgress(context, f"Baking '{mat.name}'->{channel} on '{obj.name}'", obj)

        canBakeBSDF = self.canBsdfBeBaked(bsdf)

        mat = self.prepareBake(context, obj, bsdf, mat, canBakeBSDF)

        # AO cannot be baked from inputs:
        if channel == AO_CHANNEL_NAME:
            bakedImageNode = self.bakeWithFallback(context, obj, mat, channel)
            bakedImageNode.name = "BAKED_AO"
        else:
            # Ideally, we should bake the BSDF inputs:
            if canBakeBSDF:
                bakedImageNode = self.bakeChannelInputsDirectly(context, obj, mat, bsdf, matOutput, channel)
            else:
                bakedImageNode = self.bakeWithFallback(context, obj, mat, channel)

        if not bakedImageNode:
            return mat

        # Connect baked image to baked bsdf node:
        node_tree = mat.node_tree
        bsdf = node_tree.nodes[mat[BAKED_MATERIAL_SHADER]]
        inputChannelName = self.getBSDFChannelInputName(bsdf, channel)

        if channel != AO_CHANNEL_NAME:
            # Connect baked image to the input now:
            if inputChannelName == 'Normal':
                normalMapNode = node_tree.nodes.new("ShaderNodeNormalMap")
                normalMapNode.location = (bakedImageNode.location[0], bakedImageNode.location[1])
                bakedImageNode.location = (bakedImageNode.location[0] - 300, bakedImageNode.location[1])

                node_tree.links.new(bakedImageNode.outputs[0], normalMapNode.inputs['Color'])
                node_tree.links.new(normalMapNode.outputs[0], bsdf.inputs[inputChannelName])
            else:
                node_tree.links.new(bakedImageNode.outputs[0], bsdf.inputs[inputChannelName])

        return mat

    def execute(self, context):
        self.startTime = time.time()
        self.context = context
        self.bakeTask = self.bake()

        return {'RUNNING_MODAL'}

    def stop(self, context):
        wm = context.window_manager
        wm.event_timer_remove(self.timer)

    def invoke(self, context, event):
        if not os.access(context.scene.meshsync_bake_settings.bakedTexturesPath, os.W_OK):
            self.report({'WARNING'}, "The folder to save baked textures to does not exist!")
            return {'CANCELLED'}

        wm = context.window_manager
        self.timer = wm.event_timer_add(0, window=context.window)
        context.window_manager.modal_handler_add(self)

        # Store area for later use, these won't exist on modal timer callbacks:
        self.area = context.area

        return self.execute(context)

    def modal(self, context, event):
        # Allow cancellation by pressing escape:
        if event.type == 'ESC':
            self.stop(context)
            self.incrementProgress(context, "Baking cancelled by user", reset=True)
            return {'FINISHED'}

        # Refresh context each run:
        self.context = context
        for _ in self.bakeTask:
            return {'RUNNING_MODAL'}

        msb_log(f"Finished baking. Time taken: {datetime.timedelta(seconds=(time.time() - self.startTime))}",
                LogLevel.ERROR)

        self.stop(context)
        return {'FINISHED'}


class MESHSYNC_OT_select_bake_folder(bpy.types.Operator, ExportHelper):
    bl_idname = "meshsync.choose_material_bake_folder"
    bl_label = "Choose folder for baked textures"

    filename_ext = ""

    def execute(self, context):
        context.scene.meshsync_bake_settings.bakedTexturesPath = os.path.dirname(self.properties.filepath)
        return {'FINISHED'}


def msb_revertBakedMaterials(obj):
    materialsToDelete = set()

    if not msb_canObjectMaterialsBeBaked(obj):
        return

    for matSlot in obj.material_slots:
        mat = matSlot.material
        if mat is None:
            continue

        if ORIGINAL_MATERIAL in mat:
            origMatName = mat[ORIGINAL_MATERIAL]
            if origMatName not in bpy.data.materials:
                msb_log(
                    f"Cannot revert bake for material '{mat.name}' on '{obj.name}'. Original material '{origMatName}' does not exist.",
                    LogLevel.ERROR)
                continue

            origMat = bpy.data.materials[origMatName]
            matSlot.material = origMat
            materialsToDelete.add(mat)

    for mat in materialsToDelete:
        bpy.data.materials.remove(mat)


class MESHSYNC_OT_RevertBake(bpy.types.Operator):
    bl_idname = "meshsync.revert_bake_materials"
    bl_label = "Restore original materials"
    bl_description = "Removes any baked materials and restores the original materials on all objects."

    def execute(self, context):
        from .unity_mesh_sync_common import msb_apply_scene_settings, msb_context

        msb_context.flushPendingList()
        msb_apply_scene_settings()
        msb_context.setup(context)

        bakeSettings = context.scene.meshsync_bake_settings
        bakeSettings.bake_progress = 0

        for obj in context.scene.objects:
            msb_revertBakedMaterials(obj)

        return {'FINISHED'}
