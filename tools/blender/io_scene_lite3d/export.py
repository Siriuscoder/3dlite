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
    imagePackageName: StringProperty(name = "Image Package Name", default = "samples")
    meshPackageName: StringProperty(name = "Mesh Package Name", default = "samples")
    filename_ext = ""
    filter_glob: StringProperty(default = "", options = {'HIDDEN'})
    saveTangent: BoolProperty(name = "Tangents", default = True, description = "Calculate and save tangents to models data")
    saveBiTangent: BoolProperty(name = "BiTangents", default = False, description = "Calculate and save bitangents to models data")
    copyTexImages: BoolProperty(name = "Copy Texture Images", default = True, description = "Try to copy texture images to textures/images/, use absolute path while loading textures in Blender to works it correct")
    removeDoubles: BoolProperty(name = "Remove Doubles", default = False, description = "Optimize mesh, remove double vertices")
    triangulate: BoolProperty(name = "Triangulate", default = False, description = "Convert quads faces to tris")
    exportLights: BoolProperty(name = "Lights", default = True, description = "Export light sources")
    flipUV: BoolProperty(name = "Flip UVs", default = False, description = "Flip UVs if needed (uv.y = 1.0 - uv.y)")
    defaultConstantAttenuation: FloatProperty(name = "Default Attenuation Constant", precision = 6, default = 0.0, description = "Default light constant attenuation")
    defaultLinearAttenuation: FloatProperty(name = "Default Attenuation Linear", precision = 6, default = 0.01, description = "Default light linear attenuation")
    defaultQuadraticAttenuation: FloatProperty(name = "Default Attenuation Quadratic", precision = 6, default = 0.0001, description = "Default light quadratic attenuation")
    defaultInfluenceDistance: FloatProperty(name = "Default Influence Distance", precision = 6, default = 0.0, description = "Default influence distance for light source")
    defaultInfluenceMinRadiance: FloatProperty(name = "Default Influence Minimum Radiance", precision = 6, default = 0.001, description = "Default minimum light radiance which considered in light computation")
    materialTemplate: StringProperty(name = "Material template name", default = "CommonMaterialTemplate", description = "Name of material template file used by default to generate material")

    def draw(self, context):
        layout = self.layout

        box = layout.box()
        box.label(text = "Common:")
        box.prop(self, "packageName")
        box.prop(self, "imagePackageName")
        box.prop(self, "meshPackageName")
        box.label(text = "Mesh:")
        box.prop(self, "triangulate")
        box.prop(self, "removeDoubles")
        box.prop(self, "saveTangent")
        box.prop(self, "saveBiTangent")
        box.prop(self, "flipUV")
        box.label(text = "Materials:")
        box.prop(self, "copyTexImages")
        box.prop(self, "materialTemplate")
        box.label(text = "Lighting:")
        box.prop(self, "exportLights")
        box.prop(self, "defaultConstantAttenuation")
        box.prop(self, "defaultLinearAttenuation")
        box.prop(self, "defaultQuadraticAttenuation")
        box.prop(self, "defaultInfluenceDistance")
        box.prop(self, "defaultInfluenceMinRadiance")

    def execute(self, context):
        start = time.perf_counter()
        try:
            exportPath = Path(self.filepath)
            scene = Scene(exportPath.stem, 
                          exportPath.parent, 
                          self.packageName, 
                          imagePackageName = self.imagePackageName,
                          meshPackageName = self.meshPackageName, 
                          saveTangent = self.saveTangent, 
                          saveBiTangent = self.saveBiTangent,
                          copyTexImages = self.copyTexImages,
                          removeDoubles = self.removeDoubles,
                          triangulate = self.triangulate,
                          flipUV = self.flipUV,
                          defaultConstantAttenuation = self.defaultConstantAttenuation,
                          defaultLinearAttenuation = self.defaultLinearAttenuation,
                          defaultQuadraticAttenuation = self.defaultQuadraticAttenuation,
                          defaultInfluenceDistance = self.defaultInfluenceDistance,
                          defaultInfluenceMinRadiance = self.defaultInfluenceMinRadiance,
                          materialTemplate = self.materialTemplate,
                          exportLights = self.exportLights)
            
            scene.exportScene()
        except Exception as ex:
            log.error(traceback.format_exc())
            return {'CANCELLED'}

        log.info("Export OK: {:10.4f} sec".format(time.perf_counter() - start))
        return {'FINISHED'}
