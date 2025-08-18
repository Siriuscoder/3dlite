import json 
from io_scene_lite3d.logger import log

class IO:
    JsonIndent = 2

    @staticmethod
    def saveJson(path, collect, sort = False):
        with open(path, 'w') as file:
            json.dump(collect, file, indent = IO.JsonIndent, sort_keys = sort)
            log.info(f"saved ok {path}")
