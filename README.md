# nn2000_to_etrs89 elevation converter

**nn2000_to_etrs89** reads latitudes, longitudes and elevations from a XYZ file and converts the elevations from the Norwegian height reference system NN2000 to the ETRS-89 ellipsoid. The ETRS-89 is equivalent to WGS-84, except for continental drift of about 2 cm per year, which means that horizontal coordinates are off by approximately 70 cm in 2025. The precision of the geoid data is so low (points are 2 km apart) that this error will not make any noticeable difference when converting from NN2000 to WGS-84. 

## Build

### Requirements:

- A version of Clang, GCC or MSVC that supports C++20
- CMake 3.25 or later.
- LibTIFF
- libpng
- libjpeg-turbo or libjpeg

### Basic build instructions

```sh
mkdir cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```
 
 ### Using vcpkg and presets
 
 ```
 cmake --preset vcpkg
 cmake --build --preset vcpkg
 ```
 
 
 ## Run
 
Create a file containing XYZ data. An example of such a file, with two points, can be found in the examples folder.

```
./nn2000_to_etrs89 ../examples/points.txt
```
