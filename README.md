<div align="center">
  <br>
  <img src="https://raw.githubusercontent.com/cyberchriz/NGrid/main/docs/media/ngrid-logo.png" alt="NGrid Logo" width="300" height="300"/>
  <br>
  <h1 align="center">
    NGrid - High-Performance GPU Computing
  </h1>
  <br>
  <p align="center">
    A C++ library for high-performance GPU-accelerated computing, built on Vulkan.
  </p>
  <br>
  <a href="https://github.com/cyberchriz/NGrid/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/License-Non--Commercial%20Use-blue.svg?style=for-the-badge" alt="License: Non-Commercial Use" />
  </a>
  <a href="https://github.com/cyberchriz/NGrid/pulse">
    <img src="https://img.shields.io/badge/Status-Work%20in%20Progress-orange?style=for-the-badge" alt="Status: Work in Progress" />
  </a>
  <a href="https://en.cppreference.com/w/cpp/compiler_support">
    <img src="https://img.shields.io/badge/Language-C++-red.svg?style=for-the-badge&logo=c%2B%2B" alt="Language: C++" />
  </a>
</div>

---

## 🚀 Quick Start
This library requires **Vulkan** to be installed on your system.
<br><br>
The recommended way to build this project is by using the provided `CMakeLists.txt`.
<br><br>
**Using CMake:**
* CMake will automatically compile the GLSL shaders and embed them as C++ string literals into a header file (`spirv_bin.h`), which is typically located in your build directory (e.g., `../out/build/[VERSION]/generated/`). This requires `glslangValidator` to be installed and available in your system's PATH.
* If you encounter issues with CMake, please ensure your environment variables are correctly configured for your operating system.

**Using a different build system:**
* The library includes a fallback header, `spirv_bin_precompiled.h`, which contains precompiled binaries. This allows the code to work out-of-the-box without `glslangValidator` and significantly reduces compilation time.
* However, please note that any changes to the GLSL shader code will not be reflected in the precompiled binaries. You will need to use the CMake method to recompile and include your changes.

---

## 📚 Core Libraries
These are the primary components for high-performance GPU computing.

| Library | Description | Status |
| :--- | :--- | :--- |
| [`NGrid`](docs/ngrid.md) | **N-dimensional data structures for GPU compute.** The core library for general-purpose GPU computing on tensors. | ✅ Tested |
| [`CGrid`](docs/cgrid.md) | **An extension of `NGrid` with support for complex numbers.** Adds complex number functionality to the core NGrid class, leveraging the same GPU backend. | ✅ Tested |
| [`VkContext`](docs/vkcontext.md) | **High-level wrapper for Vulkan objects.** A simplified API for managing the Vulkan context, including devices, pipelines, and synchronization. | 🚧 Compute tested; Graphics functionality is implemented but not yet fully tested. |

---

## 🔧 Helper Utilities
These supporting libraries simplify development and provide additional functionality.

| Utility | Description |
| :--- | :--- |
| [`Timer`](#) | **Time logger for performance optimization.** Measure code execution time to identify bottlenecks. |
| [`Log`](#) | **A lightweight logging system** for debugging and information. |
| [`Random`](#) | **Random number generators** for various distributions. |
| [`CDF`](#) | **Cumulative distribution functions** for statistical analysis. |
| [`PDF`](#) | **Probability density functions** for statistical analysis. |
| [`Angular`](#) | **Angular measure conversion** for different units (radians, degrees, etc.). |
| [`RDocEnable`](#) | **Implements capture for RenderDoc debugging** to analyze GPU workloads. |

---

> This repository is a work in progress.

____
OLD:

IMPORTANT:

1. THIS LIBRARY REQUIRES VULKAN TO BE INSTALLED.<br><br>
2. For [`NGrid`](docs/ngrid.md) / [`CGrid`](docs/cgrid.md): If (!) CMake is used as the build system with the provided CMake file (CMakeLists.txt, RECOMMENDED!), this relies on GlslLangValidator
to be installed for the compilation of any GLSL shaders. CMake will then automatically take the compiled binaries and write them
as string literals into a C++ readable header file (`spirv_bin.h`), typically to be found in `../out/build/[VERSION]/generated/` (on Windows).<br><br>
If a different build system is used (or CMake without the provided CMakeLists.txt), `spirv_bin.h` will NOT be available.
This isn't really a problem and the code will still work, because it will use the included file [`spirv_bin_precompiled.h`](include/spirv_bin_precompiled.h)
(which has precompiled binaries) as a fallback (which also significantly reduces compilation time). However, if any changes are made to the GLSL code,
these changes can't be reflected in the precompiled binaries and WILL require recompiling with the provided method. <br><br>
If CMake fails: Please also <U>make sure to correctly configure CMake</U> (e.g. via CMakeSettings.json or CMakeGUI) for the environment variables
on the Operating System in use.
___
#   `Core`
### [______`NGrid`: n-dimensional data structures for GPU compute](docs/ngrid.md)
### [______`CGrid`: extension of the `NGrid` class with support for complex numbers](docs/cgrid.md)
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