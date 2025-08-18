import bpy

PanelName = "Lite3d add-on"

class LIGHT3D_LIGHT_PT_panel(bpy.types.Panel):
    bl_label = PanelName
    bl_idname = "LIGHT3D_LIGHT_PT_panel"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"
    bl_category = "Light"

    @classmethod
    def poll(cls, context):
        return context.light is not None  # Показывать только если выбран источник света

    def draw(self, context):
        self.layout.prop(context.light.lite3d_properties, "constantAttenuation")
        self.layout.prop(context.light.lite3d_properties, "linearAttenuation")
        self.layout.prop(context.light.lite3d_properties, "quadraticAttenuation")
        self.layout.prop(context.light.lite3d_properties, "influenceDistance")
        self.layout.prop(context.light.lite3d_properties, "influenceMinRadiance")

class LIGHT3D_MATERIAL_PT_panel(bpy.types.Panel):
    bl_label = PanelName
    bl_idname = "LIGHT3D_MATERIAL_PT_panel"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "material"

    @classmethod
    def poll(cls, context):
        return context.material is not None

    def draw(self, context):
        self.layout.prop(context.material.lite3d_properties, "materialTemplate")
        self.layout.prop(context.material.lite3d_properties, "materialTypePBR")

class LIGHT3D_MESH_PT_panel(bpy.types.Panel):
    bl_label = PanelName
    bl_idname = "LIGHT3D_MESH_PT_panel"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"
    bl_object_type = 'MESH'

    @classmethod
    def poll(cls, context):
        return context.mesh is not None

    def draw(self, context):
        self.layout.prop(context.mesh.lite3d_properties, "partition")
        self.layout.prop(context.mesh.lite3d_properties, "vertexColors")
        self.layout.prop(context.mesh.lite3d_properties, "boneBindings")

class LIGHT3D_SCENE_PT_panel(bpy.types.Panel):
    bl_label = PanelName
    bl_idname = "LIGHT3D_SCENE_PT_panel"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"

    def draw(self, context):
        self.layout.prop(context.scene.lite3d_properties, "packageName")
        self.layout.prop(context.scene.lite3d_properties, "imagePackageName")
        self.layout.prop(context.scene.lite3d_properties, "meshPackageName")

class LIGHT3D_OBJECT_PT_panel(bpy.types.Panel):
    bl_label = PanelName
    bl_idname = "LIGHT3D_OBJECT_PT_panel"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        self.layout.prop(context.object.lite3d_properties, "originObject")
        self.layout.prop(context.object.lite3d_properties, "enabled")

        physicsSettings = self.layout.box()
        physicsSettings.label(text = "Physics:")
        physicsSettings.prop(context.object.lite3d_properties, "physicsObjectType")
        if context.object.lite3d_properties.physicsObjectType != "None":
            physicsSettings.prop(context.object.lite3d_properties, "physicsCalcCenterOfMass")
            physicsSettings.prop(context.object.lite3d_properties, "physicsFriction")
            physicsSettings.prop(context.object.lite3d_properties, "physicsRollingFriction")
            physicsSettings.prop(context.object.lite3d_properties, "physicsSpinningFriction")
            physicsSettings.prop(context.object.lite3d_properties, "physicsRestitution")
            physicsSettings.prop(context.object.lite3d_properties, "physicsLinearDamping")
            physicsSettings.prop(context.object.lite3d_properties, "physicsAngularDamping")
            physicsSettings.prop(context.object.lite3d_properties, "physicsLinearSleepingThreshold")
            physicsSettings.prop(context.object.lite3d_properties, "physicsAngularSleepingThreshold")
        else:
            physicsSettings.prop(context.object.lite3d_properties, "physicsCollisionType")
            if context.object.lite3d_properties.physicsCollisionType != "None":
                physicsSettings.prop(context.object.lite3d_properties, "physicsCollisionMass")
                if context.object.lite3d_properties.physicsCollisionType == "Sphere":
                    physicsSettings.prop(context.object.lite3d_properties, "physicsCollisionRadius")
                elif context.object.lite3d_properties.physicsCollisionType in ["Cone", "Capsule"]:
                    physicsSettings.prop(context.object.lite3d_properties, "physicsCollisionRadius")
                    physicsSettings.prop(context.object.lite3d_properties, "physicsCollisionHeight")
                elif context.object.lite3d_properties.physicsCollisionType in ["Box", "Cylinder"]:
                    physicsSettings.prop(context.object.lite3d_properties, "physicsCollisionHalfExtents")
                elif context.object.lite3d_properties.physicsCollisionType == "StaticPlane":
                    physicsSettings.prop(context.object.lite3d_properties, "physicsCollisionPlaneConstant")
                    physicsSettings.prop(context.object.lite3d_properties, "physicsCollisionPlaneNormal")

def register_menus():
    bpy.utils.register_class(LIGHT3D_LIGHT_PT_panel)
    bpy.utils.register_class(LIGHT3D_MATERIAL_PT_panel)
    bpy.utils.register_class(LIGHT3D_MESH_PT_panel)
    bpy.utils.register_class(LIGHT3D_SCENE_PT_panel)
    bpy.utils.register_class(LIGHT3D_OBJECT_PT_panel)

def unregister_menus():
    bpy.utils.unregister_class(LIGHT3D_LIGHT_PT_panel)
    bpy.utils.unregister_class(LIGHT3D_MATERIAL_PT_panel)
    bpy.utils.unregister_class(LIGHT3D_MESH_PT_panel)
    bpy.utils.unregister_class(LIGHT3D_SCENE_PT_panel)
    bpy.utils.unregister_class(LIGHT3D_OBJECT_PT_panel)