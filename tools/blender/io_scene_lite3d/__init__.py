bl_info = {
    "name": "Lite3d Scene Exporter",
    "author": "Nikita Korolev, Sirius",
    "version": (2, 1, 3),  # can't read from VERSION, blender wants it hardcoded,
    "blender": ( 2, 80, 0 ),
    "api": 36079,
    "location": "File > Export > Lite3d Scene Export",
    "description": "Lite3d Scene Export, see docs",
    "warning": "",
    "wiki_url": "https://github.com/Siriuscoder/3dlite",
    "tracker_url": "https://github.com/Siriuscoder/3dlite/issues",
    "category": "Import-Export"
}

from io_scene_lite3d.logger import log
from io_scene_lite3d.export import register_export, unregister_export
from io_scene_lite3d.properties import register_properties, unregister_properties
from io_scene_lite3d.menus import register_menus, unregister_menus

log.init()

def register():
    register_export()
    register_properties()
    register_menus()
    
def unregister():
    unregister_menus()
    unregister_properties()
    unregister_export()
