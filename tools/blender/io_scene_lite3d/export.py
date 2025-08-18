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
    filename_ext = ""
    filter_glob: StringProperty(default = "", options = {'HIDDEN'})

    physics: BoolProperty(name = "Physics", default = False, description = "Export physics parameters and collision shapes")
    saveTangent: BoolProperty(name = "Tangents", default = True, description = "Calculate and save tangents to models data")
    saveBiTangent: BoolProperty(name = "BiTangents", default = False, description = "Calculate and save bitangents to models data")
    copyTexImages: BoolProperty(name = "Copy Texture Images", default = True, description = "Try to copy texture images to textures/images/, use absolute path while loading textures in Blender to works it correct")
    textureCompression: BoolProperty(name = "Texture Compression", default = False, description = "Use texture compression while loading images by engine")
    removeDoubles: BoolProperty(name = "Remove Doubles", default = False, description = "Optimize mesh, remove double vertices")
    triangulate: BoolProperty(name = "Triangulate", default = False, description = "Convert quads faces to tris")
    exportLights: BoolProperty(name = "Lights", default = True, description = "Export light sources")
    flipUV: BoolProperty(name = "Flip UVs", default = False, description = "Flip UVs if needed (uv.y = 1.0 - uv.y)")
    singlePartition: BoolProperty(name = "Single mesh partition", default = False, description = "Mark all exported meshes as in single mesh partition")
    indexedGeometry: BoolProperty(name = "Indexed geometry", default = True, description = "Export index data as well as vertex data (consume less space), otherwise index data will not be used (may be more fast render)")
    skeleton: BoolProperty(name = "Skeleton", default = False, description = "Export skeleton data as additional vertex attributes")
    animation: BoolProperty(name = "Animation", default = False, description = "Export all available animation actions")

    def draw(self, context):
        layout = self.layout

        box = layout.box()
        box.label(text = "Common:")
        box.prop(self, "physics")
        box.prop(self, "animation")
        box.prop(self, "skeleton")
        box.label(text = "Mesh:")
        box.prop(self, "triangulate")
        box.prop(self, "removeDoubles")
        box.prop(self, "saveTangent")
        box.prop(self, "saveBiTangent")
        box.prop(self, "indexedGeometry")
        box.prop(self, "flipUV")
        box.prop(self, "singlePartition")
        box.label(text = "Materials:")
        box.prop(self, "copyTexImages")
        box.prop(self, "textureCompression")
        box.label(text = "Lighting:")
        box.prop(self, "exportLights")

    def execute(self, context):
        start = time.perf_counter()
        try:
            exportPath = Path(self.filepath)
            scene = Scene(exportPath.stem, 
                          exportPath.parent, 
                          saveTangent = self.saveTangent,
                          saveBiTangent = self.saveBiTangent,
                          copyTexImages = self.copyTexImages,
                          textureCompression = self.textureCompression,
                          removeDoubles = self.removeDoubles,
                          triangulate = self.triangulate,
                          flipUV = self.flipUV,
                          exportLights = self.exportLights,
                          physics = self.physics,
                          singlePartition = self.singlePartition,
                          indexedGeometry = self.indexedGeometry,
                          skeleton = self.skeleton,
                          animation = self.animation)
            
            scene.exportScene()
        except Exception as ex:
            log.error(traceback.format_exc())
            return {'CANCELLED'}

        log.info("Export OK: {:10.4f} sec".format(time.perf_counter() - start))
        return {'FINISHED'}

def menu_func(self, context):
    self.layout.operator(Lite3dExport.bl_idname, text = "Lite3d Scene Export")

def register_export():
    bpy.utils.register_class(Lite3dExport)
    bpy.types.TOPBAR_MT_file_export.append(menu_func)

def unregister_export():
    bpy.utils.unregister_class(Lite3dExport)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func)