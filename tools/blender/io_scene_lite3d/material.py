import bpy
import json
from pathlib import PurePosixPath, Path
from io_scene_lite3d.io import IO
from io_scene_lite3d.logger import log

class Material:
    def __init__(self, material, scene):
        self.material = material
        self.name = material.name + ".material"
        self.scene = scene
        self.params = {}
        self.template = {}
        
    def getRelativePath(self):
        return PurePosixPath("materials/") / f"{self.material.name}.json"
    
    def getRelativeTemplatePath(self):
        template = self.material.get("Template")
        if template is None:
            template = self.scene.options["materialTemplate"]
        return PurePosixPath("materials/") / f"{template}.json"
    
    def considerShader(self, node):
        for socket in node.inputs:
            if socket.is_linked or socket.is_unavailable or not socket.enabled:
                continue
            if socket.name in ["Base Color", "Albedo"]:
                self.params["Albedo"] = [x for x in socket.default_value] # Color 4i
                alphaSocket = node.inputs["Alpha"]
                if not alphaSocket.is_linked and not socket.is_unavailable and socket.enabled:
                    self.params["Albedo"][3] = float(alphaSocket.default_value)
            elif socket.name in ["Emission Color", "Emission"]:
                self.params["Emission"] = [x for x in socket.default_value] # Color 4i
            elif socket.name in ["Specular IOR Level"]:
                self.params["Specular"] = float(socket.default_value)
            elif socket.name in ["Metallic", "Roughness", "IOR", "Emission Strength"]:
                self.params[socket.name] = float(socket.default_value)
            
    def considerImage(self, node):
        self.params[node.label] = node
        
    def loadTemplate(self):
        path = self.scene.getAbsSysPath(self.getRelativeTemplatePath())
        if not Path(path).is_file():
            log.error(f"{self.material.name}: Can`t locate material template file at '{path}'")
            return False
        
        with open(path, 'r') as file:
            self.template = json.load(file)
        return True
    
    def replaceSubstitution(self, template, key, keyParam):
        param = self.params.get(keyParam, None)
        if param is None:
            return
        
        if isinstance(param, bpy.types.ShaderNodeTexImage):
            image = self.scene.saveImage(param)
            template[key] = keyParam.replace(" ", "")
            template["Type"] = "sampler"
            template["TextureName"] = image.name
            template["TexturePath"] = self.scene.getAbsImagePath(image.getRelativePath())
        else:
            template[key] = keyParam.replace(" ", "")
            template["Type"] = f"v{len(param)}" if isinstance(param, list) else "float"
            template["Value"] = param

    def processParams(self, val):
        if "Name" in val and "Type" in val:
            paramName = val["Name"]
            paramType = val["Type"].lower()
            paramValue = self.material.get(paramName)
            if paramType == "float" and paramValue is not None:
                val["Value"] = float(paramValue)
            elif paramType == "v3" and paramValue is not None:
                val["Value"] = [float(x) for x in paramValue]
    
    def processTemplate(self, template):
        if not isinstance(template, dict):
            return
        
        self.processParams(template)
        for key, val in template.items():
            if isinstance(val, str):
                if len(val) > 2 and val[0] == '<' and val[-1] == '>':
                    keyParam = val.strip(" <>")
                    if keyParam in self.params.keys():
                        self.replaceSubstitution(template, key, keyParam)
                        break
            elif isinstance(val, dict):
                self.processTemplate(val)
            elif isinstance(val, list):
                for lval in val:
                    self.processTemplate(lval)

    def save(self):
        if not self.loadTemplate():
            return
        
        for node in self.material.node_tree.nodes:
            if node.label.startswith("MaterialShader"):
                self.considerShader(node)
        for node in self.material.node_tree.nodes:
            if node.type == "TEX_IMAGE":
                self.considerImage(node)
                
        self.processTemplate(self.template)
        IO.saveJson(self.scene.getAbsSysPath(self.getRelativePath()), self.template)
