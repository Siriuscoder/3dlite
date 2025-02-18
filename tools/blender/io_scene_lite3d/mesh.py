import bpy
import struct
from pathlib import PurePosixPath
from io_scene_lite3d.mesh_chunk import MeshChunk
from io_scene_lite3d.io import IO
from io_scene_lite3d.logger import log

class Mesh:
    sig = 0xBEEB0001
    version = (1 << 16) | (0 << 8) | 1
    
    def __init__(self, obj, scene):
        self.mesh = obj.data
        self.meshPartition = self.mesh.get("Partition")
        self.name = obj.data.name + ".mesh"
        self.object = obj
        self.scene = scene
        self.chunks = {}

        self.prepareMesh()

        uvLayer = self.mesh.uv_layers.active.data

        vertexColors = None
        if len(self.mesh.color_attributes) > 0 and self.scene.options["vertexColors"]:
            vertexColors = self.mesh.color_attributes.values()[0]

        for poly in self.mesh.polygons:
            if poly.loop_total != 3:
                raise Exception("N-Gon face, please triangulate faces and try again")
            
            meshChunk = self.chunks.get(poly.material_index, None)
            if meshChunk is None:
                meshChunk = MeshChunk(poly.material_index, self.scene.options)
                self.chunks[poly.material_index] = meshChunk
            
            for loopIndex in range(poly.loop_start, poly.loop_start + poly.loop_total):
                loop = self.mesh.loops[loopIndex]
                vertex = self.mesh.vertices[loop.vertex_index]
                uv = uvLayer[loopIndex].uv
                vertexColor = vertexColors.data[loop.vertex_index].color if vertexColors is not None else None
                meshChunk.appendVertex(vertex, vertexColor, loop.normal, uv, loop.tangent, loop.bitangent)

    def prepareMesh(self):
        if self.scene.options["removeDoubles"]:
            bpy.context.view_layer.objects.active = self.object
            bpy.ops.object.mode_set(mode = 'EDIT')
            bpy.ops.mesh.select_all(action = 'SELECT')
            bpy.ops.mesh.remove_doubles()
            bpy.ops.object.mode_set(mode = 'OBJECT')
        if self.scene.options["triangulate"]:
            bpy.context.view_layer.objects.active = self.object
            bpy.ops.object.mode_set(mode = 'EDIT')
            bpy.ops.mesh.select_all(action = 'SELECT')
            bpy.ops.mesh.quads_convert_to_tris()
            bpy.ops.object.mode_set(mode = 'OBJECT')

        # calc tangents anyway
        self.mesh.calc_tangents()
                
    def printStats(self):
        log.debug("Mesh {}: chunks {}".format(self.name, len(self.chunks)))
        for chunk in self.chunks.values():
            chunk.printStat()
    
    def getRelativePath(self):
        return PurePosixPath("models/meshes/") / f"{self.mesh.name}.m"

    def getRelativePathJson(self):
        return PurePosixPath("models/json/") / f"{self.mesh.name}.json"
                
    def save(self):
        if self.scene.options["singlePartition"]:
            if self.meshPartition is None:
                self.meshPartition = self.scene.name + ".mesh_partition"
        
        meshJson = {
            "Codec": "m",
            "Model": self.scene.getAbsMeshPath(self.getRelativePath())
        }

        if self.meshPartition is not None:
            meshJson["Partition"] = self.meshPartition
        
        if len(self.mesh.materials) > 0:
            materialMapping = []
            for chunkIndex in sorted(self.chunks.keys()):
                chunk = self.chunks[chunkIndex]
                material = self.scene.saveMaterial(self.mesh.materials[chunk.materialID])
                materialMapping.append({
                    "Material": {
                        "Type": "PBR" if self.scene.options["materialTypePBR"] else "Default",
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
        chunkSectionSize = sum([chunk.chunkHeaderSize() for chunk in self.chunks.values()])
        vertexSectionSize = sum([chunk.verticesSize for chunk in self.chunks.values()])
        indexSectionSize = sum([chunk.indexesSize for chunk in self.chunks.values()])
        file.write(struct.pack("=I5i", Mesh.sig, Mesh.version, chunkSectionSize, vertexSectionSize,
            indexSectionSize, chunkCount))
        return vertexSectionSize, indexSectionSize
        
    def saveModel(self):
        path = self.scene.getAbsSysPath(self.getRelativePath())
        with open(path, "wb") as file:
            (vertexSectionSize, indexSectionSize) = self.saveHeader(file)
            
            indexesOffset = 0
            verticesOffset = 0
            chunksIndexes = sorted(self.chunks.keys())
            for chunkIndex in chunksIndexes:
                chunk = self.chunks[chunkIndex]
                chunk.saveChunkHeader(indexesOffset, verticesOffset, file)
                indexesOffset += chunk.indexesSize
                verticesOffset += chunk.verticesSize
        
            # Save vertex data for each chunk one by one
            for chunkIndex in chunksIndexes:
                chunk = self.chunks[chunkIndex]
                chunk.saveVertexBlock(file)

            # Save index data for each chunk one by one (if exist)
            if indexSectionSize > 0:
                for chunkIndex in chunksIndexes:
                    chunk = self.chunks[chunkIndex]
                    chunk.saveIndexBlock(file)
                
            log.info(f"saved ok {path}, vertexes {vertexSectionSize} bytes, indexes {indexSectionSize} bytes")
