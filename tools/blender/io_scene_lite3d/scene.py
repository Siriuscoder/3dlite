import json
import bpy
import mathutils

from pathlib import PurePosixPath, Path
from io_scene_lite3d.mesh import Mesh
from io_scene_lite3d.material import Material
from io_scene_lite3d.image import Image
from io_scene_lite3d.io import IO

class Scene:
    def __init__(self, name, path, package, **opts):
        self.name = name
        self.path = path
        self.package = package
        self.meshes = {}
        self.objectsList = []
        self.savedObjects = []
        self.images = {}
        self.materials = {}
        self.options = opts
        self.exportTypes = ["MESH", "EMPTY", "ARMATURE"]
        self.sceneJson = {}
        self.physicsObjectTypes = ["Dynamic", "Static", "Kinematic"]
        self.physicsCollisionsTypes = [
            "Box", 
            "Sphere", 
            "StaticPlane", 
            "Cylinder", 
            "Capsule", 
            "Cone", 
            "ConvexHull", 
            "StaticTriangleMesh", 
            "GimpactTriangleMesh"
        ]

        if self.options["exportLights"]:
            self.exportTypes.append("LIGHT")

    def saveMaterial(self, material):
        if material.name in self.materials.keys():
            return self.materials[material.name]
        
        mat = Material(material, self)
        mat.save()
        self.materials[material.name] = mat
        return mat
    
    def saveImage(self, texture):
        if texture.image.name in self.images.keys():
            return self.images[texture.image.name]
        
        im = Image(texture, self)
        im.save()
        self.images[texture.image.name] = im
        return im
        
    def getRelativePathObject(self, objName):
        return PurePosixPath("objects") / f"{objName}.json"
    
    def getRelativePathScene(self, name):
        return PurePosixPath("scenes") / f"{self.name}.json"
        
    def getAbsPath(self, relPath):
        return f"{self.package}:{relPath}"
    
    def getAbsImagePath(self, relPath):
        imagePackageName = self.options["imagePackageName"]
        return f"{imagePackageName}:{relPath}"
    
    def getAbsMeshPath(self, relPath):
        meshPackageName = self.options["meshPackageName"]
        return f"{meshPackageName}:{relPath}"
    
    def getAbsSysPath(self, relPath):
        path = self.path.joinpath(relPath)
        path.parent.mkdir(parents = True, exist_ok = True)
        return str(path)
        
    def saveObject(self, obj, objectRoot):
        IO.saveJson(self.getAbsSysPath(self.getRelativePathObject(obj.name)), objectRoot)
        self.savedObjects.append(obj.name)
    
    def saveScene(self):
        self.sceneJson["Objects"] = self.objectsList
        IO.saveJson(self.getAbsSysPath(self.getRelativePathScene(self.name)), self.sceneJson)
    
    @staticmethod
    def orietation(obj, node):
        obj.rotation_mode = "QUATERNION"
        node["Position"] = [
            obj.location.x,
            obj.location.y,
            obj.location.z,
        ]
            
        node["Rotation"] = [
            obj.rotation_quaternion.x,
            obj.rotation_quaternion.y,
            obj.rotation_quaternion.z,
            obj.rotation_quaternion.w
        ]
    
        node["Scale"] = [
            obj.scale.x,
            obj.scale.y,
            obj.scale.z
        ]
        
    def exportMesh(self, obj, node):
        mesh = None
        if not obj.data.name in self.meshes:
            mesh = Mesh(obj, self)
            mesh.save()
            self.meshes[obj.data.name] = mesh
        else:
            mesh = self.meshes[obj.data.name]

        if not self.exportPhysicsInfo(obj, node, mesh):
            node["Mesh"] = {
                "Mesh": self.getAbsMeshPath(mesh.getRelativePathJson()), 
                "Name": mesh.name
            }
            # export vertex groups per object basis. 
            # vertex groups are used in the skeleton deform with appropriate bones
            vertexGroups = []
            for group in obj.vertex_groups:
                vertexGroups.append({"name": group.name, "index": group.index})
            if len(vertexGroups) > 0:
                node["VertexGroups"] = vertexGroups
        
    def exportLight(self, obj, node):
        light = obj.data
        lightJson = {}
        if light.type == "POINT":
            lightJson["Type"] = "Point"
        elif light.type == "SUN":
            lightJson["Type"] = "Directional"
        elif light.type == "SPOT":
            lightJson["Type"] = "Spot"
        else: 
            return
        
        lightJson["Name"] = light.name + "_" + obj.name
        lightJson["Diffuse"] = [
            light.color.r,
            light.color.g,
            light.color.b
        ]
        
        lightJson["Radiance"] = light.energy
        if light.type in ["POINT", "SPOT"]:
            ac = light.get("AttenuationConstant", self.options["defaultConstantAttenuation"])
            al = light.get("AttenuationLinear", self.options["defaultLinearAttenuation"])
            aq = light.get("AttenuationQuadratic", self.options["defaultQuadraticAttenuation"])
            d = light.get("InfluenceDistance", self.options["defaultInfluenceDistance"])
            md = light.get("InfluenceMinRadiance", self.options["defaultInfluenceMinRadiance"])
            
            lightJson["LightSize"] = 0.0
            lightJson["Attenuation"] = {
                "Constant": ac,
                "Linear": al,
                "Quadratic": aq,
                "InfluenceDistance": d,
                "InfluenceMinRadiance": md
            }
            
            if light.type == "SPOT":
                lightJson["SpotFactor"] = {
                    "AngleInnerCone": light.spot_size * (1.0 - light.spot_blend),
                    "AngleOuterCone": light.spot_size,
                }
                
        lightJson["Direction"] = [
            0.0, 0.0, -1.0
        ]
        
        lightJson["Position"] = [
            0.0, 0.0, 0.0
        ]
        
        node["Light"] = lightJson

    def exportSkeletonBone(self, boneJson, bone):
        localPosition = bone.head
        # extend position by parent bone lenght (Y-forward)
        if bone.parent is not None:
            localPosition = localPosition + mathutils.Vector((0.0, bone.parent.length, 0.0))

        localRotation = bone.matrix.to_quaternion()

        boneJson["Name"] = bone.name
        boneJson["Position"] = [
            localPosition.x, 
            localPosition.y, 
            localPosition.z
        ]

        boneJson["Head"] = [
            bone.head.x, 
            bone.head.y, 
            bone.head.z
        ]

        boneJson["Rotation"] = [
            localRotation.x,
            localRotation.y,
            localRotation.z,
            localRotation.w
        ]

        boneJson["Length"] = bone.length

        for childBone in bone.children:
            childBoneJson = {}
            self.exportSkeletonBone(childBoneJson, childBone)
            # Добавляем только если что то добавилось из потомков
            if "Bones" in boneJson.keys():
                boneJson["Bones"].append(childBoneJson)
            else:
                boneJson["Bones"] = [childBoneJson]

    def exportSkeleton(self, obj, node):
        armature = obj.data
        skeletonJson = []
        # exporting the skeleton in bind-pose
        for bone in armature.bones:
            if bone.parent is None:
                boneJson = {}
                self.exportSkeletonBone(boneJson, bone)
                skeletonJson.append(boneJson)

        node["Skeleton"] = skeletonJson

    def exportNode(self, obj, node):
        if obj.type not in self.exportTypes:
            return
        
        node["Name"] = obj.name
        if obj.type == "MESH":
            self.exportMesh(obj, node)
        elif obj.type == "LIGHT":
            self.exportLight(obj, node)
        elif obj.type == "EMPTY":
            self.exportPhysicsInfo(obj, node)
        elif obj.type == "ARMATURE" and self.options["skeleton"]:
            self.exportSkeleton(obj, node)
        
        if obj.parent is not None:
            Scene.orietation(obj, node)
            
        for child in obj.children:
            childNode = {}
            self.exportNode(child, childNode)
            # Добавляем только если что то добавилось из потомков
            if len(childNode) > 0:
                if "Nodes" in node.keys():
                    node["Nodes"].append(childNode)
                else:
                    node["Nodes"] = [childNode]

    def exportPhysicsInfo(self, obj, node, mesh = None):
        if self.options["physics"]:
            objType = obj.get("physicsObjectType", "")
            objCollisionType = obj.get("physicsCollisionType", "")

            if objType in self.physicsObjectTypes:
                physicsConf = {}
                physicsConf["Type"] = objType
                for optName in obj.keys():
                    if optName in ["physicsFriction", "physicsRollingFriction", "physicsSpinningFriction", "physicsRestitution",
                                   "physicsLinearDamping", "physicsAngularDamping", "physicsLinearSleepingThreshold", "physicsAngularSleepingThreshold"]:
                        physicsConf[optName.replace("physics", "")] = obj.get(optName)
                node["Physics"] = physicsConf
                return False

            elif objCollisionType in self.physicsCollisionsTypes:
                collisionShapeConf = {}
                collisionShapeConf["Type"] = objCollisionType
                for optName in obj.keys():
                    if optName in ["physicsCollisionMass", "physicsCollisionRadius", "physicsCollisionHeight", "physicsCollisionPlaneConstant"]:
                        collisionShapeConf[optName.replace("physicsCollision", "")] = obj.get(optName)
                    elif optName in ["physicsCollisionHalfExtents", "physicsCollisionPlaneNormal"]:
                        collisionShapeConf[optName.replace("physicsCollision", "")] = [x for x in obj.get(optName)]

                if objCollisionType in ["ConvexHull", "StaticTriangleMesh", "GimpactTriangleMesh"]:
                    if mesh is not None:
                        collisionShapeConf["CollisionMesh"] = {
                            "Mesh": self.getAbsMeshPath(mesh.getRelativePathJson()), 
                            "Name": mesh.name
                        }
                node["CollisionShape"] = collisionShapeConf
                return True
            
        return False
                
    def exportObject(self, obj):
        if obj.type not in self.exportTypes:
            return
        # originObject можно указать имя обьекта который мы хотим переиспользовать 
        objectName = obj.get("originObject")
        visible = obj.get("visible", True)
        if objectName is None:
            objectRoot = {"Root": {}}
            self.exportPhysicsInfo(obj, objectRoot["Root"])
            self.exportNode(obj, objectRoot["Root"])
            self.saveObject(obj, objectRoot)
            objectName = obj.name
        
        if visible:
            object = {
                "Name": obj.name,
                "Object": self.getAbsPath(self.getRelativePathObject(objectName))
            }
            
            Scene.orietation(obj, object)
            self.objectsList.append(object)

    def preloadScene(self):
        sceneFilePath = self.getAbsSysPath(self.getRelativePathScene(self.name))
        if Path(sceneFilePath).is_file():
            with open(sceneFilePath, 'r') as file:
                self.sceneJson = json.load(file)
        
    def exportScene(self):
        self.preloadScene()

        scene = bpy.context.scene
        for obj in scene.objects:
            if obj.parent is None:
                self.exportObject(obj)
            
        self.saveScene()
    