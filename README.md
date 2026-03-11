# Lite 3D Rendering Engine

[![Build status](https://github.com/Siriuscoder/3dlite/actions/workflows/ci.yaml/badge.svg?branch=master)](https://github.com/Siriuscoder/3dlite/actions/workflows/ci.yaml)
[![Release](https://github.com/Siriuscoder/3dlite/actions/workflows/release.yaml/badge.svg?event=push)](https://github.com/Siriuscoder/3dlite/actions/workflows/release.yaml)
[![Lite3D CodeQL](https://github.com/Siriuscoder/3dlite/actions/workflows/codeql.yml/badge.svg?branch=master)](https://github.com/Siriuscoder/3dlite/actions/workflows/codeql.yml)

Lite3D is a lightweight OpenGL-based rendering helper library that provides a higher-level API
for building real-time 3D applications and rendering experiments.

![](/media/screenshots/sponza1.png "Sponza")

## About

Lite3D is a lightweight 3D/2D graphics rendering helper library. The term *"engine"* is not entirely accurate for this project.  
It acts as an adapter that translates the low-level OpenGL API into a simpler, higher-level API.

This API introduces higher-level concepts such as **scene**, **material**, **mesh**, and **shader**, while OpenGL itself exposes low-level constructs like **framebuffers**, **vertex buffers**, and **texture buffers**.

Lite3D currently supports **OpenGL** as the only graphics backend.  
The library focuses strictly on rendering — it does not include built-in sound or game logic systems.

![Code Size](https://img.shields.io/github/languages/code-size/Siriuscoder/3dlite)
![Downloads](https://img.shields.io/github/downloads/Siriuscoder/3dlite/latest/total)
![Activity](https://img.shields.io/github/commit-activity/m/Siriuscoder/3dlite)

## Features

Rendering features demonstrated in the engine and sample applications:

- Physically Based Rendering (PBR)
- Deferred and forward rendering pipelines
- HDR rendering
- Dynamic shadow mapping with PCF filtering
- Instanced rendering
- Cubemap reflections
- Image-based lighting
- Global illumination approximation using light probes
- Multiple light buffer techniques (UBO, TBO, SSBO)
- Scene graph with meshes, materials, shaders, and cameras
- Optional Bullet Physics integration

## API

The library provides two API layers:

1. **Low-level C API (`3dlite`)** — provides common graphics operations.
2. **High-level C++ API (`3dlitepp`)** — provides user-friendly C++ components and scripting support.

See the **samples** for usage examples.

## Dependencies

* [DevIL](https://github.com/DentonW/DevIL)
* [Assimp](https://github.com/assimp/assimp)
* [SDL2](https://github.com/libsdl-org/SDL)
* [FreeType](https://github.com/freetype/freetype)
* [Bullet](https://github.com/bulletphysics/bullet3)

## Building the Project

Use [CMake](http://www.cmake.org/) **3.21.0 or higher**.

Base dependencies are already provided in the [deps](https://github.com/Siriuscoder/3dlite/tree/master/deps) directory (except **Bullet**).  
However, on **Windows it is recommended to use vcpkg** for dependency management and building.

All instructions assume that `/path/to/3dlite/` is the root directory of the project.

### Build with existing dependencies (Linux)

```
$ cd /path/to/3dlite
$ mkdir build/
$ cmake --preset=Linux64_ci_gcc_release
$ cmake --build build/ --config Release
``` 

### Build with existing dependencies (Windows)

It is recommended to use **CMake GUI** to generate Visual Studio projects on Windows.

Alternatively:

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

Or simply run build_win_2022.bat

### Available CMake presets

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

See the [samples documentation](samples/README.md)

***
Check the **master** branch to get stable changes and follow new releases.
> Copyright © 2014-2026 Nikita Korolev (Sirius)
