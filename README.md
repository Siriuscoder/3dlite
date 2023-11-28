Lite 3d rendering engine
========================
[![Build status](https://ci.appveyor.com/api/projects/status/o83rtxvu3g8dvd0g?svg=true)](https://ci.appveyor.com/project/Siriuscoder/3dlite)

About
-----
Light weight 3d/2d graphics rendering helper. "Engine" is not correct word for this library. 
It is designed as adapter from low level OpenGL API to more simple high level API. This API contains some high level 
concepts like scene, material, mesh, shader.. etc. whereas OpenGL API provides low level concepts like Framebuffer, 
vertex buffer, texture buffer and others.

Lite3d library support only OpenGL graphics API as back end for GPU operations. No sound, no physics, graphics only!
One of the reason of using OpenGL API was a potential support of many OS. Windows/Linux supports now, but 
library may be simply ported on others platforms. 

API
---
Library provides two API levels:
  1. Low level C API (3dlite), common graphic operations.
  2. High level C++ API (3dlitepp), user friendly C++ components and scripting API.
(See samples for more details)

Dependencies
------------
* [DevIL](http://example.net/)
* [Assimp](https://www.libsdl.org/index.php)
* [SDL2](http://assimp.sourceforge.net/)
* [Bullet](https://github.com/bulletphysics/bullet3)
* Also Lite3d use [Angelscript](http://www.angelcode.com/angelscript/) for scripting and
some other libraries (see [LICENCE](LICENCE.3rdparty))

Compiling
---------
Use [cmake](http://www.cmake.org/) 3.12.0 or higher.
```
$ mkdir build
$ cmake /path/to/3dlite/sources
$ make all
```
You can use cmake to generate Visual Studio projects on windows.

Samples
---------
After successfully compiling you can find some samples in bin directory. To provide samples work correctly you must download 
resources packs [vault.pkg](https://drive.google.com/file/d/1JhsirjKwMq51IBg7GknUeZLGvrVKp1Sz/view?usp=sharing) and [sponza.pkg](https://drive.google.com/file/d/1GGtPep7soS1wPJsf4Y2FTiF23E-JXFQ9/view?usp=sharing) and put its to media/packs/

You can download resources for Vault_111 sample from [here](https://drive.google.com/file/d/1oxnfRbR7WcTECcI7BDKBBqPwPlCcbx9J/view?usp=sharing), this 7z archive contains textures and models folders, you must copy this folders to media/vault_111 and anjoy this awesome sample.

Screenshots
-----------
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

***
Check branch master to fetch stable changes =)
> Copyright © 2014-2023 Nikita Korolev (Sirius)
