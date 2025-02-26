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
    
    def save(self):
        for fcurve in self.action.fcurves:
            path = fcurve.data_path
            if not any(path.endswith(x) for x in ["location", "rotation_quaternion", "scale"]):
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
            actionJson["BonesFrames"] = self.skeletonFrames
        if len(self.frames) > 0:
            actionJson["Frames"] = self.frames

        IO.saveJson(self.scene.getAbsSysPath(self.getRelativePath()), actionJson, True)
