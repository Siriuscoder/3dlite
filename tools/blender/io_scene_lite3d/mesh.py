import struct
from pathlib import PurePosixPath
from io_scene_lite3d.mesh_chunk import MeshChunk
from io_scene_lite3d.io import IO
from io_scene_lite3d.logger import log

class Mesh:
    sig = 0xBEEB0001
    version = (1 << 16) | (0 << 8) | 1
    chunkSize = (149 + (2 * 4))
    vertexSize = 4 * (3 + 3 + 2 + 3)
    indexSize = 4
    layoutCount = 4
    
    def __init__(self, data, scene):
        self.mesh = data
        self.name = data.name + ".mesh"
        self.scene = scene
        self.chunks = {}
        self.indexesOffset = 0
        self.verticesOffset = 0
        
        self.mesh.calc_tangents()
        for poly in self.mesh.polygons:
            uvLayer = self.mesh.uv_layers.active.data
            
            meshChunk = self.chunks.get(poly.material_index, None)
            if meshChunk is None:
                meshChunk = MeshChunk(poly.material_index)
                self.chunks[poly.material_index] = meshChunk
            
            for loopIndex in range(poly.loop_start, poly.loop_start + poly.loop_total):
                loop = self.mesh.loops[loopIndex]
                vertex = self.mesh.vertices[loop.vertex_index]
                uv = uvLayer[loopIndex].uv
                meshChunk.appendVertex(vertex, loop.normal, uv, loop.tangent)
                
    def printStats(self):
        log.debug("Mesh {}: chunks {}".format(self.name, len(self.chunks)))
        for chunk in self.chunks.values():
            chunk.printStat()
    
    def getRelativePath(self):
        return PurePosixPath("models/meshes/") / f"{self.mesh.name}.m"

    def getRelativePathJson(self):
        return PurePosixPath("models/json/") / f"{self.mesh.name}.json"
                
    def save(self):
        meshJson = {
            "Codec": "m",
            "Model": self.scene.getAbsPath(self.getRelativePath())
        }
        
        materialMapping = []
        for chunkIndex in sorted(self.chunks.keys()):
            chunk = self.chunks[chunkIndex]
            material = self.scene.saveMaterial(self.mesh.materials[chunk.materialID])
            materialMapping.append({
                "Material": {
                    "Name": material.name,
                    "Material": self.scene.getAbsPath(material.getRelativePath())
                },
                "MaterialIndex": chunk.materialID
            })
            
        meshJson["MaterialMapping"] = materialMapping
        IO.saveJson(self.scene.getAbsSysPath(self.getRelativePathJson()), meshJson)
        
        self.saveModel()
        self.printStats()
        
    def saveHeader(self, file):
        chunkCount = len(self.chunks)
        chunkSectionSize = chunkCount * Mesh.chunkSize
        vertexSectionSize = Mesh.vertexSize * sum([len(chunk.vertices) for chunk in self.chunks.values()])
        indexSectionSize = Mesh.indexSize * sum([len(chunk.indexes) for chunk in self.chunks.values()])
        file.write(struct.pack("=I5i", Mesh.sig, Mesh.version, chunkSectionSize, vertexSectionSize,
            indexSectionSize, chunkCount))
        
    def saveChunksInfo(self, chunk, file):
        indexesCount = len(chunk.indexes)
        indexesSize = indexesCount * Mesh.indexSize
        verticesCount = len(chunk.vertices)
        verticesSize = verticesCount * Mesh.vertexSize
        file.write(struct.pack("=8iBI", Mesh.chunkSize, Mesh.layoutCount, indexesCount, 
            indexesSize, self.indexesOffset, verticesCount, verticesSize, self.verticesOffset, 
            self.indexSize, chunk.materialID))
            
        self.indexesOffset += indexesSize
        self.verticesOffset += verticesSize
        
        chunk.saveBoundingVol(file)
        chunk.saveLayout(file)
        
    def saveModel(self):
        path = self.scene.getAbsSysPath(self.getRelativePath())
        with open(path, "wb") as file:
            self.saveHeader(file)
            
            self.indexesOffset = 0
            self.verticesOffset = 0
            chunksIndexes = sorted(self.chunks.keys())
            for chunkIndex in chunksIndexes:
                chunk = self.chunks[chunkIndex]
                self.saveChunksInfo(chunk, file)
        
            for chunkIndex in chunksIndexes:
                chunk = self.chunks[chunkIndex]
                chunk.save(file)
                
            log.info(f"saved ok {path}")
