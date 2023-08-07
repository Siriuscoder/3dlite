import bpy
from pathlib import PurePosixPath
from io_scene_lite3d.mesh import Mesh
from io_scene_lite3d.material import Material
from io_scene_lite3d.image import Image
from io_scene_lite3d.io import IO

class Scene:
    ExportTypes = ["MESH", "LIGHT", "EMPTY"]

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
    
    def getAbsSysPath(self, relPath):
        path = self.path.joinpath(relPath)
        path.parent.mkdir(parents = True, exist_ok = True)
        return str(path)
        
    def saveObject(self, obj, objectRoot):
        IO.saveJson(self.getAbsSysPath(self.getRelativePathObject(obj.name)), objectRoot)
        self.savedObjects.append(obj.name)
    
    def saveScene(self):
        IO.saveJson(self.getAbsSysPath(self.getRelativePathScene(self.name)), self.objectsList)
    
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
            
        node["Mesh"] = {
            "Mesh": self.getAbsPath(mesh.getRelativePathJson()), 
            "Name": mesh.name
        }
        
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
        
        lightJson["Name"] = obj.name + light.name
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

    def exportNode(self, obj, node):
        if obj.type not in Scene.ExportTypes:
            return
        
        node["Name"] = obj.name
        if obj.type == "MESH":
            self.exportMesh(obj, node)
        elif obj.type == "LIGHT":
            self.exportLight(obj, node)
        
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
                
    def exportObject(self, obj):
        if obj.type not in Scene.ExportTypes:
            return
        # originObject можно указать имя обьекта который мы хотим переиспользовать 
        objectName = obj.get("originObject")
        if objectName is None or objectName not in self.savedObjects:
            objectRoot = {"Root": {}}
            self.exportNode(obj, objectRoot["Root"])
            self.saveObject(obj, objectRoot)
            objectName = obj.name
        
        object = {
            "Name": obj.name,
            "Object": self.getAbsPath(self.getRelativePathObject(objectName))
        }
        
        Scene.orietation(obj, object)
        self.objectsList.append(object)
        
    def exportScene(self):
        scene = bpy.context.scene
        for obj in scene.objects:
            if obj.parent is None:
                self.exportObject(obj)
            
        self.saveScene()
    