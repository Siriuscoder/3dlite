import sys
import json
from pathlib import Path

def replaceLightParams(light):
    lightType = light["Type"]
    del light["Ambient"]
    del light["Specular"]
    direction = light.pop("SpotDirection", None)
    attenuation = light.pop("Attenuation", None)
    spotFactor = light.pop("SpotFactor", None)
    position = light.pop("Position", None)

    if lightType != "Directional":
        if attenuation is not None:
            attenuationObj = {}
            attenuationObj["Constant"] = attenuation[0]
            attenuationObj["Linear"] = attenuation[1]
            attenuationObj["Quadratic"] = attenuation[2]
            attenuationObj["InfluenceDistance"] = attenuation[3]
            attenuationObj["InfluenceMinRadiance"] = 0.0
            light["Attenuation"] = attenuationObj

    if lightType != "Point":
        if direction is not None:
            light["Direction"] = direction

    if lightType == "Spot":
        if spotFactor is not None:
            spotFactorObj = {}
            spotFactorObj["AngleInnerCone"] = spotFactor[0]
            spotFactorObj["AngleOuterCone"] = spotFactor[1]
            light["SpotFactor"] = spotFactorObj

    if lightType in ["Point", "Spot"]:
        light["LightSize"] = 0.0
        light["Position"] = position

    light["Radiance"] = 1.0

    print(light)

def lookupLight(obj):
    if isinstance(obj, list):
        for o in obj:
            lookupLight(o)
    elif isinstance(obj, dict):
        for k, v in obj.items():
            if k == "Light":
                replaceLightParams(v)
            else:
                lookupLight(v)

if len(sys.argv) < 2:
    print("Error: Specify scene file")
    sys.exit(1)

sceneFilePath = Path(sys.argv[1])
if not sceneFilePath.is_file():
    print(f"Path is not a file or not exist: {sceneFilePath}")
    sys.exit(1)

sceneJson = None
with sceneFilePath.open() as sceneFile:
    sceneJson = json.load(sceneFile)
    lookupLight(sceneJson)

with sceneFilePath.open("w") as sceneFile:
    json.dump(sceneJson, sceneFile, indent = 2) 