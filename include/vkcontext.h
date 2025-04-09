#ifndef VKCONTEXT_H
#define VKCONTEXT_H

/* This library simplifies working with Vulkan objects
 
Summary of a typical Vulkan workflow
1. Instance Creation (with necessary extensions)                                        class Instance
2. Physical Device selection and creation of an associated logical device               class Device
3. Surface creation                                                                     class Surface
4. Swapchain creation (to manage image presentation)                                    class Swapchain
        ImageView creation
        FrameBuffer creation
5. RenderPass creation (defines rendering operations and attachments)                   class RenderPass
6. Pipeline creation                                                                    class GraphicsPipeline, ComputePipeline, TransferPipeline
        DescritorPool creation                                                          class DescriptorPool
        DescriptorSets creation                                                         class DescriptorSet
        bind buffers (index / vertex / storage) to DescriptorSets                       method DescriptorSet::bind_buffer()
        load Shaders                                                                    class ShaderModule
        VertexDescriptions                                                              class VertexDescriptions
7. CommandPool allocation                                                               class CommandPool
8. CommandBuffer allocations (to record rendering commands)                             class CommandBuffer
9. main loop (repeat)
        Data Buffer allocations (vertex / index / storage buffers)                      class Buffer
        CommandBuffer recording                                                         
            begin render pass                                                           method CommandBuffer::begin_renderpass()
            bind pipeline                                                               method CommandBuffer::bind_pipeline()
            draw commands                                                               method CommandBuffer::draw()
        Submit CommandBuffer to Queue                                                   method CommandBuffer::submit()
10. Clean-Up (release resources)

helper classes for signaling: Event, Fence, Semaphore

This library also comes with the option to handle Instance, Device and CommandPool on a high level via a 'VkManager' singleton class 
*/

// include headers
#pragma once // not strictly necessary if the included header files also all come with include guards, but just to be on the safe side
#define NOMINMAX
#include "log.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>
#include <array>
#include <variant>
#include <optional>
#include <spirv_bin.h>

// --- Platform-Specific Headers ---
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>            // required for HWND, HINSTANCE
#include <vulkan/vulkan_win32.h>
#include <__msvc_ostream.hpp>

#elif defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#include <android/native_window.h> // required for ANativeWindow
#include <vulkan/vulkan_android.h>

#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#include <xcb/xcb.h>            // required for xcb_connection_t, xcb_window_t
#include <vulkan/vulkan_xcb.h>

#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC
// Using Metal Surface is generally preferred on macOS with MoltenVK
#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan_metal.h>
#else
#error "surface support for iOS/other Apple platform not implemented"
#endif
#include <optional>

#else
#error "Unsupported platform: No Vulkan WSI platform defined."
#endif

// forward declarations
class Device;
class CommandPool;
class CommandBuffer;
class ShaderModule;
class DescriptorSet;
class Event;
class ImageView;
class Image;
class MemoryBarrier;
template<typename T> class BufferMemoryBarrier;
class ImageMemoryBarrier;

enum BufferUsage {
    VERTEX,
    STORAGE,
    UNIFORM,
    INDEX,
    TRANSFER
};

enum DescriptorType {
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    STORAGE_IMAGE,
    SAMPLED_IMAGE,
	COMBINED_IMAGE_SAMPLER
};

enum QueueFamily {
    GRAPHICS,
    COMPUTE,
    TRANSFER,
    UNKNOWN
};

/// class for managing a Vulkan Instance,
/// which is a wrapper for all other Vulkan objects
class Instance {
public:

    // default constructor
    Instance() {
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    }

    // Move constructor
    Instance(Instance&& other) noexcept {
        this->instance = std::exchange(other.instance, nullptr);
        this->application_info = std::move(other.application_info);
        this->extensions = std::move(other.extensions);
        this->layers = std::move(other.layers);
        // Reset 'other' instance
        other.instance = nullptr;
    }
    
    // move assignment
    Instance& operator=(Instance&& other) noexcept {
        if (this != &other) {
            // Release resources held by the current object
            if (instance != nullptr) {
                vkDestroyInstance(instance, nullptr);
                instance = nullptr;
                Log::info("[OLD INSTANCE DESTROYED]");
            }
            // Move resources from the 'other' object
            instance = std::exchange(other.instance, nullptr);
            application_info = std::move(other.application_info);
            extensions = std::move(other.extensions);
            layers = std::move(other.layers);
            // Reset the 'other' object's managed resource
            other.instance = nullptr;
        }
        return *this;
    }

    // delete copy constructors
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;
    
    // destructor
    ~Instance() {
        if (instance != nullptr) {
            vkDestroyInstance(instance, nullptr);
            instance = nullptr;
            Log::info("[INSTANCE DESTROYED]");
        }
    }

    // set application name and version
    void init_application(const char* name = "Vulkan Application", uint32_t major_version = 1, uint32_t minor_version = 0, uint32_t patch_version = 0) {
        application_info.pApplicationName = name;
        application_info.applicationVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    }

    // initialize Vulkan engine and version
    void init_engine(const char* name = "", uint32_t major_version = 0, uint32_t minor_version = 0, uint32_t patch_version = 0) {
        application_info.pEngineName = name;
        application_info.engineVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    }

    // initialize Vulkan API version
    void init_api_version(uint32_t version = VK_API_VERSION_1_2) {
        application_info.apiVersion = version;
    }

    // log names of available instance layers
    void log_available_layers() {
        if (Log::get_level() >= LogLevel::LEVEL_INFO) {
            uint32_t count;
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> properties(count);
            vkEnumerateInstanceLayerProperties(&count, properties.data());
            Log::info(count, " layer types available");
            for (uint32_t i = 0; i < count; i++) {
                Log::debug("(", i + 1, ") ", properties[i].layerName);
            }
        }
    }

    // add Vulkan layers
    void enable_layers(std::vector<const char*>& layer_names) {
        for (const char* name: layer_names) {
            layers.push_back(name);
        }
    }
    void enable_layers(const char* layer_name) {
        layers.push_back(layer_name);
    }

    // log names of available ínstance extensions
    void log_available_extensions() {
        // log available extensions
        if (Log::get_level() >= LogLevel::LEVEL_INFO) {
            uint32_t count;
            vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
            Log::info(count, " instance extensions available");
            for (uint32_t i = 0; i < count; i++) {
                Log::debug("(", i + 1, ") ", extensions[i].extensionName);
            }
        }
    }

    // add Vulkan extensions
    void enable_extensions(const std::vector<const char*>& extension_names) {
        for (const char* name : extension_names) {
            extensions.push_back(name);
        }
    }
    void enable_extensions(const char* extension_name) {
        extensions.push_back(extension_name);
    }

    // create Vulkan instance
    void create(VkInstanceCreateFlags flags = 0, const void* pNext = nullptr) {
        // destroy any previous instance
        if (instance != nullptr) {
            vkDestroyInstance(instance, nullptr);
            instance = nullptr;
            Log::info("[OLD INSTANCE DESTROYED]");
        }

        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.enabledLayerCount = layers.size();
        instance_create_info.ppEnabledLayerNames = layers.data();
        instance_create_info.enabledExtensionCount = extensions.size();
        instance_create_info.ppEnabledExtensionNames = extensions.data();
        instance_create_info.pNext = pNext;
        instance_create_info.flags = flags;
        instance_create_info.pApplicationInfo = &application_info;

        VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
        if (result == VK_SUCCESS) {
            Log::info("Vulkan instance successfully created.");
        }
        else {
            Log::error("Failed to create Vulkan Instance (VkResult=", result, ")");
        }
    }
    
    // return a reference to the Vulkan instance
    VkInstance get() const {
        return instance;
    }
    
protected:
    VkInstance instance = nullptr;
    VkApplicationInfo application_info = {};
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};

// class for mangaging physical and logical GPU devices
class Device {
public:
    // delete default constructor
    Device() = delete;

    // parametric constructor
    Device(Instance& instance, VkPhysicalDeviceFeatures enabled_features = {}, const std::vector<const char*>& enabled_extension_names = {}, uint32_t id = 0) {
        // confirm valid instance
        if (instance.get() == nullptr) {
            Log::error("Device constructor called with invalid instance parameter: create a valid object of the Instance() class first!");
        }
        // search for physical devices with Vulkan support
        uint32_t num_devices = 0;
        VkResult result = vkEnumeratePhysicalDevices(instance.get(), &num_devices, NULL);
        if (num_devices == 0) {
            Log::warning("No device(s) with Vulkan support found!");
            return;
        }
        if (result == VK_INCOMPLETE) {
            Log::warning("search for physical devices was incomplete");
        }

        // list available physical devices with Vulkan support
        std::vector<VkPhysicalDevice> devices(num_devices);
        vkEnumeratePhysicalDevices(instance.get(), &num_devices, devices.data());
        
        // default: select first available device (at index 0)
        uint32_t selected_index = 0;
        physical = devices[selected_index];
        uint32_t selected_id = 0;
        Log::info("available physical devices with Vulkan support:");

        for (uint32_t i = 0; i < num_devices; i++) {
            vkGetPhysicalDeviceProperties(devices[i], &properties);
            if (i == selected_index) {
                selected_id = properties.deviceID;
            }
            Log::info("(", i, ") ", properties.deviceName, ", deviceID ", properties.deviceID, ", vendorID ", properties.vendorID,
                ", type ", properties.deviceType, ", API version ", properties.apiVersion, ", driver version ", properties.driverVersion);
            // chose specific device (instead of default index 0) if passed id matches
            if (id == properties.deviceID) {
                physical = devices[i];
                selected_id = properties.deviceID;
                selected_index = i;
            }
        }
        Log::info("Selected physical device ", selected_index, " with ID ", selected_id);

        // store properties for selected device
        vkGetPhysicalDeviceProperties(physical, &properties);
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        properties2.pNext = nullptr;
        vkGetPhysicalDeviceProperties2(physical, &properties2);

        // log available extensions
        if (Log::get_level() >= LogLevel::LEVEL_INFO) {
            uint32_t count;
            vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, extensions.data());
            Log::debug(count, " device extensions available");
            for (uint32_t i = 0; i < count; i++) {
                Log::debug("(", i + 1, ") ", extensions[i].extensionName);
            }
        }

        // add requested device extensions to device create info
        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        this->extensions = std::move(enabled_extension_names);
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        device_create_info.ppEnabledExtensionNames = extensions.data();

        // Queue creation
        uint32_t num_queue_families;
        float priority = 1.0f; // default priority for all queue types
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &num_queue_families, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &num_queue_families, queue_families.data());
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        // Iterate over the queue families to find appropriate queue indices;
		// check for graphics, compute, and transfer queue support;
		// assign the first available queue family index for each type;
		// try to use dedicated indices for each queue type if available;
        int graphics_fallback = -1;
		int compute_fallback = -1;
		int transfer_fallback = -1;

        for (uint32_t i = 0; i < num_queue_families; ++i) {
            const VkQueueFamilyProperties& queue_family = queue_families[i];

            // Check for graphics queue support
            if (!graphics_queue_assigned && (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                graphics_queue_family_index = i;
                queue_create_infos.push_back({});
                queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_infos[i].queueFamilyIndex = i;
                queue_create_infos[i].queueCount = 1;
                queue_create_infos[i].pQueuePriorities = &priority;
                graphics_queue_assigned = true;
                Log::info("GRAPHICS queue supported -> added to queue_create_infos for this device");
				compute_fallback = queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT ? i : -1;
				transfer_fallback = queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT ? i : -1;
                continue;
            }

            // Check for compute queue support
            if (!compute_queue_assigned && (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                compute_queue_family_index = i;
                queue_create_infos.push_back({});
                queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_infos[i].queueFamilyIndex = i;
                queue_create_infos[i].queueCount = 1;
                queue_create_infos[i].pQueuePriorities = &priority;
                compute_queue_assigned = true;
                Log::info("COMPUTE queue supported -> added to queue_create_infos for this device");
				graphics_fallback = queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ? i : -1;
				transfer_fallback = queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT ? i : -1;
                continue;
            }

            // Check for transfer queue support
            if (!transfer_queue_assigned && (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT)) {
                transfer_queue_family_index = i;
                queue_create_infos.push_back({});
                queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_infos[i].queueFamilyIndex = i;
                queue_create_infos[i].queueCount = 1;
                queue_create_infos[i].pQueuePriorities = &priority;
                transfer_queue_assigned = true;
                Log::info("TRANSFER queue supported -> added to queue_create_infos for this device");
				graphics_fallback = queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ? i : -1;
				compute_fallback = queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT ? i : -1;
                continue;
            }
        }

		// If no dedicated queue family was found for a type, use the fallback
        if (!graphics_queue_assigned) {
            if (graphics_fallback != -1) {
                graphics_queue_family_index = graphics_fallback;
                graphics_queue_assigned = true;
                Log::info("no dedicated GRAPHICS queue family found; using fallback queue family index ", graphics_queue_family_index, " (shared queue)");
            }
			else {
				Log::warning("no dedicated GRAPHICS queue family found; no fallback available");
            }
        }
		if (!compute_queue_assigned) {
			if (compute_fallback != -1) {
				compute_queue_family_index = compute_fallback;
				compute_queue_assigned = true;
				Log::info("no dedicated COMPUTE queue family found; using fallback queue family index ", compute_queue_family_index, " (shared queue)");
			}
            else {
				Log::warning("no dedicated COMPUTE queue family found; no fallback available");
            }
		}
        if (!transfer_queue_assigned) {
            if (transfer_fallback != -1) {
			    transfer_queue_family_index = transfer_fallback;
			    transfer_queue_assigned = true;
			    Log::info("no dedicated TRANSFER queue family found; using fallback queue family index ", transfer_queue_family_index, " (shared queue)");
			}
			else {
				Log::warning("no dedicated TRANSFER queue family found; no fallback available");
			}
		}

        // Create logical device
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.ppEnabledExtensionNames = device_extension_names.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        device_create_info.pEnabledFeatures = &enabled_features;
        VkResult result = vkCreateDevice(physical, &device_create_info, nullptr, &logical);
        if (result == VK_SUCCESS) {
            Log::info("successfully created logical device (handle: ", logical, ")");
        }
        else{
            Log::error("Failed to create Vulkan logical device (VkResult=", result, ")");
        }

        // Acquire queue handles for this logical device
        if (graphics_queue == nullptr) {
            vkGetDeviceQueue(logical, graphics_queue_family_index, 0, &graphics_queue);
            Log::info("adding graphics queue to logical device (handle: ", graphics_queue, ")");
        }

        if (compute_queue == nullptr) {
            vkGetDeviceQueue(logical, compute_queue_family_index, 0, &compute_queue);
            Log::info("adding compute queue to logical device (handle: ", compute_queue, ")");
        }

        if (transfer_queue == nullptr) {
            vkGetDeviceQueue(logical, transfer_queue_family_index, 0, &transfer_queue);
            Log::info("adding transfer queue to logical device (handle: ", transfer_queue, ")");
        }

        Log::info("[DEVICE COMPLETED]");
    }

    // Move Constructor
    Device(Device&& other) noexcept {
        move_resources(other);
    }

    // Move Assignment
    Device& operator=(Device&& other) noexcept {
        if (this != &other) {
            destroy(); // release resource from 'this'
            move_resources(other);
            Log::info("Device resources from 'other' moved to 'this'");
        }
        return *this;
    }

    // delete copy constructors
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // getter functions
    VkDevice& get_logical() { return logical; }
    VkPhysicalDevice& get_physical() { return physical; }
    VkQueue& get_graphics_queue() { return graphics_queue; }
    VkQueue& get_compute_queue() { return compute_queue; }
    VkQueue& get_transfer_queue() { return transfer_queue; }
    uint32_t get_graphics_queue_family_index() const { return graphics_queue_family_index; }
    uint32_t get_compute_queue_family_index() const { return compute_queue_family_index; }
    uint32_t get_transfer_queue_family_index() const { return transfer_queue_family_index; }
    VkPhysicalDeviceProperties& get_properties() { return properties; }
    VkPhysicalDeviceProperties2& get_properties2() { return properties2; }
    std::vector<const char*>& get_extensions() { return extensions; }
	std::vector<const char*>& get_device_extension_names() { return device_extension_names; }
	
    VkPhysicalDeviceMemoryProperties& get_memory_properties() {
        static bool properties_queried = false;
		if (!properties_queried) {
			vkGetPhysicalDeviceMemoryProperties(physical, &memory_properties);
			properties_queried = true;
		}
        return memory_properties;
    }

    // destructor
    ~Device() {
        destroy();
    }

protected:
    // helper method to release resources
    void destroy() {
        // destroy logical device
        if (logical != nullptr) {
            vkDeviceWaitIdle(logical);
            vkDestroyDevice(logical, nullptr);
            logical = nullptr;
            Log::info("[LOGICAL DEVICE DESTROYED]");
        }
    }

    // helper method for move constructor and move assignment
    void move_resources(Device& other) {
        // Transfer ownership of Vulkan handles/resources using std::exchange;
        // std::move may not be supported with some Vulkan objects
        this->physical = std::exchange(other.physical, nullptr);
        this->logical = std::exchange(other.logical, nullptr);
        this->graphics_queue = std::exchange(other.graphics_queue, nullptr);
        this->compute_queue = std::exchange(other.compute_queue, nullptr);
        this->transfer_queue = std::exchange(other.transfer_queue, nullptr);
        this->graphics_queue_assigned = std::move(other.graphics_queue_assigned);
        this->compute_queue_assigned = std::move(other.compute_queue_assigned);
        this->transfer_queue_assigned = std::move(other.transfer_queue_assigned);
        this->graphics_queue_family_index = std::move(other.graphics_queue_family_index);
        this->compute_queue_family_index = std::move(other.compute_queue_family_index);
        this->transfer_queue_family_index = std::move(other.transfer_queue_family_index);
        this->properties = std::exchange(other.properties, VkPhysicalDeviceProperties{});
        this->properties2 = std::exchange(other.properties2, VkPhysicalDeviceProperties2{});
        this->extensions = std::move(other.get_extensions());
		this->device_extension_names = std::move(other.device_extension_names);
		this->memory_properties = std::exchange(other.memory_properties, VkPhysicalDeviceMemoryProperties{});
    }

    VkPhysicalDevice physical = nullptr;
    VkDevice logical = nullptr;
    VkQueue graphics_queue = nullptr;
    VkQueue compute_queue = nullptr;
    VkQueue transfer_queue = nullptr;
    bool graphics_queue_assigned = false;
    bool compute_queue_assigned = false;
    bool transfer_queue_assigned = false;
    uint32_t graphics_queue_family_index = 0;
    uint32_t compute_queue_family_index = 0;
    uint32_t transfer_queue_family_index = 0;
    VkPhysicalDeviceProperties properties = {};
    VkPhysicalDeviceProperties2 properties2 = {};
    std::vector<const char*> extensions;
    std::vector<const char*> device_extension_names;
    VkPhysicalDeviceMemoryProperties memory_properties = {};
};

class Image {
    friend class ImageView;
public:
    Image() = delete;
    Image(
        Device& device,
        VkImageType type,
        VkFormat format,
        VkExtent3D extent,
        uint32_t mip_levels = 1,
        uint32_t array_layers = 1,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED
    ) : logical(device.get_logical()), format(format), extent(extent), layout(initial_layout) {

        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = type;
        image_info.format = format;
        image_info.extent = extent;
        image_info.mipLevels = mip_levels;
        image_info.arrayLayers = array_layers;
        image_info.samples = samples;
        image_info.tiling = tiling;
        image_info.usage = usage;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // or VK_SHARING_MODE_CONCURRENT if needed
        image_info.initialLayout = initial_layout;

        VkResult result = vkCreateImage(logical, &image_info, nullptr, &image);
        if (result != VK_SUCCESS) {
            Log::error("in constructor Image::Image(...): failed to create image (VkResult=", result, ")");
        }

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(logical, image, &memory_requirements);

        uint32_t memory_type_index = find_memory_type(device, memory_properties, memory_requirements.memoryTypeBits);
        if (memory_type_index == UINT32_MAX) {
            vkDestroyImage(logical, image, nullptr);
            image = VK_NULL_HANDLE;
            Log::error("in constructor Image::Image(...): could not find suitable memory type for image.");
        }

        VkMemoryAllocateInfo memory_allocation_info = {};
        memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocation_info.allocationSize = memory_requirements.size;
        memory_allocation_info.memoryTypeIndex = memory_type_index;

        result = vkAllocateMemory(logical, &memory_allocation_info, nullptr, &memory);
        if (result != VK_SUCCESS) {
            vkDestroyImage(logical, image, nullptr);
            image = VK_NULL_HANDLE;
            Log::error("in constructor Image::Image(...): Failed to allocate image memory (VkResult=", result, ")");
        }

        result = vkBindImageMemory(logical, image, memory, 0);
        if (result != VK_SUCCESS) {
            vkFreeMemory(logical, memory, nullptr);
            vkDestroyImage(logical, image, nullptr);
            memory = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            Log::error("in constructor Image::Image(...): Failed to bind image memory (VkResult=", result, ")");
        }
        Log::debug("in constructor Image::Image(...): image created successfully (handle: ", image, ")");
    }

    // Move semantics
    Image(Image&& other) noexcept :
        logical(std::exchange(other.logical, nullptr)),
        image(std::exchange(other.image, VK_NULL_HANDLE)),
        memory(std::exchange(other.memory, VK_NULL_HANDLE)),
        format(other.format),
        extent(other.extent),
        layout(other.layout)
    {
    }

    Image& operator=(Image&& other) noexcept {
        if (this != &other) {
            destroy();
            logical = std::exchange(other.logical, nullptr);
            image = std::exchange(other.image, VK_NULL_HANDLE);
            memory = std::exchange(other.memory, VK_NULL_HANDLE);
            format = other.format;
            extent = other.extent;
            layout = other.layout;
        }
        return *this;
    }

    // Delete copy
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    ~Image() {
        destroy();
    }

    // --- Getters ---
    VkImage get() const { return image; }
    
    VkFormat get_format() const { return format; }
    
    VkExtent3D get_extent() const { return extent; }
    
    VkImageLayout get_layout() const { return layout; }
	
    VkDeviceMemory get_memory() const { return memory; }

protected:
    void destroy() {
        if (memory != VK_NULL_HANDLE) {
            vkFreeMemory(logical, memory, nullptr);
            memory = VK_NULL_HANDLE;
        }
        if (image != VK_NULL_HANDLE) {
            Log::info("Destroying image (handle: ", image, ")");
            vkDestroyImage(logical, image, nullptr);
            image = VK_NULL_HANDLE;
        }
    }

    // Helper (can reuse from Buffer or Device)
    uint32_t find_memory_type(Device& device, VkMemoryPropertyFlags properties, uint32_t type_filter) {
        const auto& mem_properties = device.get_memory_properties();
        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        return UINT32_MAX;
    }

    VkDevice logical = nullptr;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkFormat format;
    VkExtent3D extent;
    VkImageLayout layout; // Track current layout
};

class ImageView {
public:
    ImageView() = delete;
    ImageView(Device& device, const Image& image, VkImageViewType view_type, VkImageAspectFlags aspect_flags, uint32_t base_mip_level = 0, uint32_t level_count = 1, uint32_t base_array_layer = 0, uint32_t layer_count = 1)
        : logical(device.get_logical()) {

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = image.get(); // Use image handle from Image class
        view_info.viewType = view_type;
        view_info.format = image.get_format(); // Use format from Image class
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.subresourceRange.aspectMask = aspect_flags;
        view_info.subresourceRange.baseMipLevel = base_mip_level;
        view_info.subresourceRange.levelCount = level_count;
        view_info.subresourceRange.baseArrayLayer = base_array_layer;
        view_info.subresourceRange.layerCount = layer_count;

        VkResult result = vkCreateImageView(logical, &view_info, nullptr, &image_view);
        if (result != VK_SUCCESS) {
            Log::error("Failed to create image view (VkResult=", result, ")");
            // Handle error
            return;
        }
        Log::info("ImageView created successfully (handle: ", image_view, ")");
    }

    // Move semantics
    ImageView(ImageView&& other) noexcept :
        logical(std::exchange(other.logical, nullptr)),
        image_view(std::exchange(other.image_view, VK_NULL_HANDLE))
    {
    }

    ImageView& operator=(ImageView&& other) noexcept {
        if (this != &other) {
            destroy();
            logical = std::exchange(other.logical, nullptr);
            image_view = std::exchange(other.image_view, VK_NULL_HANDLE);
        }
        return *this;
    }

    // Delete copy
    ImageView(const ImageView&) = delete;
    ImageView& operator=(const ImageView&) = delete;


    ~ImageView() {
        destroy();
    }

    VkImageView get() const { return image_view; }

protected:
    void destroy() {
        if (image_view != VK_NULL_HANDLE) {
            Log::info("Destroying image view (handle: ", image_view, ")");
            vkDestroyImageView(logical, image_view, nullptr);
            image_view = VK_NULL_HANDLE;
        }
    }

    VkDevice logical = nullptr;
    VkImageView image_view = VK_NULL_HANDLE;
};

class RenderAttachment {
public:
    enum Type {
        INPUT,
        COLOR,
        DEPTH,
        RESOLVE,
        PRESERVE
    };

    Type type;
    VkAttachmentDescription* description;
    uint32_t index;

    // Constructor
    RenderAttachment(RenderAttachment::Type type, VkAttachmentDescription* attachment_description, uint32_t index)
        : type(type), description(attachment_description), index(index) {
    }

    // Move constructor
    RenderAttachment(RenderAttachment&& other) noexcept : type(other.type), description(std::move(other.description)), index(other.index) {}

    // Move assignment
    RenderAttachment& operator=(RenderAttachment&& other) noexcept {
        if (this != &other) {
            type = other.type;
            description = std::move(other.description);
            index = other.index;
        }
        return *this;
    }

    // Deleted copy and copy assignment constructors
    RenderAttachment(const RenderAttachment&) = delete;
    RenderAttachment& operator=(const RenderAttachment&) = delete;

private:
    // Deleted default constructor
    RenderAttachment() = delete;
};

class SubPass {
    friend class RenderPass;
public:
    SubPass() {}

    // add a reference to an attachment from the pool of attachment descriptions in the main RenderPass
    void add_attachment(RenderAttachment& attachment, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED) {
        if (finalized) {
            Log::warning("in method SubPass::add_attachment(): this subpass has already been finalized, as it's already been added to a parent RenderPass; hence no more attachments can be added at this point");
            return;
        }
        else {
            switch (attachment.type) {
                case RenderAttachment::Type::COLOR: {
                    uint32_t id = color_attachment_reference.value().size();
                    color_attachment_reference.value().resize(id + 1);
                    color_attachment_reference.value()[id].attachment = attachment.index;
                    color_attachment_reference.value()[id].layout = layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : layout;
                    break;
                }
                case RenderAttachment::Type::DEPTH: {
                    depth_attachment_reference.value().attachment = attachment.index;
                    depth_attachment_reference.value().layout = layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : layout;
                    break;
                }
                case RenderAttachment::Type::INPUT: {
                    uint32_t id = input_attachment_reference.value().size();
                    input_attachment_reference.value().resize(id + 1);
                    input_attachment_reference.value()[id].attachment = attachment.index;
                    input_attachment_reference.value()[id].layout = layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : layout;
                    break;
                }
                case RenderAttachment::Type::PRESERVE: {
                    if (layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                        Log::warning("in method SubPass::add_attachment: please make sure to use a valid layout parameter (VkImageLayout) for the preserve attachment");
                    }
                    uint32_t id = preserve_attachment_reference.value().size();
                    preserve_attachment_reference.value().resize(id + 1);
                    preserve_attachment_reference.value()[id] = attachment.index;
                    break;
                }
                case RenderAttachment::Type::RESOLVE: {
                    if (layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                        Log::warning("in method SubPass::add_attachment: please make sure to use a valid layout parameter (VkImageLayout) for the resolve attachment");
                    }
                    resolve_attachment_reference.value().attachment = attachment.index;
                    resolve_attachment_reference.value().layout = layout;
                    break;
                }
                default: {
                    Log::warning("in method SubPass::add_attachment(): RenderAttachment has invalid type; failed to add the attachment to the subpass");
                }
            }
        }
    }
protected:
    std::optional<std::vector<VkAttachmentReference>> color_attachment_reference;
    std::optional<std::vector<VkAttachmentReference>> input_attachment_reference;
    std::optional<std::vector<uint32_t>> preserve_attachment_reference;
    std::optional<VkAttachmentReference> depth_attachment_reference;
    std::optional<VkAttachmentReference> resolve_attachment_reference;
    bool finalized = false; // this flag is updated by the parent RenderPass class (as a friend class) after the subpass has been added
};

// a render pass defines the structure and dependencies of graphics rendering operations
class RenderPass {
public:
    // delete default constructor
    RenderPass() = delete;

    // parametric constructor
    RenderPass(Device& device, uint32_t multisample_count = 1) {
        this->logical = device.get_logical();
        this->multisample_count = multisample_count;
    }

    // add an attachment description
    RenderAttachment add_attachment(
        RenderAttachment::Type type,
            VkFormat format = VK_FORMAT_R32G32_SFLOAT,
            VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            VkImageLayout final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
            VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE
        ) {
        uint32_t id = attachment_description.size();

        attachment_description.resize(id + 1);
        attachment_description[id] = {};
        attachment_description[id].flags = 0; // or: VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT
        attachment_description[id].format = format;
        attachment_description[id].initialLayout = initial_layout;
        attachment_description[id].finalLayout = final_layout;

        switch (type) {
        case RenderAttachment::Type::COLOR: {
            attachment_description[id].samples = static_cast<VkSampleCountFlagBits>(multisample_count);
            attachment_description[id].loadOp = load_op;
            attachment_description[id].storeOp = store_op;
            break;
        }
        case RenderAttachment::Type::DEPTH: {
            attachment_description[id].samples = static_cast<VkSampleCountFlagBits>(multisample_count);
            attachment_description[id].stencilLoadOp = load_op;
            attachment_description[id].stencilStoreOp = store_op;
            depth_stencil_flag = true;
            break;
        }
        case RenderAttachment::Type::INPUT: {
            attachment_description[id].samples = VK_SAMPLE_COUNT_1_BIT; // Input attachments are usually single-sampled
            break;
        }
        case RenderAttachment::Type::PRESERVE: {
            attachment_description[id].samples = VK_SAMPLE_COUNT_1_BIT; // preserve attachments are usually single-sampled
            attachment_description[id].stencilLoadOp = load_op;
            attachment_description[id].stencilStoreOp = store_op;
            break;
        }
        case RenderAttachment::Type::RESOLVE: {
            attachment_description[id].samples = VK_SAMPLE_COUNT_1_BIT; // resolve attachments are usually single-sampled
            attachment_description[id].loadOp = load_op;
            attachment_description[id].storeOp = store_op;
            break;
        }
        default: {
            Log::error("method RenderPass::add_attachment() has been called with invalid RenderAttachment::Type type argument (", type, ")");
        }
        }

        return RenderAttachment(type, &attachment_description[id], id);
    }

    // adds a subpass to the RenderPass and returns the index of this new subpass
    uint32_t add_subpass(SubPass& subpass, VkSubpassDescriptionFlags flags, VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) {
        if (subpass.finalized) {
            Log::warning("in method RenderPass::add_subpass(): this subpass has already been added to a parent RenderPass");
            return UINT32_MAX;
        }
        uint32_t id = subpass_description.size();
        subpass_description.resize(id + 1);
        subpass_description[id].flags = flags;
        subpass_description[id].pipelineBindPoint = bind_point;
        
        if (subpass.input_attachment_reference.has_value()) {
            subpass_description[id].inputAttachmentCount = static_cast<uint32_t>(subpass.input_attachment_reference.value().size());
            subpass_description[id].pInputAttachments = subpass.input_attachment_reference.value().empty() ? nullptr : subpass.input_attachment_reference.value().data();
        }
        else {
            subpass_description[id].inputAttachmentCount = 0;
            subpass_description[id].pInputAttachments = nullptr;
        }

        if (subpass.color_attachment_reference.has_value()) {
            subpass_description[id].colorAttachmentCount = static_cast<uint32_t>(subpass.color_attachment_reference.value().size());
            subpass_description[id].pColorAttachments = subpass.color_attachment_reference.value().empty() ? nullptr : subpass.color_attachment_reference.value().data();
        }
        else {
            subpass_description[id].colorAttachmentCount = 0;
            subpass_description[id].pColorAttachments = nullptr;
        }

        if (subpass.resolve_attachment_reference.has_value()) {
            subpass_description[id].pResolveAttachments = &subpass.resolve_attachment_reference.value();
        }
        else {
            subpass_description[id].pResolveAttachments = nullptr;
        }

        if (subpass.depth_attachment_reference.has_value()) {
            subpass_description[id].pDepthStencilAttachment = &subpass.depth_attachment_reference.value();
            depth_stencil_flag = true;
        }
        else {
            subpass_description[id].pDepthStencilAttachment = nullptr;
        }

        if (subpass.preserve_attachment_reference.has_value()) {
            subpass_description[id].preserveAttachmentCount = static_cast<uint32_t>(subpass.preserve_attachment_reference.value().size());
            subpass_description[id].pPreserveAttachments = subpass.preserve_attachment_reference.value().empty() ? nullptr : subpass.preserve_attachment_reference.value().data();
        }
        else {
            subpass_description[id].preserveAttachmentCount = 0;
            subpass_description[id].pPreserveAttachments = nullptr;
        }
        subpass.finalized = true;
        return id;
    }

    VkSubpassDependency add_subpass_dependency(
        uint32_t source,
        uint32_t destination,
        VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VkAccessFlags src_access_mask = 0,
        VkAccessFlags dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    ) {
        uint32_t id = subpass_dependency.size();
        subpass_dependency.resize(id + 1);
        subpass_dependency[id] = {};
        subpass_dependency[id].srcSubpass = source;
        subpass_dependency[id].dstSubpass = destination;
        subpass_dependency[id].srcStageMask = src_stage_mask;
        subpass_dependency[id].srcAccessMask = src_access_mask;
        subpass_dependency[id].dstStageMask = dst_stage_mask;
        subpass_dependency[id].dstAccessMask = dst_access_mask;
        return subpass_dependency[id];
    }

    void finalize() {

        VkRenderPassCreateInfo renderpass_create_info = {};
        renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = static_cast<uint32_t>(attachment_description.size());
        renderpass_create_info.pAttachments = attachment_description.data();
        renderpass_create_info.subpassCount = static_cast<uint32_t>(subpass_description.size());
        renderpass_create_info.pSubpasses = subpass_description.data();
        renderpass_create_info.dependencyCount = static_cast<uint32_t>(subpass_dependency.size());
        renderpass_create_info.pDependencies = subpass_dependency.data();

        if (vkCreateRenderPass(logical, &renderpass_create_info, nullptr, &renderpass) != VK_SUCCESS) {
            Log::error("failed to create render pass!");
        }
    }

    // move constructor
    RenderPass(RenderPass&& other) noexcept {
        move_resources(other);
    }

    // move assignment constructor
    RenderPass& operator=(RenderPass&& other) noexcept {
        if (this != &other) {
            // Release resources held by the current object (if any)
            this->destroy();

            // Move the state from 'other' to 'this'
            move_resources(other);
        }
        return *this;
    }

    // delete copy constructors
    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;

    // getter functions
    VkRenderPass& get() { return renderpass; }
    uint32_t get_multisample_count() const { return multisample_count; }
    uint32_t get_attachment_count() const { return static_cast<uint32_t>(attachment_description.size()); }
    uint32_t get_subpass_count() const { return subpass_description.size(); }
    bool has_depth_stencil() const { return depth_stencil_flag; }
    std::vector<VkAttachmentDescription>& get_attachment_descriptions() { return attachment_description; }

    void destroy() {
        if (renderpass != nullptr) {
            vkDestroyRenderPass(logical, renderpass, nullptr);
            renderpass = nullptr;
        }
    }

    // destructor
    ~RenderPass() {
        destroy();
    }

protected:
    // helper method to move resources for the move constructor and move assignment
    void move_resources(RenderPass& other) {
        logical = std::exchange(other.logical, nullptr);
        renderpass = std::exchange(other.renderpass, nullptr);
        multisample_count = std::move(other.multisample_count);
        attachment_description = std::move(other.attachment_description);
        subpass_description = std::move(other.subpass_description);
    }

    VkRenderPass renderpass = nullptr;
    VkDevice logical = nullptr;
    uint32_t multisample_count = 1;
    bool depth_stencil_flag = false;
    std::vector<VkAttachmentDescription> attachment_description;
    std::vector<VkSubpassDescription> subpass_description;
    std::vector<VkSubpassDependency> subpass_dependency;
};

// Platform Agnostic Surface Class
class Surface {
public:
    // Delete default constructor (Surface always needs an instance and platform info)
    Surface() = delete;

    // Platform-Specific Constructors

    #ifdef VK_USE_PLATFORM_WIN32_KHR
        Surface(Instance& instance, HINSTANCE hinstance, HWND hwnd) {
            instance_handle = instance.get();
            if (instance_handle == nullptr) {
                Log::error("Surface creation failed: Provided VkInstance is NULL.");
            }
            if (!hinstance || !hwnd) {
                Log::error("Surface creation failed: Provided Win32 HINSTANCE or HWND is NULL.");
            }

            VkWin32SurfaceCreateInfoKHR create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.hinstance = hinstance;
            create_info.hwnd = hwnd;

            VkResult result = vkCreateWin32SurfaceKHR(instance_handle, &create_info, nullptr, &surface);

            if (result != VK_SUCCESS) {
                surface = VK_NULL_HANDLE; // Ensure null on failure
                Log::error("Failed to create Win32 surface (VkResult=", result, ")");
            }
            else {
                Log::info("Win32 Vulkan surface created successfully (handle: ", surface, ")");
            }
        }
    #endif

    #ifdef VK_USE_PLATFORM_ANDROID_KHR
        Surface(Instance& instance, ANativeWindow* window) {
            instance_handle = instance.get();
            surface = VK_NULL_HANDLE;
            if (!instance_handle) {
                Log::error("Surface creation failed: Provided VkInstance is NULL.");
            }
            if (!window) {
                Log::error("Surface creation failed: Provided Android ANativeWindow is NULL.");
            }

            VkAndroidSurfaceCreateInfoKHR create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.window = window;

            VkResult result = vkCreateAndroidSurfaceKHR(instance_handle, &create_info, nullptr, &surface);
            if (result != VK_SUCCESS) {
                surface = VK_NULL_HANDLE;
                Log::error("Failed to create Android surface (VkResult=", result, ")");
            }
            else {
                Log::info("Android Vulkan surface created successfully (handle: ", surface, ")");
            }
        }
    #endif

    #ifdef VK_USE_PLATFORM_XCB_KHR
        Surface(Instance& instance, xcb_connection_t* connection, xcb_window_t window)
            : instance_handle(instance.get()), surface(VK_NULL_HANDLE)
        {
            if (!instance_handle) {
                Log::error("Surface creation failed: Provided VkInstance is NULL.");
            }
            if (!connection) { // xcb_window_t can be 0, might be valid depending on context
                Log::error("Surface creation failed: Provided XCB connection is NULL.");
            }

            VkXcbSurfaceCreateInfoKHR create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            create_info.connection = connection;
            create_info.window = window;

            VkResult result = vkCreateXcbSurfaceKHR(instance_handle, &create_info, nullptr, &surface);
            if (result != VK_SUCCESS) {
                surface_ = VK_NULL_HANDLE;
                Log::error("Failed to create XCB surface (VkResult=", result, ")");
            }
            else {
                Log::info("XCB Vulkan surface created successfully (handle: ", surface, ")");
            }
        }
    #endif

    #ifdef VK_USE_PLATFORM_METAL_EXT
        // Using void* to pass CAMetalLayer* to avoid Objective-C type in header
        Surface(Instance& instance, void* caMetalLayer) {
            instance_handle = instance.get();
            surface = VK_NULL_HANDLE;
            if (!instance_handle) {
                Log::error("Surface creation failed: Provided VkInstance is NULL.");
            }
            if (!caMetalLayer) {
                Log::error("Surface creation failed: Provided CAMetalLayer pointer is NULL.");
            }

            VkMetalSurfaceCreateInfoEXT create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
            create_info.pNext = nullptr;
            create_info.flags = 0;
            // User *must* ensure the void* is actually a CAMetalLayer*
            // In Objective-C++, CAMetalLayer* is compatible with `id`, which is compatible with `void*`
            create_info.pLayer = (CAMetalLayer*)caMetalLayer; // Cast needed

            // Requires VK_EXT_metal_surface extension enabled
            VkResult result = vkCreateMetalSurfaceEXT(instance_handle, &create_info, nullptr, &surface);
            if (result != VK_SUCCESS) {
                surface = VK_NULL_HANDLE;
                Log::error("Failed to create Metal surface (VkResult=", result, ")");
            }
            else {
                Log::info("Metal Vulkan surface created successfully (handle: ", surface, ")");
            }
        }
    #endif


    // Move constructor
    Surface(Surface&& other) noexcept {
        instance_handle = std::exchange(other.instance_handle, nullptr);
        surface = std::exchange(other.surface, VK_NULL_HANDLE);
    }

    // Move assignment
    Surface& operator=(Surface&& other) noexcept {
        if (this != &other) {
            destroy(); // Clean up existing resource before move
            instance_handle = std::exchange(other.instance_handle, nullptr);
            surface = std::exchange(other.surface, VK_NULL_HANDLE);
        }
        return *this;
    }

    // Delete copy operations
    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;

    // Destructor
    ~Surface() {
        destroy();
    }

    // Get the underlying VkSurfaceKHR handle
    VkSurfaceKHR get() const {
        return surface;
    }

    // Check if the surface handle is valid
    bool is_valid() const {
        return surface != VK_NULL_HANDLE;
    }

    // Check if a specific queue family on a physical device supports presentation to this surface
    bool get_physical_device_support(Device& device, QueueFamily queue_family) const {
        if (surface == VK_NULL_HANDLE || device.get_physical() == VK_NULL_HANDLE) {
            Log::warning("Attempted to check surface support with null surface or physical device.");
            return VK_FALSE;
        }
        VkBool32 present_support = VK_FALSE;
        VkResult result;
        switch (queue_family) {
            case QueueFamily::COMPUTE:
                result = vkGetPhysicalDeviceSurfaceSupportKHR(device.get_physical(), device.get_compute_queue_family_index(), surface, &present_support);
                break;
            case QueueFamily::GRAPHICS:
                result = vkGetPhysicalDeviceSurfaceSupportKHR(device.get_physical(), device.get_graphics_queue_family_index(), surface, &present_support);
                break;
            case QueueFamily::TRANSFER:
                result = vkGetPhysicalDeviceSurfaceSupportKHR(device.get_physical(), device.get_transfer_queue_family_index(), surface, &present_support);
                break;
            default:
                result = VK_ERROR_UNKNOWN;
        }
        if (result != VK_SUCCESS) {
            Log::warning("Failed to query physical device surface support, i.e. invalid device or unsupported surface extension (VkResult=", result, ")");
        }
        return bool(present_support);
    }

    // Get the capabilities of the surface for a specific physical device
    VkSurfaceCapabilitiesKHR get_capabilities(Device& device) const {
        VkSurfaceCapabilitiesKHR capabilities = {};
        if (surface == VK_NULL_HANDLE || device.get_physical() == VK_NULL_HANDLE) {
            Log::warning("Attempted to get surface capabilities with null surface or physical device.");
            return capabilities; // Return empty struct
        }
        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_physical(), surface, &capabilities);
        if (result != VK_SUCCESS) {
            Log::error("Failed to query physical device surface capabilities (VkResult=", result, ")");
        }
        return capabilities;
    }

    // Get the supported surface formats for a specific physical device
    std::vector<VkSurfaceFormatKHR> get_formats(Device& device) const {
        std::vector<VkSurfaceFormatKHR> formats;
        if (surface == VK_NULL_HANDLE || device.get_physical() == VK_NULL_HANDLE) {
            Log::warning("Attempted to get surface formats with null surface or physical device.");
            return formats; // Return empty vector
        }
        uint32_t formatCount = 0;
        // Query count first
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_physical(), surface, &formatCount, nullptr);
        if (result != VK_SUCCESS || formatCount == 0) {
            Log::warning("Failed to query physical device surface format count or none available (VkResult=", result, ")");
            return formats; // Return empty vector
        }

        formats.resize(formatCount);
        // Query actual formats
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_physical(), surface, &formatCount, formats.data());
        if (result != VK_SUCCESS) {
            Log::error("Failed to query physical device surface formats (VkResult=", result, ")");
        }
        return formats;
    }

    // Get the supported presentation modes for a specific physical device
    std::vector<VkPresentModeKHR> get_present_modes(Device& device) const {
        std::vector<VkPresentModeKHR> presentModes;
        if (surface == VK_NULL_HANDLE || device.get_physical() == VK_NULL_HANDLE) {
            Log::warning("Attempted to get surface present modes with null surface or physical device.");
            return presentModes; // Return empty vector
        }
        uint32_t presentModeCount = 0;
        // Query count first
        VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_physical(), surface, &presentModeCount, nullptr);
        if (result != VK_SUCCESS || presentModeCount == 0) {
            Log::warning("Failed to query physical device surface present mode count or none available (VkResult=", result, ")");
            return presentModes; // Return empty vector
        }

        presentModes.resize(presentModeCount);
        // Query actual modes
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_physical(), surface, &presentModeCount, presentModes.data());
        if (result != VK_SUCCESS) {
            Log::error("Failed to query physical device surface present modes (VkResult=", result, ")");
        }
        return presentModes;
    }

protected:
    // Helper to destroy the surface (called by destructor and move assignment)
    void destroy() {
        if (surface != VK_NULL_HANDLE && instance_handle != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance_handle, surface, nullptr);
            Log::info("[SURFACE DESTROYED] (handle: ", surface, ")");
        }
        surface = VK_NULL_HANDLE;
    }

    VkInstance instance_handle = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};

class SurfaceFormat {
public:
	SurfaceFormat(VkFormat format = VK_FORMAT_R32G32_SFLOAT, VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) : format(format), color_space(color_space) {
		surface_format.format = format;
		surface_format.colorSpace = color_space;
    }
	
    // Move constructor
	SurfaceFormat(SurfaceFormat&& other) noexcept : format(std::exchange(other.format, VK_FORMAT_UNDEFINED)), color_space(std::exchange(other.color_space, VK_COLOR_SPACE_MAX_ENUM_KHR)) {}
	
    // Move assignment
	SurfaceFormat& operator=(SurfaceFormat&& other) noexcept {
		if (this != &other) {
			format = std::exchange(other.format, VK_FORMAT_UNDEFINED);
			color_space = std::exchange(other.color_space, VK_COLOR_SPACE_MAX_ENUM_KHR);
		}
		return *this;
	}
	// Deleted copy constructor and assignment
	SurfaceFormat(const SurfaceFormat&) = delete;
	SurfaceFormat& operator=(const SurfaceFormat&) = delete;

	// Destructor
	~SurfaceFormat() {}

	// Getters
    VkSurfaceFormatKHR get() const { return surface_format; }
    VkFormat get_format() const { return format; }
	VkColorSpaceKHR get_color_space() const { return color_space; }

    // Setters
    void set_format(VkFormat format) { this->format = format; surface_format.format = format; }
    void set_color_space(VkColorSpaceKHR color_space) { this->color_space = color_space; surface_format.colorSpace = color_space; }
private:
	VkFormat format;
	VkColorSpaceKHR color_space;
    VkSurfaceFormatKHR surface_format = {};
};

class Swapchain {
public:
    // constructor
    Swapchain() = delete;
    Swapchain(
        Device& device,
        Surface& surface,
        SurfaceFormat& surface_format,
        RenderPass& renderpass,
        VkImageUsageFlags usage,
        uint32_t extent_width = 1920,
        uint32_t extent_height = 1080,
        uint32_t min_image_count = 3,
        VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D,
        VkPresentModeKHR present_mode_preference = VK_PRESENT_MODE_FIFO_KHR
    ) {
		// store member variables according to constructor arguments
        if (!surface.get_physical_device_support(device, QueueFamily::GRAPHICS)) {
            Log::error("invalid swapchain call: physical device doesn't support graphics queue family present to this surface");
        }
        this->device = &device;
        this->logical = device.get_logical();
        surface_capabilities = surface.get_capabilities(device);
		this->view_type = view_type;
		this->surface_format = &surface_format;
        this->renderpass = &renderpass;
        this->surface = &surface;
        this->usage = usage;

        // chose present mode
        std::vector<VkPresentModeKHR> available_present_modes = surface.get_present_modes(device);
        if (available_present_modes.empty()) {
            Log::error("swapchain creation failed; no suitable present modes available for the surface");
        }
        VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        bool success = false;
        for (const auto& available : available_present_modes) {
            if (available == present_mode_preference) {
                selected_present_mode = present_mode_preference;
                success = true;
            }
        }
        if (!success) {
            Log::warning("in swapchain constructor: preferred present mode not available -> falling back to FIFO as default");
        }

        // adjust image count
        uint32_t image_count = min_image_count;
        if (image_count > surface_capabilities.maxImageCount) {
            image_count = surface_capabilities.maxImageCount;
            Log::warning("in swapchain constructor: min image count exceeds max image count of surface capabilities -> reduced to ", image_count);
        }
        if (image_count < surface_capabilities.minImageCount) {
            image_count = surface_capabilities.minImageCount;
            Log::warning("in swapchain constructor: surface capabilities require min image count of >=", image_count, " -> adjusted accordingly");
        }

        // adjust image extent
        // (note: std::min & std::max can't be used to replace the if statements due to an IntelliSense bug)
        extent.width = extent_width;
        if (extent.width > surface_capabilities.maxImageExtent.width) {
            extent.width = surface_capabilities.maxImageExtent.width;
        }
        else if (extent.width < surface_capabilities.minImageExtent.width) {
            extent.width = surface_capabilities.minImageExtent.width;
        }
        extent.height = extent_height;
        if (extent.height > surface_capabilities.maxImageExtent.height) {
            extent.height = surface_capabilities.maxImageExtent.height;
        }
        else if (extent.height < surface_capabilities.minImageExtent.height) {
            extent.height = surface_capabilities.minImageExtent.height;
        }

        // setup swapchain details
        VkSwapchainCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = surface.get();
        create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.get_format(); // e.g., VK_FORMAT_B8G8R8A8_SRGB
		create_info.imageColorSpace = surface_format.get_color_space(); // e.g., VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1; // Use > 1 for stereoscopic rendering
        create_info.imageUsage = usage; // e.g., VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional for exclusive
        create_info.pQueueFamilyIndices = nullptr; // Optional for exclusive
        create_info.preTransform = surface_capabilities.currentTransform; // e.g. VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.clipped = VK_TRUE; // Allow clipping occluded pixels
        create_info.oldSwapchain = VK_NULL_HANDLE; // Required for recreation, null for initial creation
        create_info.presentMode = selected_present_mode;

        // finalize swapchain
        VkResult result = vkCreateSwapchainKHR(logical, &create_info, nullptr, &swapchain);
        if (result != VK_SUCCESS) {
            Log::error("Failed to create swapchain (VkResult=", result, ")");
        }
        else {
            Log::debug("Swapchain created successfully.");
        }

        // get images
        vkGetSwapchainImagesKHR(logical, swapchain, &num_images, nullptr);
        image.resize(num_images);
        vkGetSwapchainImagesKHR(logical, swapchain, &num_images, image.data());

        // create image views for swapchain images
        color_image_views.resize(num_images);
        for (uint32_t i = 0; i < num_images; i++) {
            VkImageViewCreateInfo view_info = {};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = image[i];
            view_info.viewType = view_type; // e.g. VK_IMAGE_VIEW_TYPE_2D (assuming 2D)
			view_info.format = surface_format.get_format();
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            result = vkCreateImageView(logical, &view_info, nullptr, &color_image_views[i]);
            if (result != VK_SUCCESS) {
                Log::error("Failed to create swapchain image view ", i);
            }
        }
        Log::info("Swapchain created with ", num_images, " images/views.");

    }

    void create_framebuffers(RenderPass& renderpass, const std::vector<ImageView>& attachments_imageviews) {

        framebuffer.resize(num_images);
        uint32_t expected_attachments = renderpass.get_attachment_count();

        for (uint32_t i = 0; i < num_images; i++) {
            std::vector<VkImageView> attachments;
            // add swapchain color view first (assuming it's attachment 0 in the render pass)
            attachments.push_back(color_image_views[i]);
            // add other attachments provided externally (if any), e.g. depth buffer view;
            // the order must match the order in the render pass !!
            for (uint32_t j = 0; j < attachments_imageviews.size(); j++) {
                if (attachments_imageviews[j].get() != nullptr) {
                    attachments.push_back(attachments_imageviews[j].get());
                }
            }
            if (attachments.size() != expected_attachments) {
                Log::error("Framebuffer creation failed: Mismatched attachment count (expected ", expected_attachments, ", got ", attachments.size(), ")");
            }
            VkFramebufferCreateInfo framebuffer_create_info = {};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = renderpass.get();
            framebuffer_create_info.attachmentCount = expected_attachments;
            framebuffer_create_info.pAttachments = attachments.data();
            framebuffer_create_info.width = extent.width;
            framebuffer_create_info.height = extent.height;
            framebuffer_create_info.layers = 1;

            VkResult result = vkCreateFramebuffer(logical, &framebuffer_create_info, nullptr, &framebuffer[i]);
            if (result != VK_SUCCESS) {
                Log::error("Failed to create framebuffer ", i, " (VkResult=", result, ")");
            }
        }

        Log::info("Swapchain framebuffers created successfully.");
    }

    void acquire_next_image(Semaphore image_available_semaphore, const std::optional<Fence>& fence = std::nullopt, uint64_t timeout = UINT64_MAX) {
        VkResult result;
        if (fence.has_value()) {
			result = vkAcquireNextImageKHR(logical, swapchain, timeout, image_available_semaphore.get(), fence.value().get(), &current_image_index);
		}
        else {
            result = vkAcquireNextImageKHR(logical, swapchain, timeout, image_available_semaphore.get(), nullptr, &current_image_index);
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Log::warning("Swapchain out of date during acquire -> recreating");
            this->recreate();
        }
        else if (result == VK_SUBOPTIMAL_KHR) {
            Log::info("Swapchain suboptimal during acquire. Okay to continue, but should be recreated soon.");
        }
        else if (result != VK_SUCCESS) {
            Log::error("Failed to acquire swapchain image (VkResult=", result, ")");
        }
        // else: Success
    }

	// present the rendered image to the graphics queue (method overload without semaphores)
    VkResult present_rendered_image() {
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 0;
        present_info.pWaitSemaphores = nullptr;

        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &current_image_index;
        present_info.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(device->get_graphics_queue(), &present_info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Log::warning("Swapchain out of date during present -> recreating");
            this->recreate();
        }
        else if (result == VK_SUBOPTIMAL_KHR) {
            Log::info("Swapchain suboptimal during present. Okay to continue, but should be recreated soon");
        }
        else if (result != VK_SUCCESS) {
            Log::error("Failed to present swapchain image (VkResult=", result, ")");
        }
        // else: Success

        return result;
    }

    // present the rendered image to the graphics queue (method overload with single semaphore)
    VkResult present_rendered_image(const Semaphore& wait_semaphore) {
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &wait_semaphore.get(); // Semaphore to wait on (e.g., rendering finished)

        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
		present_info.pImageIndices = &current_image_index;
        present_info.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(device->get_graphics_queue(), &present_info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Log::warning("Swapchain out of date during present -> recreating");
            this->recreate();
        }
        else if (result == VK_SUBOPTIMAL_KHR) {
            Log::info("Swapchain suboptimal during present. Okay to continue, but should be recreated soon");
        }
        else if (result != VK_SUCCESS) {
            Log::error("Failed to present swapchain image (VkResult=", result, ")");
        }
        // else: Success

        return result;
    }

	// present the rendered image to the graphics queue (method overload with multiple semaphores)
    VkResult present_rendered_image(const std::vector<Semaphore>& wait_semaphores) {
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
		std::vector<VkSemaphore> wait_semaphore_handles(wait_semaphores.size());
		for (Semaphore semaphore : wait_semaphores) {
			wait_semaphore_handles.push_back(semaphore.get());
		}
        present_info.pWaitSemaphores = wait_semaphore_handles.data(); // Semaphore to wait on (e.g., rendering finished)
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &current_image_index;
        present_info.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(device->get_graphics_queue(), &present_info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Log::warning("Swapchain out of date during present -> recreating");
            this->recreate();
        }
        else if (result == VK_SUBOPTIMAL_KHR) {
            Log::info("Swapchain suboptimal during present. Okay to continue, but should be recreated soon");
        }
        else if (result != VK_SUCCESS) {
            Log::error("Failed to present swapchain image (VkResult=", result, ")");
        }
        // else: Success

        return result;
    }

    void recreate() {
		destroy();
		// Recreate swapchain with the same parameters
		Swapchain new_swapchain(
			*device,
			*surface,
			*surface_format,
			*renderpass,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			extent.width,
			extent.height,
			num_images,
			view_type
		);
		swapchain = new_swapchain.get();
		image = new_swapchain.image;
		color_image_views = new_swapchain.color_image_views;
		framebuffer = new_swapchain.framebuffer;
		framebuffer_image_views = new_swapchain.framebuffer_image_views;
    }

    // getters

    uint32_t get_width() const { return extent.width; }

    uint32_t get_height() const { return extent.height; }
	
    VkExtent2D get_extent() const { return extent; }
	
    VkSwapchainKHR get() const { return swapchain; }
	
    VkImageView get_color_image_view(uint32_t index) const {
		if (index < color_image_views.size()) {
			return color_image_views[index];
		}
		else {
			Log::error("Invalid index for color image view: ", index);
			return VK_NULL_HANDLE;
		}
	}

	VkImageView get_framebuffer_image_view(uint32_t index) const {
		if (index < framebuffer_image_views.size()) {
			return framebuffer_image_views[index];
		}
		else {
			Log::error("Invalid index for framebuffer image view: ", index);
			return VK_NULL_HANDLE;
		}
	}

	VkFramebuffer get_framebuffer(uint32_t index) const {
		if (index < framebuffer.size()) {
			return framebuffer[index];
		}
		else {
			Log::error("Invalid index for framebuffer: ", index);
			return VK_NULL_HANDLE;
		}
	}

	VkImage get_image(uint32_t index) const {
		if (index < image.size()) {
			return image[index];
		}
		else {
			Log::error("Invalid index for image: ", index);
			return VK_NULL_HANDLE;
		}
	}

	VkImage get_current_image() const { return image[current_image_index]; }
	
    uint32_t get_current_image_index() const { return current_image_index; }

	uint32_t get_image_count() const { return num_images; }

    // destructor
	~Swapchain() {
		destroy();
	}

    void destroy() {
		if (swapchain != nullptr) {
			vkDestroySwapchainKHR(logical, swapchain, nullptr);
			swapchain = nullptr;
		}
		for (uint32_t i = 0; i < num_images; i++) {
			vkDestroyImageView(logical, color_image_views[i], nullptr);
			vkDestroyFramebuffer(logical, framebuffer[i], nullptr);
		}
		color_image_views.clear();
		framebuffer_image_views.clear();
		framebuffer.clear();
		image.clear();
		num_images = 0;
		Log::info("Swapchain destroyed.");
    }

protected:
    uint32_t num_images = 0;
    std::vector<VkImage> image;
    std::vector<VkImageView> color_image_views;
    std::vector<VkImageView> framebuffer_image_views;
    std::vector<VkFramebuffer> framebuffer;
    VkSwapchainKHR swapchain = nullptr;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkExtent2D extent = { 1920, 1080 };
    SurfaceFormat* surface_format;
    VkImageUsageFlags usage;
    VkColorSpaceKHR color_space;
    Device* device;
    RenderPass* renderpass;
	Surface* surface;
    VkDevice logical = nullptr;
    uint32_t current_image_index = 0;
    VkImageViewType view_type;
};

class CommandPool {
public:
    // constructor
    CommandPool() = delete;
    CommandPool(Device& device, QueueFamily usage) {
        this->logical = device.get_logical();

        // setup command pool
        VkCommandPoolCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (usage == QueueFamily::GRAPHICS) {
            create_info.queueFamilyIndex = device.get_graphics_queue_family_index();
        }
        else if (usage == QueueFamily::COMPUTE) {
            create_info.queueFamilyIndex = device.get_compute_queue_family_index();
        }
        else if (usage == QueueFamily::TRANSFER) {
            create_info.queueFamilyIndex = device.get_transfer_queue_family_index();
        }
        else {
            Log::error("in CommandPool constructor: invalid QueueFamily argument!");
        }
        VkResult result = vkCreateCommandPool(logical, &create_info, nullptr, &pool);
        if (result == VK_SUCCESS) {
            Log::info("command pool created (handle: ", pool, ")");
        }
        else {
            Log::error("failed to create command pool (VkResult=", result, ")");
        }
    }

    void destroy() {
        if (pool != nullptr) {
            // destroy command pool
            Log::info("CommandPool destructor: destroying command pool with handle ", pool);
            vkDestroyCommandPool(logical, pool, nullptr);
            pool = nullptr;
        }
    }

    ~CommandPool() {
        destroy();
    }

    void trim() const {
        vkTrimCommandPool(logical, pool, NULL);
    }

    VkResult reset(VkCommandPoolResetFlags flags = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) const {
        return vkResetCommandPool(logical, pool, flags);
    }

    VkCommandPool& get() { return pool; }
private:
    VkCommandPool pool = nullptr;
    VkDevice logical = nullptr;
};

class VertexDescriptions {
public:
    VertexDescriptions() {}

    ~VertexDescriptions() {}

    void add_attribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset = 0) {
        uint32_t id = attribute_descriptions.size();
        attribute_descriptions.resize(id + 1);
        attribute_descriptions[id].binding = binding;
        attribute_descriptions[id].location = location;
        attribute_descriptions[id].format = format;
        attribute_descriptions[id].offset = offset;
    }

    // adds a binding to the vertex descriptions and returns the index of the new binding
    uint32_t add_binding(uint32_t stride, VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX) {
        uint32_t id = binding_descriptions.size();
        binding_descriptions.resize(id + 1);
        binding_descriptions[id].binding = id;
        binding_descriptions[id].inputRate = input_rate;
        binding_descriptions[id].stride = stride;
        return id;
    }

    // getters
    const std::vector<VkVertexInputAttributeDescription>& get_attribute_descriptions() const { return attribute_descriptions; }
    const std::vector<VkVertexInputBindingDescription>& get_input_bindings() const { return binding_descriptions; }

protected:
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {};
    std::vector<VkVertexInputBindingDescription> binding_descriptions = {};
};

class ShaderModule {
public:
    // constructor
    ShaderModule() = delete;

	// constructor with binary data
    ShaderModule(Device& device, const unsigned char* binary, const size_t& size_bytes) {
        this->logical = device.get_logical();
        VkShaderModuleCreateInfo shader_module_create_info = {};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize = size_bytes;
        shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(binary);

        // free old resource first in case a previous module exists
        if (module != nullptr) {
            Log::info("destroying previous shader module");
            vkDestroyShaderModule(logical, module, nullptr);
        }

        // allocate new module
        VkResult result = vkCreateShaderModule(logical, &shader_module_create_info, nullptr, &module);
        if (result == VK_SUCCESS) {
            Log::debug("new shader module successfully created (handle: ", module, ")");
        }
        else {
            Log::error("failed to create shader module (VkResult = ", result, ")");
        }
    }

	// constructor with folder & filename as string literals
    ShaderModule(Device& device, const char* foldername, const char* filename) {
        ShaderModule(device, std::string(foldername), std::string(filename));
    }

	// constructor with folder & filename as std::string
    ShaderModule(Device& device, const std::string& foldername, const std::string& filename) {
        this->logical = device.get_logical();
        
        std::string file_path;
        if (foldername.back() != '/') {
            file_path = foldername + '/' + filename;
        }
        else {
			file_path = foldername + filename;
        }
        std::string file_path = foldername + filename;
        long file_size = 0;
        FILE* file = fopen(file_path.c_str(), "rb");
        if (!file) {
            Log::error("shader file not found: ", filename);
        }
        else {
            Log::debug("reading shader file: ", file_path.c_str());
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            uint8_t* buffer = new uint8_t[file_size];
            fread(buffer, 1, file_size, file);

            VkShaderModuleCreateInfo shader_module_create_info = {};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = file_size;
            shader_module_create_info.pCode = reinterpret_cast<uint32_t*>(buffer);

            // free old resource first in case a previous module exists
            if (module != nullptr) {
                Log::info("destroying previous shader module");
                vkDestroyShaderModule(logical, module, nullptr);
            }

            // allocate new module
            VkResult result = vkCreateShaderModule(logical, &shader_module_create_info, nullptr, &module);
            if (result == VK_SUCCESS) {
                Log::debug("new shader module successfully created (handle: ", module, ")");
            }
            else {
                Log::error("failed to create shader module (VkResult = ", result, ")");
            }
            delete[] buffer;
            fclose(file);
        }
    }

    const VkShaderModule& get() const { return module; }

    // destructor
    ~ShaderModule() {
        if (module != nullptr) {
            vkDestroyShaderModule(logical, module, nullptr);
        }
    }
private:
    VkShaderModule module = nullptr;
    VkDevice logical = nullptr;
};

class PushConstants {
public:
    PushConstants() {
        range.size = 0;
        range.offset = 0;
        range.stageFlags = VK_SHADER_STAGE_ALL;
        data = new uint32_t[min_capacity / 4];
    }

    template<typename T>
    PushConstants(std::vector<T>& values) {
        PushConstants();
        add_values(values);
    }

    ~PushConstants() {
        if (data != nullptr) {
            delete[] data;
        }
    }

    // add a new value to the push constants range;
    // the size of the data type MUST be a multiple of 4;
    // returns the total size of the push constants range
    template<typename T>
    uint32_t add_values(T value) {

        // update range size
        if (sizeof(T) % 4) {
            Log::warning("in method PushConstants::add_value(T value): sizeof(T) must be a multiple of 4");
        }
        size_t old_size = range.size;
        range.size += 4 * ceil(0.25 * sizeof(T));

        // allocate memory (if existing capacity is insufficient) and copy previous data to new allocation
        if (capacity < range.size) {
            capacity = size_t(std::max(float_t(min_capacity), float_t(4 * ceil(0.25 * range.size * (1.0f + reserve)))));
            uint32_t* new_allocation = new uint32_t[capacity / 4];
            memcpy(new_allocation, data, old_size);
            delete[] data;
            data = new_allocation;
        }

        // copy value to the end of the data array
        memcpy(data + old_size / 4, &value, sizeof(T));
        return range.size;
    }

    template<typename T>
    uint32_t add_values(std::initializer_list<T> values) {
        for (T i : values) {
            this->add_values(i);
        }
        return range.size;
    }

    template<typename T>
    uint32_t add_values(std::vector<T>& new_data) {
        for (T i : new_data) {
            this->add_values(i);
        }
        return range.size;
    }

    uint32_t* get_data() { return data; }

    VkPushConstantRange& get_range() { return range; }

protected:
    static constexpr float_t reserve = 0.5;
    static constexpr size_t min_capacity = 16; // min capacity in bytes (should be a multiple of 4)
    uint32_t* data = nullptr;
    size_t capacity = min_capacity;
    VkPushConstantRange range;
};

template<typename T>
class Buffer {
public:
    Buffer() = delete; // non-parametric buffer construction not allowed

    Buffer(Device& device, BufferUsage usage, uint32_t elements, VkMemoryPropertyFlags memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        this->logical = device.get_logical();
        this->physical = device.get_physical();
        this->memory_property_flags = memory_property_flags;
        this->elements = elements;
        this->size_bytes = this->elements * sizeof(T);
        bool is_device_local_only = (memory_property_flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && !(memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        bool is_host_visible = memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        // translate BufferUsage enum argument
        VkBufferUsageFlags vk_buffer_usage;
        switch (usage) {
            case BufferUsage::VERTEX:   vk_buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            case BufferUsage::INDEX:    vk_buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            case BufferUsage::STORAGE:  vk_buffer_usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            case BufferUsage::UNIFORM:  vk_buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            case BufferUsage::TRANSFER: vk_buffer_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            default: Log::error("in method Buffer::Buffer(): invalid BufferUsage argument: ", usage);
        }

        // create buffer
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = size_bytes;
        buffer_create_info.usage = vk_buffer_usage;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assuming exclusive access for simplicity

        VkResult result = vkCreateBuffer(logical, &buffer_create_info, nullptr, &buffer);
        if (result == VK_SUCCESS) {
            Log::info("data buffer successfully created (handle: ", buffer, ")");
        }
        else {
            Log::error("failed to create data buffer, VkResult=", result);
        }

        // get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(logical, buffer, &memory_requirements);

        // find suitable memory type index
        uint32_t type_index = UINT32_MAX;
               
        VkPhysicalDeviceMemoryProperties device_memory_properties = device.get_memory_properties();

        Log::info("in Buffer::Buffer() constructor: searching for buffer memory types (requested: ", memory_property_flags, ")");
        for (uint32_t i = 0; i < device_memory_properties.memoryTypeCount; i++) {
            Log::debug("memory type ", i, ": ", device_memory_properties.memoryTypes[i].propertyFlags);
            if ((memory_requirements.memoryTypeBits & (1 << i)) && (device_memory_properties.memoryTypes[i].propertyFlags & memory_property_flags) == memory_properties) {
                type_index = i;
                Log::info("[SUCCESS]");
            }
        }
        if (type_index == UINT32_MAX) {
            Log::warning("in Buffer::Buffer() constructor:: no suitable memory type found");
        }

        // allocate memory
        VkMemoryAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = type_index;
        result = vkAllocateMemory(logical, &allocate_info, nullptr, &memory);        
        if (result != VK_SUCCESS) {
            Log::error("in Buffer::Buffer() constructor: failed to allocate buffer memory, VkResult=", result);
        }

        // bind memory to buffer
        result = vkBindBufferMemory(logical, buffer, memory, 0);
        if (result != VK_SUCCESS) {
            Log::error("in Buffer::Buffer() constructor: failed to bind buffer memory, VkResult=", result);
        }
    }
    

    // copy & copy assignment constructors (deleted)
    Buffer(const Buffer<T>& other) = delete;
    Buffer& operator=(const Buffer<T>& other) = delete;
      
    // move constructor
    Buffer(Buffer<T>&& other) noexcept
        : buffer(other.buffer),
        memory(other.memory),
        elements(other.elements),
        logical(other.logical),
        physical(other.physical),
        memory_flags(other.memory_flags),
        size_bytes(other.size_bytes),
        is_device_local_only(other.is_device_local_only),
        is_host_visible(other.is_host_visible),
        memory_property_flags(other.memory_property_flags{
        // Invalidate the source object ('other') so its destructor doesn't release the resources
        other.buffer = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        other.elements = 0;
        other.size_bytes = 0;

        if (buffer != VK_NULL_HANDLE) {
            Log::info("Buffer moved (new owner), handle: ", buffer);
        }
            }

            // move assignment
            Buffer& operator=(Buffer<T>&& other) noexcept {
        if (this != &other) { // Prevent self-assignment
            // 1. Release existing resources owned by 'this' object
            cleanup(); // Call a helper or duplicate destructor logic

            // 2. Transfer ownership of resources from 'other' to 'this'
            buffer = other.buffer;
            memory = other.memory;
            elements = other.elements;
            logical = other.logical;       // Transfer device handles
            physical = other.physical;
            size_bytes = other.size_bytes;
            is_device_local_only = other.is_device_local_only;
            is_host_visible = other.is_host_visible;
            memory_property_flags = other.memory_property_flags;

            // 3. Invalidate the source object ('other')
            other.buffer = VK_NULL_HANDLE;
            other.memory = VK_NULL_HANDLE;
            other.elements = 0;
            other.size_bytes = 0;

            if (buffer != VK_NULL_HANDLE) {
                Log::info("Buffer move assigned (new owner), handle: ", buffer);
            }
        }
        return *this;
    }

    // copy data elements from a std::vector to a host visible buffer
    // (set copied elements to 0 to copy all)
    void write(const std::vector<T>& source_vector, uint32_t copied_elements = 0, uint32_t source_offset_elements = 0, uint32_t target_offset_elements = 0) {
        if (!is_host_visible) {
            Log::error("Buffer::write() called on non-host-visible buffer");
        }
        uint32_t vector_elements = source_vector.size();
        size_t vector_size_bytes;
        if (copied_elements == 0) {
            vector_size_bytes = (vector_elements - source_offset_elements) * sizeof(T);
        }
        else {
            vector_size_bytes = copied_elements * sizeof(T);
        }
        VkDeviceSize target_offset_bytes = target_offset_elements * sizeof(T);
        uint64_t source_offset_bytes = source_offset_elements * sizeof(T);
        if (target_offset_bytes + vector_size_bytes > this->size_bytes) {
            Log::warning("Buffer::write attempting to write past buffer bounds. Clipping copy region size to fit.");
            vector_size_bytes = (this->size_bytes > target_offset_bytes) ? this->size_bytes - target_offset_bytes : 0;
        }
        if (vector_size_bytes == 0) {
            Log::debug("in Buffer<T>::write(): requested copy region has size ", vector_size_bytes, " bytes, i.e.nothing to copy");
            return;
        }
        void* data;
        vkMapMemory(logical, memory, target_offset_bytes, vector_size_bytes, VkMemoryMapFlags(0), &data);
        memcpy(data, source_vector.data() + source_offset_bytes, vector_size_bytes);
        vkUnmapMemory(logical, memory);
    }

    // copy data elements from a std::array to a host visible buffer
    void write(const T& source_array, uint32_t copied_elements, uint32_t source_offset_elements = 0, uint32_t target_offset_elements = 0) {
        if (!is_host_visible) {
            Log::error("Buffer::write() called on non-host-visible buffer");
        }
        size_t array_size_bytes = copied_elements * sizeof(T);
        VkDeviceSize target_offset_bytes = target_offset_elements * sizeof(T);
        uint64_t source_offset_bytes = source_offset_elements * sizeof(T);
        if (target_offset_bytes + array_size_bytes > this->size_bytes) {
            Log::warning("Buffer::write attempting to write past buffer bounds. Clipping copy region size to fit.");
            array_size_bytes = (this->size_bytes > target_offset_bytes) ? this->size_bytes - target_offset_bytes : 0;
        }
        if (array_size_bytes <= 0) {
            Log::debug("in Buffer<T>::write(): requested copy region has size ", array_size_byte, " bytes, i.e.nothing to copy");
            return;
        }
        void* data;
        vkMapMemory(logical, memory, target_offset_bytes, array_size_bytes, VkMemoryMapFlags(0), &data);
        memcpy(data, &source_array + source_offset_bytes, array_size_bytes);
        vkUnmapMemory(logical, memory);
    }

    // copy data elements from one host visible buffer to another
    // (set copied elements to 0 to copy all)
    void write(const Buffer<T>& sourcebuffer, uint32_t copied_elements=0, uint32_t source_offset_elements=0, uint32_t target_offset_elements=0) {
        if (!is_host_visible) {
            Log::error("Buffer::write() called on non-host-visible buffer");
        }
        size_t source_size_bytes;
        if (copied_elements == 0) {
            source_size_bytes = (sourcebuffer.get_elements() - source_offset_elements) * sizeof(T);
        }
        else {
            source_size_bytes = copied_elements * sizeof(T);
        }
        VkDeviceSize target_offset_bytes = target_offset_elements * sizeof(T);
        VkDeviceSize source_offset_bytes = source_offset_elements * sizeof(T);
        if (target_offset_bytes + source_size_bytes > this->size_bytes) {
            Log::warning("Buffer::write() attempting to write past buffer bounds. Clipping copy region size to fit.");
            source_size_bytes = (this->size_bytes > target_offset_bytes) ? this->size_bytes - target_offset_bytes : 0;
        }
        if (source_size_bytes == 0) {
            Log::debug("in Buffer<T>::write(): requested copy region has size 0, i.e. nothing to copy");
            return;
        }
        void* source = nullptr;
        void* target = nullptr;
        vkMapMemory(logical, sourcebuffer.memory, source_offset_bytes, source_size_bytes, VkMemoryMapFlags(0), &source);
        vkMapMemory(logical, this->memory, target_offset_bytes, source_size_bytes, VkMemoryMapFlags(0), &target);
        memcpy(target, source, source_size_bytes);
        vkUnmapMemory(logical, this->memory);
        vkUnmapMemory(logical, sourcebuffer.memory);
    }

    // returns a continous data sequence from a host visible data buffer as a std::vector<T>
    // (set read_elements to 0 to read all)
    std::vector<T> read(uint32_t read_elements=0, uint32_t source_offset_elements=0) {
        if (!is_host_visible) {
            Log::error("Buffer<T>::read() called on non-host-visible buffer");
        }
        uint32_t source_elements;
        if (read_elements = 0) {
            source_elements = this->elements - source_offset_elements;
            
        }
        else {
            source_elements = read_elements;
        }
        if (source_offset_elements + source_elements > this->elements) {
            Log::warning("Buffer::read() attempting to read past buffer bounds). Clipping read region size to fit.");
            source_elements = (this->elements > source_offset_elements) ? this->elements - source_offset_elements : 0;
        }
        VkDeviceSize source_size_bytes = source_elements * sizeof(T);
        VkDeviceSize source_offset_bytes = source_offset_elements * sizeof(T);
        std::vector<T> result(source_elements);
        if (source_size_bytes == 0) {
            Log::debug("in Buffer<T>::read(): requested region has size 0; returning an empty vector");
            return result;
        }       
        void* data;
        vkMapMemory(logical, memory, source_offset_bytes, source_size_bytes, VkMemoryMapFlags(0), &data);
        memcpy(result.data(), data, source_size_bytes);
        vkUnmapMemory(logical, memory);
        return result;
    }

    // returns a single element from a host visible data buffer
    T get(uint32_t element_index) const {
        if (!is_host_visible) {
            Log::error("Buffer::get(uint32_t element_index) called on non-host-visible buffer");
        }
        if (element_index >= this->elements) {
            Log::error("in method Buffer::get(): element index ", element_index, " is out of bounds (allowed indices: 0-", this->elements - 1, ")");
        }
        void* data = nullptr;
        T element;
        VkResult result = vkMapMemory(logical, memory, element_index * sizeof(T), sizeof(T), VkMemoryMapFlags(0), &data);
        if (result != VK_SUCCESS) {
            Log::error("in method Buffer<T>::get(uint32_t element_index): failed to map buffer memory (VkResult = ", result, ")");
        }
        memcpy(&element, data, sizeof(T));
        vkUnmapMemory(logical, memory);
        return element;
    }

    // assigns a single element of a host visible data buffer
    void set(uint32_t element_index, T value) {
        if (!is_host_visible()) {
            Log::error("method Buffer<T>::set() called on non-host-visible buffer.");
        }
        if (element_index >= this->elements) {
            Log::error("in method Buffer::set(): element index ", element_index, " is out of bounds (allowed indices: 0-", this->elements - 1, ")");
        }
        void* data = nullptr;
        VkResult result = vkMapMemory(logical, memory, element_index * sizeof(T), sizeof(T), VkMemoryMapFlags(0), &data);
        if (result != VK_SUCCESS) {
            Log::error("in method Buffer<T>::set(uint32_t element_index, T value): failed to map buffer memory (VkResult = ", result, ")");
        }
        memcpy(data, &value, sizeof(T));
        vkUnmapMemory(logical, memory);
    }

    // assigns the same value to continous sequence of buffer elements
    // (set write_elements to 0 to assign all)
    void set_all(T value, uint32_t offset_elements=0, uint32_t write_elements=0) {
        if (!is_host_visible()) {
            Log::error("method Buffer<T>::set_all() called on non-host-visible buffer.");
        }
        size_t element_size = sizeof(T);
        if (offset_elements >= this->elements) {
            Log::error("in method Buffer::set_all(): invalid offset argument; value is ", offset_elements, " but the buffer only has ",
                this->elements, " elements, i.e. the max allowed offset argument is ", this->elements - 1);
        }
        if (write_elements > this->elements - offset_elements) {
            Log::error("in method Buffer::set_all(): invalid write_elements argument; value is ", write_elements, " for a buffer of ",
                this->elements, " elements, i.e. only ", this->elements - offset_elements, " elements can be written beyond the desired offset of ", offset_elements, " elements)");
        }
        VkDeviceSize offset_bytes = offset_elements * element_size;
        VkDeviceSize write_bytes = write_elements == 0 ? (this->elements - offset_elements) * element_size : write_elements * element_size;
        void* data;
        vkMapMemory(logical, memory, offset_bytes, write_bytes, VkMemoryMapFlags(0), &data);
        for (size_t offset = 0; offset < write_bytes; offset += element_size) {
            memcpy((T*)data + offset, &value, element_size);
        }
        vkUnmapMemory(logical, memory);
    }

    // getters
    uint32_t get_elements() const { return this->elements; }
    uint64_t get_size_bytes() const { return size_bytes; }
    VkDeviceMemory get_memory() const { return memory; }
    VkBuffer get() const { return buffer; }
    VkMemoryPropertyFlags get_memory_flags() const { return memory_flags; }

    // destructor
    ~Buffer() {
        if (buffer != VK_NULL_HANDLE) {
            Log::info("executing Buffer destructor (buffer handle: ", buffer, ")");
            vkFreeMemory(logical, memory, nullptr);
            memory = VK_NULL_HANDLE;
            vkDestroyBuffer(logical, buffer, nullptr);
            buffer = VK_NULL_HANDLE;
        }
    }

protected:
    VkBuffer buffer = nullptr;
    VkDeviceMemory memory = nullptr;
    uint32_t elements = 0;
    VkDevice logical = nullptr;
    VkPhysicalDevice physical = nullptr;
    VkMemoryPropertyFlags memory_flags = 0;
    uint64_t size_bytes = 0;
};

// TODO: make constructor more configurable
class Sampler {
public:
	Sampler() = delete;
	Sampler(Device& device) {
		this->logical = device.get_logical();
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.pNext = NULL;
		sampler_create_info.magFilter = VK_FILTER_LINEAR;
		sampler_create_info.minFilter = VK_FILTER_LINEAR;
		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.anisotropyEnable = VK_TRUE;
		sampler_create_info.maxAnisotropy = 16.0f;
		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_create_info.unnormalizedCoordinates = VK_FALSE;
		sampler_create_info.compareEnable = VK_FALSE;
		sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		vkCreateSampler(logical, &sampler_create_info, nullptr, &sampler);
	}
	~Sampler() {
		if (sampler != nullptr) {
			vkDestroySampler(logical, sampler, nullptr);
			Log::info("destroyed image sampler (handle: ", sampler, ")");
			sampler = nullptr;
		}
	}
	const VkSampler& get() const { return sampler; }
protected:
	VkSampler sampler = nullptr;
	VkDevice logical = nullptr;
	VkSamplerCreateInfo sampler_create_info = {};
};

class DescriptorPool {
    friend class DescriptorSet;
public:
    DescriptorPool() = delete;
    DescriptorPool(Device& device, uint32_t max_sets = 20, std::vector<VkDescriptorPoolSize> pool_sizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20}}) {
        this->logical = device.get_logical();
        this->max_sets = max_sets;

        // setup create info
        VkDescriptorPoolCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        create_info.maxSets = max_sets;
        create_info.pPoolSizes = pool_sizes.data();
        create_info.poolSizeCount = pool_sizes.size();
        create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; //  VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        create_info.pNext = NULL;

        // Create the descriptor pool
        VkResult result = vkCreateDescriptorPool(logical, &create_info, nullptr, &pool);
        if (result == VK_SUCCESS) {
            Log::debug("successfully created descriptor pool (handle: ", pool, ")");
        }
        else {
            Log::error("failed to create descriptor pool (VkResult =  ", result, ")");
        }
    }

    ~DescriptorPool() {
        if (pool != nullptr) {
            remove_all_sets();
            vkDestroyDescriptorPool(logical, pool, nullptr);
        }
    }

    VkDescriptorPool& get() {
        return pool;
    }

    std::vector<VkDescriptorSet>& get_sets() { return sets; }

    uint32_t get_max_sets() const { return max_sets; }

    uint32_t get_current_sets_count() const{
        return uint32_t(sets.size());
    }

    // removes a single descriptor set from the pool;
    // returns the number of remaining descriptor sets allocated to the pool
    uint32_t remove_set(uint32_t set_index) {
        if (sets[set_index] != nullptr) {
            VkResult result = vkFreeDescriptorSets(logical, pool, 1, &sets[set_index]);
            if (result == VK_SUCCESS) {
                Log::debug("memory allocation freed for descriptor set at index ", set_index);
            }
            else {
                Log::warning("failed to free descriptor set at index ", set_index, " (VkResult = ", result, ")");
            }
        }
        sets.erase(sets.begin() + set_index);
        return uint32_t(sets.size());
    }

    void remove_all_sets() {
        if (sets.empty()) { return; }
        VkResult result = vkFreeDescriptorSets(logical, pool, sets.size(), sets.data());
        if (result == VK_SUCCESS) {
            Log::debug("all descriptor sets removed from pool, memory allocation freed");
        }
        else {
            Log::warning("failed to remove descriptor sets from pool (VkResult = ", result, ")");
        }
        sets.clear();
    }

    uint32_t allocate_set(DescriptorSet& descriptor_set) {
        if (sets.size() >= max_sets) {
            Log::error("in method DescriptorPool::allocate_set(): max number of sets for this pool is ", max_sets, " (as defined by the pool constructor) and has been reached; no more descriptor sets can be added");
        }
        if (!descriptor_set.layout_finalized) {descriptor_set.finalize_layout();}
        VkDescriptorSetAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocate_info.descriptorPool = pool;
        allocate_info.descriptorSetCount = 1;
        allocate_info.pSetLayouts = &descriptor_set.layout;
        VkResult result = vkAllocateDescriptorSets(logical, &allocate_info, &descriptor_set.set);
        if (result != VK_SUCCESS) {
            Log::error("failed to allocate descriptor set (VkResult ", result, ")");
        }
        uint32_t index = sets.size();
        Log::debug("adding new descriptor set (set index = ", index, ") to descriptor pool (pool handle: ", pool, ")");
        sets.push_back(descriptor_set.set);
        return index;
    }

private:
    VkDescriptorPool pool = nullptr;
    VkDevice logical = nullptr;
    std::vector<VkDescriptorSet> sets;
    uint32_t max_sets = 0;
};

// DescriptorSets hold binding information for shader resources
class DescriptorSet {
    friend class DescriptorPool;
public:
    DescriptorSet() = delete;

    DescriptorSet(Device& device) {
        this->logical = device.get_logical();
    }

	// finalizes the descriptor set layout and creates the descriptor set
    void finalize_layout() {
        layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.pNext = NULL;
        layout_create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        layout_create_info.pBindings = layout_bindings.data();
        layout_create_info.bindingCount = layout_bindings.size();
        VkResult result = vkCreateDescriptorSetLayout(logical, &layout_create_info, nullptr, &layout);
        if (result == VK_SUCCESS) {
            Log::info("descriptor set layout created (", layout_bindings.size(), " bindings, layout handle : ", layout, ")");
        }
        else {
            Log::error("in method DescriptorPool::allocate_set(): failed to finalize descriptor set layout (VkResult ", result, ")");
        }
        layout_finalized = true;
    }

    // binds a buffer to the next available binding index and returns this index
    template<typename T>
    uint32_t bind_buffer(Buffer<T>& buffer, DescriptorType type, VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL) {
        if (layout_finalized) {
            Log::error("in method DescriptorSet::bind_buffer(): the descriptor set layout has already been finalized, i.e. no new buffers can be added");
        }
        uint32_t binding_index = layout_bindings.size();
        layout_bindings.resize(binding_index + 1);
        layout_create_info.bindingCount = binding_index + 1;

        layout_bindings[binding_index] = {};
        layout_bindings[binding_index].binding = binding_index;
        layout_bindings[binding_index].descriptorType = get_descriptor_type(type);
        layout_bindings[binding_index].descriptorCount = 1;
        layout_bindings[binding_index].stageFlags = shader_stage_flags;
        layout_bindings[binding_index].pImmutableSamplers = nullptr;

        Log::debug("binding buffer ", buffer.get(), " to descriptor set (handle: ", set, ") at binding index ", binding_index);
        return binding_index;
    }

	// replaces the buffer at the specified binding index with a new one
    template<typename T>
    void replace_buffer(uint32_t target_binding_index, Buffer<T>& buffer, DescriptorType type) {
        if (target_binding_index >= layout_bindings.size()) {
            Log::warning("in method DescriptorSet::replace_buffer(): argument for the target binding index is invalid; value is ", target_binding_index, " but the highest available index is ", layout_bindings.size() - 1);
        }
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.get();
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.pNext = NULL;
        descriptor_write.dstSet = set;
        descriptor_write.dstBinding = target_binding_index;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = get_descriptor_type(type);
        descriptor_write.pImageInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;
        descriptor_write.pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(logical, 1, &descriptor_write, 0, nullptr);
    }

	// binds an image view to the next available binding index and returns this index
    uint32_t bind_image(const ImageView& image_view, DescriptorType type, VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL, const Sampler& sampler) {
        if (layout_finalized) {
            Log::error("in method DescriptorSet::bind_image(): the descriptor set layout has already been finalized, i.e. no new images can be added");
        }
        uint32_t binding_index = static_cast<uint32_t>(layout_bindings.size());
        layout_bindings.resize(binding_index + 1);
        layout_create_info.bindingCount = binding_index + 1;

        layout_bindings[binding_index] = {};
        layout_bindings[binding_index].binding = binding_index;
        layout_bindings[binding_index].descriptorType = get_descriptor_type(type);
        layout_bindings[binding_index].descriptorCount = 1;
        layout_bindings[binding_index].stageFlags = shader_stage_flags;
        layout_bindings[binding_index].pImmutableSamplers = nullptr;

        Log::debug("binding image view ", image_view.get(), " to descriptor set (handle: ", set, ") at binding index ", binding_index);

        // Store the image view and sampler for updating the descriptor set later
		ImageBindingInfo image_binding; // = custom struct, not part of the Vulkan API
        image_binding.binding_index = binding_index;
        image_binding.image_view = image_view.get();
        image_binding.sampler = sampler.get();
        image_binding.descriptor_type = get_descriptor_type(type);
        image_bindings.push_back(image_binding);

        return binding_index;
    }

	// replaces the image view at the specified binding index with a new one
    void replace_image(uint32_t target_binding_index, const ImageView& image_view, DescriptorType type, const std::optional<Sampler>& sampler = std::nullopt) {
        if (target_binding_index >= layout_bindings.size()) {
            Log::warning("in method DescriptorSet::replace_image(): argument for the target binding index is invalid; value is ", target_binding_index, " but the highest available index is ", layout_bindings.size() - 1);
            return;
        }

        VkDescriptorImageInfo image_info = {};
        if (type == DescriptorType::SAMPLED_IMAGE || type == DescriptorType::COMBINED_IMAGE_SAMPLER) {
            if (!sampler.has_value()) {
                Log::error("in method DescriptorSet::replace_image(): sampler is required for SAMPLED_IMAGE or COMBINED_IMAGE_SAMPLER");
                return;
            }
            image_info.sampler = sampler->get();
        }
        image_info.imageView = image_view.get();
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Assuming this is the desired layout

        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.pNext = nullptr;
        descriptor_write.dstSet = set;
        descriptor_write.dstBinding = target_binding_index;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = get_descriptor_type(type);
        descriptor_write.pImageInfo = &image_info;
        descriptor_write.pTexelBufferView = nullptr;
        descriptor_write.pBufferInfo = nullptr;

        vkUpdateDescriptorSets(logical, 1, &descriptor_write, 0, nullptr);

        // Update the stored image binding info if it exists
        for (auto& binding_info : image_bindings) {
            if (binding_info.binding_index == target_binding_index) {
                binding_info.image_view = image_view.get();
                binding_info.sampler = sampler ? sampler->get() : VK_NULL_HANDLE;
                binding_info.descriptor_type = get_descriptor_type(type);
                break;
            }
        }
    }

	// updates the descriptor set with the current image bindings
    void update() {
        std::vector<VkWriteDescriptorSet> descriptor_writes;
        std::vector<VkDescriptorImageInfo> image_infos;

        for (const auto& binding_info : image_bindings) {
            VkDescriptorImageInfo image_info{};
            if (binding_info.descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || binding_info.descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                image_info.sampler = binding_info.sampler;
            }
            image_info.imageView = binding_info.image_view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_infos.push_back(image_info);

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = set;
            descriptor_write.dstBinding = binding_info.binding_index;
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = binding_info.descriptor_type;
            descriptor_write.pImageInfo = &image_infos.back();
            descriptor_write.pBufferInfo = nullptr;
            descriptor_write.pTexelBufferView = nullptr;
            descriptor_writes.push_back(descriptor_write);
        }

        vkUpdateDescriptorSets(logical, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
    }

	// getters
    VkDescriptorSet& get() { return set; }
    VkDescriptorSetLayout& get_layout() { return layout; }

    // destructor
    ~DescriptorSet() {
        if (layout != nullptr) {
            vkDestroyDescriptorSetLayout(logical, layout, nullptr);
            layout = nullptr;
        }
    }

protected:
    struct ImageBindingInfo {
        uint32_t binding_index;
        VkImageView image_view;
        VkSampler sampler;
        VkDescriptorType descriptor_type;
    };

    VkDescriptorType get_descriptor_type(DescriptorType type) {
        switch (type) {
        case DescriptorType::STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::SAMPLED_IMAGE:  return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::STORAGE_IMAGE:  return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        default: Log::error("Invalid descriptor type."); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    VkDevice logical = nullptr;
    VkDescriptorSetLayoutCreateInfo layout_create_info = {};
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
    std::vector<ImageBindingInfo> image_bindings;
    VkDescriptorSet set = nullptr;
    VkDescriptorSetLayout layout = nullptr;
    bool layout_finalized = false;
};

class GraphicsPipeline {
public:
    // constructor
    GraphicsPipeline() = delete;
    GraphicsPipeline(
        Device& device,
        RenderPass& renderpass,
        uint32_t subpass_index,
        Swapchain& swapchain,
        ShaderModule& vertex_shader_module,
        const std::optional<ShaderModule>& fragment_shader_module = std::nullopt,
        const std::optional<ShaderModule>& hull_shader_module = std::nullopt,
        const std::optional<ShaderModule>& domain_shader_module = std::nullopt,
        uint32_t tessellation_patch_control_points = 3,
        const std::optional<VertexDescriptions>& vertex_descriptions = std::nullopt,
        const std::optional<PushConstants>& push_constants = std::nullopt,
        const std::optional<DescriptorSet>& descriptor_set = std::nullopt,
        VkPipelineDepthStencilStateCreateFlagBits depth_stencil_flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT,
        bool color_blend = false,
        const std::optional<std::vector<VkDynamicState>>& dynamic_states = std::nullopt
    ) {
        this->logical = device.get_logical();

        VkGraphicsPipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        
        // setup vertex shader stage
        std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_info;
        if (vertex_shader_module.get() != nullptr) {
            uint32_t i = shader_stage_create_info.size();
            shader_stage_create_info.push_back({});
            shader_stage_create_info[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shader_stage_create_info[i].module = vertex_shader_module.get();
            shader_stage_create_info[i].pName = "main";
        }

        // setup fragement shader stage
        if (fragment_shader_module.has_value() && fragment_shader_module.value().get() != nullptr) {
            uint32_t i = shader_stage_create_info.size();
            shader_stage_create_info.push_back({});
            shader_stage_create_info[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info[i].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shader_stage_create_info[i].module = fragment_shader_module.value().get();
            shader_stage_create_info[i].pName = "main";
        }
        
        // add shader stage infos to pipeline create info
        pipeline_create_info.stageCount = shader_stage_create_info.size();
        pipeline_create_info.pStages = shader_stage_create_info.data();

        // setup tesselation stage
        VkPipelineTessellationStateCreateInfo tessellation_state_create_info = {};
        if (hull_shader_module.has_value() && domain_shader_module.has_value()) {
            if (hull_shader_module.value().get() != nullptr) {
                uint32_t i = shader_stage_create_info.size();
                shader_stage_create_info.push_back({});
                shader_stage_create_info[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stage_create_info[i].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                shader_stage_create_info[i].module = hull_shader_module.value().get();
                shader_stage_create_info[i].pName = "main";
            }

            if (fragment_shader_module.has_value() && fragment_shader_module.value().get() != nullptr) {
                uint32_t i = shader_stage_create_info.size();
                shader_stage_create_info.push_back({});
                shader_stage_create_info[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stage_create_info[i].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                shader_stage_create_info[i].module = domain_shader_module.value().get();
                shader_stage_create_info[i].pName = "main";
            }
         
            tessellation_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
            tessellation_state_create_info.pNext = nullptr;
            tessellation_state_create_info.flags = 0; // reserved for future use
            tessellation_state_create_info.patchControlPoints = tessellation_patch_control_points;
            
            pipeline_create_info.pTessellationState = &tessellation_state_create_info;
        }
        else {
            pipeline_create_info.pTessellationState = nullptr;
        }

        // setup vertex input state
        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
        if (vertex_descriptions.has_value()) {
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount = uint32_t(vertex_descriptions.value().get_input_bindings().size());
            vertex_input_state_create_info.pVertexBindingDescriptions = vertex_descriptions.value().get_input_bindings().data();
            vertex_input_state_create_info.vertexAttributeDescriptionCount = uint32_t(vertex_descriptions.value().get_attribute_descriptions().size());
            vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_descriptions.value().get_attribute_descriptions().data();
            
            pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
        }
        else {
            pipeline_create_info.pVertexInputState = nullptr;
        }

        // setup input assembly state
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {};
        input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;

        // setup viewport
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapchain.get_width();
        viewport.height = (float)swapchain.get_height();

        // setup viewport state
        VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
        viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports = &viewport;
        viewport_state_create_info.scissorCount = 1;
        VkRect2D scissor = { {0,0}, {viewport.width, viewport.height} };
        viewport_state_create_info.pScissors = &scissor;

        pipeline_create_info.pViewportState = &viewport_state_create_info;

        // setup rasterization state
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
        rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;      
        rasterization_state_create_info.pNext = nullptr;
        rasterization_state_create_info.flags = 0; // reserved for future use
        rasterization_state_create_info.depthClampEnable = VK_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = VK_TRUE;
        rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
        rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable = VK_FALSE;
        /*
        rasterization_state_create_info.depthBiasConstantFactor = 
        rasterization_state_create_info.depthBiasClamp = 
        rasterization_state_create_info.depthBiasSlopeFactor =
        */
        rasterization_state_create_info.lineWidth = 1.0f;

        pipeline_create_info.pRasterizationState = &rasterization_state_create_info;

        // setup pipeline layout
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        
        if (descriptor_set.has_value()) {
            layout_create_info.setLayoutCount = 1;
            layout_create_info.pSetLayouts = &descriptor_set.get_layout();
        }
        else {
            layout_create_info.setLayoutCount = 0;
            layout_create_info.pSetLayouts = nullptr;
        }

        if (push_constants.has_value() && push_constants.get_data() == nullptr) { // check for empty PushConstants object
            layout_create_info.pushConstantRangeCount = 0;
            layout_create_info.pPushConstantRanges = nullptr;
        }
        else {
            layout_create_info.pushConstantRangeCount = 1;
            layout_create_info.pPushConstantRanges = &push_constants.get_range();
        }

        VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &layout);
        if (result == VK_SUCCESS) {
            Log::info("created pipeline layout for graphics pipeline (handle: ", layout, ")");
            pipeline_create_info.layout = layout;
        }
        else {
            Log::error("failed to create graphics pipeline layout (VkResult=", result, ")");
        }

        // setup multisample state
        VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
        multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.rasterizationSamples = static_cast<VkSampleCountFlagBits>(renderpass.get_multisample_count());
        pipeline_create_info.pMultisampleState = &multisample_state_create_info;

        // setup color blend state
        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
        VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
        if (color_blend) {
            color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment_state.blendEnable = color_blend;
            
            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.attachmentCount = 1;
            color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
            
            pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
        }
        else {
            pipeline_create_info.pColorBlendState = nullptr;
        }

        // setup depth-stencil state
        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {};
        if (renderpass.has_depth_stencil()) {
            depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_state_create_info.pNext = nullptr;
            depth_stencil_state_create_info.flags = depth_stencil_flags;
            depth_stencil_state_create_info.depthTestEnable = VK_FALSE;
            depth_stencil_state_create_info.depthWriteEnable = VK_FALSE;
            /*
            depth_stencil_state_create_info.depthCompareOp =
            depth_stencil_state_create_info.depthBoundsTestEnable =
            depth_stencil_state_create_info.stencilTestEnable =
            depth_stencil_state_create_info.front =
            depth_stencil_state_create_info.back =
            depth_stencil_state_create_info.minDepthBounds =
            depth_stencil_state_create_info.maxDepthBounds =
            */
            pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
        }
        else {
            pipeline_create_info.pDepthStencilState = nullptr;
        }

        // setup dynamic states
        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
        if (dynamic_states.has_value() {
            dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state_create_info.pNext = nullptr;
            dynamic_state_create_info.flags = 0;
            dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
            dynamic_state_create_info.pDynamicStates = dynamic_states.data();

            pipeline_create_info.pDynamicState = &dynamic_state_create_info;
        }
        else {
            pipeline_create_info.pDynamicState = nullptr;
        }

        // finalize graphics pipeline
        pipeline_create_info.renderPass = renderpass.get();
        pipeline_create_info.subpass = renderpass.get_subpass_count() > 0 ? subpass_index : 0;
        VkResult result = vkCreateGraphicsPipelines(logical, 0, 1, &pipeline_create_info, nullptr, &pipeline);
        if (result == VK_SUCCESS) {
            Log::info("graphics pipeline successfully created");
        }
        else {
            Log::error("failed to create graphics pipeline (VkResult=", result, ")");
        }
    }

    VkPipeline& get() { return pipeline; }

    VkPipelineLayout& get_layout() { return layout; }

    ~GraphicsPipeline() {
        Log::info("destroying graphics pipeline");
        vkDestroyPipeline(logical, pipeline, nullptr);
        vkDestroyPipelineLayout(logical, layout, nullptr);
    }

protected:
    VkPipeline pipeline = nullptr;
    VkPipelineLayout layout = nullptr;
    VkDevice logical = nullptr;
    VkViewport viewport = {};
};

class ComputePipeline {
public:

    // constructor
    ComputePipeline() = delete;
    ComputePipeline(Device& device, ShaderModule& compute_shader_module, PushConstants& push_constants, DescriptorSet& descriptor_set, uint32_t workgroup_size_x, uint32_t workgroup_size_y = 1, uint32_t workgroup_size_z = 1) {
        this->logical = device.get_logical();
        this->set = &descriptor_set;
        this->constants = &push_constants;
        this->workgroup_size_x = workgroup_size_x;
        this->workgroup_size_y = workgroup_size_y;
        this->workgroup_size_z = workgroup_size_z;

        // setup specialization constants for the workgroup dimensions
        std::vector<uint32_t> specialization_data = { workgroup_size_x, workgroup_size_y, workgroup_size_z };
        std::vector<VkSpecializationMapEntry> specialization_map_entries;
        
        VkSpecializationMapEntry workgroup_x_entry = {};
        workgroup_x_entry.constantID = 0; // for the GLSL shader: local_size_x_id = 0
        workgroup_x_entry.offset = 0;
        workgroup_x_entry.size = sizeof(uint32_t);
        specialization_map_entries.push_back(workgroup_x_entry);

        VkSpecializationMapEntry workgroup_y_entry = {};
        workgroup_y_entry.constantID = 1; // for the GLSL shader: local_size_y_id = 1
        workgroup_y_entry.offset = static_cast<uint32_t>(sizeof(uint32_t));
        workgroup_y_entry.size = sizeof(uint32_t);
        specialization_map_entries.push_back(workgroup_y_entry);

        VkSpecializationMapEntry workgroup_z_entry = {};
        workgroup_z_entry.constantID = 2; // for the GLSL shader: local_size_z_id = 2
        workgroup_z_entry.offset = static_cast<uint32_t>(2 * sizeof(uint32_t));
        workgroup_z_entry.size = sizeof(uint32_t);
        specialization_map_entries.push_back(workgroup_z_entry);

        VkSpecializationInfo specialization_info = {};
        specialization_info.mapEntryCount = 3;
        specialization_info.pMapEntries = specialization_map_entries.data();
        specialization_info.dataSize = 3 * sizeof(uint32_t);
        specialization_info.pData = specialization_data.data();

        // setup pipeline layout        
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount = 1; // = number of descriptor sets
        layout_create_info.pSetLayouts = &descriptor_set.get_layout();
        if (push_constants.get_data() == nullptr) { // check for empty PushConstants object
            layout_create_info.pushConstantRangeCount = 0;
            layout_create_info.pPushConstantRanges = NULL;
        }
        else {
            layout_create_info.pushConstantRangeCount = 1;
            layout_create_info.pPushConstantRanges = &push_constants.get_range();
        }
        layout_create_info.pNext = NULL;
        VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &layout);
        if (result == VK_SUCCESS) {
            Log::info("created pipeline layout for compute pipeline (handle: ", layout, ")");
        }
        else {
            Log::error("failed to create compute pipeline layout (VkResult=", result, ")");
        }

        // setup shader stage
        VkPipelineShaderStageCreateInfo shader_stage_create_info = {};
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.pNext = NULL;
        shader_stage_create_info.flags = NULL;
        shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shader_stage_create_info.module = compute_shader_module.get();
        shader_stage_create_info.pName = "main";
        shader_stage_create_info.pSpecializationInfo = &specialization_info;

        // finalize compute pipeline
        VkComputePipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_create_info.pNext = NULL;
        pipeline_create_info.flags = NULL;
        pipeline_create_info.stage = shader_stage_create_info;
        pipeline_create_info.layout = layout;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        result = vkCreateComputePipelines(device.get_logical(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline);
        if (result == VK_SUCCESS) {
            Log::info("compute pipeline successfully created (handle: ", pipeline, ")");
        }
        else {
            Log::error("failed to create compute pipeline (VkResult=", result, ")");
        }
    }

    ~ComputePipeline() {
        if (pipeline != nullptr) {
            Log::info("destroying compute pipeline");
            vkDestroyPipeline(logical, pipeline, nullptr);
            pipeline = nullptr;
        }
        if (layout != nullptr) {
            Log::info("destroying pipeline layout");
            vkDestroyPipelineLayout(logical, layout, nullptr);
            layout = nullptr;
        }
    }

    VkPipeline& get() { return pipeline; }

    VkPipelineLayout& get_layout() { return layout; }

    DescriptorSet* get_set() { return set; }

    PushConstants* get_constants() { return constants; }

    uint32_t get_workgroup_size_x() const { return workgroup_size_x; }
    uint32_t get_workgroup_size_y() const { return workgroup_size_y; }
    uint32_t get_workgroup_size_z() const { return workgroup_size_z; }

private:
    VkPipeline pipeline = nullptr;
    VkPipelineLayout layout = nullptr;
    VkDevice logical = nullptr;
    DescriptorSet* set = nullptr;
    PushConstants* constants = nullptr;
    uint32_t workgroup_size_x = 0;
    uint32_t workgroup_size_y = 0;
    uint32_t workgroup_size_z = 0;

};

// for synchronization between GPU and CPU
class Fence {
public:
    Fence(Device& device, bool signaled = false) {
        this->logical = device.get_logical();
        VkFenceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        vkCreateFence(logical, &create_info, nullptr, &fence);
    }

    ~Fence() {
        vkDestroyFence(logical, fence, nullptr);
    }

    bool signaled() {
        return vkGetFenceStatus(logical, fence) == VK_SUCCESS;
    }

    VkResult reset() {
        return vkResetFences(logical, 1, &fence);
    }

    VkResult wait(uint64_t timeout_nanosec = UINT64_MAX) {
        return vkWaitForFences(logical, 1, &fence, VK_TRUE, timeout_nanosec);
    }

    VkFence& get() const { return fence; }
private:
    VkFence fence = nullptr;
    VkDevice logical = nullptr;
};

// for synchronization on the GPU
class Semaphore {
public:
    Semaphore(Device& device, VkSemaphoreType type = VK_SEMAPHORE_TYPE_BINARY, uint64_t initial_value = 0) {
        this->logical = device.get_logical();
        this->type = type;

        VkSemaphoreTypeCreateInfo type_create_info = {};
        type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        type_create_info.pNext = NULL;
        type_create_info.semaphoreType = type;
        type_create_info.initialValue = type == VK_SEMAPHORE_TYPE_BINARY ? 0 : initial_value;

        VkSemaphoreCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        create_info.pNext = &type_create_info;
        create_info.flags = 0;
        vkCreateSemaphore(logical, &create_info, nullptr, &semaphore);
    }

    ~Semaphore() {
        vkDestroySemaphore(logical, semaphore, nullptr);
        delete semaphore; semaphore = nullptr;
    }

    VkResult wait(uint64_t timeout_nanosec = UINT64_MAX) {
        wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        wait_info.pNext = NULL;
        wait_info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &semaphore;
        return vkWaitSemaphores(logical, &wait_info, timeout_nanosec);
    }

    uint64_t counter() {
        uint64_t value;
        vkGetSemaphoreCounterValue(logical, semaphore, &value);
        return value;
    }

    void signal(uint64_t value) {
        signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signal_info.pNext = NULL;
        signal_info.semaphore = semaphore;
        signal_info.value = value;
        vkSignalSemaphore(logical, &signal_info);
    }

    VkSemaphore& get() const { return semaphore; }

private:
    VkSemaphore semaphore = nullptr;
    VkDevice logical = nullptr;
    VkSemaphoreType type;
    VkSemaphoreWaitInfo wait_info = {};
    VkSemaphoreSignalInfo signal_info = {};
};

class Event {
public:
    // constructor
    Event() = delete;
    Event(Device& device) {
        // If the VK_KHR_portability_subset extension is enabled, and VkPhysicalDevicePortabilitySubsetFeaturesKHR::events is VK_FALSE,
        // then the implementation does not support events, and vkCreateEvent must not be used !!!
        this->logical = device.get_logical();
        VkEventCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
        vkCreateEvent(logical, &create_info, nullptr, &event);
    }

    // destructor
    ~Event() {
        vkDestroyEvent(logical, event, nullptr);
    }

    // query event status
    bool signaled() {
        return vkGetEventStatus(logical, event) == VK_EVENT_SET;
    }

    // getters
    VkEvent& get() { return event; }
    VkDependencyInfo& get_dependency_info() { return dependency_info; }


protected:
    VkDependencyInfo dependency_info = {};
    VkEvent event = nullptr;
    VkDevice logical = nullptr;
    
};

class MemoryBarrier {
public:
    // constructor
    MemoryBarrier() = delete;
    MemoryBarrier(
        VkPipelineStageFlags2 source_stage_flags,
        VkAccessFlags2 source_access_flags,
        VkPipelineStageFlags2 target_stage_flags,
        VkAccessFlags2 target_access_flags,
    ) {
        buffer_memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
        memory_barrier.pNext = nullptr;
        memory_barrier.srcStageMask = source_stage_flags;
        memory_barrier.srcAccessMask = source_access_flags;
        memory_barrier.dstStageMask = target_stage_flags;
        memory_barrier.dstAccessMask = target_access_flags;
    }
    // destructor
    ~MemoryBarrier() {}
    VkMemoryBarrier2& get() const { return memory_barrier; }
protected:
    VkMemoryBarrier2 memory_barrier = {};
};

template<typename T>
class BufferMemoryBarrier {
public:
    // constructor
    BufferMemoryBarrier() = delete;
    BufferMemoryBarrier(
        Buffer<T>& buffer,
        VkPipelineStageFlags2 source_stage_flags,
        VkAccessFlags2 source_access_flags,
        VkPipelineStageFlags2 target_stage_flags,
        VkAccessFlags2 target_access_flags,
        uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
        uint32_t target_queue_family_index = VK_QUEUE_FAMILY_IGNORED
    ) {
        buffer_memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        buffer_memory_barrier.pNext = nullptr;
        buffer_memory_barrier.srcStageMask = source_stage_flags;
        buffer_memory_barrier.srcAccessMask = source_access_flags;
        buffer_memory_barrier.dstStageMask = target_stage_flags;
        buffer_memory_barrier.dstAccessMask = target_access_flags;
        buffer_memory_barrier.srcQueueFamilyIndex = source_queue_family_index;
        buffer_memory_barrier.dstQueueFamilyIndex = target_queue_family_index;
        buffer_memory_barrier.buffer = buffer.get();
        buffer_memory_barrier.offset = 0;
        buffer_memory_barrier.size = VK_WHOLE_SIZE;
    }
    // destructor
    ~BufferMemoryBarrier() {}
    VkBufferMemoryBarrier2& get() const { return buffer_memory_barrier; }
protected:
    VkBufferMemoryBarrier2 buffer_memory_barrier = {};
};

class ImageMemoryBarrier {
public:
    // constructor
    ImageMemoryBarrier() = delete;
    ImageMemoryBarrier(
        VkImage image,
        VkImageSubresourceRange subresource_range,
        VkPipelineStageFlags2 source_stage_flags,
        VkAccessFlags2 source_access_flags,
        VkPipelineStageFlags2 target_stage_flags,
        VkAccessFlags2 target_access_flags,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
        uint32_t target_queue_family_index = VK_QUEUE_FAMILY_IGNORED
    ) {
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        image_memory_barrier.pNext = nullptr;
        image_memory_barrier.srcStageMask = source_stage_flags;
        image_memory_barrier.srcAccessMask = source_access_flags;
        image_memory_barrier.dstStageMask = target_stage_flags;
        image_memory_barrier.dstAccessMask = target_access_flags;
        image_memory_barrier.oldLayout = old_layout;
        image_memory_barrier.newLayout = new_layout;
        image_memory_barrier.srcQueueFamilyIndex = source_queue_family_index;
        image_memory_barrier.dstQueueFamilyIndex = target_queue_family_index;
        image_memory_barrier.image = image;
        image_memory_barrier.subresourceRange = subresource_range;
    }
    // destructor
    ~ImageMemoryBarrier() {}
    VkImageMemoryBarrier2& get() const { return image_memory_barrier; }
protected:
    VkImageMemoryBarrier2 image_memory_barrier = {};
};

class CommandBuffer {
public:
    // constructor
    CommandBuffer() = delete;
    CommandBuffer(Device& device, QueueFamily usage, CommandPool& pool) {
        this->device = &device;
        this->logical = device.get_logical();
        this->graphics_queue = device.get_graphics_queue();
        this->compute_queue = device.get_compute_queue();
        this->transfer_queue = device.get_transfer_queue();
        this->usage = usage;
        this->pool = pool.get();

        // set pipeline bind point according to command pool QueueFamily setting
        if (usage == QueueFamily::GRAPHICS) {
            bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
        }
        else if (usage == QueueFamily::COMPUTE) {
            bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
        }
        else if (usage == QueueFamily::TRANSFER) {
            // do nothing (reason: transfer operations don't use pipelines)
        }
        else {
            Log::error("in CommandBuffer constructor: invalid QueueFamily argument!");
        }

        // destroy any previous command buffer
        if (buffer != nullptr) {
            vkFreeCommandBuffers(logical, this->pool, 1, &buffer);
            buffer = nullptr;
            Log::info("[OLD COMMAND BUFFER DESTROYED]");
        }

        // setup command buffer
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = pool.get();
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = 1;
        VkResult result = vkAllocateCommandBuffers(logical, &allocate_info, &buffer);
        if (result == VK_SUCCESS) {
            Log::info("successfully allocated command buffer (handle: ", buffer, ")");
        }
        else {
            Log::warning("in CommandBuffer constructor: memory allocation failed (VkResult=", result, ")!");
        }
        
        begin_recording();
    }

    ~CommandBuffer() {
        if (buffer != nullptr) {
            vkFreeCommandBuffers(logical, pool, 1, &buffer);
            Log::info("[COMMAND BUFFER DESTROYED]");
            buffer = nullptr;
        }
    }

    void reset(VkCommandBufferResetFlags flags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) {
        VkResult result = vkResetCommandBuffer(buffer, flags);
        if (result == VK_SUCCESS) {
            Log::debug("successfully reset command buffer");
        }
        else {
            Log::warning("failed to reset command buffer (handle: ", buffer, ", VkResult = ", result, ")");
        }
        begin_recording();
    }

    // definition outside class (because it depends on CommandBuffer::get(), which needs to be defined first)
    Event& set_event(CommandBuffer& command_buffer, std::vector<VkMemoryBarrier2> memory_barriers, std::vector<VkBufferMemoryBarrier2> buffer_memory_barriers, std::vector<VkImageMemoryBarrier2> image_memory_barriers, VkDependencyFlags flags) {
        Event event;
        event.get_dependency_info().sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        event.get_dependency_info().pNext = NULL;
        event.get_dependency_info().dependencyFlags = flags;
        event.get_dependency_info().memoryBarrierCount = static_cast<uint32_t>(memory_barriers.size());
        event.get_dependency_info().pMemoryBarriers = memory_barriers.data();
        event.get_dependency_info().bufferMemoryBarrierCount = static_cast<uint32_t>(buffer_memory_barriers.size());
        event.get_dependency_info().pBufferMemoryBarriers = buffer_memory_barriers.data();
        event.get_dependency_info().imageMemoryBarrierCount = static_cast<uint32_t>(image_memory_barriers.size());
        event.get_dependency_info().pImageMemoryBarriers = image_memory_barriers.data();
        vkCmdSetEvent2(this->buffer, event.get(), &event.get_dependency_info());
        return Event;
    }

    void reset_event(Event& event, VkPipelineStageFlags& stage_mask) {
        vkCmdResetEvent(buffer, event.get(), stage_mask);
    }

    void wait_event(Event&) {
        vkCmdWaitEvents2(buffer, 1, &event.get(), event.get_dependency_info());
    }

    void bind_pipeline(GraphicsPipeline& pipeline) {
        if (usage != QueueFamily::GRAPHICS) {
            Log::error("invalid usage of CommandBuffer::bind_pipeline(): this command buffer doesn't support graphics");
        }
        if (pipeline.get() != nullptr) {
            vkCmdBindPipeline(buffer, bind_point, pipeline.get());
        }
        else {
            Log::error("CommandBuffer::bind_pipeline() has invalid pipeline argument");
        }
    }

    void bind_pipeline(ComputePipeline& pipeline) {
        if (usage != QueueFamily::COMPUTE) {
            Log::error("invalid usage of CommandBuffer::bind_pipeline(): this command buffer doesn't support compute");
        }
        if (pipeline.get() != nullptr) {
            Log::debug("binding pipeline ", pipeline.get(), " to bindpoint type ", bind_point, " at command buffer ", buffer);
            vkCmdBindPipeline(buffer, bind_point, pipeline.get());
        }
        else {
            Log::error("CommandBuffer::bind_pipeline() has invalid pipeline argument");
        }
        workgroup_size_x = pipeline.get_workgroup_size_x();
        workgroup_size_y = pipeline.get_workgroup_size_y();
        workgroup_size_z = pipeline.get_workgroup_size_z();
        pipeline_layout = pipeline.get_layout();
    }

    void bind_descriptor_set(DescriptorSet& set) {
        if (pipeline_layout == nullptr) {
            Log::error("invalid usage of CommandBuffer::bind_descriptor_set(): please use CommandBuffer::bind_pipeline() first!");
        }
        Log::debug("binding descriptor sets to command buffer, bindpoint ", bind_point);
        vkCmdBindDescriptorSets(buffer, bind_point, pipeline_layout, 0, 1, &set.get(), 0, nullptr);
    }

    void push_constants(PushConstants& push_constants) {
        vkCmdPushConstants(
            buffer,
            pipeline_layout,
            push_constants.get_range().stageFlags,
            push_constants.get_range().offset,
            push_constants.get_range().size,
            push_constants.get_data()
        );
    }

    // records a buffer copy command;
    // an be used for Staging->Device, Device->Staging, Device->Device
    template<typename SrcT, typename DstT>
    void copy_buffer(const Buffer<SrcT>& src_buffer, Buffer<DstT>& dst_buffer, VkDeviceSize size_bytes, VkDeviceSize src_offset_bytes = 0, VkDeviceSize dst_offset_bytes = 0) {
        if (size_bytes == 0) return; // Nothing to copy

        // bound checks
        if (src_offset_bytes + size_bytes > src_buffer.get_size_bytes()) {
            Log::error("CommandBuffer::copy_buffer: Source region exceeds source buffer bounds.");
            return; // Or handle error
        }
        if (dst_offset_bytes + size_bytes > dst_buffer.get_size_bytes()) {
            Log::error("CommandBuffer::copy_buffer: Destination region exceeds destination buffer bounds.");
            return; // Or handle error
        }

        VkBufferCopy copy_region = {};
        copy_region.srcOffset = src_offset_bytes;
        copy_region.dstOffset = dst_offset_bytes;
        copy_region.size = size_bytes;
        vkCmdCopyBuffer(buffer, src_buffer.get(), dst_buffer.get(), 1, copy_region);
    }

    // add memory barrier
    void add_barrier(MemoryBarrier& barrier) {
        VkDependencyInfo dependency_info = {};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.pNext = nullptr;
        dependency_info.memoryBarrierCount = 1;
        dependency_info.pMemoryBarriers = barrier.get();
        dependency_info.bufferMemoryBarrierCount = 0;
        dependency_info.pBufferMemoryBarriers = nullptr;
        dependency_info.imageMemoryBarrierCount = 0;
        dependency_info.pImageMemoryBarriers = nullptr;
        vkCmdPipelineBarrier2(buffer, &dependency_info);
    }

	// add buffer memory barrier
    void add_barrier(BufferMemoryBarrier& barrier) {
        VkDependencyInfo dependency_info = {};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.pNext = nullptr;
        dependency_info.memoryBarrierCount = 0;
        dependency_info.pMemoryBarriers = nullptr;
        dependency_info.bufferMemoryBarrierCount = 1;
        dependency_info.pBufferMemoryBarriers = barrier.get();
        dependency_info.imageMemoryBarrierCount = 0;
        dependency_info.pImageMemoryBarriers = nullptr;
        vkCmdPipelineBarrier2(buffer, &dependency_info);
    }

	// add image memory barrier
    void add_barrier(ImageMemoryBarrier& barrier) {
        VkDependencyInfo dependency_info = {};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.pNext = nullptr;
        dependency_info.memoryBarrierCount = 0;
        dependency_info.pMemoryBarriers = nullptr;
        dependency_info.bufferMemoryBarrierCount = 0;
        dependency_info.pBufferMemoryBarriers = nullptr;
        dependency_info.imageMemoryBarrierCount = 1;
        dependency_info.pImageMemoryBarriers = barrier.get();
        vkCmdPipelineBarrier2(buffer, &dependency_info);
    }

	// add multiple barriers
    void add_barriers(
            std::optional<std::vector<MemoryBarrier>>& memory_barriers,
            std::optional<std::vector<BufferMemoryBarrier>>& buffer_memory_barriers,
            std::optional<std::vector<ImageMemoryBarrier>>& image_memory_barriers
        ) {
        VkDependendyInfo dependency_info = {};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.pNext = nullptr;
        
        // setup memory barriers
        if (memory_barriers.has_value() && !memory_barriers.value().empty()) {
            uint32_t barriers_count = memory_barriers.size();
            std::vector<VkMemoryBarrier2> barrier_handles(barriers_count);
            for (uint32_t i = 0; i < barriers_count; i++) {
                barrier_handles[i] = memory_barriers[i].get();
            }
            dependency_info.memoryBarrierCount = barriers_count;
            dependency_info.pMemoryBarriers = barrier_handles.data();
        }
        else {
            dependency_info.memoryBarrierCount = 0;
            dependency_info.pMemoryBarriers = nullptr;
        }

        // setup buffer memory barriers
        if (buffer_memory_barriers.has_value() && !buffer_memory_barriers.value().empty()) {
            uint32_t barriers_count = buffer_memory_barriers.size();
            std::vector<VkBufferMemoryBarrier2> barrier_handles(barriers_count);
            for (uint32_t i = 0; i < barriers_count; i++) {
                barrier_handles[i] = buffer_memory_barriers[i].get();
            }
            dependency_info.bufferMemoryBarrierCount = barriers_count;
            dependency_info.pBufferMemoryBarriers = barrier_handles.data();
        }
        else {
            dependency_info.bufferMemoryBarrierCount = 0;
            dependency_info.pBufferMemoryBarriers = nullptr;
        }

        // setup image memory barriers
        if (image_memory_barriers.has_value() && !image_memory_barriers.value().empty()) {
            uint32_t barriers_count = image_memory_barriers.size();
            std::vector<VkImageMemoryBarrier2> barrier_handles(barriers_count);
            for (uint32_t i = 0; i < barriers_count; i++) {
                barrier_handles[i] = memory_barriers[i].get();
            }
            dependency_info.imageMemoryBarrierCount = barriers_count;
            dependency_info.pImageMemoryBarriers = barrier_handles.data();
        }
        else {
            dependency_info.imageMemoryBarrierCount = 0;
            dependency_info.pImageMemoryBarriers = nullptr;
        }

        vkCmdPipelineBarrier2(buffer, &dependency_info);
    }

	// transition image layout
    void transition_image_layout(Image image, VkImageLayout new_layout) {
        VkImageMemoryBarrier2 image_memory_barrier = {};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        image_memory_barrier.oldLayout = image.get_layout();
        image_memory_barrier.newLayout = new_layout;
        image_memory_barrier.srcQueueFamilyIndex = device->get_graphics_queue_family_index();
        image_memory_barrier.dstQueueFamilyIndex = device->get_graphics_queue_family_index();
        image_memory_barrier.image = image.get();
        image_memory_barrier.subresourceRange.aspectMask = aspect_mask;
        image_memory_barrier.subresourceRange.baseMipLevel = 0;
        image_memory_barrier.subresourceRange.levelCount = 1;
        image_memory_barrier.subresourceRange.baseArrayLayer = 0;
        image_memory_barrier.subresourceRange.layerCount = 1;

        // Determine stage and access masks based on layouts
        VkImageAspectFlags aspect_mask;
        VkPipelineStageFlags src_stage;
        VkPipelineStageFlags dst_stage;
        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            image_memory_barrier.srcAccessMask = 0;
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            image_memory_barrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            image_memory_barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            image_memory_barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            image_memory_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            // Add more common layout transition cases or make masks parameters
            Log::warning("Image::transition_layout: Unhandled layout transition, using default masks/stages.");
            image_memory_barrier.srcAccessMask = 0; // Be conservative
            image_memory_barrier.dstAccessMask = 0; // Be conservative
        }
        
		// Setup dependency info
        VkDependencyInfo dependency_info = {};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.memoryBarrierCount = 0;
        dependency_info.pMemoryBarriers = nullptr;
        dependency_info.bufferMemoryBarrierCount = 0;
        dependency_info.pBufferMemoryBarriers = nullptr;
        dependency_info.imageMemoryBarrierCount = 1;
        dependency_info.pImageMemoryBarriers = &image_memory_barrier;

        vkCmdPipelineBarrier2(buffer, &dependency_info);

        image.layout = new_layout;
    }

    void draw(uint32_t& vertex_count, uint32_t instance_count=1, uint32_t first_vertex=0, uint32_t first_instance=0) {
        vkCmdDraw(buffer, vertex_count, instance_count, first_vertex, first_instance);
    }

    void dispatch(uint32_t global_size_x, uint32_t global_size_y = 1, uint32_t global_size_z = 1) {
        // dispatch for compute
        if (usage == QueueFamily::COMPUTE) {
            const uint32_t workgroups_x = (global_size_x + workgroup_size_x - 1) / workgroup_size_x;
            const uint32_t workgroups_y = (global_size_y + workgroup_size_y - 1) / workgroup_size_y;
            const uint32_t workgroups_z = (global_size_z + workgroup_size_z - 1) / workgroup_size_z;
            vkCmdDispatch(buffer, workgroups_x, workgroups_y, workgroups_z);
        }
        else {
            Log::error("invalid call of method CommandBuffer::dispatch, only allowed for usage type QueueFamily::COMPUTE");
        }
    }

    void begin_render(VkOffset2D offset, VkExtent2D extent, VkRenderingFlags flags, std::vector<VkRenderingAttachmentInfo>& color_attachments, VkRenderingAttachmentInfo& depth_attachment, VkRenderingAttachmentInfo& stencil_attachment) {
        rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        rendering_info.pNext = NULL;
        rendering_info.flags = flags;
        rendering_info.renderArea = { offset, extent }; // VkRect2D
        rendering_info.layerCount = 1;
        rendering_info.viewMask = 0; // =multiview disabled by default
        rendering_info.colorAttachmentCount = color_attachments.size();
        rendering_info.pColorAttachments = color_attachments.data();
        rendering_info.pDepthAttachment = &depth_attachment;
        rendering_info.pStencilAttachment = &stencil_attachment;
        vkCmdBeginRendering(buffer, &rendering_info);
    }

    void begin_renderpass(RenderPass& renderpass, VkOffset2D offset, VkExtent2D extent, std::vector<VkClearValue>& clear_value) {
        renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.pNext = NULL;
        renderpass_begin_info.renderPass = renderpass.get();
        renderpass_begin_info.renderArea = { offset, extent }; // VkRect2D
        renderpass_begin_info.clearValueCount = clear_value.size();
        renderpass_begin_info.pClearValues = clear_value.data();

        subpass_begin_info.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
        subpass_begin_info.pNext = NULL;
        subpass_begin_info.contents = VK_SUBPASS_CONTENTS_INLINE;

        vkCmdBeginRenderPass2(buffer, &renderpass_begin_info, &subpass_begin_info);
    }

    void end_renderpass() {
        vkCmdEndRenderPass(buffer);
    }

    void next_subpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) {
        vkCmdNextSubpass(buffer, contents);
    }

    void submit(Fence& fence) {
        // stop command buffer recording state (thus triggering executable state)
        vkEndCommandBuffer(buffer);

        // submit to queue (triggers command buffer pending state)
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pCommandBuffers = &buffer;
        submit_info.commandBufferCount = 1;
        
        if (usage == QueueFamily::GRAPHICS) {
            vkQueueSubmit(graphics_queue, 1, &submit_info, fence.get());
        }
        else if (usage == QueueFamily::COMPUTE) {
            vkQueueSubmit(compute_queue, 1, &submit_info, fence.get());
        }
        else if (usage == QueueFamily::TRANSFER) {
            vkQueueSubmit(transfer_queue, 1, &submit_info, fence.get());
        }
    }

    void submit() {
        // stop command buffer recording state (thus triggering executable state)
        vkEndCommandBuffer(buffer);

        // submit to queue (triggers command buffer pending state)
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pCommandBuffers = &buffer;
        submit_info.commandBufferCount = 1;

        if (usage == QueueFamily::GRAPHICS) {
            vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
        }
        else if (usage == QueueFamily::COMPUTE) {
            vkQueueSubmit(compute_queue, 1, &submit_info, VK_NULL_HANDLE);
        }
        else if (usage == QueueFamily::TRANSFER) {
            vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
        }
    }

    VkCommandBuffer& get() { return buffer; }

    // shorthand for bind_pipeline -> push_constants -> dispatch -> submit;
    // (note: a fence will only be used if fence_timeout_nanosec != 0)
    void compute(ComputePipeline& pipeline, uint32_t global_size_x, uint32_t global_size_y = 1, uint32_t global_size_z = 1, uint64_t fence_timeout_nanosec = 10000) {
        bind_pipeline(pipeline);
        bind_descriptor_set(*pipeline.get_set());
        push_constants(*pipeline.get_constants());
        dispatch(global_size_x, global_size_y, global_size_z);
        if (fence_timeout_nanosec != 0) {
            Fence fence(*device,false);
            submit(fence);
            while (!fence.signaled()) {
                fence.wait(fence_timeout_nanosec);
            };
        }
        else {
            submit();
        }
        reset();
    }

    // start command buffer recording state
    void begin_recording() {
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pNext = NULL;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // specifies that each recording of the command buffer will only be submitted once, and the command buffer will be reset and recorded again between each submission
        begin_info.pInheritanceInfo = nullptr; // pointer to a VkCommandBufferInheritanceInfo struct; only relevant for secondary command buffers
        VkResult result = vkBeginCommandBuffer(buffer, &begin_info);
        if (result == VK_SUCCESS) {
            Log::debug("beginning command buffer recording state");
        }
        else {
            Log::warning("failed to begin command buffer recording state (VkResult = ", result, ")");
        }
    }

protected:
    VkCommandBuffer buffer = nullptr;
    QueueFamily usage = QueueFamily::UNKNOWN;
    VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    VkPipelineLayout pipeline_layout = nullptr;
    VkDevice logical = nullptr;
    Device* device = nullptr;
    VkQueue graphics_queue = nullptr;
    VkQueue compute_queue = nullptr;
    VkQueue transfer_queue = nullptr;
    VkCommandBufferAllocateInfo allocate_info = {};
    VkCommandBufferBeginInfo begin_info = {};
    VkRenderingInfo rendering_info = {};
    VkRenderPassBeginInfo renderpass_begin_info = {};
    VkSubpassBeginInfo subpass_begin_info = {};
    VkSubmitInfo submit_info = {};
    VkCommandPool pool = nullptr;
    uint32_t workgroup_size_x = 0; // only used for compute pipelines
    uint32_t workgroup_size_y = 0; // only used for compute pipelines
    uint32_t workgroup_size_z = 0; // only used for compute pipelines
};

// shared instance and device manager as singleton class
class VkManager {
public:
    static VkManager* make_singleton(std::vector<const char*>& instance_layer_names,
                                     std::vector<const char*>& instance_extension_names,
                                     std::vector<const char*> device_extension_names,
                                     char* application_name = "Shared Vulkan Manager",
                                     uint32_t application_major_version = 1,
                                     uint32_t application_minor_version = 0,
                                     uint32_t application_patch_version = 0,
                                     uint32_t default_device_id = 0) {
        if (shared == nullptr) {
            shared_instance_layer_names = instance_layer_names;
            shared_instance_extension_names = instance_extension_names;
            shared_device_extension_names = device_extension_names;
            shared_default_device_id = default_device_id;
            shared_application_name = application_name;
            shared_application_major_version = application_major_version;
            shared_application_minor_version = application_minor_version;
            shared_application_patch_version = application_patch_version;
            // calling the private constructor
            shared = new VkManager;
            // register static destructor
            std::atexit(&VkManager::destroy_singleton);
        }
        return shared;
    }

    static Device& get_device() {return *device;}
    static Instance& get_instance() {return *instance;}
    static VkManager* get_singleton() { return shared; }
    static CommandPool& get_command_pool_graphics() { return *command_pool_graphics; }
    static CommandPool& get_command_pool_compute() { return *command_pool_compute; }
    static CommandPool& get_command_pool_transfer() { return *command_pool_transfer; }

 private:
    // shared members
    static Instance* instance;
    static Device* device;
    static VkManager* shared;
    static std::vector<const char*> shared_instance_layer_names;
    static std::vector<const char*> shared_instance_extension_names;
    static std::vector<const char*> shared_device_extension_names;
    static uint32_t shared_default_device_id;
    static char* shared_application_name;
    static uint32_t shared_application_major_version;
    static uint32_t shared_application_minor_version;
    static uint32_t shared_application_patch_version;
    static CommandPool* command_pool_compute;
    static CommandPool* command_pool_graphics;
    static CommandPool* command_pool_transfer;

    // private constructor: one-time initialization on first call of get_singleton()
    VkManager() {
        instance = new Instance();

        // set create flags
        VkInstanceCreateFlags shared_instance_create_flags = 0;

        // finalize instance creation
        instance->init_api_version(VK_API_VERSION_1_2);
        instance->init_application(shared_application_name, shared_application_major_version, shared_application_minor_version, shared_application_patch_version);
        instance->init_extensions(shared_instance_extension_names);
        instance->init_layers(shared_instance_layer_names);
        instance->create(shared_instance_create_flags);

        // enable device features
        VkPhysicalDeviceFeatures enabled_device_features = {};
        enabled_device_features.imageCubeArray = VK_TRUE;

        // finalize device creation
        device = new Device(*instance, enabled_device_features, shared_device_extension_names, shared_default_device_id);

        // setup command pools
        Log::debug("creating new graphics command pool");
        command_pool_graphics = new CommandPool(*device, QueueFamily::GRAPHICS);
        Log::debug("creating new compute command pool");
        command_pool_compute = new CommandPool(*device, QueueFamily::COMPUTE);
        Log::debug("creating new transfer command pool");
        command_pool_transfer = new CommandPool(*device, QueueFamily::TRANSFER);
    }

    // private custom destructor method
    static void destroy_singleton() {
        if (shared != nullptr) {
            Log::debug("singleton manager destructor invoked");
            delete command_pool_graphics; command_pool_graphics = nullptr;
            delete command_pool_compute; command_pool_compute = nullptr;
            delete command_pool_transfer; command_pool_transfer = nullptr;
            delete device; device = nullptr;
            delete instance; instance = nullptr;
            delete shared; shared = nullptr;
        }
    }
};

// initialization of VkManager static members
Instance* VkManager::instance = nullptr;
Device* VkManager::device = nullptr;
VkManager* VkManager::shared = nullptr;
CommandPool* VkManager::command_pool_compute = nullptr;
CommandPool* VkManager::command_pool_graphics = nullptr;
CommandPool* VkManager::command_pool_transfer = nullptr;
std::vector<const char*> VkManager::shared_instance_layer_names = {};
std::vector<const char*> VkManager::shared_instance_extension_names = {};
std::vector<const char*> VkManager::shared_device_extension_names = {};
uint32_t VkManager::shared_default_device_id = 0;
char* VkManager::shared_application_name = "";
uint32_t VkManager::shared_application_major_version = 1;
uint32_t VkManager::shared_application_minor_version = 0;
uint32_t VkManager::shared_application_patch_version = 0;
#endif

