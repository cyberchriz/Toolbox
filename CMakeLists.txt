# set CMake version
set(MIN_VERSION "3.9" CACHE STRING "minimum CMake version")
cmake_minimum_required(VERSION ${MIN_VERSION}...${CMAKE_VERSION})
if (${CMAKE_VERSION} VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_VERSION})
endif()

# set core variables of this project (with cmake-gui support)
set(CMAKE_PROJECT_NAME "unnamed" CACHE STRING "project name")
set(CMAKE_PROJECT_VERSION "1.0.0" CACHE STRING "project version (major.minor.revision)")
set(CMAKE_PROJECT_DESCRIPTION "no description" CACHE STRING "project description")

# set the build type if not specified (with Debug as default)
if(NOT $ENV{CMAKE_BUILD_TYPE})
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "build type (Debug, Release, RelWithDebInfo, MinSizeRel)")
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
		set(ENV{CMAKE_BUILD_TYPE} ${CMAKE_BUILD_TYPE})
endif()

# set essential folders
set(CMAKE_SOURCE_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH "project root path (top level)")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/sources" CACHE PATH "sources folder corresponding to the current CMakeLists.txt")
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/out/build" CACHE PATH "binary build directory of the project")
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR})
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}")

# language setup
set(CMAKE_PROJECT_LANGUAGES "C;CXX" CACHE STRING "available languages: C, CXX, CSharp, CUDA, OBJC, OBJCXX, Fortran, HIP, ISPC, Swift, ASM, ASM_NASM, ASM_MARMASM, ASM_MASM, ASM-ATT") 
set(CMAKE_CXX_STANDARD_DEFAULT "17" CACHE STRING "required C/C++ standard")
	set_property(CACHE CMAKE_CXX_STANDARD_DEFAULT PROPERTY STRINGS 98 11 14 17 20 23)
	set(CMAKE_CXX_STANDARD_REQUIRED "TRUE")
enable_language(${CMAKE_PROJECT_LANGUAGES})

# compiler setup
set(PROJECT_COMPILER "AUTO" CACHE STRING "compiler model")
	set_property(CACHE PROJECT_COMPILER PROPERTY STRINGS AUTO GCC Clang DPC++ MSVC MinGW AppleClang IntelC++ LLVM)
if (PROJECT_COMPILER STREQUAL "GCC")
	# TODO: compiler-specific flags for GCC
elseif (PROJECT_COMPILER STREQUAL "Clang")
	# TODO: compiler-specific flags for Clang
elseif (PROJECT_COMPILER STREQUAL "DPC++")
	set(ENV{CXX} icpx)
	find_package(IntelSYCL REQUIRED)
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fsycl)
	target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -fsycl)
elseif (PROJECT_COMPILER STREQUAL "MSVC")
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
	set(PROJECT_COMPILE_OPTIONS "-Wall" CACHE STRING "project compile options")
	add_compile_options(${PROJECT_COMPILE_OPTIONS})
elseif (PROJECT_COMPILER STREQUAL "MinGW")
	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};C:/Program Files/GNU/mingw/lib")
elseif (PROJECT_COMPILER STREQUAL "AppleClang")
	# TODO: compiler-specific flags for AppleClang
elseif (PROJECT_COMPILER STREQUAL "IntelC++")
	# TODO: compiler-specific flags for IntelC++
elseif (PROJECT_COMPILER STREQUAL "LLVM")
	# TODO: compiler-specific flags for LLVM
endif()

# define project
project(
	${CMAKE_PROJECT_NAME}
	VERSION ${CMAKE_PROJECT_VERSION}
	DESCRIPTION ${CMAKE_PROJECT_DESCRIPTION}
	LANGUAGES ${CMAKE_PROJECT_LANGUAGES}
)

# Automatically add subdirectories that have CMakeLists.txt files
file(GLOB SUBDIRECTORIES "${CMAKE_SOURCE_DIR}/*")
foreach(SUBDIR ${SUBDIRECTORIES})
	if(IS_DIRECTORY "${SUBDIR}" AND EXISTS "${SUBDIR}/CMakeLists.txt")
		add_subdirectory("${SUBDIR}")
	endif()
endforeach()

# Define allowed file extensions to be compiled
set(ALLOWED_EXTENSIONS ".cpp;.cxx;.cc" CACHE STRING "allowed extensions of files to be compiled")

# Filter SOURCE_DIR for specified file extensions
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*")
foreach(SOURCE ${SOURCES})
    get_filename_component(EXTENSION ${SOURCE} EXT)
	set(EXT_INDEX -1)
    list(FIND ALLOWED_EXTENSIONS ${EXTENSION} EXT_INDEX)
    if(EXT_INDEX EQUAL -1)
        list(REMOVE_ITEM SOURCES ${SOURCE})
    endif()
endforeach()

# Check if there are any sources found, then add the executable
if (SOURCES)
    message("found sources in folder ${SOURCE_DIR}:")
    message("-- ${SOURCES}")
    add_executable(
        ${CMAKE_PROJECT_NAME}
        ${SOURCES}
    )
else()
    message("${SOURCE_DIR}: no source files found")
endif()


# add static libraries
set(PROJECT_ADD_STATIC_LIBS "" CACHE STRING "static libraries to be added")
if (PROJECT_ADD_STATIC_LIBS)
    foreach(ITEM IN ${PROJECT_ADD_STATIC_LIBS})
        # Extract the library name from the item (remove any folder structure)
        get_filename_component(LIB_NAME ${ITEM} NAME_WE)
        
        # Create the library in the libs folder
        add_library(
            ${LIB_NAME} STATIC
            ${ITEM}
        )
        set_target_properties(${LIB_NAME} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/libs"
        )
    endforeach()
endif()

# set default path for project headers
set(HEADERS "${CMAKE_SOURCE_DIR}/headers" CACHE PATH "default headers folder of this project")

# set and include default paths for external dependencies
if (WIN32)
	set(DEPENDENCIES_PATH "C:/Dependencies" CACHE PATH "default path(s) for external depencencies")
elseif (UNIX)
	set(DEPENDENCIES_PATH "usr/local/lib;/usr/include" CACHE PATH "default path(s) for external depencencies")
endif()
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${DEPENDENCIES_PATH}")

target_include_directories(
	${CMAKE_PROJECT_NAME}
	PUBLIC
	${CMAKE_MODULE_PATH}
	${HEADERS}
)

# select required external packages and libraries
set(USE_VULKAN "TRUE" CACHE BOOL "whether to include the Vulkan library")
if (USE_VULKAN)
	set($ENV{VULKAN_SDK} "C:/VulkanSDK")
	set(VULKAN_LIBRARY "C:/VulkanSDK/1.3.268.0/Lib" CACHE PATH "location of Vulkan Lib folder")
	set(VULKAN_INCLUDE_DIR "C:/VulkanSDK/1.3.268.0/Include/vulkan" CACHE PATH "location of Vulkan Include folder")
	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${VULKAN_LIBRARY};${VULKAN_INCLUDE_DIR}")
	find_package(Vulkan REQUIRED)
	if  (Vulkan_FOUND)
		target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${VULKAN_LIBRARY} ${VULKAN_INCLUDE_DIR})
		target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})
	endif()
endif()

set(USE_GLFW "TRUE" CACHE BOOL "whether to include the GLFW library")
if (USE_GLFW)
	set(GLFW_LIB_PATH "C:/Dependencies/glfw-3.3.8.bin.WIN64/lib-vc2022" CACHE PATH "location of GLFW Lib folder")
	set(GLFW_INCLUDE_PATH "C:/Dependencies/glfw-3.3.8.bin.WIN64/include" CACHE PATH "location of GLFW Include folder")
	target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${GLFW_LIB_PATH} ${GLFW_INCLUDE_PATH})
	target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${GLFW_LIBRARIES})
endif()

set(USE_GLM "TRUE" CACHE BOOL "whether to include the GLM library")
if (USE_GLM)
	set(GLM_HEADERS_PATH "C:/Dependencies/glm-0.9.9.8/glm/glm" CACHE PATH "location of glm headers folder")
	target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${GLM_HEADERS_PATH})
endif()

# install build
set(INSTALL_PROJECT "FALSE" CACHE BOOL "install build")
if (INSTALL_PROJECT)
	install(
		TARGETS
			${CMAKE_PROJECT_NAME}
		DESTINATION
			${CMAKE_PROJECT_NAME}
	)
endif()

# first-ever execution of this script?
if (NOT EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
	message("\n ==========================================================================")
	message(  " THIS SEEMS TO BE THE FIRST-TIME EXECUTION OF THIS CMakeLists.txt FILE")
	message(  " It's recommended to configure the project details by editing CMakeCache.txt")
	message(  " with cmake-gui or ccmake")
	message(  " ==========================================================================\n")
endif()