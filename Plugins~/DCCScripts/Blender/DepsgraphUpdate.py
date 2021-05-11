bl_info = {
    "name": "Move X Axis",
    "blender": (2, 91, 2),
    "category": "Object",
}

import bpy

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
        layout.operator("object.move_x", text="Move X By One")        


class ObjectMoveX(bpy.types.Operator):
    """My Object Moving Script"""      # Use this as a tooltip for menu items and buttons.
    bl_idname = "object.move_x"        # Unique identifier for buttons and menu items to reference.
    bl_label = "Move X by One"         # Display name in the interface.
    bl_options = {'REGISTER', 'UNDO'}  # Enable undo for the operator.

    def execute(self, context):        # execute() is called when running the operator.

        # The original script
        scene = context.scene
        for obj in scene.objects:
            obj.location.x += 1.0

        return {'FINISHED'}            # Lets Blender know the operator finished successfully.


classes = (
    ToolMainPanel,
    MoveXByOnePanel,
    ObjectMoveX,
)

def register():
    for c in classes:
        bpy.utils.register_class(c)


def unregister():
    for c in classes:
        bpy.utils.unregister_class(c)


if __name__ == "__main__":
    register()