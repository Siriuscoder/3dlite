# Lite 3d rendering engine

[![Build status](https://github.com/Siriuscoder/3dlite/actions/workflows/ci.yaml/badge.svg?branch=master)](https://github.com/Siriuscoder/3dlite/actions/workflows/ci.yaml)
[![Release](https://github.com/Siriuscoder/3dlite/actions/workflows/release.yaml/badge.svg?event=push)](https://github.com/Siriuscoder/3dlite/actions/workflows/release.yaml)
[![Lite3D CodeQL](https://github.com/Siriuscoder/3dlite/actions/workflows/codeql.yml/badge.svg?branch=master)](https://github.com/Siriuscoder/3dlite/actions/workflows/codeql.yml)  

## About

Light weight 3d/2d graphics rendering helper. "Engine" is not correct word for this library. 
It is designed as adapter from low level OpenGL API to more simple high level API. This API contains some high level 
concepts like scene, material, mesh, shader.. etc. whereas OpenGL API provides low level concepts like Framebuffer, 
vertex buffer, texture buffer and others.

Lite3d library support only OpenGL graphics API as back end for GPU operations. No sound, no physics, graphics only!
One of the reason of using OpenGL API was a potential support of many OS. Windows/Linux supports now, but 
library may be simply ported on others platforms.  

![Code Size](https://img.shields.io/github/languages/code-size/Siriuscoder/3dlite)
![Downloads](https://img.shields.io/github/downloads/Siriuscoder/3dlite/latest/total)
![Activity](https://img.shields.io/github/commit-activity/m/Siriuscoder/3dlite)

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

Use [cmake](http://www.cmake.org/) 3.21.0 or higher. Base dependencies already provided in [deps](https://github.com/Siriuscoder/3dlite/tree/master/deps) directory (except Bullet). Hovewer on windows is recomended to use vcpkg build. 

All intructions assume that /path/to/3dlite/ is the root of the project.

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

See [here](samples/README.md)

## Screenshots

![](/media/screenshots/sponza.png "Sponza")
![](/media/screenshots/sponza1.png "Sponza")
![](/media/screenshots/vault_111_0.png "Vault111")
![](/media/screenshots/vault_111_3.png "Vault111")
![](/media/screenshots/vault_111_19.png "Vault111")
![](/media/screenshots/vault_room_0.png "Vault room")

***
Check branch master to fetch stable changes, and follow new releases =)
> Copyright © 2014-2024 Nikita Korolev (Sirius)
