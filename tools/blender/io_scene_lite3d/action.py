import copy
import mathutils
from pathlib import PurePosixPath
from io_scene_lite3d.io import IO

class AnimationAction:
    def __init__(self, scene, action):
        self.scene = scene
        self.name = action.name + ".action"
        self.action = action
        self.skeletonFrames = {}
        self.frames = {}

    def getRelativePath(self):
        return PurePosixPath("actions/") / f"{self.action.name}.json"
    
    def enrich(self, frames):
        # Sort by the frame
        framesSorted = dict(sorted(frames.items(), key = lambda x: float(x[0])))
        result = {}
        transform = {
            "location": 3 * [None],
            "rotation_quaternion": 4 * [None],
            "rotation_euler": 3 * [None],
            "scale": 3 * [None],
        }

        for frameNo, curves in framesSorted.items():
            for curve, value in curves.items():
                transform[curve[:-1]][['X', 'Y', 'Z', 'W'].index(curve[-1])] = value
            transformCopy = copy.deepcopy(transform)
            # Преобразуем вращение заданное углами эйлера в кватернион, если такое есть
            rotationEuler = transformCopy["rotation_euler"]
            if all([x is not None for x in rotationEuler]):
                rotationQ = mathutils.Euler(rotationEuler).to_quaternion()
                transformCopy["rotation_quaternion"] = [x for x in rotationQ]
            # Подчищаем пустные треки трансформации
            curvesToDel = [x[0] for x in transformCopy.items() if any([y is None for y in x[1]])]
            curvesToDel.append("rotation_euler")
            for x in set(curvesToDel):
                del transformCopy[x]
            result[frameNo] = transformCopy

        return result

    def save(self):
        for fcurve in self.action.fcurves:
            path = fcurve.data_path
            if not any(path.endswith(x) for x in ["location", "rotation_quaternion", "rotation_euler", "scale"]):
                continue

            axis = ['X', 'Y', 'Z', 'W'][fcurve.array_index]
            boneProbe = path.split('pose.bones["')
            frames = self.frames
            # this is bone animation curve
            if len(boneProbe) > 1:
                boneName = boneProbe[1].split('"]')[0]
                if not boneName in self.skeletonFrames:
                    self.skeletonFrames[boneName] = {}
                frames = self.skeletonFrames[boneName]
            # save animation keys
            for keyframe in fcurve.keyframe_points:
                frameKey = str(keyframe.co.x)
                if not frameKey in frames:
                    frames[frameKey] = {}
                frames[frameKey][path.split(".")[-1] + axis] = keyframe.co.y

        actionJson = {}
        actionJson["Name"] = self.name
        actionJson["MinFrame"] = self.action.curve_frame_range.x
        actionJson["MaxFrame"] = self.action.curve_frame_range.y

        if len(self.skeletonFrames) > 0:
            for name, bone in self.skeletonFrames.items():
                self.skeletonFrames[name] = self.enrich(bone)
            actionJson["SkeletonFrames"] = self.skeletonFrames
        if len(self.frames) > 0:
            actionJson["Frames"] = self.enrich(self.frames)

        IO.saveJson(self.scene.getAbsSysPath(self.getRelativePath()), actionJson)
