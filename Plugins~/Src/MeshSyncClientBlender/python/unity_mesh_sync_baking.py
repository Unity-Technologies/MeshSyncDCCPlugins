import bpy, os, datetime, time
from bpy_extras.io_utils import ExportHelper
from bpy.app.handlers import persistent
import functools

from .unity_mesh_sync_common import MESHSYNC_PT

# Constants:
ORIGINAL_MATERIAL = 'ORIGINAL_MATERIAL'
BAKED_MATERIAL_SHADER = 'BAKED_MATERIAL_SHADER'
UV_OVERRIDE = 'UV_OVERRIDE'

# Commented out ones will be supported in next version:
BAKED_CHANNELS = ["Base Color",
                  "Metallic",
                  "Roughness",
                  "Clearcoat",
                  "Emission",
                  "Normal"]

# The above channels may not exist in all BSDF types, mapping of alternative names:
synonymMap = {"Base Color": ["Color"]}

channelNameToBakeName = {
    'Base Color': 'DIFFUSE',
    'Color': 'DIFFUSE',
    'Roughness': 'ROUGHNESS',
    'Normal': 'NORMAL',
}


def msb_canObjectMaterialsBeBaked(obj: bpy.types.Object) -> bool:
    hasMaterials = obj.data is not None and hasattr(obj.data, 'materials')
    if not hasMaterials:
        return False
    # If it's a mesh, make sure it actually has vertices, or we'll get errors baking it later:
    if obj.type == 'MESH':
        if len(obj.data.vertices) == 0:
            return False

    return True

# Methods to help getting and setting nested attributes:
def rsetattr(obj, attr, val):
    pre, _, post = attr.rpartition('.')
    return setattr(rgetattr(obj, pre) if pre else obj, post, val)

def rgetattr(obj, attr, *args):
    def _getattr(obj, attr):
        return getattr(obj, attr, *args)

    return functools.reduce(_getattr, [obj] + attr.split('.'))

class MESHSYNC_BakeChannelSetting(bpy.types.PropertyGroup):
    name: bpy.props.StringProperty(name="")
    bakeChannelEnabled: bpy.props.BoolProperty(name="", description="Whether to bake this channel or not", default=True)


def msb_bakeAllChanged(self, context):
    for channelSetting in context.scene.meshsync_bake_channel_settings:
        channelSetting.bakeChannelEnabled = context.scene.meshsync_bake_all_channels


def msb_bakeAllGet(scene):
    for channelSetting in scene.meshsync_bake_channel_settings:
        if not channelSetting.bakeChannelEnabled:
            return False
    return True


def msb_bakeAllSet(scene, newValue):
    for channelSetting in scene.meshsync_bake_channel_settings:
        channelSetting.bakeChannelEnabled = newValue


@persistent
def msb_setBakingDefaults(dummy):
    context = bpy.context
    if len(context.scene.meshsync_bake_channel_settings) != len(BAKED_CHANNELS):
        context.scene.meshsync_bake_channel_settings.clear()
        for channel in BAKED_CHANNELS:
            channelSettings = context.scene.meshsync_bake_channel_settings.add()
            channelSettings.name = channel


class MESHSYNC_PT_Baking(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Baking"
    bl_parent_id = "MESHSYNC_PT_Main"
    bl_order = -10

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False

        layout.prop(context.scene, "meshsync_bake_selection", expand=True)

        box = layout.box()
        box.alignment = 'LEFT'
        box.label(text="Material channels to bake")
        box.prop(context.scene, "meshsync_bake_all_channels")
        for channelSetting in context.scene.meshsync_bake_channel_settings:
            row = box.row()
            row.prop(channelSetting, "bakeChannelEnabled", text=channelSetting.name)

        layout.prop(context.scene, "meshsync_generate_uvs", expand=True)
        layout.operator("meshsync.bake_materials")
        row = layout.row()
        row.prop(context.scene, "meshsync_bakedTexturesPath")
        row.operator("meshsync.choose_material_bake_folder", icon="FILE_FOLDER", text="")
        layout.prop(context.scene, "meshsync_bakedTextureSize")
        layout.operator("meshsync.revert_bake_materials")



class MESHSYNC_OT_Bake(bpy.types.Operator):
    bl_idname = "meshsync.bake_materials"
    bl_label = "Bake to individual materials"
    bl_description = "Bakes textures, creates material copies and assigns the baked materials for all materials that " \
                     "cannot be exported without baking them to textures"

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
            elif node.type == 'OUTPUT_MATERIAL' and len(node.inputs[0].links) == 1:
                outputNode = node
                # Blender uses the last OUTPUT_MATERIAL node, so don't stop search here

        return outputNode

    def findMaterialOutputNodeAndInput(self, mat):
        outputNode = self.findMaterialOutputNode(mat.node_tree)

        if outputNode is None:
            print(f"Cannot find material output node with a surface input. Cannot bake {mat.name}!")
            return None, None

        # Get used shader or whatever is connected to the material output node:
        input = self.traverseReroutes(outputNode.inputs[0].links[0].from_node)
        if input is None:
            print(f"Cannot find material output node with a valid surface input. Cannot bake {mat.name}!")
            return outputNode, None

        if input.mute:
            print(f"Input to material output is muted. Cannot bake {mat.name}!")
            return outputNode, None

        if input.type in ['HOLDOUT']:
            print(f"Input to material output is an unsupported shader type: {input.type}!")
            return outputNode, None

        return outputNode, input

    def deselectAllMaterialNodes(self, mat):
        for node in mat.node_tree.nodes:
            node.select = False

    def cleanUpNodeTreeAndConnectBakedBSDF(self, bakedMat, matOutput):
        node_tree = bakedMat.node_tree
        bakedBSDF = node_tree.nodes[bakedMat[BAKED_MATERIAL_SHADER]]
        matOutput = node_tree.nodes[matOutput.name]
        node_tree.links.new(bakedBSDF.outputs[0], matOutput.inputs[0])

        # The currently selected nodes are used by baked version:
        selected = []
        nodesToDelete = []
        for node in node_tree.nodes:
            if node.select or node == matOutput:
                selected.append(node)
            else:
                nodesToDelete.append(node)

        for node in nodesToDelete:
            node_tree.nodes.remove(node)

    def isChannelBakeEnabled(self, context, channel):
        for channelSetting in context.scene.meshsync_bake_channel_settings:
            if channelSetting.name == channel:
                return channelSetting.bakeChannelEnabled
        return False

    def bakeObject(self, context, obj):
        if not msb_canObjectMaterialsBeBaked(obj):
            return

        print(f"********** Processing object '{obj.name}' **********")

        # We might want to support baking all materials into one:
        bakeIndividualMats = True

        # In order to bake individual materials, we need to remove any other materials.
        # To do this we need to:
        # - Create a list of all materials that are on the object
        # - Clear all material slots on the object but don't delete them
        # - Iterate over the materials and assign each one and bake it
        # - Assign the baked materials to the slots in the order of the original materials
        materials = []
        finalMaterials = []
        for matSlot in obj.material_slots:
            materials.append(matSlot.material)

        if bakeIndividualMats:
            for matSlot in obj.material_slots:
                matSlot.material = None

        try:
            for matIndex, mat in enumerate(materials):
                obj.material_slots[matIndex].material = mat

                if not self.canMaterialBeBaked(mat):
                    continue

                obj.active_material_index = matIndex

                self.bakedImageNodeYOffset = 0

                matOutput, bsdf = self.findMaterialOutputNodeAndInput(mat)

                if matOutput is None or bsdf is None:
                    continue

                # Ensure object is not hidden, otherwise baking will fail:
                wasHidden = obj.hide_get()
                obj.hide_set(False)
                context.view_layer.objects.active = obj

                self.deselectAllMaterialNodes(mat)

                print(f"********** Checking if '{mat.name}' on '{obj.name}' needs baked materials. **********")

                # If any channel was baked, it will be on a new material,
                # store that to frame all new nodes after everything is baked:
                bakedMat = mat
                for channel in BAKED_CHANNELS:
                    if not self.isChannelBakeEnabled(context, channel):
                        continue

                    didBake, newMat = self.bakeBSDFChannelIfNeeded(context, obj, mat, bsdf, matOutput, channel)
                    if didBake:
                        bakedMat = newMat

                if bakedMat != mat:
                    print(f"Baked '{mat.name}' on '{obj.name}'.\n")
                    self.cleanUpNodeTreeAndConnectBakedBSDF(bakedMat, matOutput)

                # Needed for restore afterwards:
                finalMaterials.append(bakedMat)

                obj.material_slots[matIndex].material = None

                # Restore state:
                obj.select_set(False)
                obj.hide_set(wasHidden)

            if UV_OVERRIDE in obj.data and len(obj.data.uv_layers) > 1:
                print(f"New UVs were generated for '{obj.name}' for baking. Old UVs need to be deleted so the baked textures work correctly.")
                bakedUVLayer = obj.data[UV_OVERRIDE]
                for uvLayerIndex in range(len(obj.data.uv_layers) - 1, -1, -1):
                    uvLayer = obj.data.uv_layers[uvLayerIndex]
                    if uvLayer.name != bakedUVLayer:
                        print(f"Deleting uv layer: {uvLayer.name}")
                        obj.data.uv_layers.remove(uvLayer)
                del obj.data[UV_OVERRIDE]
        except Exception as e:
            finalMaterials = materials
            print(f"Error: {e}")

        if bakeIndividualMats:
            # Restore material slots:
            for matIndex, mat in enumerate(finalMaterials):
                obj.material_slots[matIndex].material = mat

    def bakeAll(self, context):
        for obj in context.scene.objects:
            self.bakeObject(context, obj)

    def bake(self, context):
        # Make sure meshsync is finished and ready:
        from .unity_mesh_sync_common import msb_apply_scene_settings, msb_context

        # if not msb_context.is_setup:
        msb_context.flushPendingList()
        msb_apply_scene_settings()
        msb_context.setup(context)

        activeObject = context.object

        self.objectsProcessedForUVs = []

        self.setupRenderSettings(context)

        bakeSelection = context.scene.meshsync_bake_selection
        if bakeSelection == 'ALL':
            self.bakeAll(context)
        elif bakeSelection == 'SELECTED':
            if len(context.selected_objects) > 0:
                for obj in context.selected_objects:
                    self.bakeObject(context, obj)
            else:
                print("No objects selected, nothing to bake!")

        # Restore state:
        self.restoreOriginalSettings(context)
        context.view_layer.objects.active = activeObject

    def checkForUV0(self, obj, uvMapName, channel):
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
                print(
                    f"Cannot bake '{obj.name}' because it does not have any UV channels but the input for '{channel}' needs a '{uvMapName}' UV map.")
                return [False]

            # If this is UV0 of the object, don't bake:
            if obj.data.uv_layers.find(uvMapName) == -1:
                print(
                    f"Cannot bake '{obj.name}' because it does not have the UV map {uvMapName} that the input for '{channel}' needs.")
                return [False]

            if obj.data.uv_layers[0].name == uvMapName:
                return [False]

            return [True, "Image UV input is not UV0."]

    def doesImageNodeUseUv0(self, obj, link, imageNode, channel):
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

            return self.checkForUV0(obj, uvMapName, channel)

    def handleImageNode(self, obj, link, channel, imageNode):
        if channel == 'Normal':
            return [True, "Normals require a normal map node as input."]

        return self.doesImageNodeUseUv0(obj, link, imageNode, channel)

    def handleNormalNode(self, obj, link, channel, normalMapNode):
        if channel != 'Normal':
            return [True, "The 'normal map' node is only supported as input for normals."]

        uvmapCheck = self.checkForUV0(obj, normalMapNode.uv_map, channel)
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

            return self.doesImageNodeUseUv0(obj, colorInput.links[0], colorInputNode, channel)

        return [False]

    def doesBSDFChannelNeedBaking(self, obj, bsdf,
                                  channel: str) -> list:
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

        result = None

        if nodeConnectedToChannelSocket.type == 'TEX_IMAGE':
            result = self.handleImageNode(obj, link, channel, nodeConnectedToChannelSocket)
        elif nodeConnectedToChannelSocket.type == 'NORMAL_MAP':
            result = self.handleNormalNode(obj, link, channel, nodeConnectedToChannelSocket)

        if result is None:
            result = [True, "Node input is procedural."]

        return result

    def bakeBSDFChannelIfNeeded(self, context, obj, mat, bsdf, matOutput, channel):
        result = self.doesBSDFChannelNeedBaking(obj, bsdf, channel)
        if result[0]:
            print(f"Baking {channel} for '{obj.name}'. Reason: {result[1]}")
            bakedMat = self.bakeChannel(context, obj, mat, bsdf, matOutput, channel)
            return True, bakedMat

        return False, mat

    def createImage(self, context, obj, name, colorSpace, alpha=False):
        imageName = name.replace(" ", "_")

        # Delete any existing image with this name to ensure the dimensions and alpha settings are correct:
        existingImageIndex = bpy.data.images.find(imageName)
        if existingImageIndex >= 0:
            bpy.data.images.remove(bpy.data.images[existingImageIndex])

        result = bpy.data.images.new(imageName,
                                     width=context.scene.meshsync_bakedTextureSize[0],
                                     height=context.scene.meshsync_bakedTextureSize[1],
                                     alpha=alpha)
        result.colorspace_settings.name = colorSpace

        return result

    def ensureUVs(self, context, obj):
        if obj in self.objectsProcessedForUVs:
            return

        self.objectsProcessedForUVs.append(obj)

        generateUVs = context.scene.meshsync_generate_uvs == 'ALWAYS' or len(obj.data.uv_layers) == 0

        if generateUVs:
            if context.scene.meshsync_generate_uvs == 'OFF':
                raise Exception(f"Object: '{obj.name}' has no UVs. Automatically generating UVs is disabled, so this object cannot be baked!")
        else:
            # Even though there are UVs, they might not be useful for baking.
            # Make sure they're not all in the same spot and in the 0..1 range:
            import numpy as np
            uvOutOfBounds = False
            for i in range(len(obj.data.uv_layers)):
                uv_map = obj.data.uv_layers[i]
                if uv_map.active:
                    nl = len(obj.data.loops)
                    uv_verts = np.zeros(nl * 2)
                    uv_map.data.foreach_get("uv", uv_verts)
                    # uv_verts.shape = nl, 2
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
                if context.scene.meshsync_generate_uvs == 'OFF':
                    raise Exception(f"Object: '{obj.name}' has no usable UVs. Automatically generating UVs is disabled, so this object cannot be baked!")

                print("UVs are not in 0..1 range for baking, generating new UVs.")
                generateUVs = True
                # bakeUVLayer = obj.data.uv_layers.new(name="Baked")
                # obj.data.uv_layers.active = bakeUVLayer
                # # Store uv override on the mesh, we'll need this later to delete all other UV layers,
                # # otherwise the baked images will use the wrong UVs!
                # obj.data[UV_OVERRIDE] = bakeUVLayer.name

        if generateUVs:
            print(f"Auto generating UVs for object: '{obj.name}'.")

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

    def getNodeYLocation(self, node):
        location = node.location[1]
        if node.parent is not None:
            location += self.getNodeYLocation(node.parent)
        return location

    def prepareBake(self, context, obj, bsdf, mat, canBakeBSDF):
        if context.object is not None:
            bpy.ops.object.mode_set(mode='OBJECT')
        for ob in context.selected_objects:
            ob.select_set(False)
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj

        self.ensureUVs(context, obj)

        # Make material copy if this is not a copy already:
        if ORIGINAL_MATERIAL not in mat:
            # Use existing copy if there is one:
            matCopyName = f"{mat.name}_{obj.name}_baked"
            matCopyIndex = bpy.data.materials.find(matCopyName)
            if matCopyIndex >= 0:
                matCopy = bpy.data.materials[matCopyIndex]
            else:
                mat.use_fake_user = True  # Make sure this does not get deleted when it's not referenced anymore
                matCopy = mat.copy()
                matCopy[ORIGINAL_MATERIAL] = mat.name
                matCopy.name = f"{mat.name}_{obj.name}_baked"

                matIndex = obj.material_slots.find(mat.name)
                obj.material_slots[matIndex].material = matCopy

                # Ungroup all node groups for easy, error-free access:
                for node in matCopy.node_tree.nodes:
                    node.select = False

                # Need to set the context area type for the group_ungroup operator to work:
                area = context.area
                old_type = area.type
                area.ui_type = 'ShaderNodeTree'

                space = context.space_data
                space.node_tree = matCopy.node_tree

                for node in matCopy.node_tree.nodes:
                    if node.type == 'GROUP':
                        node.select = True
                        bpy.ops.node.group_ungroup()
                        node.select = False

                for node in matCopy.node_tree.nodes:
                    node.select = False

                # Restore context area type:
                area.type = old_type

                print(f"Creating material copy '{mat.name}'->'{matCopy.name}'")

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

                bakedBSDF.location = bsdf.location # (bsdf.location[0], minYLocation - 1000)
                # Give bsdf a name and set its name on the material, so we can find it again:
                bakedBSDF.name = BAKED_MATERIAL_SHADER
                matCopy[BAKED_MATERIAL_SHADER] = bakedBSDF.name

            mat = matCopy

        if obj.mode == 'EDIT':
            bpy.ops.object.mode_set(mode='OBJECT')

        return mat

    def setupRenderSettings(self, context):
        if context.preferences.addons["cycles"].preferences.compute_device_type == "None":
            print("The cycles render device is not set. Baking would be faster if this is set to CUDA or OptiX.")

        self.originalSceneSettings = []

        self.setRestorableContextSetting(context, "scene.render.engine", "CYCLES")
        self.setRestorableContextSetting(context, "scene.cycles.device", "GPU")

        self.setRestorableContextSetting(context, "scene.cycles.samples", 10)     # TODO: This could be a setting for the user to change

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
        self.originalSceneSettings.append((settingName, rgetattr(context, settingName)))

        rsetattr(context, settingName, value)
        pass

    def restoreOriginalSettings(self, context):
        '''
        Restores original settings that were set using 'setRestorableContextSetting'.
        '''
        for originalSettingName, originalSettingValue in self.originalSceneSettings:
            rsetattr(context, originalSettingName, originalSettingValue)

    def traverseReroutes(self, node):
        if node.mute:
            return None

        if node is None or node.type != 'REROUTE':
            return node

        if len(node.inputs[0].links) == 0:
            return None

        return self.traverseReroutes(node.inputs[0].links[0].from_node)

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
        print("Baking in progress...")
        bpy.ops.object.bake(type=bakeType, use_clear=True, use_selected_to_active=False, use_split_materials=True)
        bakeImage.filepath_raw = os.path.join(context.scene.meshsync_bakedTexturesPath, bakeImage.name + ".png")
        bakeImage.file_format = "PNG"
        bakeImage.save()
        bakeImage.colorspace_settings.name = colorSpace

        return bakedImageNode

    def getChannelColourSpace(self, channel):
        if channel in ["Base Color", "Color"]:
            return 'sRGB'
        else:
            return 'Non-Color'

    def bakeChannelInputsDirectly(self, context, obj, mat, bsdf, matOutput, channel) -> bpy.types.ShaderNodeTexImage :
        '''
        Connects BSDF channel input directly to the material output and bakes it:
        :param context:
        :param obj:
        :param mat:
        :param bsdf:
        :param matOutput:
        :param channel:
        :return:
        '''
        bsdfChannelSocketName = self.getBSDFChannelInputName(bsdf, channel)

        print(f"Baking inputs of channel: '{bsdfChannelSocketName}'.")

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

        channelInput = bsdfChannelSocket.links[0].from_socket
        link(channelInput, matOutput.inputs[0])

        return self.bakeToImage(context, obj, mat, bakedBSDF, 'EMIT', channel)

    def bakeWithFallback(self, context, obj, mat, channel):
        if channel in channelNameToBakeName:
            bakeType = channelNameToBakeName[channel]
        else:
            print(
                f"Unable to bake {channel} for {mat.name} on {obj.name}. The channel is not supported in fallback mode.\n")
            return None

        print(f"Baking {channel} in fallback mode as {bakeType}")

        node_tree = mat.node_tree
        bsdf = node_tree.nodes[mat[BAKED_MATERIAL_SHADER]]

        return self.bakeToImage(context, obj, mat, bsdf, bakeType, channel)

    def getChannelNameSynonyms(self, channel):
        if channel in synonymMap:
            return synonymMap[channel]
        return []

    def getBSDFChannelInputName(self, bsdf, channel):
        if channel in bsdf.inputs:
            return channel

        for c in self.getChannelNameSynonyms(channel):
            if c in bsdf.inputs:
                return c

        return None

    def bakeChannel(self, context, obj, mat, bsdf, matOutput, channel):
        canBakeBSDF = self.canBsdfBeBaked(bsdf)

        mat = self.prepareBake(context, obj, bsdf, mat, canBakeBSDF)

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
        if not os.access(context.scene.meshsync_bakedTexturesPath, os.W_OK):
            self.report({'WARNING'}, "The folder to save baked textures to does not exist!")
            return {'CANCELLED'}

        startTime = time.time()
        # Make sure previous bake is undone:
        bpy.ops.meshsync.revert_bake_materials()

        self.bake(context)

        print(f"Finished baking. Time taken: {datetime.timedelta(seconds=(time.time() - startTime))}")
        return {'FINISHED'}


class MESHSYNC_OT_select_bake_folder(bpy.types.Operator, ExportHelper):
    bl_idname = "meshsync.choose_material_bake_folder"
    bl_label = "Choose folder for baked textures"

    filename_ext = ""

    def execute(self, context):
        context.scene.meshsync_bakedTexturesPath = os.path.dirname(self.properties.filepath)
        return {'FINISHED'}


class MESHSYNC_OT_RevertBake(bpy.types.Operator):
    bl_idname = "meshsync.revert_bake_materials"
    bl_label = "Restore original materials"
    bl_description = "Removes any baked materials and restores the original materials on all objects."

    def execute(self, context):
        from .unity_mesh_sync_common import msb_apply_scene_settings, msb_context

        msb_context.flushPendingList()
        msb_apply_scene_settings()
        msb_context.setup(context)

        materialsToDelete = set()

        for obj in context.scene.objects:
            if not msb_canObjectMaterialsBeBaked(obj):
                continue

            for matSlot in obj.material_slots:
                mat = matSlot.material
                if mat is None:
                    continue

                if ORIGINAL_MATERIAL in mat:
                    origMatName = mat[ORIGINAL_MATERIAL]
                    if origMatName not in bpy.data.materials:
                        print(
                            f"Cannot revert bake for material '{mat.name}' on '{obj.name}'. Original material '{origMatName}' does not exist.")
                        continue

                    origMat = bpy.data.materials[origMatName]
                    matSlot.material = origMat
                    materialsToDelete.add(mat)

        for mat in materialsToDelete:
            bpy.data.materials.remove(mat)

        return {'FINISHED'}
