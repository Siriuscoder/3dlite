# Lite 3d rendering engine

[![Build status](https://github.com/Siriuscoder/3dlite/actions/workflows/ci.yaml/badge.svg)](https://github.com/Siriuscoder/3dlite/actions/workflows/ci.yaml)
[![Release](https://github.com/Siriuscoder/3dlite/actions/workflows/release.yaml/badge.svg?event=push)](https://github.com/Siriuscoder/3dlite/actions/workflows/release.yaml)

## About

Light weight 3d/2d graphics rendering helper. "Engine" is not correct word for this library. 
It is designed as adapter from low level OpenGL API to more simple high level API. This API contains some high level 
concepts like scene, material, mesh, shader.. etc. whereas OpenGL API provides low level concepts like Framebuffer, 
vertex buffer, texture buffer and others.

Lite3d library support only OpenGL graphics API as back end for GPU operations. No sound, no physics, graphics only!
One of the reason of using OpenGL API was a potential support of many OS. Windows/Linux supports now, but 
library may be simply ported on others platforms. 

## API

Library provides two API levels:
  1. Low level C API (3dlite), common graphic operations.
  2. High level C++ API (3dlitepp), user friendly C++ components and scripting API.
(See samples for more details)

## Dependencies

* [DevIL](https://github.com/DentonW/DevIL)
* [Assimp](https://github.com/assimp/assimp)
* [SDL2](https://github.com/libsdl-org/SDL)
* [FreeType](https://github.com/freetype/freetype)
* [Bullet](https://github.com/bulletphysics/bullet3)

## Building project

Use [cmake](http://www.cmake.org/) 3.21.0 or higher. Base dependencies already provided in [deps](https://github.com/Siriuscoder/3dlite/tree/master/deps) directory (except Bullet). Hovewer on windows recomended to use vcpkg build. 

All intructions assume that /path/to/3dlite/ is root of the project.

### Build with existing dependencies (Linux)

```
$ cd /path/to/3dlite
$ mkdir build/
$ cmake --preset=Linux64_ci_gcc_release
$ cmake --build build/ --config Release
``` 

### Build with existing dependencies (Windows)

Recommended to use cmake GUI to generate Visual Studio projects on windows OR

```
$ cd /path/to/3dlite
$ mkdir build/
$ cmake --preset=Windows64_ci_release
$ cmake --build build/ --config Release
``` 

### Build using vcpkg (Linux)

```
$ cd /path/to/3dlite
$ mkdir build/
$ cmake --preset=Linux64_vcpkg
$ cmake --build build/ --config Release
``` 

### Build using vcpkg (Windows, Visual Studio 2022)

```
$ cd /path/to/3dlite
$ mkdir build/
$ cmake --preset=Windows64_vcpkg
$ cmake --build build/ --config Release
``` 
Or just call build_win_2022.bat

### All presets

* Linux64_vcpkg_debug 
* Linux64_vcpkg
* Linux64_ci_gcc_debug
* Linux64_ci_gcc_release
* Linux64_ci_clang_debug (with -fsanitize)
* Linux64_ci_clang_release
* Windows64_vcpkg_debug
* Windows64_vcpkg
* Windows64_ci_debug
* Windows64_ci_release

## Samples

After successfully compiling you can find some samples in bin directory. To provide samples work correctly you must download 
resources pack [vault.pkg](https://drive.google.com/file/d/1JhsirjKwMq51IBg7GknUeZLGvrVKp1Sz/view?usp=sharing) and put it to media/packs/

You can download resources for Vault_111 and Vault room sample from [here](https://drive.google.com/file/d/10sDEAlVEK4MVDGqzJgYaqVL5O0V8kRxo/view?usp=sharing), this 7z archive contains textures and models folders, you must copy this folders to media/vault_111 and anjoy this awesome samples!

Also you able to download resources for sponza remastered scene from [here](https://drive.google.com/file/d/1rhB6k-BmfULFcVFAaw4KaPaILxeD34ne/view?usp=sharing), unpack and copy textures and models folders to media/sponza.

## Screenshots

Robots instancing sample
![](/media/screenshots/robots.png "Robots")
![](/media/screenshots/robots1.png "Robots")
Reflection via cubemap sample
![](/media/screenshots/reflection1.png "ReflectionSky")
![](/media/screenshots/reflection2.png "ReflectionSky")
Sponza sample with MSAA render target and PCF shadows
![](/media/screenshots/sponza.png "Sponza")
![](/media/screenshots/sponza1.png "Sponza")
![](/media/screenshots/sponza2.png "Sponza")
![](/media/screenshots/sponza3.png "Sponza")
![](/media/screenshots/sponza4.png "Sponza")
Vault scene (Fallout3 models) with dynamic lighting with multiple lights and big range of materials. Supports two render modes: Deffered shading and Direct shading, also supports several light buffer techniques such as UBO, TBO, SSBO.
![](/media/screenshots/vault.png "Vault")
![](/media/screenshots/vault1.png "Vault")
![](/media/screenshots/vault2.png "Vault")
![](/media/screenshots/vault3.png "Vault")
![](/media/screenshots/vault4.png "Vault")
Vault 111 scene (Fallout4 models) with complete PBR lighting and HDR rendering, Deffered shading pipeline, dynamic PCF shadows from multiple light sources, Physical Bloom effect and huge range of materials and models.
![](/media/screenshots/vault_111_0.png "Vault111")
![](/media/screenshots/vault_111_1.png "Vault111")
![](/media/screenshots/vault_111_2.png "Vault111")
![](/media/screenshots/vault_111_3.png "Vault111")
![](/media/screenshots/vault_111_4.png "Vault111")
![](/media/screenshots/vault_111_5.png "Vault111")
![](/media/screenshots/vault_111_6.png "Vault111")
![](/media/screenshots/vault_111_7.png "Vault111")
![](/media/screenshots/vault_111_8.png "Vault111")
![](/media/screenshots/vault_111_9.png "Vault111")
![](/media/screenshots/vault_111_10.png "Vault111")
![](/media/screenshots/vault_111_11.png "Vault111")
![](/media/screenshots/vault_111_12.png "Vault111")
![](/media/screenshots/vault_111_13.png "Vault111")
![](/media/screenshots/vault_111_14.png "Vault111")
![](/media/screenshots/vault_111_15.png "Vault111")
![](/media/screenshots/vault_111_16.png "Vault111")
![](/media/screenshots/vault_111_17.png "Vault111")
![](/media/screenshots/vault_111_18.png "Vault111")
![](/media/screenshots/vault_111_19.png "Vault111")
![](/media/screenshots/vault_111_20.png "Vault111")
![](/media/screenshots/vault_111_21.png "Vault111")
![](/media/screenshots/vault_111_22.png "Vault111")
![](/media/screenshots/vault_111_23.png "Vault111")
![](/media/screenshots/vault_111_24.png "Vault111")
![](/media/screenshots/vault_111_25.png "Vault111")
![](/media/screenshots/vault_111_26.png "Vault111")
Vault 111 room scene (Fallout4 models) with complete PBR lighting and HDR rendering + Bullet Physics support.
![](/media/screenshots/vault_room_0.png "Vault room")
![](/media/screenshots/vault_room_1.png "Vault room")
![](/media/screenshots/vault_room_2.png "Vault room")
![](/media/screenshots/vault_room_3.png "Vault room")
![](/media/screenshots/vault_room_4.png "Vault room")

***
Check branch master to fetch stable changes, and follow new releases =)
> Copyright © 2014-2024 Nikita Korolev (Sirius)
