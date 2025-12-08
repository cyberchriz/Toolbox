<div align="center">
  <br>
    <img src="./docs/media/ngrid-logo.png" alt="NGrid Logo" width="800" height="350"/>
  <br>
  <h1 align="center">
    NGrid - High-Performance GPU Computing
  </h1>
  <br>
  <p align="center">
    A C++ library for high-performance GPU-accelerated computing, built on Vulkan.
  </p>
  <br>
  <a href="./LICENSE">
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

**Using a different build system (not encouraged):**
* The library includes a fallback header, `spirv_bin_precompiled.h`, which contains precompiled binaries (may be out of date!). This allows the code to work out-of-the-box; otherwise `glslangValidator` is required for shader compilation.
* However, please note that any changes to the GLSL shader code will not automatically be reflected in the precompiled binaries.

---

## 📚 Core Libraries
These are the primary components for high-performance GPU computing.

| Library | Description | Status |
| :--- | :--- | :--- |
| [`ngrid.h`](docs/ngrid.md) | **N-dimensional data structures for GPU compute.** The core library for general-purpose GPU computing on tensors. | ✅ Tested |
| [`cgrid.h`](docs/cgrid.md) | **An extension of `NGrid` with support for complex numbers.** Adds complex number functionality to the core NGrid class, leveraging the same GPU backend. | ✅ Tested |
| [`vkcontext.h`](docs/vkcontext.md) | **High-level wrapper for Vulkan objects.** A simplified API for managing the Vulkan context, including devices, pipelines, and synchronization. | 🚧 Compute tested; Graphics functionality partially tested. |

---

## 🔧 Helper Utilities
These supporting libraries simplify development and provide additional functionality.

| Utility | Description |
| :--- | :--- |
| [`log.h`](#) | **A lightweight logging system** for debugging and information. |
| [`rnd.h`](#) | **Random number generators** for various distributions. |
| [`cdf.h`](#) | **Cumulative distribution functions** for statistical analysis. |
| [`pdf.h`](#) | **Probability density functions** for statistical analysis. |
| [`angular.h`](#) | **Angular measure conversion** for different units (radians, degrees, etc.). |
| [`vkdebug.h`](#) | **Implements capture for RenderDoc debugging** to analyze GPU workloads. |

___

<div align="center">
  <br>
    <img src="./docs/media/Khronos_Damaged_Helmet.png" alt="NGrid Mandelbrot Set" width="1000" height="850"/>
  <br>
  <p align="center">
    example: TEST RENDER using the vkcontext.h library (<em>Source Model Credit: Khronos Group</em>)
</div>

---

> This repository is a work in progress.