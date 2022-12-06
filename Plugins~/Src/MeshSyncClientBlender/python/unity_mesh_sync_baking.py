import bpy, os, datetime, time
from bpy_extras.io_utils import ExportHelper

from .unity_mesh_sync_common import MESHSYNC_PT

# Constants:
ORIGINAL_MATERIAL = 'ORIGINAL_MATERIAL'
BAKED_MATERIAL_SHADER = 'BAKED_MATERIAL_SHADER'


BAKED_CHANNELS = ["Base Color", "Roughness", "Metallic", "Normal", "Emission",
                  "Emission Strength", "Clearcoat"]

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

class MESHSYNC_PT_Baking(MESHSYNC_PT, bpy.types.Panel):
    bl_label = "Baking"
    bl_parent_id = "MESHSYNC_PT_Main"
    bl_order = -10

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False

        layout.prop(context.scene, "meshsync_bake_selection", expand=True)
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
        return mat is not None and\
            not self.isMaterialCopy(mat) and\
            mat.use_nodes

    def findMaterialOutputNode(self, mat):
        node_tree = mat.node_tree

        outputNode = None
        for node in node_tree.nodes:
            if node.type == 'OUTPUT_MATERIAL' and len(node.inputs[0].links) == 1:
                outputNode = node
                # Blender uses the last OUTPUT_MATERIAL node, so don't stop search here

        return outputNode

    def findMaterialOutputNodeInput(self, mat):
        outputNode = self.findMaterialOutputNode(mat)

        if outputNode is None:
            print(f"Cannot find material output node with an input. Cannot bake {mat.name}!")
            return None

        # Get used shader or whatever is connected to the material output node:
        input = outputNode.inputs[0].links[0].from_node

        if input.mute:
            print(f"Input too material output is muted. Cannot bake {mat.name}!")
            return None

        return input

    def deselectAllMaterialNodes(self, mat):
        for node in mat.node_tree.nodes:
            node.select = False

    def frameSelectedNodes(self, mat):
        # Frame selected nodes:
        selected = []
        for node in mat.node_tree.nodes:
            if node.select:
                selected.append(node)

        frame = mat.node_tree.nodes.new(type='NodeFrame')
        frame.label = "Baked"

        for node in selected:
            node.parent = frame

    def connectBakedBSDF(self, bakedMat, bsdf):
        # Find copies of the material and
        # for mat in bpy.data.materials:
        #     if ORIGINAL_MATERIAL in mat and \
        #             mat[ORIGINAL_MATERIAL] == originalMat.name:
        #         node_tree = mat.node_tree
        #         bakedBSDF = node_tree.nodes[mat[BAKED_MATERIAL_SHADER]]
        #         materialOutputNodeName = bsdf.outputs[0].links[0].to_node.name
        #         node_tree.links.new(bakedBSDF.outputs[0], node_tree.nodes[materialOutputNodeName].inputs[0])
        node_tree = bakedMat.node_tree
        bakedBSDF = node_tree.nodes[bakedMat[BAKED_MATERIAL_SHADER]]
        materialOutputNodeName = bsdf.outputs[0].links[0].to_node.name
        node_tree.links.new(bakedBSDF.outputs[0], node_tree.nodes[materialOutputNodeName].inputs[0])

    def bakeObject(self, context, obj):
        if not msb_canObjectMaterialsBeBaked(obj):
            return

        for matSlot in obj.material_slots:
            mat = matSlot.material
            if not self.canMaterialBeBaked(mat):
                continue

            self.bakedImageNodeYOffset = 0

            bsdf = self.findMaterialOutputNodeInput(mat)

            # Ensure object is not hidden, otherwise baking will fail:
            wasHidden = obj.hide_get()
            obj.hide_set(False)
            context.view_layer.objects.active = obj

            self.deselectAllMaterialNodes(mat)

            print(f"Checking if '{mat.name}' on '{obj.name}' needs baked materials.")

            # If any channel was baked, it will be on a new material,
            # store that to frame all new nodes after everything is baked:
            bakedMat = mat
            for channel in BAKED_CHANNELS:
                didBake, newMat = self.bakeBSDFChannelIfNeeded(context, obj, mat, bsdf, channel)
                if didBake:
                    bakedMat = newMat

            if bakedMat != mat:
                print(f"Baked '{mat.name}' on '{obj.name}'.\n")
                self.frameSelectedNodes(bakedMat)
                self.connectBakedBSDF(bakedMat, bsdf)

            # Restore state:
            obj.select_set(False)
            obj.hide_set(wasHidden)

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

        if 'bsdf' not in bsdf.type.lower():
            return [True, f"Material output is not connected to a shader but a node of type: {bsdf.type}."]

        if channel not in bsdf.inputs:
            # If the bsdf doesn't have "Base Color", it might have "Color":
            if channel == "Base Color":
                return self.doesBSDFChannelNeedBaking(obj, bsdf, "Color")

            return [False]

        inputSocket = bsdf.inputs[channel]

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

    def bakeBSDFChannelIfNeeded(self, context, obj, mat, bsdf, channel):
        result = self.doesBSDFChannelNeedBaking(obj, bsdf, channel)
        if result[0]:
            print(f"Baking {channel} for {obj.name}. Reason: {result[1]}")
            bakedMat = self.bakeChannel(context, obj, mat, bsdf, channel)
            return True, bakedMat

        return False, mat

    def findOrCreateImage(self, context, obj, suffix, alpha=False):
        imageName = f"{obj.name}_baked_{suffix}"

        result = None

        for image in bpy.data.images:
            if image.name == imageName:
                result = image
                break

        if result is None:
            result = bpy.data.images.new(imageName,
                                         width=context.scene.meshsync_bakedTextureSize[0],
                                         height=context.scene.meshsync_bakedTextureSize[1],
                                         alpha=alpha)
        return result

    def ensureUVs(self, context, obj):
        # test:
        createUVs = len(obj.data.uv_layers) == 0

        if createUVs:
            if bpy.context.object.mode == 'OBJECT':
                bpy.ops.object.mode_set(mode='EDIT')

            bpy.ops.mesh.select_mode(use_extend=False, use_expand=False, type='VERT')
            bpy.ops.mesh.select_all(action='SELECT')
            bpy.ops.mesh.select_linked(delimit={'SEAM'})
            bpy.ops.uv.smart_project(island_margin=0.01, scale_to_bounds=True)
            bpy.ops.uv.pack_islands(rotate=True, margin=0.001)

    def getNodeYLocation(self, node):
        location = node.location[1]
        if node.parent is not None:
            location += self.getNodeYLocation(node.parent)
        return location

    def prepareBake(self, context, obj, bsdf, mat):
        if context.object is not None:
            bpy.ops.object.mode_set(mode='OBJECT')
        # bpy.ops.object.select_all(view_3d, action='DESELECT')
        for ob in context.selected_objects:
            ob.select_set(False)
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj

        # Make material copy if this is not a copy already:
        if ORIGINAL_MATERIAL not in mat:
            # Use existing copy if there is one:
            matCopyName = f"{mat.name}_{obj.name}_baked"
            matCopyIndex = bpy.data.materials.find(matCopyName)
            if matCopyIndex >= 0:
                matCopy = bpy.data.materials[matCopyIndex]
            else:
                mat.use_fake_user = True    # Make sure this does not get deleted when it's not referenced anymore
                matCopy = mat.copy()
                matCopy[ORIGINAL_MATERIAL] = mat.name
                matCopy.name = f"{mat.name}_{obj.name}_baked"

                print(f"Creating material copy '{mat.name}'->'{matCopy.name}'")

                bakedBSDF = matCopy.node_tree.nodes.new(type='ShaderNodeBsdfPrincipled')

                # Find lowest node in the tree and put the baked bsdf under that:
                minYLocation = bsdf.location[1]
                for node in matCopy.node_tree.nodes:
                    minYLocation = min(minYLocation, self.getNodeYLocation(node))

                bakedBSDF.location = (bsdf.location[0], minYLocation - 1000)
                # Give bsdf a name and set its name on the material, so we can find it again:
                bakedBSDF.name = BAKED_MATERIAL_SHADER
                matCopy[BAKED_MATERIAL_SHADER] = bakedBSDF.name

                matIndex = obj.material_slots.find(mat.name)
                obj.material_slots[matIndex].material = matCopy
            # obj.data.materials.pop(index=obj.data.materials.find(mat.name))
            # obj.data.materials.append(matCopy)

            mat = matCopy

        self.ensureUVs(context, obj)

        scene = context.scene

        if obj.mode == 'EDIT':
            bpy.ops.object.mode_set(mode='OBJECT')

        scene.render.engine = "CYCLES"
        scene.cycles.device = "GPU"
        scene.cycles.samples = 10       # TODO: This could be a setting for the user to change
        scene.render.bake.use_pass_direct = False
        scene.render.bake.use_pass_indirect = False
        scene.render.bake.use_pass_color = True

        return mat

    def bakeChannel(self, context, obj, mat, bsdf, channel):
        mat = self.prepareBake(context, obj, bsdf, mat)

        if channel in channelNameToBakeName:
            bakeType = channelNameToBakeName[channel]
        else:
            bakeType = 'EMIT'

        bakeImage = self.findOrCreateImage(context, obj, bakeType.lower(), alpha=True)
        # roughnessBakeImage = self.findOrCreateImage(self.mapNameRoughness)
        # aoBakeImage = self.findOrCreateImage(self.mapNameAO)

        node_tree = mat.node_tree
        # Make sure bsdf points to material copy:
        bsdf = node_tree.nodes[mat[BAKED_MATERIAL_SHADER]]

        # Set up image node
        bakedImageNode = node_tree.nodes.new("ShaderNodeTexImage")
        bakedImageNode.select = True
        node_tree.nodes.active = bakedImageNode
        bakedImageNode.image = bakeImage
        bakedImageNode.location = (bsdf.location[0] - 500, bsdf.location[1] - self.bakedImageNodeYOffset)
        self.bakedImageNodeYOffset += 300

        # Bake
        bpy.ops.object.bake(type=bakeType, use_clear=True, use_selected_to_active=False)
        bakeImage.filepath_raw = os.path.join(context.scene.meshsync_bakedTexturesPath, bakeImage.name + ".png")
        bakeImage.file_format = 'PNG'
        bakeImage.save()

        inputChannelName = channel
        # Adapt input name for bsdf:
        if channel == 'Color':
            inputChannelName = 'Base Color'

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
                        print(f"Cannot revert bake for material '{mat.name}' on '{obj.name}'. Original material '{origMatName}' does not exist.")
                        continue

                    origMat = bpy.data.materials[origMatName]
                    obj.data.materials.pop(index=obj.data.materials.find(mat.name))
                    obj.data.materials.append(origMat)
                    materialsToDelete.add(mat)

        for mat in materialsToDelete:
            bpy.data.materials.remove(mat)

        return {'FINISHED'}

# class BakeHelper():
#     bakeWidth = 512
#     bakeHeight = 512
#
#     bakedImages = {}
#
#     mapNameDiffuse = "diffuse"
#     mapNameRoughness = "roughness"
#     mapNameAO = "ao"
#
#     def __init__(self, objectToBake):
#         self.objectToBake = objectToBake
#
#     def findOrCreateImage(self, suffix, alpha=False):
#         imageName = f"{self.objectToBake.name}_baked_{suffix}"
#
#         result = None
#
#         for image in bpy.data.images:
#             if image.name == imageName:
#                 result = image
#                 break
#
#         if result is None:
#             result = bpy.data.images.new(imageName, width=self.bakeWidth, height=self.bakeHeight, alpha=alpha)
#
#         self.bakedImages[suffix] = result
#
#         return result
#
#     def bake(self, bakeFolder):
#         scene = bpy.context.scene
#         self.bakedImages.clear()
#
#         from .unity_mesh_sync_common import msb_apply_scene_settings, msb_context
#
#         # if not msb_context.is_setup:
#         msb_context.flushPendingList()
#         msb_apply_scene_settings()
#         msb_context.setup(bpy.context)
#
#         # test:
#         smartUV = True
#         samples = 4
#
#         hasfolder = os.access(bakeFolder, os.W_OK)
#         if hasfolder is False:
#             self.report({'WARNING'}, "Selected an invalid export folder")
#             return {'FINISHED'}
#
#         if smartUV:
#             if bpy.context.object.mode == 'OBJECT':
#                 bpy.ops.object.mode_set(mode='EDIT')
#
#             bpy.ops.mesh.select_mode(use_extend=False, use_expand=False, type='VERT')
#             bpy.ops.mesh.select_all(action='SELECT')
#             bpy.ops.mesh.select_linked(delimit={'SEAM'})
#             bpy.ops.uv.smart_project(island_margin=0.01, scale_to_bounds=True)
#             bpy.ops.uv.pack_islands(rotate=True, margin=0.001)
#
#         if bpy.context.object.mode == 'EDIT':
#             bpy.ops.object.mode_set(mode='OBJECT')
#
#         scene.render.engine = "CYCLES"
#         scene.cycles.device = "GPU"
#         scene.cycles.samples = samples
#
#         diffuseBakeImage = self.findOrCreateImage(self.mapNameDiffuse, alpha=True)
#         roughnessBakeImage = self.findOrCreateImage(self.mapNameRoughness)
#         aoBakeImage = self.findOrCreateImage(self.mapNameAO)
#
#         # ----- DIFFUSE -----
#
#         for mat in self.objectToBake.data.materials:
#             node_tree = mat.node_tree
#             node = node_tree.nodes.new("ShaderNodeTexImage")
#             node.select = True
#             node_tree.nodes.active = node
#             node.image = diffuseBakeImage
#
#         scene.render.bake.use_pass_direct = False
#         scene.render.bake.use_pass_indirect = False
#         scene.render.bake.use_pass_color = True
#
#         bpy.ops.object.bake(type='DIFFUSE', use_clear=True, use_selected_to_active=False)
#         diffuseBakeImage.filepath_raw = os.path.join(bakeFolder, diffuseBakeImage.name + ".png")
#         diffuseBakeImage.file_format = 'PNG'
#         diffuseBakeImage.save()
#
#         # ----- ROUGHNESS -----
#
#         for mat in self.objectToBake.data.materials:
#             node_tree = mat.node_tree
#             node = node_tree.nodes.active
#             node.image = roughnessBakeImage
#
#         bpy.ops.object.bake(type='ROUGHNESS', use_clear=True, use_selected_to_active=False)
#
#         roughnessBakeImage.filepath_raw = os.path.join(bakeFolder, roughnessBakeImage.name + ".png")
#         roughnessBakeImage.file_format = 'PNG'
#         roughnessBakeImage.save()
#
#         # ----- AO -----
#
#         for mat in self.objectToBake.data.materials:
#             node_tree = mat.node_tree
#             node = node_tree.nodes.active
#             node.image = aoBakeImage
#
#         bpy.ops.object.bake(type='AO', use_clear=True, use_selected_to_active=False)
#         aoBakeImage.filepath_raw = os.path.join(bakeFolder, aoBakeImage.name + ".png")
#         aoBakeImage.file_format = 'PNG'
#         aoBakeImage.save()
#
#         # ----- UV -----
#         # uvSuffix = "_uv"
#         #
#         # bpy.ops.object.mode_set(mode='EDIT')
#         # bpy.ops.mesh.select_all(action='SELECT')
#         # bpy.ops.object.mode_set(mode='OBJECT')
#         #
#         # original_type = bpy.context.area.type
#         # bpy.context.area.type = "IMAGE_EDITOR"
#         # uvfilepath = bakeFolder + active_object.name + bakePrefix + uvSuffix + ".png"
#         # bpy.ops.uv.export_layout(filepath=uvfilepath, size=(self.bakeWidth, self.bakeHeight))
#         # bpy.context.area.type = original_type
#
#         # self.setupBakedMaterial(active_object)
#
#         return {'FINISHED'}
#
#     def setupImageNode(self, mat, name, bsdf, inputName):
#         nodes = mat.node_tree.nodes
#         link = mat.node_tree.links.new
#
#         imageNode = bpy.types.ShaderNodeTexImage(nodes.new('ShaderNodeTexImage'))
#         imageNode.image = self.bakedImages[name]
#
#         link(imageNode.outputs[0], bsdf.inputs[inputName])
#
#     def setupBakedMaterial(self, bakedObj):
#         # Create a new material and assign the baked textures:
#         mat = bpy.data.materials.new(name=f"{bakedObj.name}_baked")
#         mat.use_nodes = True
#
#         originalMats = bakedObj.data.materials
#         bakedObj.data.materials.clear()
#         bakedObj.data.materials.append(mat)
#
#         nodes = mat.node_tree.nodes
#         bsdf = nodes['Principled BSDF']
#
#         self.setupImageNode(mat, self.mapNameDiffuse, bsdf, "Base Color")
#         self.setupImageNode(mat, self.mapNameRoughness, bsdf, "Roughness")
#         # self.setupImageNode(mat, self.mapNameAO, bsdf, "?")
#
#         # Restore original materials
#         # Needs to be called separately after export:
#         if False:
#             bakedObj.data.materials.clear()
#             for origMat in originalMats:
#                 bakedObj.data.materials.appen(origMat)
#
#
# def bakeMaterials(folder):
#     print("bakeMaterials")
#     import json
#     textureList = []
#     jsonString = bpy.context.scene.bakedTextures
#     if jsonString is not None and len(jsonString) > 0:
#         textureList = json.loads(bpy.types.Scene.bakedTextures)
#
#     for obj in bpy.context.scene.objects:
#         baker = BakeHelper(obj)
#         baker.bake(folder)
#
#     bpy.types.Scene.bakedTextures = json.dumps(textureList)
#     print(f"textureList: {textureList}")
