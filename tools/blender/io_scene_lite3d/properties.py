import bpy
from bpy.props import StringProperty, BoolProperty, FloatProperty, PointerProperty, EnumProperty, FloatVectorProperty

class LITE3D_LightSettings(bpy.types.PropertyGroup):
    constantAttenuation: FloatProperty(name = "Attenuation Constant", precision = 6, default = 1.0, description = "Light constant attenuation")
    linearAttenuation: FloatProperty(name = "Attenuation Linear", precision = 6, default = 0.01, description = "Light linear attenuation")
    quadraticAttenuation: FloatProperty(name = "Attenuation Quadratic", precision = 6, default = 0.0001, description = "Light quadratic attenuation")
    influenceDistance: FloatProperty(name = "Influence Distance", precision = 6, default = 0.0, description = "Influence distance for light source")
    influenceMinRadiance: FloatProperty(name = "Influence Minimum Radiance", precision = 6, default = 0.001, description = "Minimum light radiance which is considered in light computation")

class LITE3D_MaterialSettings(bpy.types.PropertyGroup):
    materialTemplate: StringProperty(name = "Template", default = "bsdf", description = "Material template file name (without extension). Template is used as foundation to generate material")
    materialTypePBR: BoolProperty(name = "PBR", default = True, description = "Using PBR material template")

class LITE3D_MeshSettings(bpy.types.PropertyGroup):
    partition: StringProperty(name = "Partition", description = "Mesh partition name. Mesh partitioning is used to combine one or more meshes into a single vertex buffer object (VBO).")
    vertexColors: BoolProperty(name = "Vertex colors", default = False, description = "Export vertex colors. Vertex colors are stored in the vertex layout, adding 4 floats to it.")
    boneBindings: BoolProperty(name = "Bones Bindings", default = False, description = "Export bone information. Bone information contains bone weights and bone indexes for each vertex, extends vertex layout by 4 floats and 4 int")

class LITE3D_SceneSettings(bpy.types.PropertyGroup):
    packageName: StringProperty(name = "Package", default = "samples")
    imagePackageName: StringProperty(name = "Image Package", default = "samples")
    meshPackageName: StringProperty(name = "Mesh Package", default = "samples")

class LITE3D_ObjectSettings(bpy.types.PropertyGroup):
    originObject: PointerProperty(name="Origin", type = bpy.types.Object, description="Gives ability to reuse the origin object as this object. Name of this object remains unchanged.")
    enabled: BoolProperty(name = "Enabled", default = True, description = "Enables or disables export of this object")
    physicsCalcCenterOfMass: BoolProperty(name = "Calculate center of mass", default = False, description = "Adjust center of mass for complex compound objects")
    physicsObjectType: EnumProperty(name = "Type", description="Type of the physics object in terms of Bullet. Must be EMPTY and root of the physics object", 
        default = "None",
        items = [
            ("Dynamic", "Dynamic", "Bullet dynamic physics object"),
            ("Static", "Static", "Bullet static physics object (walls, floors, rocks ..)"),
            ("Kinematic", "Kinematic", "Bullet physics object with custom animation"),
            ("None", "None", "Do not use physics simulations for this object")
        ])
    
    physicsCollisionType: EnumProperty(name = "Collision Type", description="Treat this object as a collision object; it must be nested under the root node of the physics object.", 
        default = "None",
        items = [
            ("Box", "Box", ""),
            ("Sphere", "Sphere", ""),
            ("StaticPlane", "StaticPlane", ""),
            ("Cylinder", "Cylinder", ""),
            ("Capsule", "Capsule", ""),
            ("Cone", "Cone", ""),
            ("ConvexHull", "ConvexHull", "Generate simplified convex shape from this mesh object"),
            ("StaticTriangleMesh", "StaticTriangleMesh", "Use this mesh as a collision shape. This approach is best suited for complex static objects."),
            ("GimpactTriangleMesh", "GimpactTriangleMesh", "Use this mesh as a collision shape. This approach is best suited for complex dynamic objects. Avoid using overly dense meshes to maintain performance."),
            ("None", "None", "Do not use collisions for this object")
        ])
    
    physicsFriction: FloatProperty(
        name = "Friction",
        precision = 4,
        default = 0.5,
        description = "Coefficient of friction. Controls resistance to sliding between objects. Range: 0 (no friction) to 1 (high friction)."
    )

    physicsRollingFriction: FloatProperty(
        name = "Rolling Friction",
        precision = 4,
        default = 0.0,
        description = "Rolling friction coefficient. Affects how easily rounded objects roll. Helps stop objects from rolling forever. Range: 0 to 1."
    )

    physicsSpinningFriction: FloatProperty(
        name = "Spinning Friction",
        precision = 4,
        default = 0.0,
        description = "Spinning friction coefficient. Controls resistance to rotation around the contact normal. Helps reduce uncontrolled spinning. Range: 0 to 1."
    )

    physicsRestitution: FloatProperty(
        name = "Restitution",
        precision = 4,
        default = 0.0,
        description = "Bounciness of the object. 0 means no bounce, 1 means perfectly elastic collisions. Affects how much energy is conserved on impact."
    )

    physicsLinearDamping: FloatProperty(
        name = "Linear Damping",
        precision = 4,
        default = 0.0,
        description = "Reduces linear velocity over time. Simulates air or medium resistance. 0 means no damping, 1 stops the object immediately."
    )

    physicsAngularDamping: FloatProperty(
        name = "Angular Damping",
        precision = 4,
        default = 0.0,
        description = "Reduces angular velocity over time. Used to slow down spinning objects. 0 means no damping, 1 stops rotation quickly."
    )

    physicsLinearSleepingThreshold: FloatProperty(
        name = "Linear Sleeping Threshold",
        precision = 4,
        default = 0.8,
        description = "If linear velocity is below this threshold, the object may be put to sleep for optimization."
    )

    physicsAngularSleepingThreshold: FloatProperty(
        name = "Angular Sleeping Threshold",
        precision = 4,
        default = 1.0,
        description = "If angular velocity is below this threshold, the object may be put to sleep for optimization."
    )

    physicsCollisionMass: FloatProperty(
        name = "Mass",
        precision = 6,
        default = 0.0,
        description = "Mass of the collision object. Determines how it responds to forces and collisions. A mass of zero makes the object static."
    )

    physicsCollisionRadius: FloatProperty(
        name = "Radius",
        precision = 6,
        default = 0.0,
        description = "Radius of the collision shape. Used for spherical collision shapes to define their size."
    )

    physicsCollisionHeight: FloatProperty(
        name = "Height",
        precision = 6,
        default = 0.0,
        description = "Height of the collision shape. Relevant for capsule and cylinder shapes to define their vertical size."
    )

    physicsCollisionPlaneConstant: FloatProperty(
        name = "Plane Constant",
        precision = 6,
        default = 0.0,
        description = "Distance of the plane from the origin along its normal. Defines the position of an infinite plane shape."
    )

    physicsCollisionHalfExtents: FloatVectorProperty(
        name = "Half Extents",
        size = 2,
        subtype = 'XYZ',
        default = (0.0, 0.0),
        description = "Half the size of the collision box along each axis. Defines the dimensions of box-shaped collision objects."
    )

    physicsCollisionPlaneNormal: FloatVectorProperty(
        name = "Plane Normal",
        size = 3,
        subtype = 'XYZ',
        default = (0.0, 0.0, 0.0),
        description = "Normal vector of the collision plane. Determines the orientation of an infinite plane shape."
    )

def register_properties():
    bpy.utils.register_class(LITE3D_LightSettings)
    bpy.utils.register_class(LITE3D_MaterialSettings)
    bpy.utils.register_class(LITE3D_MeshSettings)
    bpy.utils.register_class(LITE3D_SceneSettings)
    bpy.utils.register_class(LITE3D_ObjectSettings)
    bpy.types.Light.lite3d_properties = PointerProperty(type = LITE3D_LightSettings)
    bpy.types.Material.lite3d_properties = PointerProperty(type = LITE3D_MaterialSettings)
    bpy.types.Mesh.lite3d_properties = PointerProperty(type = LITE3D_MeshSettings)
    bpy.types.Scene.lite3d_properties = PointerProperty(type = LITE3D_SceneSettings)
    bpy.types.Object.lite3d_properties = PointerProperty(type = LITE3D_ObjectSettings)

def unregister_properties():
    bpy.utils.unregister_class(LITE3D_LightSettings)
    bpy.utils.unregister_class(LITE3D_MaterialSettings)
    bpy.utils.unregister_class(LITE3D_MeshSettings)
    bpy.utils.unregister_class(LITE3D_SceneSettings)
    bpy.utils.unregister_class(LITE3D_ObjectSettings)
    del bpy.types.Light.lite3d_properties
    del bpy.types.Material.lite3d_properties
    del bpy.types.Mesh.lite3d_properties
    del bpy.types.Scene.lite3d_properties
    del bpy.types.Object.lite3d_properties