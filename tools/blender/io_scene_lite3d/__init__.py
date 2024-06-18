bl_info = {
    "name": "Lite3d Scene Exporter",
    "author": "Nikita Korolev, Sirius",
    "version": (2, 1, 0),  # can't read from VERSION, blender wants it hardcoded,
    "blender": ( 2, 80, 0 ),
    "api": 36079,
    "location": "File > Export > Lite3d Scene Export",
    "description": "Lite3d Scene Export, see docs",
    "warning": "",
    "wiki_url": "https://github.com/Siriuscoder/3dlite",
    "tracker_url": "https://github.com/Siriuscoder/3dlite/issues",
    "category": "Import-Export"
}

import bpy
from io_scene_lite3d.logger import log
from io_scene_lite3d.export import Lite3dExport

log.init()

def menu_func(self, context):
    self.layout.operator(Lite3dExport.bl_idname, text = "Lite3d Scene Export")

def register():
    bpy.utils.register_class(Lite3dExport)
    bpy.types.TOPBAR_MT_file_export.append(menu_func)

def unregister():
    bpy.utils.unregister_class(Lite3dExport)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func)
