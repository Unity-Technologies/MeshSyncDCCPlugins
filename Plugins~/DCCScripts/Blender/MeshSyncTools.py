bl_info = {
    "name": "Custom Tools",
    "blender": (2, 91, 2),
    "category": "Object",
}

import bpy;
import time;

class ToolView:
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tool"

class ToolMainPanel(ToolView, bpy.types.Panel):
    bl_label = "ToolView"

    def draw(self, context):
        pass
       
class MoveXByOnePanel(ToolView, bpy.types.Panel):
    bl_label = "Test"
    bl_parent_id = "ToolMainPanel"

    def draw(self, context):
        scene = bpy.context.scene
        layout = self.layout
        layout.operator("object.update_depsgraph", text="Update Depsgraph for each frame")        


class UpdateDepsgraph(bpy.types.Operator):
    """Update Depsgraph for each frame"""        # tooltip
    bl_idname = "object.update_depsgraph"        # Unique ID
    bl_label = "Update Depsgraph for each frame" # Interface label
    bl_options = {'REGISTER', 'UNDO'}  

    def execute(self, context):        
    
        scene = bpy.context.scene
        depsgraph = bpy.context.evaluated_depsgraph_get()
        id_types = ['ACTION', 'ARMATURE', 'BRUSH', 'CAMERA', 'CACHEFILE', 'CURVE', 'FONT', 'GREASEPENCIL', 'COLLECTION', 'IMAGE', 'KEY', 'LIGHT', 'LIBRARY', 'LINESTYLE', 'LATTICE', 'MASK', 'MATERIAL', 'META', 'MESH', 'MOVIECLIP', 'NODETREE', 'OBJECT', 'PAINTCURVE', 'PALETTE', 'PARTICLE', 'LIGHT_PROBE', 'SCENE', 'SIMULATION', 'SOUND', 'SPEAKER', 'TEXT', 'TEXTURE', 'HAIR', 'POINTCLOUD', 'VOLUME', 'WINDOWMANAGER', 'WORLD', 'WORKSPACE']        

        for i in range(scene.frame_start, scene.frame_end):
            startTime = time.time()
            
            scene.frame_current = i    
            depsgraph.update()    

            for j in id_types:
                updated = depsgraph.id_type_updated(j)
                if (updated):
                    print ("  Change status of %s : %d" % (j, updated))

            endTime = time.time()
            print("Frame: %s Elapsed Time: %s seconds" % (i, (endTime - startTime)))
            
        scene.frame_current = 0    
        depsgraph.update()

        return {'FINISHED'}   


#----------------------------------------------------------------------------------------------------------------------

classes = (
    ToolMainPanel,
    MoveXByOnePanel,
    UpdateDepsgraph,
)

def register():
    for c in classes:
        bpy.utils.register_class(c)


def unregister():
    for c in classes:
        bpy.utils.unregister_class(c)


#----------------------------------------------------------------------------------------------------------------------

if __name__ == "__main__":
    register()