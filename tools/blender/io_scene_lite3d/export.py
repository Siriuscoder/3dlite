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
    packagename: StringProperty(name = "Package Name", default = "Samples")
    filename_ext = ""
    filter_glob : StringProperty(default = "", options = {'HIDDEN'})

    def execute(self, context):
        start = time.perf_counter()
        try:
            exportPath = Path(self.filepath)
            scene = Scene(exportPath.stem, exportPath.parent, self.packagename)
            scene.exportScene()
        except Exception as ex:
            log.error(traceback.format_exc())
            return {'CANCELLED'}

        log.info("Export OK: {:10.4f} sec".format(time.perf_counter() - start))
        return {'FINISHED'}
