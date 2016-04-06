Lite 3d rendering engine
========================

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
* Also Lite3d use [Angelscript](http://www.angelcode.com/angelscript/) for scripting and
some other libraries (see [LICENCE](LICENCE.3rdparty))

Compiling
---------
Use [cmake](http://www.cmake.org/) 2.8.10 or higher.
```
$ mkdir build
$ cmake /path/to/3dlite/sources
$ make all
```
You can use cmake to generate Visual Studio projects on windows.

Screenshots
---------
![](/media/screenshots/minigun.png "Minigun")
![](/media/screenshots/robots.png "Robots")
![](/media/screenshots/sponza.png "Sponza")
![](/media/screenshots/vault.png "Vault")

***
> Copyright © 2014-2016 Nikita Korolev (Sirius)
