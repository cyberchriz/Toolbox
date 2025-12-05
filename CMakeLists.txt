#==============================================================================
# Generic CMake Configuration File
# Author: Christian Suer (cyberchriz)
#==============================================================================

# --- Basic Setup ---
cmake_minimum_required(VERSION 3.5)

# --- Project Definition ---
set(PROJECT_NAME "MyProject" CACHE STRING "Name of the project")
set(PROJECT_VERSION "1.0.0" CACHE STRING "Project version (major.minor.patch)")
set(PROJECT_DESCRIPTION "generic C++/C project" CACHE STRING "Project description")
set(PROJECT_LANGUAGES "CXX;C" CACHE STRING "Enabled languages (e.g., CXX, C, CUDA)")
set(PROJECT_CPP_STANDARD "20" CACHE STRING "Required C++ standard (e.g., 11, 14, 17, 20, 23)")
    set_property(CACHE PROJECT_CPP_STANDARD PROPERTY STRINGS 11 14 17 20 23)
set(PROJECT_C_STANDARD "11" CACHE STRING "Required C standard (99, 11, 17, 23)")
    set_property(CACHE PROJECT_C_STANDARD PROPERTY STRINGS 99 11 17 23)
option(PROJECT_ENABLE_COMPILER_EXTENSIONS "Allow compiler-specific extensions (vs standard features)" OFF)
set(PROJECT_GLSL_TARGET_ENV "vulkan1.2" CACHE STRING "Target environment for SPIR-V compilation (e.g., vulkan1.2, opengl)")

# Set C++ Standard globally
set(CMAKE_CXX_STANDARD ${PROJECT_CPP_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ${PROJECT_ENABLE_COMPILER_EXTENSIONS})

# Set C Standard if C is enabled
if("C" IN_LIST PROJECT_LANGUAGES)
    set(CMAKE_C_STANDARD ${PROJECT_C_STANDARD})
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS ${PROJECT_ENABLE_COMPILER_EXTENSIONS})
endif()

# Define the project
project(${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "${PROJECT_DESCRIPTION}"
    LANGUAGES ${PROJECT_LANGUAGES}
)
message(STATUS "Configuring project: ${PROJECT_NAME} v${PROJECT_VERSION}")

# --- First Run Message ---
if(NOT IS_DIRECTORY "${CMAKE_BINARY_DIR}" OR NOT EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
    message(STATUS "\n=================================================================================")
    message(STATUS " This looks like the first run of CMake for this project.")
    message(STATUS " Please make sure all environment variables are configured correctly.")
    message(STATUS " Review and configure options using:")
    message(STATUS "   - cmake-gui or ccmake")
    message(STATUS "   - Command line: cmake .. -D<OPTION>=<VALUE>")
    message(STATUS "   - Visual Studio: CMake Settings editor (CMakeSettings.json)")
    message(STATUS " Ensure all required system dependencies (VulkanSDK, build tools, etc.) are installed.")
    message(STATUS " For find_package, you might need to set CMAKE_PREFIX_PATH or specific *_ROOT variables.")
    message(STATUS " FetchContent options allow downloading some dependencies automatically.")
    message(STATUS "===================================================================================\n")
endif()

# --- Build Configuration ---
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as default.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# --- Standard Directory Structure ---
include(GNUInstallDirs) # Defines CMAKE_INSTALL_BINDIR, CMAKE_INSTALL_LIBDIR, etc.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/third_party")
set(PROJECT_ENTRYPOINT "${PROJECT_SOURCE_DIR}/main.cpp")
set(PROJECT_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(PROJECT_GLSL_DIR "${PROJECT_RESOURCES_DIR}/shaders/glsl")
set(PROJECT_SPIRV_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders/spirv")

# --- Executable Target ---
message(STATUS "Defining executable target: ${PROJECT_NAME}")
add_executable(${PROJECT_NAME} "")

# --- Source Files ---
message(STATUS "DEBUG: Using PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
if(EXISTS "${PROJECT_SOURCE_DIR}")
    file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/*.cpp" "${PROJECT_SOURCE_DIR}/*.cxx" "${PROJECT_SOURCE_DIR}/*.cc" "${PROJECT_SOURCE_DIR}/*.c"
    )
    message(STATUS "DEBUG: Found source files: ${PROJECT_SOURCES}")
    if(EXISTS "${PROJECT_ENTRYPOINT}")
        if(NOT PROJECT_ENTRYPOINT IN_LIST PROJECT_SOURCES)
            list(APPEND PROJECT_SOURCES ${PROJECT_ENTRYPOINT}) # Ensure entry point is included
        endif()
        message(STATUS "Using entry point: ${PROJECT_ENTRYPOINT}")
    else()
        message(FATAL_ERROR "Entry point file not found: ${PROJECT_ENTRYPOINT}")
    endif()

    if(PROJECT_SOURCES)
        target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
        message(STATUS "Added source files from: ${PROJECT_SOURCE_DIR}")
    else()
        message(WARNING "No source files found in ${PROJECT_SOURCE_DIR}")
    endif()
else()
    message(FATAL_ERROR "Project source directory not found: ${PROJECT_SOURCE_DIR}")
endif()

# --- Include Directories ---
if(EXISTS "${PROJECT_INCLUDE_DIR}")
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    message(STATUS "Added public include directory: ${PROJECT_INCLUDE_DIR}")
endif()

if(EXISTS "${PROJECT_THIRD_PARTY_DIR}")
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_THIRD_PARTY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    message(STATUS "Added public third_party directory: ${PROJECT_INCLUDE_DIR}")
endif()

set(GENERATED_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated") # add Include Directory for Generated Headers
target_include_directories(${PROJECT_NAME} PRIVATE # Add this directory to the include path for the main target
    ${GENERATED_HEADER_DIR}
)
message(STATUS "Added generated include directory: ${GENERATED_HEADER_DIR}")

#==============================================================================
# Section: Compiler Options
#==============================================================================
message(STATUS "Applying compiler options...")

# Add options for specific flags if desired
option(ENABLE_EXTRA_WARNINGS "Enable extra compiler warnings (-Wextra, /Wall)" ON)
option(ENABLE_PEDANTIC_WARNINGS "Enable pedantic compiler warnings (-Wpedantic, /permissive-)" ON)

# --- Compiler-Specific Flags ---
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    # Base warnings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
    if(ENABLE_EXTRA_WARNINGS)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wextra)
    endif()
    if(ENABLE_PEDANTIC_WARNINGS)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wpedantic)
    endif()
    # Other common warnings (add more as needed)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
        -Woverloaded-virtual -Wconversion -Wsign-conversion -Wdouble-promotion -Wformat=2
        -fno-strict-aliasing
        $<$<BOOL:${ENABLE_PEDANTIC_WARNINGS}>:-Wno-unused-parameter> # Allow unused if pedantic, else -Wunused handles it
    )
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wimplicit-fallthrough -Wnull-dereference)
    endif()
    # Build type flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
        $<$<CONFIG:MinSizeRel>:-Os -g -DNDEBUG>
    )

elseif (MSVC)
    # Base warnings
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    if(ENABLE_EXTRA_WARNINGS)
        # /Wall is often too noisy, /W4 is usually sufficient
        # target_compile_options(${PROJECT_NAME} PRIVATE /Wall)
    endif()
    if(ENABLE_PEDANTIC_WARNINGS)
        target_compile_options(${PROJECT_NAME} PRIVATE /permissive-)
    endif()
    # Other common flags/warnings
    target_compile_options(${PROJECT_NAME} PRIVATE
        /WX- /wd4251 /wd4275 /wd4100 /EHsc /MP /utf-8 /Zc:__cplusplus
        /bigobj # To increase the object file section limit for large template/header-only files.
    )
    # Build type flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od /RTC1>
        $<$<CONFIG:Release>:/O2 /GL /Gy /DNDEBUG>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /Gy /DNDEBUG>
        $<$<CONFIG:MinSizeRel>:/O1 /Zi /Gy /DNDEBUG>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
        $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
        $<$<CONFIG:RelWithDebInfo>:/DEBUG /OPT:REF /OPT:ICF>
        $<$<CONFIG:MinSizeRel>:/DEBUG /OPT:REF /OPT:ICF>
    )
else()
    message(WARNING "Compiler ${CMAKE_CXX_COMPILER_ID} not explicitly supported for flags. Using CMake defaults.")
endif()

message(STATUS "Compiler options applied.")


#==============================================================================
# Section: Code Quality Tools
#==============================================================================
message(STATUS "Configuring code quality tools...")

# --- Static Analysis (Clang-Tidy) ---
option(USE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Enabling clang-tidy integration")
        # Customize checks as needed, e.g., using a .clang-tidy file
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE} --quiet) # Add checks via file or command line
        if("C" IN_LIST PROJECT_LANGUAGES)
            set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY_CMD})
        endif()
    else()
        message(WARNING "clang-tidy executable not found. Disabling clang-tidy integration.")
    endif()
endif()

# --- Sanitizers (Example for GCC/Clang) ---
option(USE_ADDRESS_SANITIZER "Enable AddressSanitizer (ASan) in Debug builds" OFF)
option(USE_UNDEFINED_SANITIZER "Enable UndefinedBehaviorSanitizer (UBSan) in Debug builds" OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    if(USE_ADDRESS_SANITIZER)
        message(STATUS "Enabling AddressSanitizer for Debug builds")
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
        target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
    endif()
    if(USE_UNDEFINED_SANITIZER)
         message(STATUS "Enabling UndefinedBehaviorSanitizer for Debug builds")
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=undefined>)
        target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=undefined>)
    endif()
endif()


# --- Code Formatting Target (Clang-Format) ---
option(ADD_FORMAT_TARGET "Add a 'format' target using clang-format" OFF)
if(ADD_FORMAT_TARGET)
    find_program(CLANG_FORMAT_EXE clang-format)
    if(CLANG_FORMAT_EXE)
        # Ensure source/include dirs are set and files exist
        set(FORMAT_FILES) # Initialize empty list
        if(IS_DIRECTORY "${PROJECT_SOURCE_DIR}")
            file(GLOB_RECURSE FORMAT_SOURCES CONFIGURE_DEPENDS
                 "${PROJECT_SOURCE_DIR}/*.cpp" "${PROJECT_SOURCE_DIR}/*.cxx" "${PROJECT_SOURCE_DIR}/*.cc" "${PROJECT_SOURCE_DIR}/*.c"
                 "${PROJECT_SOURCE_DIR}/*.hpp" "${PROJECT_SOURCE_DIR}/*.h"
            )
            list(APPEND FORMAT_FILES ${FORMAT_SOURCES})
        endif()
         if(IS_DIRECTORY "${PROJECT_INCLUDE_DIR}")
             file(GLOB_RECURSE FORMAT_HEADERS CONFIGURE_DEPENDS
                 "${PROJECT_INCLUDE_DIR}/*.hpp" "${PROJECT_INCLUDE_DIR}/*.h"
             )
             list(APPEND FORMAT_FILES ${FORMAT_HEADERS})
        endif()
        list(REMOVE_DUPLICATES FORMAT_FILES)

        if(FORMAT_FILES)
            add_custom_target(format
                COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${FORMAT_FILES}
                COMMENT "Formatting C++/C sources and headers with clang-format"
                VERBATIM
            )
            message(STATUS "Added 'format' target (run manually, e.g., 'cmake --build . --target format')")
        else()
             message(STATUS "No source/header files found for 'format' target.")
        endif()
    else()
        message(WARNING "clang-format executable not found. Cannot add 'format' target.")
    endif()
endif()

message(STATUS "Code quality tools configured.")


#==============================================================================
# Section: Dependency Management
#==============================================================================
message(STATUS "Configuring external dependencies...")

# Setup FetchContent base directory
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps" CACHE PATH "Base directory for FetchContent downloads")

# --- Vulkan (Requires SDK installation - find_package only) ---
option(USE_VULKAN "Find and link Vulkan" ON)
if(USE_VULKAN)
    find_package(Vulkan REQUIRED QUIET)
    if(Vulkan_FOUND)
        message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})
    else()
        message(WARNING "Vulkan installation not found via find_package !!! Make sure the Vulkan SDK is installed.")
    endif()
endif()

# --- GLFW3 ---
option(USE_GLFW "Find or fetch and link GLFW3" ON)

if(USE_GLFW)
    # Check if GLFW is already installed
    message(STATUS "Attempting to find pre-installed GLFW3 via find_package")
    find_package(glfw3 3.4 QUIET) # Note: Package name is glfw3 here, not glfw
    if(glfw3_FOUND)
        message(STATUS "Found GLFW3: ${glfw3_LIBRARIES}")
        # Use imported target if available (modern FindGLFW3.cmake defines glfw::glfw)
        if(TARGET glfw::glfw)
            target_link_libraries(${PROJECT_NAME} PUBLIC glfw::glfw)
            # target_include_directories usually not needed for imported target
        else()
            # Fallback for older find modules
            target_include_directories(${PROJECT_NAME} PUBLIC ${glfw3_INCLUDE_DIRS})
            target_link_libraries(${PROJECT_NAME} PUBLIC ${glfw3_LIBRARIES})
        endif()
    else()
        message(STATUS "glfw3 not found, falling back to FetchContent...")
        FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4) # Check for latest tag
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Options for fetched dependency
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glfw)
        target_include_directories(${PROJECT_NAME} PUBLIC ${glfw_SOURCE_DIR}/include)
        target_link_libraries(${PROJECT_NAME} PUBLIC glfw) # Link target created by FetchContent
        message(STATUS "Using GLFW3 via FetchContent")
        message(STATUS "GLFW Source Dir: ${glfw_SOURCE_DIR}")
    endif()
endif()

# --- GLM (Header Only) ---
option(USE_GLM "Find or fetch and use GLM" ON)

if(USE_GLM)
    # Check if GLM is installed
    find_path(GLM_INCLUDE_DIR glm/glm.hpp)
    if(GLM_INCLUDE_DIR)
        message(STATUS "Found installed GLM at: ${GLM_INCLUDE_DIR}")
        add_library(glm INTERFACE)
        target_include_directories(glm INTERFACE ${GLM_INCLUDE_DIR})
    else()
        message(STATUS "GLM not found, falling back to FetchContent...")
        FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1) # Check for latest tag
        FetchContent_MakeAvailable(glm) # Handles includes for header-only
        message(STATUS "Now using GLM via FetchContent")
        # verify again that GLM is now available (=after FetchContent)
    endif()
    # Ensure GLM is linked
    target_link_libraries(${PROJECT_NAME} PRIVATE glm)
endif()

# --- RenderDoc API (for programmatic capture) ---
# This is a header-only API for dynamic loading.
# No linking against a RenderDoc library is performed.
# ... (rest of your CMakeLists.txt before this block) ...
option(USE_RENDERDOC_API "Enable programmatic RenderDoc capture API calls" ON)
mark_as_advanced(USE_RENDERDOC_API)

if(USE_RENDERDOC_API)

    # This list will hold all potential RenderDoc API include paths
    set(RENDERDOC_SEARCH_PATHS)

    # Windows specific paths
    if(WIN32)
        # CMAKE_PROGRAM_FILES is usually "C:/Program Files"
        if(CMAKE_PROGRAM_FILES)
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES}/RenderDoc/renderdoc/api")
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES}/RenderDoc/renderdoc")
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES}/RenderDoc")
        endif()
        # CMAKE_PROGRAM_FILES_X86 is "C:/Program Files (x86)" on 64-bit Windows
        if(CMAKE_PROGRAM_FILES_X86)
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES_X86}/RenderDoc/renderdoc/api")
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES_X86}/RenderDoc/renderdoc")
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES_X86}/RenderDoc")
        endif()
        # You could also add the %LOCALAPPDATA% path if RenderDoc has a user-specific install option
        # For example, if RenderDoc installs to C:\Users\YourUser\AppData\Local\RenderDoc\renderdoc\api
        # list(APPEND RENDERDOC_SEARCH_PATHS "$ENV{LOCALAPPDATA}/RenderDoc/renderdoc/api")

    else()
    # Linux specific paths (and potentially macOS, though RenderDoc installation differs there)
    # These are typically hardcoded because there aren't standard CMake variables like on Windows for /usr/local
    list(APPEND RENDERDOC_SEARCH_PATHS
        "/usr/local/include/renderdoc/api"
        "/opt/renderdoc/include/renderdoc/api"
        "/usr/include/renderdoc/api"
    )
    endif()

    find_path(RENDERDOC_API_INCLUDE_DIR
        NAMES renderdoc_app.h
        PATHS ${RENDERDOC_SEARCH_PATHS} # Pass the list of generated paths
        DOC "Path to RenderDoc's 'api' directory containing renderdoc_app.h"
        NO_DEFAULT_PATH # Only search the paths we've explicitly listed
    )

    if(RENDERDOC_API_INCLUDE_DIR)
        message(STATUS "Found RenderDoc API header at: ${RENDERDOC_API_INCLUDE_DIR}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${RENDERDOC_API_INCLUDE_DIR})
        target_compile_definitions(${PROJECT_NAME} PRIVATE USE_RENDERDOC_API_DEFINE=1)
    else()
        message(WARNING "Could not find RenderDoc API header (renderdoc_app.h). Programmatic capture will not be available.")
        message(STATUS "Please set RENDERDOC_API_INCLUDE_DIR manually: it should point to the 'api' folder within your RenderDoc installation.")
        message(STATUS "     Example: C:/Program Files/RenderDoc/renderdoc/api")
        message(STATUS "     Example: C:/Program Files (x86)/RenderDoc/renderdoc/api")
        message(STATUS "     Example: /usr/local/include/renderdoc/api")
    endif()

    if(WIN32)
        # set the directory for renderdoc.dll (if not found in PATH)
        # (use same path as the include directory as default; alternatively
        # the dll may e.g. be located in C:\Program Files\RenderDoc\bin\x64 or
        # C:\Program Files\RenderDoc\bin\x86)
        set(RENDERDOC_DLL_DIR "${RENDERDOC_API_INCLUDE_DIR}" CACHE PATH "location of renderdoc.dll")
    else()
        # On Linux, RenderDoc is usually installed in /usr/lib or /usr/local/lib
        set(RENDERDOC_DLL_DIR "/usr/lib" CACHE PATH "location of renderdoc.so")
    endif()

    # The `PRIVATE` keyword means it only applies to sources in this target;
    # STRINGIFY_IMPL and STRINGIFY are needed to turn the path string into a C++ string literal
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        RENDERDOC_DLL_RUNTIME_PATH="${RENDERDOC_DLL_DIR}"
    )
endif()

# --- Boost ---
option(USE_BOOST "Find and link Boost" OFF)
# Fetching Boost is complex, sticking to find_package
if(USE_BOOST)
    message(STATUS "Attempting to find pre-installed Boost via find_package")
    set(Boost_USE_STATIC_LIBS ON)
    find_package(Boost 1.88.0 COMPONENTS system filesystem)
    if(Boost_FOUND)
        message(STATUS "Found Boost ${Boost_VERSION_STRING}: ${Boost_LIBRARIES}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
    else()
        message(WARNING "CMake option USE_BOOST is set to ON, but Boost couldn't be found !'")
    endif()
endif()

# --- LLVM Tools Setup ---
option(ENABLE_LLVM_TOOLS "Enable automatic download of llvm-objcopy if not found" ON)
if(ENABLE_LLVM_TOOLS)
    set(LLVM_VERSION "21.1.5")
    set(LLVM_TAG "llvmorg-${LLVM_VERSION}")
    
    # 1. Determine Target OS and File Suffix and HASH
    if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
        set(LLVM_OS_SUFFIX "x86_64-pc-windows-msvc")
        set(LLVM_HASH_VALUE "eba824f1379fdb1a385f6dff8d19275a57348f621c752ce93b6d11256741e349") # 21.1.5 Windows MSVC x64
        set(LLVM_EXT_TYPE "tar.xz")
    elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        set(LLVM_OS_SUFFIX "x86_64-linux-gnu-ubuntu-22.04")
        set(LLVM_HASH_VALUE "9622d1f05479075e7a93540ce8c16d56d43e5b30646c0d83272993ed61a46979") # 21.1.5 Linux x64
        set(LLVM_EXT_TYPE "tar.xz")
    elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
        set(LLVM_OS_SUFFIX "x86_64-apple-darwin")
        set(LLVM_HASH_VALUE "2a149c5855217822501a31627c5cc1cc7994e43f1ed16f88d223a54b64d1421f") # 21.1.5 macOS x64
        set(LLVM_EXT_TYPE "tar.xz")
    else()
        message(WARNING "Unsupported OS for LLVM binary download: ${CMAKE_SYSTEM_NAME}. Skipping automatic tool fetch.")
        set(ENABLE_LLVM_TOOLS OFF)
    endif()

    if(ENABLE_LLVM_TOOLS)
        # 2. Dynamic Path Calculation
        set(LLVM_FILENAME "clang+llvm-${LLVM_VERSION}-${LLVM_OS_SUFFIX}.${LLVM_EXT_TYPE}")
        set(LLVM_DOWNLOAD_URL 
            "https://github.com/llvm/llvm-project/releases/download/${LLVM_TAG}/${LLVM_FILENAME}"
        )
        set(LLVM_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/_deps/llvm_download")
        set(LLVM_ARCHIVE_PATH "${LLVM_DOWNLOAD_DIR}/${LLVM_FILENAME}")
        set(LLVM_EXTRACT_ROOT "${LLVM_DOWNLOAD_DIR}/clang+llvm-${LLVM_VERSION}-${LLVM_OS_SUFFIX}") 
        set(LLVM_BIN_DIR "${LLVM_EXTRACT_ROOT}/bin")
        file(MAKE_DIRECTORY ${LLVM_DOWNLOAD_DIR})
    endif()
endif()
# --- End LLVM Setup ---

# === END OF DEPENDENCY MANAGEMENT ===
message(STATUS "Configuration of external dependencies is now complete.")

#==============================================================================
# Section: GLSL Shader Compilation
#==============================================================================
option(COMPILE_GLSL_SHADERS "Enable automatic GLSL shader (re)compilation" ON)

if(COMPILE_GLSL_SHADERS AND EXISTS "${PROJECT_GLSL_DIR}")
    # --- Check for GLSL Compiler ---
    find_program(GLSLANG_VALIDATOR_EXECUTABLE NAMES glslangValidator glslc)
    
    if(NOT GLSLANG_VALIDATOR_EXECUTABLE)
        message(WARNING "Option COMPILE_GLSL_SHADERS is ON, but GLSL compiler (glslangValidator) can't be found. Compilation will be skipped.")
        set(COMPILE_GLSL_SHADERS OFF)
    else()
        message(STATUS "Using GLSL compiler: ${GLSLANG_VALIDATOR_EXECUTABLE}")
        file(MAKE_DIRECTORY "${PROJECT_SPIRV_DIR}")
        
        file(GLOB_RECURSE GLSL_SHADER_SOURCES CONFIGURE_DEPENDS
            "${PROJECT_GLSL_DIR}/*.vert" "${PROJECT_GLSL_DIR}/*.frag" "${PROJECT_GLSL_DIR}/*.comp"
            "${PROJECT_GLSL_DIR}/*.tesc" "${PROJECT_GLSL_DIR}/*.tese" "${PROJECT_GLSL_DIR}/*.geom"
            "${PROJECT_GLSL_DIR}/*.glsl")

        if(GLSL_SHADER_SOURCES)
            message(STATUS "Found GLSL shaders to compile...")
            set(SPIRV_CPP_SOURCES "") # Holds the list of generated .cpp files
            set(SPV_COMPILE_TARGETS "") # Holds the list of custom targets
            set(SPIRV_DECL_LIST "") # Holds variable names for header generation

            # --- Vulkan SDK Tool Path Derivation (Kept for completeness) ---
            if(Vulkan_FOUND AND Vulkan_LIBRARIES)
                get_filename_component(VULKAN_LIB_DIR ${Vulkan_LIBRARIES} DIRECTORY)
                get_filename_component(VULKAN_SDK_ROOT ${VULKAN_LIB_DIR} DIRECTORY)
                set(VULKAN_TOOLS_BIN "${VULKAN_SDK_ROOT}/Tools/bin")
                list(APPEND CMAKE_PROGRAM_PATH ${VULKAN_TOOLS_BIN})
                message(STATUS "Added Vulkan SDK tools path to search: ${VULKAN_TOOLS_BIN}")
            endif()
            
            # =======================================
            # --- Per-Shader File Generation Loop ---
            # =======================================
            set(SPIRV_CUSTOM_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/spirv_sources")
            file(MAKE_DIRECTORY ${SPIRV_CUSTOM_SRC_DIR})

            foreach(GLSL_INPUT ${GLSL_SHADER_SOURCES})
                get_filename_component(SHADER_NAME ${GLSL_INPUT} NAME)
                get_filename_component(BASE_NAME_WE ${SHADER_NAME} NAME_WE)
                
                # Calculate paths and names
                set(SPIRV_OUTPUT "${PROJECT_SPIRV_DIR}/${SHADER_NAME}.spv")
                
                # 1. Sanitize Names for C++
                string(REGEX REPLACE "\\..*" "" SANITIZED_BASE_NAME ${BASE_NAME_WE})
                string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" SANITIZED_BASE_NAME ${SANITIZED_BASE_NAME})
                string(TOUPPER "${SANITIZED_BASE_NAME}" VAR_NAME)
                
                # 2. Define Generated C++ paths
                set(SPIRV_GENERATED_SRC "${SPIRV_CUSTOM_SRC_DIR}/${VAR_NAME}_embedded.cpp")
                list(APPEND SPIRV_CPP_SOURCES ${SPIRV_GENERATED_SRC})
                list(APPEND SPIRV_DECL_LIST ${VAR_NAME})

                # --- CUSTOM COMMAND 1: GLSL to SPV Compilation (Defines rule for ${SPIRV_OUTPUT}) ---
                add_custom_command(
                    OUTPUT ${SPIRV_OUTPUT}
                    COMMAND ${GLSLANG_VALIDATOR_EXECUTABLE} -V ${GLSL_INPUT} --target-env ${PROJECT_GLSL_TARGET_ENV} -o ${SPIRV_OUTPUT}
                    DEPENDS ${GLSL_INPUT}
                    COMMENT "Compiling GLSL: ${GLSL_INPUT} -> ${SPIRV_OUTPUT}"
                    VERBATIM
                )
            
                set(SPV_TARGET_NAME "spv_file_${VAR_NAME}")
                add_custom_target(${SPV_TARGET_NAME} ALL DEPENDS ${SPIRV_OUTPUT})

                set_property(TARGET ${SPV_TARGET_NAME} PROPERTY 
                    ADDITIONAL_CLEAN_FILES ${SPIRV_OUTPUT}
                )
                
                # --- CUSTOM COMMAND 2: BUILD-TIME C++ Source Generation (Hex Conversion) ---
                # This uses an inline Python script to convert the binary .spv file to a C++ hex array.
                add_custom_command(
                    OUTPUT ${SPIRV_GENERATED_SRC}
                    # Executing the external Python script (simpler for cmd.exe)
                    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/cmake/spirv_to_cpp.py 
                            ${SPIRV_OUTPUT} 
                            ${SPIRV_GENERATED_SRC} 
                            ${VAR_NAME}
                    
                    DEPENDS ${SPIRV_OUTPUT}
                    COMMENT "Generating hex-encoded C++ source: ${SPIRV_GENERATED_SRC}"
                    VERBATIM
                )
                
                list(APPEND SPV_COMPILE_TARGETS ${SPV_TARGET_NAME})

            endforeach()

            # --- 3. Object Library and Linkage ---
            message(STATUS "Integrating compiled SPIR-V files into an Object Library...")
            
            add_library(SPIRV_BINARIES OBJECT)
            
            # The generated .cpp files are added as sources; CMake will compile them.
            target_sources(SPIRV_BINARIES PRIVATE ${SPIRV_CPP_SOURCES})
            
            # Link the object files to your main executable
            target_link_libraries(${PROJECT_NAME} PRIVATE $<TARGET_OBJECTS:SPIRV_BINARIES>)

            # Ensure the SPIRV_BINARIES target depends on the SPV file targets
            add_dependencies(SPIRV_BINARIES ${SPV_COMPILE_TARGETS})

            # ==========================================
            # --- 4. Generate C++ Declaration Header ---
            # ==========================================
            message(STATUS "Generating C++ header with 'extern' declarations...")

            set(SPIRV_DECL_HEADER "${GENERATED_HEADER_DIR}/spirv_bin.h")
            set(DECLARATION_CODE "")

            foreach(VAR_NAME ${SPIRV_DECL_LIST})
                string(APPEND DECLARATION_CODE
                    "extern \"C\" const size_t ${VAR_NAME}_SPIRV_BYTES;\n"
                    "extern \"C\" const unsigned char ${VAR_NAME}_SPIRV_BIN[];\n"
                )
            endforeach()

            set(HEADER_CONTENT
"#ifndef SPIRV_BIN_H
#define SPIRV_BIN_H

// SPIRV-V binaries generated by CMake. Do not edit.
#include <cstddef> // for size_t
#include <cstdint> // for unsigned char

${DECLARATION_CODE}

#endif // SPIRV_BIN_H"
            )

            file(WRITE "${GENERATED_HEADER_DIR}/tmp_header_content.h.in" "${HEADER_CONTENT}")
            configure_file(
                "${GENERATED_HEADER_DIR}/tmp_header_content.h.in"
                "${SPIRV_DECL_HEADER}"
                COPYONLY
            )
            file(REMOVE "${GENERATED_HEADER_DIR}/tmp_header_content.h.in")
            
            # Ensure the generated header directory is included in the project
            target_include_directories(${PROJECT_NAME} PUBLIC ${GENERATED_HEADER_DIR})
            message(STATUS "Successfully configured SPIR-V integration. Include: <spirv_bin.h>")

        else()
            message(STATUS "No GLSL shader files found in ${PROJECT_GLSL_DIR}")
            set(COMPILE_GLSL_SHADERS OFF)
        endif()
    endif()
endif()

#==============================================================================
# Section: Copy Resources
#==============================================================================
file(COPY "${CMAKE_SOURCE_DIR}/resources" DESTINATION "${CMAKE_BINARY_DIR}")

#==============================================================================
# Section: Installation
#==============================================================================
option(INSTALL_PROJECT "Enable installation of the project" OFF)
if(INSTALL_PROJECT)
    message(STATUS "Installation enabled (Prefix: ${CMAKE_INSTALL_PREFIX})")
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    if(EXISTS "${PROJECT_INCLUDE_DIR}")
        install(DIRECTORY ${PROJECT_INCLUDE_DIR}/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME} # Install to include/<project_name>
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
        )
        message(STATUS "Headers will be installed to: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
    endif()
    # Check COMPILE_GLSL_SHADERS flag to see if SPIRV dir *should* exist
    if(COMPILE_GLSL_SHADERS AND IS_DIRECTORY "${PROJECT_SPIRV_DIR}")
         install(DIRECTORY ${PROJECT_SPIRV_DIR}/
            DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/shaders # share/<project_name>/shaders
        )
        message(STATUS "SPIR-V shaders will be installed to: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/shaders")
    endif()
    # include(CPack) # Add CPack configuration here if needed
endif()

# --- End of Configuration ---
message(STATUS "CMake configuration finished for ${PROJECT_NAME}")