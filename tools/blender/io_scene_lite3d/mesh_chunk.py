import sys
import math
import struct
import mathutils
from io_scene_lite3d.io import IO
from io_scene_lite3d.logger import log

class Vertex:
    def __init__(self, v, n, uv, t):
        self.v = v      # vertex
        self.n = n      # normal
        self.uv = uv    # UV
        self.t = t      # tangent
        
    def save(self, file):
        file.write(struct.pack("=11f", self.v.x, self.v.y, self.v.z,
            self.n.x, self.n.y, self.n.z, self.uv.x, self.uv.y, 
            self.t.x, self.t.y, self.t.z))

class MeshChunk:
    def __init__(self, materialID):
        self.materialID = materialID
        self.vertices = []
        self.indexes = []
        self.normalsSplit = {}
        self.minVec = mathutils.Vector([sys.float_info.max] * 3)
        self.maxVec = mathutils.Vector([sys.float_info.min] * 3)
        
    def compareNear(a, b):
        return math.isclose(a.x, b.x) and math.isclose(a.y, b.y) and math.isclose(a.z, b.z)
        
    def insertByIndex(self, vi, vertex, n, uv, t):
        self.indexes.append(vi)
        if vi >= len(self.vertices):
            self.vertices.append(Vertex(vertex.co, n, uv, t))
        
    def appendVertex(self, vertex, n, uv, t): 
        lv = len(self.vertices)
        if not vertex.index in self.normalsSplit.keys():
            self.normalsSplit[vertex.index] = [(n, lv)]
        else:
            normalSplit = self.normalsSplit[vertex.index]
            for ins in normalSplit:
                if MeshChunk.compareNear(ins[0], n):
                    self.insertByIndex(ins[1], vertex, n, uv, t)
                    return
                
            normalSplit.append((n, lv))
                
        self.insertByIndex(lv, vertex, n, uv, t)
        self.minmaxVec(vertex.co)
        
    def minmaxVec(self, co):
        self.minVec.x = min(self.minVec.x, co.x)
        self.minVec.y = min(self.minVec.y, co.y)
        self.minVec.z = min(self.minVec.z, co.z)
                
        self.maxVec.x = max(self.maxVec.x, co.x)
        self.maxVec.y = max(self.maxVec.y, co.y)
        self.maxVec.z = max(self.maxVec.z, co.z)
        
    def saveBoundingVol(self, file):
        vmin = self.minVec
        vmax = self.maxVec
        # Размеры коробки описывающей данный chunk обьекта
        l = vmax.x - vmin.x
        w = vmax.y - vmin.y
        h = vmax.z - vmin.z
        # Стороны коробки
        file.write(struct.pack("=3f", vmin.x,     vmin.y,     vmin.z    ))
        file.write(struct.pack("=3f", vmin.x,     vmin.y,     vmin.z + h))
        file.write(struct.pack("=3f", vmin.x,     vmin.y + w, vmin.z + h))
        file.write(struct.pack("=3f", vmin.x,     vmin.y + w, vmin.z    ))
        file.write(struct.pack("=3f", vmin.x + l, vmin.y,     vmin.z    ))
        file.write(struct.pack("=3f", vmin.x + l, vmin.y,     vmin.z + h))
        file.write(struct.pack("=3f", vmin.x + l, vmin.y + w, vmin.z    ))
        file.write(struct.pack("=3f", vmax.x,     vmax.y,     vmax.z    ))
        # Центр сферы описыващей данный chunk обьекта
        center = mathutils.Vector([l / 2.0, w / 2.0, h / 2.0])
        sphereCenter = vmin + center 
        file.write(struct.pack("=3f", sphereCenter.x, sphereCenter.y, sphereCenter.z))
        file.write(struct.pack("=f", center.length))
    
    def saveLayout(self, file):
        file.write(struct.pack("=2B", 0x0, 3)) # LITE3D_BUFFER_BINDING_VERTEX
        file.write(struct.pack("=2B", 0x2, 3)) # LITE3D_BUFFER_BINDING_NORMAL
        file.write(struct.pack("=2B", 0x3, 2)) # LITE3D_BUFFER_BINDING_TEXCOORD
        file.write(struct.pack("=2B", 0x5, 3)) # LITE3D_BUFFER_BINDING_TANGENT
        
    def save(self, file):
        for v in self.vertices:
            v.save(file)
        for i in self.indexes:
            file.write(struct.pack("=I", i))
        
    def printStat(self):
        log.debug("  - ID {}, Vertices: {}, Indexes {}".format(self.materialID, len(self.vertices), len(self.indexes)))
     