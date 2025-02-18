import sys
import math
import struct
import mathutils
from io_scene_lite3d.io import IO
from io_scene_lite3d.logger import log

class Vertex:
    def __init__(self, v, c, n, uv, t, bt, saveTangent, saveBiTangent, vertexColor, skeleton, flipUV):
        # vertex, normal, UV is always required
        self.block = [v.co.x, v.co.y, v.co.z, n.x, n.y, n.z, uv.x, 1.0 - uv.y if flipUV else uv.y]
        self.format = f"={len(self.block)}f"

        if saveTangent:
            # tangent is optional
            self.block.extend([t.x, t.y, t.z])
            self.format += "3f"

        if saveBiTangent:
            # bitangent is optional
            self.block.extend([bt.x, bt.y, bt.z])
            self.format += "3f"

        if vertexColor:
            # vertex color is optional
            self.block.extend([x for x in c])
            self.format += "4f"

        if skeleton:
            # Support only first 4 groups, ignore others
            for i in range(4):
                self.block.append(v.groups[i].group if i < len(v.groups) else -1)
            self.format += "4i"

            for i in range(4):
                self.block.append(v.groups[i].weight if i < len(v.groups) else 0.0)
            self.format += "4f"

    def size(self):
        return struct.calcsize(self.format)
        
    def save(self, file):
        file.write(struct.pack(self.format, *self.block))

class MeshChunk:
    indexSize = 4

    def __init__(self, materialID, opts):
        self.materialID = materialID
        self.saveTangent = opts["saveTangent"]
        self.saveBiTangent = opts["saveBiTangent"]
        self.flipUV = opts["flipUV"]
        self.indexedGeometry = opts["indexedGeometry"]
        self.vertexColors = opts["vertexColors"]
        self.skeleton = opts["skeleton"]
        self.vertices = []
        self.indexes = []
        self.normalsSplit = {}
        self.minVec = mathutils.Vector([sys.float_info.max] * 3)
        self.maxVec = mathutils.Vector([sys.float_info.min] * 3)
        self.verticesSize = 0
        self.indexesSize = 0
        self.layoutCount = 3
        self.layoutCount += 1 if self.saveTangent else 0
        self.layoutCount += 1 if self.saveBiTangent else 0
        self.layoutCount += 1 if self.vertexColors else 0
        self.layoutCount += 2 if self.skeleton else 0
        self.chunkHeaderFormat = "=8iBI"

    def chunkHeaderSize(self):
        boundingVolSize = struct.calcsize("=28f")
        return struct.calcsize(f"={self.layoutCount * 2}B") + struct.calcsize(self.chunkHeaderFormat) + boundingVolSize
        
    def compareNear(a, b):
        return math.isclose(a.x, b.x) and math.isclose(a.y, b.y) and math.isclose(a.z, b.z)
        
    def insertByIndex(self, vi, v, c, n, uv, t, bt):
        self.indexes.append(vi)
        self.indexesSize += MeshChunk.indexSize
        if vi >= len(self.vertices):
            self.insertVertex(v, c, n, uv, t, bt)

    def insertVertex(self, v, c, n, uv, t, bt):
        self.vertices.append(Vertex(v, c, n, uv, t, bt, self.saveTangent, self.saveBiTangent, 
            self.vertexColors, self.skeleton, self.flipUV))
        self.verticesSize += self.vertices[-1].size()
        
    def appendVertex(self, v, c, n, uv, t, bt):
        # Indexed geometry
        if self.indexedGeometry:
            lv = len(self.vertices)
            if not v.index in self.normalsSplit.keys():
                self.normalsSplit[v.index] = [(n, lv)]
            else:
                normalSplit = self.normalsSplit[v.index]
                for ins in normalSplit:
                    if MeshChunk.compareNear(ins[0], n):
                        self.insertByIndex(ins[1], v, c, n, uv, t, bt)
                        return
                normalSplit.append((n, lv))
            self.insertByIndex(lv, v, c, n, uv, t, bt)
        # Non indexed geometry
        else:
            self.insertVertex(v, c, n, uv, t, bt)

        self.minmaxVec(v.co)
        
    def minmaxVec(self, co):
        self.minVec.x = min(self.minVec.x, co.x)
        self.minVec.y = min(self.minVec.y, co.y)
        self.minVec.z = min(self.minVec.z, co.z)
                
        self.maxVec.x = max(self.maxVec.x, co.x)
        self.maxVec.y = max(self.maxVec.y, co.y)
        self.maxVec.z = max(self.maxVec.z, co.z)

    def saveChunkHeader(self, indexesOffset, verticesOffset, file):
        file.write(struct.pack(self.chunkHeaderFormat, self.chunkHeaderSize(), self.layoutCount, len(self.indexes), 
            self.indexesSize, indexesOffset, len(self.vertices), self.verticesSize, verticesOffset, 
            self.indexSize, self.materialID))
        
        self.saveBoundingVol(file)
        self.saveLayout(file)
        
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
        if self.saveTangent:
            file.write(struct.pack("=2B", 0x5, 3)) # LITE3D_BUFFER_BINDING_TANGENT
        if self.saveBiTangent:
            file.write(struct.pack("=2B", 0x6, 3)) # LITE3D_BUFFER_BINDING_BINORMAL
        if self.vertexColors:
            file.write(struct.pack("=2B", 0x1, 4)) # LITE3D_BUFFER_BINDING_COLOR
        if self.skeleton:
            file.write(struct.pack("=2B", 0x7, 4)) # LITE3D_BUFFER_BINDING_BONES
            file.write(struct.pack("=2B", 0x8, 4)) # LITE3D_BUFFER_BINDING_BONES_WEIGHT
    
    def saveVertexBlock(self, file):
        for v in self.vertices:
            v.save(file)

    def saveIndexBlock(self, file):
        file.write(struct.pack(f"={len(self.indexes)}I", *self.indexes))
        
    def printStat(self):
        log.debug("  - ID {}, Vertices: {}, Indexes {}".format(self.materialID, len(self.vertices), len(self.indexes)))
     