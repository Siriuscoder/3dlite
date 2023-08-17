import bpy
import json
from pathlib import PurePosixPath, Path
from io_scene_lite3d.io import IO
from io_scene_lite3d.logger import log

class Material:
    def __init__(self, mat, scene):
        self.material = mat
        self.name = mat.name + ".material"
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
    
    def considerBSDF(self, node):
        for socket in node.inputs:
            if socket.is_linked or socket.is_unavailable or not socket.enabled:
                continue
            if socket.name == "Base Color":
                self.params["Albedo"] = [x for x in socket.default_value] # Color 4i
                alphaSocket = node.inputs["Alpha"]
                if not alphaSocket.is_linked and not socket.is_unavailable and socket.enabled:
                    self.params["Albedo"][3] = float(alphaSocket.default_value)
            elif socket.name == "Emission":
                self.params["Emission"] = [x for x in socket.default_value] # Color 4i
            elif socket.name in ["Metallic", "Specular", "Roughness", "IOR"]:
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
            template[key] = keyParam
            template["Type"] = "sampler"
            template["TextureName"] = image.name
            template["TextureName"] = self.scene.getAbsPath(image.getRelativePath())
        else:
            template[key] = keyParam
            template["Type"] = f"v{len(param)}" if isinstance(param, list) else "float"
            template["Value"] = param

    
    def processTemplate(self, template):
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
            if node.type == "BSDF_PRINCIPLED":
                self.considerBSDF(node)
        for node in self.material.node_tree.nodes:
            if node.type == "TEX_IMAGE":
                self.considerImage(node)
                
        self.processTemplate(self.template)
        IO.saveJson(self.scene.getAbsSysPath(self.getRelativePath()), self.template)
