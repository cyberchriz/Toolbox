// This is a header-only file to provide RenderDoc capture functionality
// by using the functions rdoc_start_capture() and rdoc_end_capture();
// for this to properly work, RenderDoc must be installed and
// the CMake option USE_RENDERDOC_API_DEFINE must be set to ON.
// Also, the renderdoc dll must be in the system PATH or
// the CMake variable RENDERDOC_DLL_RUNTIME_PATH must be set to the
// directory where the renderdoc dll is located.
// The CMake variable RENDERDOC_API_INCLUDE_DIR must be set to the
// directory where the renderdoc_app.h header file is located.

#ifndef VKDEBUG_H
#define VKDEBUG_H

#include <algorithm> // Required for std::remove
#include <cstdlib>   // Required for std::atexit
#include <log.h>
#include <string>    // Required for std::string
#include <vulkan/vulkan.h>

// Platform-specific includes
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h> // Required for HMODULE, LoadLibraryA, GetProcAddress
#else // For Linux/Unix-like systems
#include <dlfcn.h>   // Required for dlopen, dlsym
#endif

// These two macros are standard tricks to turn a preprocessor macro into a string literal
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

// --- RenderDoc Integration Block ---
// Everything inside this block is conditional on USE_RENDERDOC_API_DEFINE being set by CMake.
// This ensures that if the option is OFF, none of RenderDoc's code/types are compiled.
#ifdef USE_RENDERDOC_API_DEFINE
#ifndef _RELEASE
#include <renderdoc_app.h> // CMake must set the include path correctly for this!


// Global variables (inline ensures single definition across translation units)
inline RENDERDOC_API_1_6_0* rdoc_api = NULL;
inline bool rdoc_initialized = false;

#if defined(_WIN32) || defined(_WIN64)
inline HMODULE renderdoc_module = NULL;
#else
inline void* renderdoc_module = NULL;
#endif

// Helper function to get the API (renamed to avoid conflict with the type 'pRENDERDOC_GetAPI')
typedef pRENDERDOC_GetAPI PFN_RENDERDOC_GetAPI; // Typedef for clarity

#endif  // _RELEASE
#endif  // USE_RENDERDOC_API_DEFINE


// class for debugging
class VkDebug {
public:


#ifdef USE_RENDERDOC_API_DEFINE

	// API shutdown function (now static)
	static inline void rdoc_shutdown_api() {
#ifndef _RELEASE
		if (renderdoc_module) {
#if defined(_WIN32) || defined(_WIN64)
			FreeLibrary(renderdoc_module);
#else
			dlclose(renderdoc_module);
#endif
			renderdoc_module = NULL;
			rdoc_api = NULL;
			rdoc_initialized = false; // Reset flag
		}
#endif
	}

	static void init(VkInstance instance) {
#ifndef _RELEASE // Further restrict to non-release builds
		if (rdoc_initialized) {
			Log::warning("RenderDoc API already initialized.");
			return; // Avoid re-initialization
		}

#if defined(_WIN32) || defined(_WIN64)
		// Load RenderDoc dynamically on Windows
		renderdoc_module = LoadLibraryA("renderdoc.dll");

		// alternatively, try path defined by CMake variable (RENDERDOC_DLL_DIR)
		if (!renderdoc_module) {
			std::string dll_path = STRINGIFY(RENDERDOC_DLL_RUNTIME_PATH);

			// Remove any unnecessary double quotes
			dll_path.erase(std::remove(dll_path.begin(), dll_path.end(), '"'), dll_path.end());

			// Remove any unnecessary (leading) backslashes
			// Note: This line might be problematic if the path genuinely starts with a backslash
			// and is not intended to be removed. Consider if this is truly needed.
			// For robustness, you might want to only remove leading backslashes if they are part of a macro expansion.
			dll_path.erase(std::remove(dll_path.begin(), dll_path.end(), '\\'), dll_path.end());

			dll_path += "/renderdoc.dll"; // Or "\\renderdoc.dll" if backslashes are preferred, but "/" usually works
			renderdoc_module = LoadLibraryA(dll_path.c_str());
		}

		// If LoadLibraryA failed, renderdoc_module will be NULL
		if (renderdoc_module) {
			PFN_RENDERDOC_GetAPI RENDERDOC_GetAPI_Func = (PFN_RENDERDOC_GetAPI)GetProcAddress(renderdoc_module, "RENDERDOC_GetAPI");
			if (RENDERDOC_GetAPI_Func) {
				// RENDERDOC_API_VERSION_1_6_0 is now visible from renderdoc_app.h
				int ret = RENDERDOC_GetAPI_Func(RENDERDOC_Version(eRENDERDOC_API_Version_1_6_0), (void**)&rdoc_api);
				if (ret != 1) {
					// API not available (e.g., wrong version)
					rdoc_api = NULL; // Ensure it's null if something went wrong
					FreeLibrary(renderdoc_module);
					renderdoc_module = NULL;
					Log::warning("RenderDoc API version mismatch or not available.");
				}
			}
			else {
				// GetProcAddress failed
				FreeLibrary(renderdoc_module);
				renderdoc_module = NULL;
				Log::warning("Failed to get RENDERDOC_GetAPI function address from renderdoc.dll.");
			}
		}
		else {
			Log::warning("Failed to load renderdoc.dll. Make sure RenderDoc is installed and the DLL is in the correct path.");
		}

#else // For Linux/Unix-like systems
		// Load RenderDoc dynamically on Linux-like systems
		renderdoc_module = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);

		// alternatively, try path defined by CMake variable (RENDERDOC_DLL_DIR)
		if (!renderdoc_module) {
			std::string dll_path = STRINGIFY(RENDERDOC_DLL_RUNTIME_PATH);

			// Remove any unnecessary double quotes
			dll_path.erase(std::remove(dll_path.begin(), dll_path.end(), '"'), dll_path.end());

			dll_path += "/librenderdoc.so";
			renderdoc_module = dlopen(dll_path.c_str(), RTLD_NOW | RTLD_NOLOAD); // dlopen expects const char*
		}

		// If dlopen failed, renderdoc_module will be NULL
		if (renderdoc_module) {
			PFN_RENDERDOC_GetAPI RENDERDOC_GetAPI_Func = (PFN_RENDERDOC_GetAPI)dlsym(renderdoc_module, "RENDERDOC_GetAPI");
			if (RENDERDOC_GetAPI_Func) {
				int ret = RENDERDOC_GetAPI_Func(RENDERDOC_API_VERSION_1_6_0, (void**)&rdoc_api);
				if (ret != 1) {
					rdoc_api = NULL;
					dlclose(renderdoc_module);
					renderdoc_module = NULL;
					Log::warning("RenderDoc API version mismatch or not available.");
				}
			}
			else {
				dlclose(renderdoc_module);
				renderdoc_module = NULL;
				Log::warning("Failed to get RENDERDOC_GetAPI function address from librenderdoc.so.");
			}
		}
		else {
			Log::warning("Failed to load librenderdoc.so. Make sure RenderDoc is installed and the lib is in the correct path.");
		}
#endif
		// define pointers for labeling functions
		begin_label_func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
		end_label_func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
		insert_label_func = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
		initialized = true;

		std::atexit(&VkDebug::rdoc_shutdown_api); // Corrected: Pass static member function using class name
#endif // _RELEASE
	}

	static void capture_start() {
#ifndef _RELEASE
		if (!initialized) {
			Log::warning("VkDebug is not initialized. Failed to start capture.");
			return;
		}
		if (rdoc_api) { // Only attempt capture if rdoc_api is valid
			rdoc_api->StartFrameCapture(NULL, NULL);
			Log::info("RenderDoc capture started.");
		}
		else {
			Log::warning("RenderDoc API not available. Capture cannot be started.");
		}
#endif
	}

	static void capture_stop() {
#ifndef _RELEASE
		if (!initialized) {
			Log::warning("VkDebug is not initialized. Capture is not available.");
			return;
		}
		if (rdoc_api) { // Only attempt end capture if rdoc_api is valid
			rdoc_api->EndFrameCapture(NULL, NULL);
			Log::info("RenderDoc capture ended.");
		}
		else {
			Log::warning("RenderDoc API not available. Capture cannot be ended.");
		}
#endif
	}

	// start RenderDoc debug label
	static void label_start(VkCommandBuffer buffer, std::string label, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) {
#ifndef _RELEASE
		if (!initialized) {
			Log::warning("Can't use VkDebug::label_start(). VkDebug must be initialized first");
			return;
		}
		// Correctly check and call the function pointer variable
		if (begin_label_func) {
			VkDebugUtilsLabelEXT labelInfo = {};
			labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			labelInfo.pLabelName = label.c_str();
			labelInfo.color[0] = r;
			labelInfo.color[1] = g;
			labelInfo.color[2] = b;
			labelInfo.color[3] = a;
			begin_label_func(buffer, &labelInfo); // Correct call
		}
#endif
	}

	static void label_stop(VkCommandBuffer buffer) {
#ifndef _RELEASE
		// Correctly check and call the function pointer variable
		if (end_label_func) {
			end_label_func(buffer);
		}
#endif
	}

	// Helper function for a single-point debug label (e.g. for RenderDoc)
	static void label_insert(VkCommandBuffer buffer, std::string label, float r = 0.0f, float g = 1.0f, float b = 0.0f, float a = 1.0f) {
#ifndef _RELEASE
		if (!initialized) {
			Log::warning("Can't use VkDebug::label_insert(). VkDebug must be initialized first");
			return;
		}
		// Correctly check and call the function pointer variable
		if (insert_label_func) {
			VkDebugUtilsLabelEXT labelInfo = {};
			labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			labelInfo.pLabelName = label.c_str();
			labelInfo.color[0] = r;
			labelInfo.color[1] = g;
			labelInfo.color[2] = b;
			labelInfo.color[3] = a;
			insert_label_func(buffer, &labelInfo); // Correct call
		}
#endif
	}

#else // USE_RENDERDOC_API_DEFINE is NOT defined

	// alternative definitions when RenderDoc is not enabled (to avoid compilation errors in the main code)
	static void init(VkInstance instance) { Log::warning("RenderDoc API is not defined. Initialization skipped."); }
	static void capture_start() { Log::warning("RenderDoc API is not defined. Capture cannot be started."); }
	static void capture_stop() { Log::warning("RenderDoc API is not defined. Capture cannot be ended."); }
	// These need to match the signature of the actual functions, even if they are no-ops
	static void label_start(VkCommandBuffer buffer, const char* label, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) { Log::warning("RenderDoc API is not defined. Label start skipped."); }
	static void label_stop(VkCommandBuffer buffer) { Log::warning("RenderDoc API is not defined. Label stop skipped."); }
	static void label_insert(VkCommandBuffer buffer, const char* label, float r = 0.0f, float g = 1.0f, float b = 0.0f, float a = 1.0f) { Log::warning("RenderDoc API is not defined. Label insert skipped."); }
	static void rdoc_shutdown_api() { Log::warning("RenderDoc API is not defined. Shutdown skipped."); }

#endif // USE_RENDERDOC_API_DEFINE
private:
	// These static inline member variables must be declared outside the conditional compilation
	// for the functions, but still within the class.
	static inline PFN_vkCmdBeginDebugUtilsLabelEXT begin_label_func = nullptr;
	static inline PFN_vkCmdEndDebugUtilsLabelEXT end_label_func = nullptr;
	static inline PFN_vkCmdInsertDebugUtilsLabelEXT insert_label_func = nullptr;
	static inline bool initialized = false;
};

#endif // VKDEBUG_H
