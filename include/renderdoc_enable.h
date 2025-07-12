// This is a header-only file to provide RenderDoc capture functionality
// by using the functions rdoc_start_capture() and rdoc_end_capture();
// for this to properly work, RenderDoc must be installed and
// the CMake option USE_RENDERDOC_API_DEFINE must be set to ON.
// Also, the renderdoc dll must be in the system PATH or
// the CMake variable RENDERDOC_DLL_RUNTIME_PATH must be set to the
// directory where the renderdoc dll is located.
// The CMake variable RENDERDOC_API_INCLUDE_DIR must be set to the
// directory where the renderdoc_app.h header file is located.

#ifndef RENDERDOC_ENABLE_H
#define RENDERDOC_ENABLE_H

#include <log.h>

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
#include <renderdoc_app.h> // CMake must set the include path correctly for this!

// Global variables
inline RENDERDOC_API_1_6_0* rdoc_api = NULL;
inline bool rdoc_initialized = false;

#if defined(_WIN32) || defined(_WIN64)
inline HMODULE renderdoc_module = NULL;
#else
inline void* renderdoc_module = NULL;
#endif

// Helper function to get the API (renamed to avoid conflict with the type 'pRENDERDOC_GetAPI')
typedef pRENDERDOC_GetAPI PFN_RENDERDOC_GetAPI; // Typedef for clarity

// forward declarations
void rdoc_shutdown_api();


inline void rdoc_init_api() {
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
		renderdoc_module = dlopen(dll_path, RTLD_NOW | RTLD_NOLOAD);
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
	std::atexit(&rdoc_shutdown_api); // register for automatic cleanup on exit
	rdoc_initialized = true; // Set flag once initialization is attempted
#endif // _RELEASE
}

inline void rdoc_shutdown_api() {
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

inline void rdoc_start_capture() {
#ifndef _RELEASE
	if (!rdoc_initialized) { // If not initialized, attempt to do so
		rdoc_init_api();
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

inline void rdoc_end_capture() {
#ifndef _RELEASE
	if (rdoc_api) { // Only attempt end capture if rdoc_api is valid
		rdoc_api->EndFrameCapture(NULL, NULL);
		Log::info("RenderDoc capture ended.");
	}
	else {
		Log::warning("RenderDoc API not available. Capture cannot be ended.");
	}
#endif
}

#else // USE_RENDERDOC_API_DEFINE

// alternative definitions when RenderDoc is not enabled (to avoid compilation errors in the main code)
inline void rdoc_start_capture() {
	Log::warning("RenderDoc API is not defined. Capture cannot be started.");
}

inline void rdoc_end_capture() {
	Log::info("RenderDoc API is not defined. Capture cannot be ended.");
}

#endif // USE_RENDERDOC_API_DEFINE

#endif // RENDERDOC_ENABLE_H