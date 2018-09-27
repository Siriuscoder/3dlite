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

Samples
---------
After successfully compiling you can find some samples in bin directory. To provide samples work correctly you must download 
resources packs [vault.pkg](https://cloud.mail.ru/public/8eAZ/BAjJhjqYE) and [sponza.pkg](https://cloud.mail.ru/public/EijZ/tHkWtW9Qs) and put its to media/packs/

Screenshots
-----------
![](/media/screenshots/robots.png "Robots")
![](/media/screenshots/robots1.png "Robots")
![](/media/screenshots/sponza.png "Sponza")
![](/media/screenshots/sponza1.png "Sponza")
![](/media/screenshots/sponza2.png "Sponza")
![](/media/screenshots/sponza3.png "Sponza")
![](/media/screenshots/vault.png "Vault")
![](/media/screenshots/vault1.png "Vault")
![](/media/screenshots/vault2.png "Vault")
![](/media/screenshots/vault3.png "Vault")
![](/media/screenshots/vault4.png "Vault")

***
Check branch work to fetch recent changes =)
> Copyright © 2014-2016 Nikita Korolev (Sirius)
