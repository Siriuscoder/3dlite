import bpy
import time
import traceback
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, FloatProperty
from pathlib import Path
from io_scene_lite3d.logger import log
from io_scene_lite3d.scene import Scene

class Lite3dExport(bpy.types.Operator, ExportHelper):
    bl_idname = "export.lite3d"
    bl_label = "Export Lite3d"
    __doc__ = "Lite3d format scene export"

    filepath: StringProperty(subtype="FILE_PATH")
    packageName: StringProperty(name = "Package Name", default = "samples")
    filename_ext = ""
    filter_glob: StringProperty(default = "", options = {'HIDDEN'})
    saveTangent: BoolProperty(defaut = True, description = "Calculate and save tangents to models data")
    saveBiTangent: BoolProperty(defaut = False, description = "Calculate and save bitangents to models data")
    copyTexImage: BoolProperty(defaut = True, description = "Try to copy texture images to textures/images/, use absolute path while loading textures in Blender to works it correct")
    removeDoubles: BoolProperty(defaut = False, description = "Optimize mesh, remove double vertices")
    triangulate: BoolProperty(defaut = False, description = "Convert quads faces to tris")
    defaultConstantAttenuation: FloatProperty(defaut = 0.0, description = "Default light constant attenuation")
    defaultLinearAttenuation: FloatProperty(defaut = 0.01, description = "Default light linear attenuation")
    defaultQuadraticAttenuation: FloatProperty(defaut = 0.0001, description = "Default light quadratic attenuation")
    defaultInfluenceDistance: FloatProperty(defaut = 0.0, description = "Default light influence distance all sources")
    defaultInfluenceMinRadiance: FloatProperty(defaut = 0.001, description = "Default minimum light radiance which considered in light computation")

    def execute(self, context):
        start = time.perf_counter()
        try:
            exportPath = Path(self.filepath)
            scene = Scene(exportPath.stem, 
                          exportPath.parent, 
                          self.packageName, 
                          saveTangent = self.saveTangent, 
                          saveBiTangent = self.saveBiTangent,
                          copyTexImage = self.copyTexImage,
                          removeDoubles = self.removeDoubles,
                          triangulate = self.triangulate,
                          defaultConstantAttenuation = self.defaultConstantAttenuation,
                          defaultLinearAttenuation = self.defaultLinearAttenuation,
                          defaultQuadraticAttenuation = self.defaultQuadraticAttenuation,
                          defaultInfluenceDistance = self.defaultInfluenceDistance,
                          defaultInfluenceMinRadiance = self.defaultInfluenceMinRadiance)
            scene.exportScene()
        except Exception as ex:
            log.error(traceback.format_exc())
            return {'CANCELLED'}

        log.info("Export OK: {:10.4f} sec".format(time.perf_counter() - start))
        return {'FINISHED'}
