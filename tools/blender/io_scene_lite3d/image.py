import bpy
import shutil
from pathlib import PurePosixPath, Path
from io_scene_lite3d.io import IO
from io_scene_lite3d.logger import log

class Image:
    def __init__(self, texture, scene):
        self.namestem = Path(texture.image.name).stem
        self.image = texture.image
        self.interpolation = texture.interpolation
        self.extension = texture.extension
        self.name = self.namestem + ".texture"
        self.scene = scene
        # В Блендере относительные пути начинаются с //
        # Внимание! ипользуйте абсолютные пути для текстур, относительные пути будут работать только 
        # на одном диске с бендерфайлом
        if self.image.filepath.startswith("//"):
            self.imagePath = Path(bpy.path.abspath("//")[2:]) / Path(self.image.filepath[2:])
        else:
            self.imagePath = Path(self.image.filepath)
        
    def getRelativePath(self):
        return PurePosixPath("textures/json") / f"{self.namestem}.json"
    
    def getRelativeImagePath(self):
        return PurePosixPath("textures/images") / self.imagePath.name
    
    def save(self):
        # genarate image json 
        imageJson = {
            "Filtering": "None" if self.interpolation == "Closest" else "Trilinear",
            "ImageFormat": self.imagePath.suffix.strip(".").upper(),
            "TextureType": "2D",
            "Compression": True,
            "Wrapping": "Repeat" if self.extension == "REPEAT" else "ClampToEdge",
            "Image": self.scene.getAbsImagePath(self.getRelativeImagePath())
        }

        if self.image.colorspace_settings.name == "sRGB":
            imageJson["sRGB"] = True
        
        IO.saveJson(self.scene.getAbsSysPath(self.getRelativePath()), imageJson)

        if self.scene.options["copyTexImages"]:
            # Copy raw image
            destPath = self.scene.getAbsSysPath(self.getRelativeImagePath())
            if self.imagePath.is_file():
                shutil.copyfile(self.imagePath, destPath)
                log.info(f"{self.imagePath} copied to {destPath}")
            else:
                self.image.save(filepath = destPath)
                log.info(f"{self.imagePath} not found, saved image to {destPath}")
