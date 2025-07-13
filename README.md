IMPORTANT:

1. For `NGrid` and `VkContext`: THIS LIBRARY REQUIRES VULKAN TO BE INSTALLED.
2. For `NGrid`: If (!) CMake is used as the build system with the provided CMake file (CMakeLists.txt, RECOMMENDED!), this relies on GlslLangValidator
to be installed for the compilation of any GLSL shaders. CMake will then automatically take the compiled binaries and write them
as string literals into a C++ readable header file (`spirv_bin.h`), typically to be found in `../out/build/[VERSION]/generated/` (on Windows).
If a different build system is used (or CMake without the provided CMakeLists.txt), `spirv_bin.h` will NOT be available.
This isn't really a problem and the code will still work, because it will use the included file `spirv_bin_precompiled.h`
(which has precompiled binaries) as a fallback (which also significantly reduces compilation time). However, if any changes are made to the GLSL code,
these changes can't be reflected in the precompiled binaries and WILL require recompiling with the provided method.
If CMake fails: Please also make sure to correctly configure CMake (via CMakeSettings.json or e.g. via CMakeGUI) for the environment variables
on the used Operating System.
___
#   `Core`
### [______`NGrid`: n-dimensional data structures for GPU compute](docs/ngrid.md)
### [______`VkContext`: high-level wrapper for Vulkan objects](docs/vkcontext.md)
_Note: Only tested for GPU compute; Graphics functionality is implemented for the most part, but not yet tested (TODO!)_
___ 
##  `Helpers / Utilities`
### [______`Timer`: time logger for performance optimization]()
### [______`Log`: logging system for debugging and information]()
### [______`Random`: random numbers from different distributions]()
### [______`CDF`: cumulative distribution functions]()
### [______`PDF`: probability density functions]()
### [______`Angular`: angular measure conversion]()
### [______`RDocEnable`: implements capture for RenderDoc debugging]()
___
THIS REPOSITORY IS 'WORK IN PROGRESS'.

Author: cyberchriz (Christian Suer).

Languages: C++, GLSL.

FREE TO USE FOR NON-COMMERCIAL PURPOSES ([LICENCE](LICENSE)).