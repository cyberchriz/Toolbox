#==============================================================================
# Generic CMake Configuration File
# Author: Christian Suer (cyberchriz)
#==============================================================================

# --- Basic Setup ---
cmake_minimum_required(VERSION 3.15)

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
    message(STATUS " Review and configure options using:")
    message(STATUS "   - cmake-gui or ccmake")
    message(STATUS "   - Command line: cmake .. -D<OPTION>=<VALUE>")
    message(STATUS "   - Visual Studio: CMake Settings editor")
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
        target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES} "include/spirv_bin_precompiled.h")
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
    find_package(Vulkan REQUIRED)
    if(Vulkan_FOUND)
        message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})
    endif()
endif()

# --- GLFW3 ---
option(USE_GLFW "Find or fetch and link GLFW3" OFF)
option(FETCH_GLFW "Download GLFW3 using FetchContent (if USE_GLFW is ON)" OFF)
mark_as_advanced(FETCH_GLFW) # Hide Fetch option by default in GUI

if(USE_GLFW)
    if(FETCH_GLFW)
        message(STATUS "Attempting to fetch GLFW3 via FetchContent")
        FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.8) # Check for latest tag
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Options for fetched dependency
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glfw)
        target_link_libraries(${PROJECT_NAME} PUBLIC glfw) # Link target created by FetchContent
        message(STATUS "Using GLFW3 via FetchContent")
    else()
        message(STATUS "Attempting to find pre-installed GLFW3 via find_package")
        find_package(glfw3 3.3 REQUIRED) # Note: Package name is glfw3 here
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
        endif()
    endif()
endif()

# --- GLM (Header Only) ---
option(USE_GLM "Find or fetch and use GLM" OFF)
option(FETCH_GLM "Download GLM using FetchContent (if USE_GLM is ON)" ON) # Default ON for header-only
mark_as_advanced(FETCH_GLM)

if(USE_GLM)
    if(FETCH_GLM)
        message(STATUS "Attempting to fetch GLM via FetchContent")
        FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 0.9.9.8) # Check for latest tag
        FetchContent_MakeAvailable(glm) # Handles includes for header-only
        message(STATUS "Using GLM via FetchContent")
    else()
        message(STATUS "Attempting to find pre-installed GLM via find_package")
        find_package(glm 0.9.9 REQUIRED)
        if(glm_FOUND)
            message(STATUS "Found GLM: ${glm_INCLUDE_DIRS}")
            target_include_directories(${PROJECT_NAME} PUBLIC ${glm_INCLUDE_DIRS})
        endif()
    endif()
endif()

# --- RenderDoc API (for programmatic capture) ---
# This is a header-only API for dynamic loading.
# No linking against a RenderDoc library is performed.
# ... (rest of your CMakeLists.txt before this block) ...
option(USE_RENDERDOC_API "Enable programmatic RenderDoc capture API calls" ON)
mark_as_advanced(USE_RENDERDOC_API)

if(USE_RENDERDOC_API)
    message(STATUS "RENDERDOC_API_INCLUDE_DIR should point to the 'api' directory inside your RenderDoc installation.")
    message(STATUS "Example: C:/Program Files/RenderDoc/renderdoc/api")
    message(STATUS "Example: C:/Program Files (x86)/RenderDoc/renderdoc/api")
    message(STATUS "Example: /usr/local/include/renderdoc/api")

    # This list will hold all potential RenderDoc API include paths
    set(RENDERDOC_SEARCH_PATHS)

    # Windows specific paths
    if(WIN32)
        # CMAKE_PROGRAM_FILES is usually "C:/Program Files"
        if(CMAKE_PROGRAM_FILES)
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES}/RenderDoc/renderdoc/api")
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES}/RenderDoc/renderdoc")
        endif()
        # CMAKE_PROGRAM_FILES_X86 is "C:/Program Files (x86)" on 64-bit Windows
        if(CMAKE_PROGRAM_FILES_X86)
            list(APPEND RENDERDOC_SEARCH_PATHS "${CMAKE_PROGRAM_FILES_X86}/RenderDoc/renderdoc/api")
        endif()
        # You could also add the %LOCALAPPDATA% path if RenderDoc has a user-specific install option
        # For example, if RenderDoc installs to C:\Users\YourUser\AppData\Local\RenderDoc\renderdoc\api
        # list(APPEND RENDERDOC_SEARCH_PATHS "$ENV{LOCALAPPDATA}/RenderDoc/renderdoc/api")
    endif()

    # Linux specific paths (and potentially macOS, though RenderDoc installation differs there)
    # These are typically hardcoded because there aren't standard CMake variables like on Windows for /usr/local
    list(APPEND RENDERDOC_SEARCH_PATHS
        "/usr/local/include/renderdoc/api"
        "/opt/renderdoc/include/renderdoc/api"
        "/usr/include/renderdoc/api"
    )

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
        message(WARNING "Could not find RenderDoc API header (renderdoc_app.h). "
                        "Programmatic capture will not be available. "
                        "Please set RENDERDOC_API_INCLUDE_DIR manually (e.g., -DRENDERDOC_API_INCLUDE_DIR=\"C:/Path/To/RenderDoc/api\") "
                        "if you need it.")
        # Optionally, turn off the option if the header isn't found
        # set(USE_RENDERDOC_API OFF CACHE BOOL "RenderDoc API disabled: header not found" FORCE)
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
    # set(Boost_USE_STATIC_LIBS ON)
    # List required components explicitly
    find_package(Boost 1.88.0 COMPONENTS system filesystem REQUIRED)
    if(Boost_FOUND)
        message(STATUS "Found Boost ${Boost_VERSION_STRING}: ${Boost_LIBRARIES}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
    endif()
endif()

message(STATUS "External dependencies configured.")


#==============================================================================
# Section: GLSL Shader Compilation
#==============================================================================
option(COMPILE_GLSL_SHADERS "Enable automatic GLSL shader (re)compilation" ON)
if(COMPILE_GLSL_SHADERS AND EXISTS "${PROJECT_GLSL_DIR}")
    find_program(GLSLANG_VALIDATOR_EXECUTABLE NAMES glslangValidator glslc)
    if(NOT GLSLANG_VALIDATOR_EXECUTABLE)
        message(WARNING "GLSL compiler not found. Skipping shader compilation.")
        set(COMPILE_GLSL_SHADERS OFF) # Disable if not found, so install doesn't fail
    else()
        message(STATUS "Using GLSL compiler: ${GLSLANG_VALIDATOR_EXECUTABLE}")
        file(MAKE_DIRECTORY "${PROJECT_SPIRV_DIR}") # Ensure output directory exists
        file(GLOB_RECURSE GLSL_SHADER_SOURCES CONFIGURE_DEPENDS
            "${PROJECT_GLSL_DIR}/*.vert"
            "${PROJECT_GLSL_DIR}/*.frag"
            "${PROJECT_GLSL_DIR}/*.comp"
            "${PROJECT_GLSL_DIR}/*.tesc"
            "${PROJECT_GLSL_DIR}/*.tese"
            "${PROJECT_GLSL_DIR}/*.geom"
            "${PROJECT_GLSL_DIR}/*.glsl")

        if(GLSL_SHADER_SOURCES)
             message(STATUS "Found GLSL shaders to compile...")
             set(SPIRV_OUTPUT_FILES "") # <--- Initialize list here

             foreach(GLSL_INPUT ${GLSL_SHADER_SOURCES})
                get_filename_component(SHADER_NAME ${GLSL_INPUT} NAME)
                set(SPIRV_OUTPUT "${PROJECT_SPIRV_DIR}/${SHADER_NAME}.spv")

                # Add the output file to the list
                list(APPEND SPIRV_OUTPUT_FILES ${SPIRV_OUTPUT})

                add_custom_command(
                    OUTPUT ${SPIRV_OUTPUT}
                    COMMAND ${GLSLANG_VALIDATOR_EXECUTABLE} -V ${GLSL_INPUT} --target-env ${PROJECT_GLSL_TARGET_ENV} -o ${SPIRV_OUTPUT}
                    DEPENDS ${GLSL_INPUT} # Depends on the input shader file
                    COMMENT "Compiling ${GLSL_INPUT} -> ${SPIRV_OUTPUT}"
                    VERBATIM
                )
            endforeach()
        else()
            message(STATUS "No GLSL shader files found in ${PROJECT_GLSL_DIR}")
            set(COMPILE_GLSL_SHADERS OFF) # Disable if none found
            set(SPIRV_OUTPUT_FILES "") # Ensure list is empty if no shaders found
        endif()
    endif()
endif()


#==============================================================================
# Section: Translate SPIR-V shaders to C++ header file
# !!! requires the script "../cmake/generate_spirv_header.cmake !!!"
#==============================================================================

message(STATUS "Configuring SPIR-V to C++ header generation...")

# This section relies on the SPIRV_OUTPUT_FILES list generated in the GLSL section
# Only run if shaders were (re)compiled
if(COMPILE_GLSL_SHADERS AND SPIRV_OUTPUT_FILES)

    # Define paths
    set(SPIRV_HEADER_OUTPUT "${GENERATED_HEADER_DIR}/spirv_bin.h")
    set(SPIRV_HEADER_GENERATOR_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/cmake/generate_spirv_header.cmake")

    if(NOT EXISTS "${SPIRV_HEADER_GENERATOR_SCRIPT}")
         message(FATAL_ERROR "SPIR-V header generator script not found: ${SPIRV_HEADER_GENERATOR_SCRIPT}")
    endif()

    # Pass list via file
    set(SPIRV_FILE_LIST_PATH "${GENERATED_HEADER_DIR}/spirv_files.lst") # Define path for the temporary file list
    file(MAKE_DIRECTORY "${GENERATED_HEADER_DIR}") # Ensure the directory for the list file exists
    file(WRITE "${SPIRV_FILE_LIST_PATH}" "") # Create or clear the list file initially
    foreach(SPV_FILE ${SPIRV_OUTPUT_FILES}) # Iterate over the list of SPIR-V files and append each path on a new line
        file(APPEND "${SPIRV_FILE_LIST_PATH}" "${SPV_FILE}\n")
    endforeach()
    message(STATUS "  -> SPIR-V file list written to: ${SPIRV_FILE_LIST_PATH}")

    if(NOT EXISTS "${SPIRV_FILE_LIST_PATH}")
        message(FATAL_ERROR "SPIR-V file list not found: ${SPIRV_FILE_LIST_PATH}")
    endif()
        
    # Define the custom command to generate the header using the script
    add_custom_command(
        OUTPUT ${SPIRV_HEADER_OUTPUT}
        # Command invokes CMake in script mode (-P)
        COMMAND ${CMAKE_COMMAND}
            -DSPIRV_FILE_LIST_PATH=${SPIRV_FILE_LIST_PATH}
            -DOUTPUT_HEADER=${SPIRV_HEADER_OUTPUT}
            -DGENERATED_HEADER_DIR=${GENERATED_HEADER_DIR}
            -DPROJECT_SPIRV_DIR=${PROJECT_SPIRV_DIR}
            -P ${SPIRV_HEADER_GENERATOR_SCRIPT}
        # Depends on all the individual SPIR-V files and on the list file being up_to-date
        DEPENDS ${SPIRV_OUTPUT_FILES} ${SPIRV_HEADER_GENERATOR_SCRIPT} ${SPIRV_FILE_LIST_PATH}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating C++ header from SPIR-V files -> ${SPIRV_HEADER_OUTPUT}"
        VERBATIM
    )

    # Define a custom target that triggers the header generation
    add_custom_target(spirv_header_target ALL # Use ALL to build automatically
        DEPENDS ${SPIRV_HEADER_OUTPUT}
    )

    # Ensure the main project target depends on the header *generation target*
    # This guarantees the header is generated before the project tries to compile files that include it.
    add_dependencies(${PROJECT_NAME} spirv_header_target)

    message(STATUS "Enabled generation of ${SPIRV_HEADER_OUTPUT}")
    message(STATUS "  -> Will be generated from SPIR-V files in ${PROJECT_SPIRV_DIR}")
    message(STATUS "  -> Generator script: ${SPIRV_HEADER_GENERATOR_SCRIPT}")
else()
     message(STATUS "SPIR-V to C++ header generation skipped (COMPILE_GLSL_SHADERS is OFF).")
endif()



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