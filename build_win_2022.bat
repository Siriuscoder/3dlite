if not exist build mkdir build
cmake --preset=Windows64_vcpkg -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release