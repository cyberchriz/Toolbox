#ifndef VKCONTEXT_H
#define VKCONTEXT_H

#include "log.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <utility>
#include <vector>
#include <vulkan_core.h>

// default variables
constexpr const char* spirv_folder = "C:/Users/Admin/source/repos/Toolbox/shaders/spirv/";
constexpr uint32_t max_storage_buffers_per_pool = 100;
constexpr uint32_t max_dynamic_storage_buffers_per_pool = 30;
constexpr uint32_t max_uniform_buffers_per_pool = 30;
constexpr uint32_t max_dynamic_uniform_buffers_per_pool = 30;

// forward declarations

class Device;
class CommandPool;
class CommandBuffer;
class ShaderModule;
class DescriptorSet;
class Event;

enum BufferUsage {
    VERTEX,
    STORAGE,
    UNIFORM,
    INDEX
};

enum DescriptorType {
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    STORAGE_IMAGE,
    SAMPLED_IMAGE
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
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    // Move constructor
    Instance(Instance&& other) noexcept : Instance() {
        // Steal resources from the 'other' instance
        this->instance = other.instance;
        this->application_info = other.application_info;
        this->instance_create_info = other.instance_create_info;
        this->enabled_instance_extension_names_ptr = other.enabled_instance_extension_names_ptr;
        this->enabled_layer_names_ptr = other.enabled_layer_names_ptr;

        // Reset 'other' instance
        other.instance = nullptr;
        other.enabled_instance_extension_names_ptr = nullptr;
        other.enabled_layer_names_ptr = nullptr;
    }

    // Copy constructor
    Instance(const Instance& other) : Instance() {
        // Copy application info
        std::memcpy(&this->application_info, &other.application_info, sizeof(VkApplicationInfo));

        // Copy instance create info
        std::memcpy(&this->instance_create_info, &other.instance_create_info, sizeof(VkInstanceCreateInfo));

        // Copy enabled instance extension names
        if (enabled_instance_extension_names_ptr != nullptr) {
            delete[] enabled_instance_extension_names_ptr;
        }
        this->enabled_instance_extension_names_ptr = new const char* [other.instance_create_info.enabledExtensionCount];
        for (uint32_t i = 0; i < other.instance_create_info.enabledExtensionCount; ++i) {
            size_t len = std::strlen(other.enabled_instance_extension_names_ptr[i]) + 1;
            this->enabled_instance_extension_names_ptr[i] = new char[len];
            std::memcpy(&this->enabled_instance_extension_names_ptr[i], &other.enabled_instance_extension_names_ptr[i], len);
        }

        // Copy enabled layer names
        if (enabled_layer_names_ptr != nullptr) {
            delete[] enabled_layer_names_ptr;
        }
        this->enabled_layer_names_ptr = new const char* [other.instance_create_info.enabledLayerCount];
        for (uint32_t i = 0; i < other.instance_create_info.enabledLayerCount; ++i) {
            size_t len = std::strlen(other.enabled_layer_names_ptr[i]) + 1;
            this->enabled_layer_names_ptr[i] = new char[len];
            std::memcpy(&this->enabled_layer_names_ptr[i], &other.enabled_layer_names_ptr[i], len);
        }

        if (other.instance != nullptr) {
            this->create();
        }
    }

    // destructor
    ~Instance() {
        if (instance != nullptr) {
            vkDestroyInstance(instance, nullptr);
            delete[] enabled_layer_names_ptr;
            delete[] enabled_instance_extension_names_ptr;
        }
        Log::log(INFO, "[INSTANCE DESTROYED]");
    }

    void init_application(const char* name = "Vulkan Application", uint32_t major_version = 1, uint32_t minor_version = 0, uint32_t patch_version = 0) {
        application_info.pApplicationName = name;
        application_info.applicationVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    }

    void init_engine(const char* name = "", uint32_t major_version = 0, uint32_t minor_version = 0, uint32_t patch_version = 0) {
        application_info.pEngineName = name;
        application_info.engineVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    }

    void init_api_version(uint32_t version = VK_API_VERSION_1_2) {
        application_info.apiVersion = version;
    }

    void init_layers(const std::vector<const char*>& enabled_layer_names) {
        // log available layers
        if (Log::at_least(INFO)) {
            uint32_t count;
            vkEnumerateInstanceLayerProperties(&count, nullptr);
            std::vector<VkLayerProperties> properties(count);
            vkEnumerateInstanceLayerProperties(&count, properties.data());
            Log::log(INFO, count, " layer types available");
            for (uint32_t i = 0; i < count; i++) {
                Log::log(DEBUG, "(", i+1, ") ", properties[i].layerName);
            }
        }
        // add requested layer names to instance create info
        uint32_t layer_count = enabled_layer_names.size();
        enabled_layer_names_ptr = new const char* [layer_count];
        if (layer_count == 0) {
            Log::log(WARNING, "no layers enabled for Vulkan instance; make sure none are required");
        }
        else {
            Log::log(INFO, "passing requested layer names to instance create info");
        }
        for (uint32_t i = 0; i < layer_count; i++) {
            Log::log(DEBUG, enabled_layer_names[i]);
            enabled_layer_names_ptr[i] = enabled_layer_names[i];
        }
        instance_create_info.enabledLayerCount = layer_count;
        instance_create_info.ppEnabledLayerNames = enabled_layer_names_ptr;
    }

    void init_extensions(const std::vector<const char*>& enabled_extension_names) {
        // log available extensions
        if (Log::at_least(INFO)) {
            uint32_t count;
            vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
            Log::log(INFO, count, " instance extensions available");
            for (uint32_t i = 0; i < count; i++) {
                Log::log(DEBUG, "(", i + 1, ") ", extensions[i].extensionName);
            }
        }

        // add requested instance extension names to instance create info
        uint32_t extension_count = enabled_extension_names.size();
        enabled_instance_extension_names_ptr = new const char* [extension_count];
        if (extension_count == 0) {
            Log::log(INFO, "no extensions enabled for Vulkan instance; make sure none are required");
        }
        else {
            Log::log(INFO, "passing requested extension names to instance create info");
        }
        for (uint32_t i = 0; i < extension_count; ++i) {
            Log::log(DEBUG, enabled_extension_names[i]);
            enabled_instance_extension_names_ptr[i] = enabled_extension_names[i];
        }
        instance_create_info.enabledExtensionCount = extension_count;
        instance_create_info.ppEnabledExtensionNames = enabled_instance_extension_names_ptr;
    }

    void create(VkInstanceCreateFlags flags = 0) {
        // destroy any previous instance
        if (instance != nullptr) {
            vkDestroyInstance(instance, nullptr);
            instance = nullptr;
            Log::log(INFO, "[OLD INSTANCE DESTROYED]");
        }

        instance_create_info.pNext = nullptr;
        instance_create_info.flags = flags;
        instance_create_info.pApplicationInfo = &application_info;

        VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "Vulkan instance successfully created.");
        }
        else {
            Log::log(ERROR, "Failed to create Vulkan Instance (VkResult=", result, ")");
        }
    }

    VkInstance& get() {
        return instance;
    }

protected:
    VkInstance instance = nullptr;
    VkApplicationInfo application_info = {};
    VkInstanceCreateInfo instance_create_info = {};
    const char** enabled_instance_extension_names_ptr = nullptr;
    const char** enabled_layer_names_ptr = nullptr;
};

class Device {
public:
    // default constructor
    Device() {}

    // parametric constructor
    Device(Instance& instance, VkPhysicalDeviceFeatures enabled_features = {}, const std::vector<const char*>& enabled_extension_names = {}, uint32_t id = 0) {
        // confirm valid instance
        if (instance.get() == nullptr) {
            Log::error("Invalid call to Device constructor: create Vulkan instance first!");
        }
        // search for physical devices with Vulkan support
        uint32_t num_devices = 0;
        vkEnumeratePhysicalDevices(instance.get(), &num_devices, NULL);
        if (num_devices == 0) {
            Log::log(WARNING, "No device(s) with Vulkan support found!");
            return;
        }
        else {
            // list available physical devices with Vulkan support
            std::vector<VkPhysicalDevice> devices(num_devices);
            vkEnumeratePhysicalDevices(instance.get(), &num_devices, devices.data());
            // default: select first available device (at index 0)
            uint32_t selected_index = 0;
            physical = devices[selected_index];
            uint32_t selected_id = 0;
            Log::log(INFO, "available physical devices with Vulkan support:");

            for (uint32_t i = 0; i < num_devices; i++) {
                vkGetPhysicalDeviceProperties(devices[i], &properties);
                if (i == selected_index) {
                    selected_id = properties.deviceID;
                }
                Log::log(INFO, "(", i, ") ", properties.deviceName, ", deviceID ", properties.deviceID, ", vendorID ", properties.vendorID,
                    ", type ", properties.deviceType, ", API version ", properties.apiVersion, ", driver version ", properties.driverVersion);
                // chose specific device (instead of default index 0) if passed id matches
                if (id == properties.deviceID) {
                    physical = devices[i];
                    selected_id = properties.deviceID;
                    selected_index = i;
                }
            }
            Log::log(INFO, "Selected physical device ", selected_index, " with ID ", selected_id);
        }

        // store properties for selected device
        vkGetPhysicalDeviceProperties(physical, &properties);
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        properties2.pNext = nullptr;
        vkGetPhysicalDeviceProperties2(physical, &properties2);

        // log available extensions
        if (Log::at_least(INFO)) {
            uint32_t count;
            vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, extensions.data());
            Log::log(DEBUG, count, " device extensions available");
            for (uint32_t i = 0; i < count; i++) {
                Log::log(DEBUG, "(", i + 1, ") ", extensions[i].extensionName);
            }
        }

        // add requested device extension names to device create info
        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        extension_count = enabled_extension_names.size();
        device_extension_names_ptr = new const char* [extension_count];
        if (extension_count == 0) {
            Log::log(WARNING, "no extensions enabled for Vulkan logical device; make sure none are required");
        }
        else {
            Log::log(INFO, "passing requested extension names to device create info");
        }
        for (uint32_t i = 0; i < extension_count; ++i) {
            Log::log(DEBUG, enabled_extension_names[i]);
            device_extension_names_ptr[i] = enabled_extension_names[i];
        }
        device_create_info.enabledExtensionCount = extension_count;
        device_create_info.ppEnabledExtensionNames = device_extension_names_ptr;

        // Queue creation
        uint32_t num_queue_families;
        float priority = 1.0f; // default priority for all queue types
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &num_queue_families, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &num_queue_families, queue_families.data());
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        // Iterate over the queue families to find appropriate queue indices
        for (uint32_t i = 0; i < num_queue_families; ++i) {
            const VkQueueFamilyProperties& queue_family = queue_families[i];

            // Check for graphics queue support
            static bool graphics_queue_assigned = false;
            if (!graphics_queue_assigned && (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                graphics_queue_family_index = i;
                queue_create_infos.push_back({});
                queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_infos[i].queueFamilyIndex = i;
                queue_create_infos[i].queueCount = 1;
                queue_create_infos[i].pQueuePriorities = &priority;
                graphics_queue_assigned = true;
                Log::log(INFO, "GRAPHICS queue supported -> added to queue_create_infos for this device");
                continue;
            }

            // Check for compute queue support
            static bool compute_queue_assigned = false;
            if (!compute_queue_assigned && (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                compute_queue_family_index = i;
                queue_create_infos.push_back({});
                queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_infos[i].queueFamilyIndex = i;
                queue_create_infos[i].queueCount = 1;
                queue_create_infos[i].pQueuePriorities = &priority;
                compute_queue_assigned = true;
                Log::log(INFO, "COMPUTE queue supported -> added to queue_create_infos for this device");
                continue;
            }

            // Check for transfer queue support
            static bool transfer_queue_assigned = false;
            if (!transfer_queue_assigned && (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT)) {
                transfer_queue_family_index = i;
                queue_create_infos.push_back({});
                queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_infos[i].queueFamilyIndex = i;
                queue_create_infos[i].queueCount = 1;
                queue_create_infos[i].pQueuePriorities = &priority;
                transfer_queue_assigned = true;
                Log::log(INFO, "TRANSFER queue supported -> added to queue_create_infos for this device");
                continue;
            }
        }

        // Create logical device
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.ppEnabledExtensionNames = device_extension_names_ptr;
        device_create_info.enabledExtensionCount = extension_count;
        device_create_info.pEnabledFeatures = &enabled_features;
        VkResult result = vkCreateDevice(physical, &device_create_info, nullptr, &logical);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "successfully created logical device (handle: ", logical, ")");
        }
        else{
            Log::log(ERROR, "Failed to create Vulkan logical device (VkResult=", result, ")");
        }

        // Acquire queue handles for this logical device
        if (graphics_queue == nullptr) {
            vkGetDeviceQueue(logical, graphics_queue_family_index, 0, &graphics_queue);
            Log::log(INFO, "adding graphics queue to logical device (handle: ", graphics_queue, ")");
        }

        if (compute_queue == nullptr) {
            vkGetDeviceQueue(logical, compute_queue_family_index, 0, &compute_queue);
            Log::log(INFO, "adding compute queue to logical device (handle: ", compute_queue, ")");
        }

        if (transfer_queue == nullptr) {
            vkGetDeviceQueue(logical, transfer_queue_family_index, 0, &transfer_queue);
            Log::log(INFO, "adding transfer queue to logical device (handle: ", transfer_queue, ")");
        }

        Log::log(INFO, "[DEVICE COMPLETED]");
    }

    // Move Constructor
    Device(Device&& other) noexcept : Device() {
        // Transfer ownership of Vulkan handles/resources using std::exchange;
        // std::move may not be supported with some Vulkan objects
        this->physical = std::exchange(other.physical, nullptr);
        this->logical = std::exchange(other.logical, nullptr);
        this->graphics_queue = std::exchange(other.graphics_queue, nullptr);
        this->compute_queue = std::exchange(other.compute_queue, nullptr);
        this->transfer_queue = std::exchange(other.transfer_queue, nullptr);
        this->graphics_queue_family_index = std::exchange(other.graphics_queue_family_index, 0);
        this->compute_queue_family_index = std::exchange(other.compute_queue_family_index, 0);
        this->transfer_queue_family_index = std::exchange(other.transfer_queue_family_index, 0);
        this->properties = std::exchange(other.properties, VkPhysicalDeviceProperties{});
        this->properties2 = std::exchange(other.properties2, VkPhysicalDeviceProperties2{});
    }


    // Copy Constructor
    Device(const Device& other) : Device() {
        this->physical = other.physical;
        this->logical = other.logical;
        this->graphics_queue = other.graphics_queue;
        this->compute_queue = other.compute_queue;
        this->transfer_queue = other.transfer_queue;
        this->graphics_queue_family_index = other.graphics_queue_family_index;
        this->compute_queue_family_index = other.compute_queue_family_index;
        this->transfer_queue_family_index = other.transfer_queue_family_index;
        this->properties = other.properties;
        this->properties2 = other.properties2;
        this->extension_count = other.extension_count;

        // Check if other device has valid extension names
        if (other.device_extension_names_ptr != nullptr) {
            // Copy extension names
            device_extension_names_ptr = new const char* [extension_count];
            for (uint32_t i = 0; i < extension_count; ++i) {
                size_t length = strlen(other.device_extension_names_ptr[i]) + 1;
                device_extension_names_ptr[i] = new char[length];
                strcpy_s(const_cast<char*>(device_extension_names_ptr[i]), length, other.device_extension_names_ptr[i]);
            }
        }
    }

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

    ~Device() {
        // destroy logical device
        if (logical != nullptr) {
            vkDeviceWaitIdle(logical);
            vkDestroyDevice(logical, nullptr);
            logical = nullptr;
            delete[] device_extension_names_ptr;
            Log::log(INFO, "[LOGICAL DEVICE DESTROYED]");
        }
    }

protected:
    VkPhysicalDevice physical = nullptr;
    VkDevice logical = nullptr;
    VkQueue graphics_queue = nullptr;
    VkQueue compute_queue = nullptr;
    VkQueue transfer_queue = nullptr;
    uint32_t graphics_queue_family_index = 0;
    uint32_t compute_queue_family_index = 0;
    uint32_t transfer_queue_family_index = 0;
    VkPhysicalDeviceProperties properties = {};
    VkPhysicalDeviceProperties2 properties2 = {};
    const char** device_extension_names_ptr = nullptr;
    uint32_t extension_count = 0;
};

class RenderPass {
public:
    // default constructor
    RenderPass() {}

    // parametric constructor
    RenderPass(Device& device, VkFormat format, QueueFamily usage) {
        this->logical = device.get_logical();

        if (renderpass == nullptr) {
            // setup attachment description
            attachment_descr.format = format;
            attachment_descr.samples = VK_SAMPLE_COUNT_1_BIT; // change for multisampling
            attachment_descr.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment_descr.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment_descr.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            // Adjust final layout based on usage
            if (usage == QueueFamily::GRAPHICS) {
                attachment_descr.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            else if (usage == QueueFamily::COMPUTE) {
                attachment_descr.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
            }
            else if (usage == QueueFamily::TRANSFER) {
                attachment_descr.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                // Additional setup for transfer if needed
            }

            // setup attachment reference
            attachment_ref.attachment = 0;
            if (usage == QueueFamily::GRAPHICS) {
                attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            else {
                attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
            }

            // setup subpass
            if (usage == QueueFamily::GRAPHICS) {
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            }
            else {
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            }
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &attachment_ref;

            // setup render pass details
            VkRenderPassCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            create_info.attachmentCount = 1;
            create_info.pAttachments = &attachment_descr;
            create_info.subpassCount = 1;
            create_info.pSubpasses = &subpass;
            vkCreateRenderPass(logical, &create_info, 0, &renderpass);
        }
    }

    // move constructor
    RenderPass(RenderPass&& other) noexcept : RenderPass() {
        // Transfer ownership of Vulkan handles/resources using std::exchange;
        // std::move may not be supported with Vulkan objects
        this->renderpass = std::exchange(other.renderpass, nullptr);
        this->attachment_descr = std::exchange(other.attachment_descr, VkAttachmentDescription{});
        this->attachment_ref = std::exchange(other.attachment_ref, VkAttachmentReference{});
        this->subpass = std::exchange(other.subpass, VkSubpassDescription{});
        this->logical = std::exchange(other.logical, nullptr);
        this->usage = std::exchange(other.usage, QueueFamily::UNKNOWN);
        this->format = std::exchange(other.format, VK_FORMAT_UNDEFINED);
    }

    // copy constructor
    RenderPass(RenderPass& other) : RenderPass() {
        this->renderpass = other.renderpass;
        this->attachment_descr = other.attachment_descr;
        this->attachment_ref = other.attachment_ref;
        this->subpass = other.subpass;
        this->logical = other.logical;
        this->usage = other.usage;
        this->format = other.format;
    }

    VkRenderPass& get() { return renderpass; }
    QueueFamily get_usage() const { return this->usage; }
    VkFormat get_format() const { return this->format; }

    ~RenderPass() {}

private:
    VkRenderPass renderpass = nullptr;
    VkAttachmentDescription attachment_descr = {};
    VkAttachmentReference attachment_ref = {};
    VkSubpassDescription subpass = {};
    VkDevice logical = nullptr;
    QueueFamily usage;
    VkFormat format;
};

class Surface {
public:
    // default constructor
    Surface() {}

    // parametric constructor
    Surface(Device& device) {

        // TODO: Implementation for GLFW or SDL2

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_physical(), surface, &capabilities);
        if (capabilities.currentExtent.width == 0xFFFFFFFF) {
            capabilities.currentExtent.width = capabilities.minImageExtent.width;
        }
        if (capabilities.currentExtent.height == 0xFFFFFFFF) {
            capabilities.currentExtent.height = capabilities.minImageExtent.height;
        }
    }

    // move constructor
    Surface(Surface&& other) noexcept : Surface() {
        // TODO
    }

    // copy constructor
    Surface(Surface& other) : Surface() {
        // TODO
    }

    // destructor
    ~Surface() {}

    VkSurfaceKHR* get() { return &surface; }

    VkSurfaceCapabilitiesKHR* get_capabilities() { return &capabilities; }

private:
    VkSurfaceKHR surface = nullptr;
    VkSurfaceCapabilitiesKHR capabilities = {};
};

class Swapchain {
public:
    Swapchain(Device& device, Surface& surface, VkImageUsageFlags& usage, RenderPass& renderpass, uint32_t min_image_count = 3, VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR) {
        this->logical = device.get_logical();
        VkBool32 supports_present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device.get_physical(), device.get_graphics_queue_family_index(), *surface.get(), &supports_present);
        if (!supports_present) {
            Log::log(ERROR, "graphics queue doesn't support present!");
        }
        width = surface.get_capabilities()->currentExtent.width;
        height = surface.get_capabilities()->currentExtent.height;

        // get available formats
        uint32_t num_formats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_physical(), *surface.get(), &num_formats, 0);
        std::vector<VkSurfaceFormatKHR> available_formats(num_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_physical(), *surface.get(), &num_formats, available_formats.data());
        if (num_formats == 0) {
            Log::log(ERROR, "no surface formats available!");
        }
        uint32_t selected_format = 0; // change if needed
        format = available_formats[selected_format].format;
        color_space = available_formats[selected_format].colorSpace;

        // setup swapchain details
        VkSwapchainCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = *surface.get();
        create_info.minImageCount = min_image_count;
        create_info.imageFormat = format;
        create_info.imageColorSpace = color_space;
        create_info.imageExtent = surface.get_capabilities()->currentExtent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = usage;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;

        // finalize swapchain
        vkCreateSwapchainKHR(logical, &create_info, nullptr, &swapchain);

        // create images
        vkGetSwapchainImagesKHR(logical, swapchain, &num_images, 0);
        image.resize(num_images);
        vkGetSwapchainImagesKHR(logical, swapchain, &num_images, image.data());

        // create image views
        image_view.resize(num_images);
        image_view_create_info.resize(num_images);
        for (uint32_t i = 0; i < num_images; i++) {
            image_view_create_info[i] = {};
            image_view_create_info[i].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info[i].image = image[i];
            image_view_create_info[i].viewType = view_type;
            image_view_create_info[i].format = format;
            image_view_create_info[i].components = { VK_COMPONENT_SWIZZLE_IDENTITY };
            image_view_create_info[i].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            vkCreateImageView(logical, &image_view_create_info[i], nullptr, &image_view[i]);
        }

        // create framebuffers
        framebuffer.resize(num_images);
        framebuffer_create_info.resize(num_images);
        for (uint32_t i = 0; i < num_images; i++) {
            framebuffer_create_info[i] = {};
            framebuffer_create_info[i].renderPass = renderpass.get();
            framebuffer_create_info[i].attachmentCount = 1;
            framebuffer_create_info[i].pAttachments = &image_view[i];
            framebuffer_create_info[i].width = width;
            framebuffer_create_info[i].height = height;
            framebuffer_create_info[i].layers = 1;
            framebuffer_create_info[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            vkCreateFramebuffer(logical, &framebuffer_create_info[i], 0, &framebuffer[i]);
        }
    }

    uint32_t get_width() const { return width; }
    uint32_t get_height() const { return height; }

    ~Swapchain() {
        for (uint32_t i = 0; i < num_images; i++) {
            vkDestroyFramebuffer(logical, framebuffer[i], nullptr);
            vkDestroyImageView(logical, image_view[i], nullptr);
        }
        framebuffer.clear();
        framebuffer_create_info.clear();
        image.clear();
        image_view.clear();
        image_view_create_info.clear();
        vkDestroySwapchainKHR(logical, swapchain, nullptr);
    }

private:
    uint32_t num_images = 0;
    std::vector<VkImage> image;
    std::vector<VkImageView> image_view;
    std::vector<VkImageViewCreateInfo> image_view_create_info;
    std::vector<VkFramebuffer> framebuffer;
    std::vector<VkFramebufferCreateInfo> framebuffer_create_info;
    VkSwapchainKHR swapchain = nullptr;
    uint32_t width;
    uint32_t height;
    VkFormat format;
    VkColorSpaceKHR color_space;
    VkDevice logical = nullptr;
};

class RenderAttachment {
public:
    RenderAttachment(VkImageView& image_view, VkImageLayout image_layout, VkAttachmentLoadOp load_op, VkAttachmentStoreOp store_op, VkClearValue clear_value) {
        attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachment.pNext = NULL;
        attachment.imageView = image_view;
        attachment.imageLayout = image_layout;
        attachment.resolveMode = VK_RESOLVE_MODE_NONE;
        attachment.loadOp = load_op;
        attachment.storeOp = store_op;
        attachment.clearValue = clear_value;
    }

    ~RenderAttachment() {}

    VkRenderingAttachmentInfo& get() { return attachment; }
private:
    VkRenderingAttachmentInfo attachment = {};
};

class FrameBuffer {
public:
    FrameBuffer(Device& device, Swapchain& swapchain, RenderPass& renderpass) {
        this->logical = device.get_logical();
        // TODO: implementation
    }
    
    ~FrameBuffer() {
        buffer.clear();
    }

    std::vector<VkFramebuffer>* get() { return &buffer; }
private:
    std::vector<VkFramebuffer> buffer;
    VkDevice logical = nullptr;
};

class CommandPool {
public:
    CommandPool(Device& device, QueueFamily usage) {
        this->logical = device.get_logical();
        this->usage = usage;

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
            Log::log(ERROR, "in CommandPool constructor: invalid QueueFamily argument!");
        }
        VkResult result = vkCreateCommandPool(logical, &create_info, nullptr, &pool);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "command pool created (handle: ", pool, ")");
        }
        else {
            Log::log(ERROR, "failed to create command pool (VkResult=", result, ")");
        }
    }

    void destroy() {
        if (pool != nullptr) {
            // destroy command pool
            Log::log(INFO, "CommandPool destructor: destroying command pool with handle ", pool);
            vkDestroyCommandPool(logical, pool, nullptr);
            pool = nullptr;
        }
    }

    ~CommandPool() {
        destroy();
    }

    void trim() {
        vkTrimCommandPool(logical, pool, NULL);
    }

    VkResult reset(VkCommandPoolResetFlags flags = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) {
        return vkResetCommandPool(logical, pool, flags);
    }

    VkCommandPool& get() { return pool; }
private:
    VkCommandPool pool = nullptr;
    QueueFamily usage;
    VkDevice logical = nullptr;
};

class VertexDescription {
public:
    VertexDescription(uint32_t dimensions) {
        this->dimensions = dimensions;
        // setup attribute description for location coordinates 
        location_attribute_description.binding = 0;
        location_attribute_description.location = 0;
        location_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
        location_attribute_description.offset = 0;

        // add to attribute list
        attribute_descriptions.push_back(location_attribute_description);

        // setup vertex input binding
        input_binding.binding = 0;
        input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        input_binding.stride += sizeof(float) * dimensions; // add one float per dimension
    }

    ~VertexDescription() {}

    void add_color_attribute() {
        color_attribute_description.binding = 0;
        color_attribute_description.location = 1;
        color_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
        color_attribute_description.offset = 0;

        // add to attribute list
        attribute_descriptions.push_back(color_attribute_description);

        // update stride distance (in bytes)
        input_binding.stride += sizeof(float) * 3; // add one float per color
    }

    std::vector<VkVertexInputAttributeDescription>& get_attribute_descriptions() {
        return attribute_descriptions;
    }

    VkVertexInputBindingDescription& get_input_binding() {
        return input_binding;
    }

    uint32_t get_attribute_descriptions_count() {
        return attribute_descriptions.size();
    }

    uint64_t get_size() const {
        return input_binding.stride;
    }

private:
    uint32_t dimensions = 0;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    VkVertexInputAttributeDescription color_attribute_description = {};
    VkVertexInputAttributeDescription location_attribute_description = {};
    VkVertexInputBindingDescription input_binding = {};
};

class ShaderModule {
public:
    ShaderModule(Device& device) {
        this->logical = device.get_logical();
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    }

    VkShaderModule& read_from_file(const char* filename) {
        std::string file_path = std::string(spirv_folder) + filename;
        long file_size = 0;
        FILE* file = fopen(file_path.c_str(), "rb");
        if (!file) {
            Log::log(ERROR, "shader file not found: ", filename);
        }
        else {
            Log::log(DEBUG, "reading shader file: ", file_path.c_str());
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            uint8_t* buffer = new uint8_t[file_size];
            fread(buffer, 1, file_size, file);

            shader_module_create_info.codeSize = file_size;
            shader_module_create_info.pCode = reinterpret_cast<uint32_t*>(buffer);

            // free old resource first in case a previous module exists
            if (module != nullptr) {
                Log::log(INFO, "destroying previous shader module");
                vkDestroyShaderModule(logical, module, nullptr);
            }

            // allocate new module
            VkResult result = vkCreateShaderModule(logical, &shader_module_create_info, nullptr, &module);
            if (result == VK_SUCCESS) {
                Log::log(DEBUG, "new shader module successfully created (handle: ", module, ")");
            }
            else {
                Log::log(ERROR, "failed to create shader module (VkResult = ", result, ")");
            }
            delete[] buffer;
            fclose(file);
        }
        return module;
    }

    VkShaderModule& get() { return module; }

    ~ShaderModule() {
        if (module != nullptr) {
            vkDestroyShaderModule(logical, module, nullptr);
        }
    }
private:
    VkShaderModule module = nullptr;
    VkDevice logical = nullptr;
    VkShaderModuleCreateInfo shader_module_create_info = {};
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
    uint32_t add_value(T value) {

        // update range size
        if (sizeof(T) % 4) {
            Log::log(WARNING, "in method PushConstants::add_value(T value): sizeof(T) must be a multiple of 4");
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
            this->add_value(i);
        }
        return range.size;
    }

    template<typename T>
    uint32_t add_values(std::vector<T>& new_data) {
        for (T i : new_data) {
            this->add_value(i);
        }
        return range.size;
    }

    uint32_t* get_data() { return data; }

    VkPushConstantRange& get_range() { return range; }

private:
    static constexpr float_t reserve = 0.5;
    static constexpr size_t min_capacity = 16; // min capacity in bytes (should be a multiple of 4)
    uint32_t* data = nullptr;
    size_t capacity = min_capacity;
    VkPushConstantRange range;
};

template<typename T>
class Buffer {
public:

    Buffer(Device& device, BufferUsage usage, uint32_t rows, uint32_t cols = 1, uint32_t depth = 1, VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        this->logical = device.get_logical();
        this->physical = device.get_physical();
        this->rows = rows;
        this->cols = cols;
        this->depth = depth;
        this->elements = rows * cols * depth;
        this->subspace_z = 1;
        this->subspace_y = this->depth;
        this->subspace_x = subspace_y * this->cols;
        this->size_bytes = this->elements * sizeof(T);
        this->usage = usage;
        this->memory_properties = memory_properties;

        // create buffer
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            buffer_create_info.size = size_bytes,
            buffer_create_info.usage = convertBufferUsage(usage);
        VkResult result = vkCreateBuffer(logical, &buffer_create_info, nullptr, &buffer);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "data buffer successfully created (handle: ", buffer, ")");
        }
        else {
            Log::log(WARNING, "failed to create data buffer, VkResult=", result);
        }

        // get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(logical, buffer, &memory_requirements);

        // find suitable memory type index
        uint32_t type_index = findMemoryType(memory_properties, memory_requirements.memoryTypeBits);
        if (type_index == UINT32_MAX) {
            Log::log(ERROR, "in constructor Buffer::Buffer(): no suitable memory type is available");
        }

        // allocate memory
        VkMemoryAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = type_index;
        vkAllocateMemory(logical, &allocate_info, nullptr, &memory);

        // bind memory to buffer
        vkBindBufferMemory(logical, buffer, memory, 0);
    }

    // copy constructor
    Buffer(const Buffer<T>& other) {

        copy_resources(other);

        // create buffer
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            buffer_create_info.size = size_bytes,
            buffer_create_info.usage = convertBufferUsage(usage);
        VkResult result = vkCreateBuffer(logical, &buffer_create_info, nullptr, &buffer);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "data buffer successfully created (handle: ", buffer, ")");
        }
        else {
            Log::log(WARNING, "failed to create data buffer, VkResult=", result);
        }

        // get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(logical, buffer, &memory_requirements);

        // find suitable memory type index
        uint32_t type_index = findMemoryType(memory_properties, memory_requirements.memoryTypeBits);
        if (type_index == UINT32_MAX) {
            Log::log(ERROR, "in constructor Buffer::Buffer(): no suitable memory type is available");
        }

        // allocate memory
        VkMemoryAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = type_index;
        vkAllocateMemory(logical, &allocate_info, nullptr, &memory);

        // bind memory to buffer
        vkBindBufferMemory(logical, buffer, memory, 0);
    }

    // move constructor
    Buffer(Buffer<T>&& other) noexcept {
        copy_resources(other);
        this->destroy();
        this->buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
        this->memory = std::exchange(other.memory, VK_NULL_HANDLE);
    }

    // copy assignment
    Buffer& operator=(const Buffer<T>& other) {
        if (this != &other) {
            // check if 'this' and 'other' buffers differ in dimensions
            if (
                this->size_bytes != other.size_bytes ||
                this->rows != other.rows ||
                this->cols != other.cols ||
                this->depth != other.depth
                ) {
                // free previous resources
                this->destroy();
                // apply copy constructor on same allocation, using the 'placement new operator'
                new (this) Buffer<T>(other);
            }
            // copy 'other' data buffer
            write(other);
        }
        return *this;
    }

    // move assignment
    Buffer& operator=(Buffer<T>&& other) noexcept {
        if (this != &other) {
            copy_resources(other);
            this->destroy();
            this->buffer = std::exchange(other.buffer, VK_NULL_HANDLE);
            this->memory = std::exchange(other.memory, VK_NULL_HANDLE);
        }
        return *this;
    }

    // free buffer resources
    void destroy() {
        if (buffer != VK_NULL_HANDLE) {
            Log::log(INFO, "executing Buffer destructor (buffer handle: ", buffer, ")");
            vkFreeMemory(logical, memory, nullptr);
            vkDestroyBuffer(logical, buffer, nullptr);
            buffer = VK_NULL_HANDLE;
        }
    }

    // destructor
    ~Buffer() {
        destroy();
    }

    // copy buffer memory from a std::vector
    void write(const std::vector<T>& buffer_data) {
        size_t vector_size_bytes = buffer_data.size() * sizeof(T);
        if (vector_size_bytes > this->size_bytes) {
            Log::log(WARNING, "in method Buffer::write(): the passed vector has ", vector_size_bytes,
                " bytes of data whilst the target buffer has space has an allocation size of ", this->size_bytes, " bytes");
        }
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* data;
        vkMapMemory(logical, memory, offset, size_bytes, flags, &data);
        memcpy(data, buffer_data.data(), std::min(size_bytes, vector_size_bytes));
        vkUnmapMemory(logical, memory);
    }

    // copy buffer memory from a std array
    void write(const T& buffer_data, size_t elements) {
        size_t source_size_bytes = elements * sizeof(T);
        if (source_size_bytes > this->size_bytes) {
            Log::log(WARNING, "in method Buffer::write(): the passed array has ", source_size_bytes,
                " bytes of data whilst the target buffer has space has an allocation size of ", this->size_bytes, " bytes");
        }
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* data;
        vkMapMemory(logical, memory, offset, size_bytes, flags, &data);
        memcpy(data, &buffer_data, std::min(size_bytes, source_size_bytes));
        vkUnmapMemory(logical, memory);
    }

    // copy buffer memory from other Buffer object
    void write(const Buffer<T>& other) {
        size_t other_elements = other.get_elements();
        if (other_elements > this->elements) {
            Log::log(WARNING, "in method Buffer::map_write(Buffer<T>& other): the passed source buffer has ", other_elements,
                " elements whilst the target buffer has space for ", this->elements, " data elements");
        }
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* source = nullptr;
        void* target = nullptr;
        vkMapMemory(logical, other.memory, offset, other_elements * sizeof(T), flags, &source);
        vkMapMemory(logical, this->memory, offset, size_bytes, flags, &target);
        memcpy(target, source, std::min(size_bytes, other_elements * sizeof(T)));
        vkUnmapMemory(logical, this->memory);
        vkUnmapMemory(logical, other.memory);
    }

    // return data from buffer memory as a std::vector<T>
    std::vector<T> read() {
        std::vector<T> result;
        result.resize(this->elements);
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* data;
        vkMapMemory(logical, memory, offset, size_bytes, flags, &data);
        memcpy(result.data(), data, size_bytes);
        vkUnmapMemory(logical, memory);
        return result;
    }

    // returns a single element from the data buffer
    T get(uint32_t index_x, uint32_t index_y = 0, uint32_t index_z = 0) const {
        if (index_x >= this->rows) {
            Log::log(ERROR, "in method Buffer::get(): index_x ", index_x, " is out of bounds (allowed indices: 0-", this->rows - 1, ")");
        }
        if (index_y >= this->cols) {
            Log::log(ERROR, "in method Buffer::get(): index_y ", index_y, " is out of bounds (allowed indices: 0-", this->cols - 1, ")");
        }
        if (index_z >= this->depth) {
            Log::log(ERROR, "in method Buffer::get(): index_z ", index_z, " is out of bounds (allowed indices: 0-", this->depth - 1, ")");
        }
        VkDeviceSize offset = (index_x * this->subspace_x + index_y * this->subspace_y + index_z) * sizeof(T);
        VkMemoryMapFlags flags = 0;
        void* data = nullptr;
        T result{};
        vkMapMemory(logical, memory, offset, sizeof(T), flags, &data);
        memcpy(&result, data, sizeof(T));
        vkUnmapMemory(logical, memory);
        return result;
    }

    // assigns a single data buffer element
    void set(T value, uint32_t index_x, uint32_t index_y = 0, uint32_t index_z = 0) {
        if (index_x >= this->rows) {
            Log::log(ERROR, "in method Buffer::get(): index_x ", index_x, " is out of bounds (allowed indices: 0-", this->rows - 1, ")");
        }
        if (index_y >= this->cols) {
            Log::log(ERROR, "in method Buffer::get(): index_y ", index_y, " is out of bounds (allowed indices: 0-", this->cols - 1, ")");
        }
        if (index_z >= this->depth) {
            Log::log(ERROR, "in method Buffer::get(): index_z ", index_z, " is out of bounds (allowed indices: 0-", this->depth - 1, ")");
        }
        VkDeviceSize offset = (index_x * this->subspace_x + index_y * this->subspace_y + index_z) * sizeof(T);
        VkMemoryMapFlags flags = 0;
        void* data = nullptr;
        vkMapMemory(logical, memory, offset, sizeof(T), flags, &data);
        memcpy(data, &value, sizeof(T));
        vkUnmapMemory(logical, memory);
    }

    void set_all(T value) {
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* data;
        vkMapMemory(logical, memory, offset, this->elements * sizeof(T), flags, &data);
        for (uint32_t i = 0; i < this->elements; i++) {
            offset = i * sizeof(T);
            memcpy((T*)data + size_t(offset), &value, sizeof(T));
        }
        vkUnmapMemory(logical, memory);
    }

    uint32_t get_elements() const {
        return this->elements;
    }

    VkDeviceMemory& get_memory() {
        return memory;
    }

    BufferUsage get_usage() const {
        return usage;
    }

    uint64_t get_size_bytes() const { return size_bytes; }

    uint32_t get_rows() const { return this->rows; }

    uint32_t get_cols() const { return this->cols; }

    uint32_t get_depth() const { return this->depth; }

    uint32_t get_subspace_x() const { return this->subspace_x; }

    uint32_t get_subspace_y() const { return this->subspace_y; }

    uint32_t get_subspace_z() const { return this->subspace_z; }

    VkBuffer& get() { return buffer; }

    void set(VkBuffer& source_buffer) {
        this buffer = source_buffer;
    }

    std::vector<std::vector<std::vector<T>>> as_vector() {
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* data = nullptr;
        vkMapMemory(logical, memory, offset, this->size_bytes, flags, &data);

        std::vector<std::vector<std::vector<T>>> result;

        result.resize(this->rows);
        for (uint32_t x = 0; x < rows; x++) {
            result[x].resize(this->cols);
            for (uint32_t y = 0; y < this->cols; y++) {
                result[x][y].resize(this->depth);
                memcpy(result[x][y].data(), (T*)data + (x * subspace_x + y * subspace_y) * sizeof(T), this->depth * sizeof(T));
            }
        }

        vkUnmapMemory(logical, memory);
        return result;
    }

    void print(std::string comment="", std::string delimiter="|", bool with_indices=false, bool rows_inline=false, int32_t precision=3) const {
        uint32_t decimals = std::pow(10, precision);
        std::cout << comment;
        if (comment != "") {
            std::cout << "\n";
        }
        uint32_t dimensions = depth > 1 ? 3 : cols > 1 ? 2 : rows > 0 ? 1 : 0;

        if (dimensions == 1 && rows_inline) {
            for (uint32_t x = 0; x < this->rows; x++) {
                if (with_indices) {
                    std::cout << "[" << x << "]=";
                }
                std::cout << this->get(x, 0, 0);
                if (x != this->rows - 1) {
                    std::cout << delimiter;
                }
            }
            std::cout << "\n" << std::flush;
        }
        else {
            for (uint32_t x = 0; x < this->rows; x++) {
                for (uint32_t y = 0; y < this->cols; y++) {
                    if (this->depth == 1) {
                        if (with_indices) {
                            if (dimensions == 1) {
                                std::cout << "[" << x << "]=";
                            }
                            else {
                                std::cout << "[" << x << "][" << y << "]=";
                            }
                        }
                        std::cout << (precision >= 0 ? std::round(this->get(x, y, 0) * decimals) / decimals : this->get(x, y, 0));
                        // add delimiter before next column
                        if (y != this->cols - 1) {
                            std::cout << delimiter;
                        }
                    }
                    else {
                        std::cout << "{";
                        for (uint32_t z = 0; z < this->depth; z++) {
                            if (with_indices) {
                                std::cout << "[" << x << "][" << y << "][" << z << "]=";
                            }
                            std::cout << (precision >= 0 ? std::round(this->get(x, y, z) * decimals) / decimals : this->get(x, y, z));
                            if (z != this->depth - 1) {
                                std::cout << delimiter;
                            }
                        }
                        std::cout << "}";
                        // add space before next column
                        if (y != this->cols - 1) {
                            std::cout << " ";
                        }
                    }
                }
                // add line break before next row
                std::cout << "\n";
            }
            // flush to console
            std::cout << std::flush;
        }
    }
protected:
    VkBufferUsageFlags convertBufferUsage(BufferUsage usage) {
        switch (usage) {
        case BufferUsage::VERTEX: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferUsage::INDEX: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BufferUsage::STORAGE: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BufferUsage::UNIFORM: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default: Log::log(ERROR, "in method Buffer::Buffer(): invalid BufferUsage argument");
            return 0;
        }
    }

    // helper function for move or copy operations
    void copy_resources(const Buffer<T>& other) {
        this->logical = other.logical;
        this->physical = other.physical;
        this->rows = other.rows;
        this->cols = other.cols;
        this->depth = other.depth;
        this->elements = other.elements;
        this->subspace_z = 1;
        this->subspace_y = other.depth;
        this->subspace_x = other.subspace_x;
        this->size_bytes = other.size_bytes;
        this->usage = other.usage;
        this->memory_properties = other.memory_properties;
    }

    // Helper function to find a suitable memory type index
    uint32_t findMemoryType(VkMemoryPropertyFlags memory_properties, uint32_t memory_type_bits) {
        VkPhysicalDeviceMemoryProperties device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical, &device_memory_properties);

        Log::log(INFO, "searching for buffer memory types (requested: ", memory_properties, ")");
        for (uint32_t i = 0; i < device_memory_properties.memoryTypeCount; i++) {
            Log::log(DEBUG, "memory type ", i, ": ", device_memory_properties.memoryTypes[i].propertyFlags);
            if ((memory_type_bits & (1 << i)) && (device_memory_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) {
                Log::log(INFO, "[SUCCESS]");
                return i;
            }
        }
        Log::log(WARNING, "in helper function findMemoryType for Buffer() constructor: no suitable memory type found");
        return UINT32_MAX;
    }

    VkBuffer buffer = nullptr;
    VkDeviceMemory memory = nullptr;
    uint32_t elements = 0;
    VkDevice logical = nullptr;
    VkPhysicalDevice physical = nullptr;
    uint64_t size_bytes = 0;
    uint32_t rows = 0;
    uint32_t cols = 0;
    uint32_t depth = 0;
    uint32_t subspace_x = 0;
    uint32_t subspace_y = 0;
    uint32_t subspace_z = 0;
    BufferUsage usage;
    VkMemoryPropertyFlags memory_properties = NULL;
};

class DescriptorPool {
    friend class DescriptorSet;
public:
    DescriptorPool(Device& device, uint32_t max_sets = 10) {
        this->logical = device.get_logical();
        this->max_sets = max_sets;

        // specify the number of descriptors per type that this pool can supply
        std::vector<VkDescriptorPoolSize> pool_sizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_storage_buffers_per_pool},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, max_dynamic_storage_buffers_per_pool},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, max_uniform_buffers_per_pool},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, max_dynamic_uniform_buffers_per_pool}
        }; // add more types if needed

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
            Log::log(DEBUG, "successfully created descriptor pool (handle: ", pool, ")");
        }
        else {
            Log::log(ERROR, "failed to create descriptor pool (VkResult =  ", result, ")");
        }
    }

    ~DescriptorPool() {
        if (pool != nullptr && pool) {
            vkDestroyDescriptorPool(logical, pool, nullptr);
            sets.clear();
        }
    }

    VkDescriptorPool& get() {
        return pool;
    }

    std::vector<VkDescriptorSet>& get_sets() { return sets; }

    uint32_t get_max_sets() const { return max_sets; }

    uint32_t remove_set(uint32_t set_index) {
        sets.erase(sets.begin() + set_index);
        return sets.size();
    }

private:
    uint32_t add_set(VkDescriptorSet& descriptor_set) {
        uint32_t index = sets.size();
        Log::log(DEBUG, "adding new descriptor set (set index = ", index, ") to descriptor pool (pool handle: ", pool, ")");
        sets.push_back(descriptor_set);
        return index;
    }

    VkDescriptorPool pool = nullptr;
    VkDevice logical = nullptr;
    std::vector<VkDescriptorSet> sets;
    uint32_t max_sets = 0;
};

// holds binding information for shader resources
class DescriptorSet {
public:
    DescriptorSet(Device& device, DescriptorPool& descriptor_pool, std::vector<DescriptorType>& descriptor_types) {
        this->logical = device.get_logical();
        this->pool = &descriptor_pool;
        this->bindings_count = descriptor_types.size();

        // convert types
        types.resize(bindings_count);
        for (uint32_t i = 0; i < bindings_count; i++) {
            if (descriptor_types[i] == DescriptorType::STORAGE_BUFFER) {
                types[i] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            }
            else if (descriptor_types[i] == DescriptorType::UNIFORM_BUFFER) {
                types[i] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            }
            else if (descriptor_types[i] == DescriptorType::SAMPLED_IMAGE) {
                types[i] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            }
            else if (descriptor_types[i] == DescriptorType::STORAGE_IMAGE) {
                types[i] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            }
            else {
                Log::log(ERROR, "in descriptor set constructor: invalid descriptor type for binding ", i);
            }
        }

        // setup bindings
        binding.resize(bindings_count);
        for (uint32_t i = 0; i < bindings_count; i++) {
            binding[i] = {};
            binding[i].binding = i;
            binding[i].descriptorType = types[i];
            binding[i].descriptorCount = 1;
            binding[i].stageFlags = VK_SHADER_STAGE_ALL; // = all shader stages can access the resource
            binding[i].pImmutableSamplers = nullptr;
        }

        // setup layout
        VkDescriptorSetLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.pNext = NULL;
        layout_create_info.flags = NULL; // VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        layout_create_info.bindingCount = bindings_count;
        layout_create_info.pBindings = binding.data();

        VkResult result = vkCreateDescriptorSetLayout(logical, &layout_create_info, nullptr, &layout);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "descriptor set layout created (", bindings_count, " bindings, layout handle : ", layout, ")");
        }
        else {
            Log::log(ERROR, "failed to create descriptor set layout (VkResult ", result, ")");
        }

        // allocate descriptor set from pool
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = pool->get();
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &layout;

        result = vkAllocateDescriptorSets(logical, &allocateInfo, &descriptor_set);
        if (result == VK_SUCCESS) {
            Log::log(DEBUG, "allocated descriptor set (set index: ", this->index, ", set handle: ",
                descriptor_set, ") from descriptor pool ", pool->get(), ", ", bindings_count, " bindings reserved");
        }
        else {
            Log::log(ERROR, "failed to allocate descriptor set (VkResult ", result, ")");
        }

        // get the index of this set
        this->index = descriptor_pool.add_set(descriptor_set);
    }

    void free() {
        if (descriptor_set != nullptr) {
            VkResult result = vkFreeDescriptorSets(logical, pool->get(), 1, &descriptor_set);
            if (result == VK_SUCCESS) {
                Log::log(DEBUG, "descriptor set ", descriptor_set, " memory allocation freed");
            }
            else {
                Log::log(WARNING, "failed to free descriptor set ", descriptor_set, " (VkResult = ", result, ")");
            }
            Log::log(DEBUG, "removing descriptor set ", descriptor_set, " (set index: ", this->index, ") from pool");
            uint32_t remaining = pool->remove_set(this->index);
            Log::log(DEBUG, "remaining sets in pool: ", remaining);
            descriptor_set = nullptr;
        }
        if (layout != nullptr) {
            vkDestroyDescriptorSetLayout(logical, layout, nullptr);
            layout = nullptr;
        }
    }

    ~DescriptorSet() {
        free();
    }

    VkDescriptorSetLayout& get_layout() {
        return layout;
    }

    VkDescriptorSet& get() {
        return descriptor_set;
    }

    // binds a data buffer to the descriptor set;
    // returns the binding slot index
    template<typename T>
    void bind_buffer(Buffer<T>& buffer, uint32_t binding_index) {
        if (buffer.get_usage() == BufferUsage::STORAGE && binding[binding_index].descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            Log::log(ERROR, "invalid attempt to bind a storage buffer to a binding of different type");
        }
        else if (buffer.get_usage() == BufferUsage::UNIFORM && binding[binding_index].descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            Log::log(ERROR, "invalid attempt to bind a uniform buffer to a binding of different type)");
        }
        Log::log(DEBUG, "binding buffer ", buffer.get(), " to descriptor set ", index, " (handle: ", descriptor_set, ") at binding ", binding_index);
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.get();
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        
        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.pNext = NULL;
        descriptor_write.dstSet = descriptor_set;
        descriptor_write.dstBinding = binding_index;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = binding[binding_index].descriptorType;
        descriptor_write.pImageInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;
        descriptor_write.pBufferInfo = &buffer_info;
        

        vkUpdateDescriptorSets(logical, 1, &descriptor_write, 0, nullptr);
    }

    // returns the index of the descriptor set within the pool
    uint32_t get_index() const { return index; }

protected:
    std::vector<VkDescriptorSetLayoutBinding> binding;
    std::vector<VkDescriptorType> types;
    VkDescriptorSet descriptor_set = nullptr;
    VkDescriptorSetLayout layout = nullptr;
    DescriptorPool* pool = nullptr;
    VkDevice logical = nullptr;
    uint32_t index = 0;
    uint32_t bindings_count = 0;
};

class GraphicsPipeline {
public:
    GraphicsPipeline(Device& device, RenderPass& renderpass, Swapchain& swapchain, VertexDescription& vertex_description, ShaderModule& vertex_shader_module, ShaderModule& fragment_shader_module, PushConstants push_constants, DescriptorSet& descriptor_set) {
        this->logical = device.get_logical();

        // setup shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_info;
        if (vertex_shader_module.get() != nullptr) {
            uint32_t i = shader_stage_create_info.size();
            shader_stage_create_info.push_back({});
            shader_stage_create_info[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shader_stage_create_info[i].module = vertex_shader_module.get();
            shader_stage_create_info[i].pName = "main";
        }

        if (fragment_shader_module.get() != nullptr) {
            uint32_t i = shader_stage_create_info.size();
            shader_stage_create_info.push_back({});
            shader_stage_create_info[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info[i].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shader_stage_create_info[i].module = fragment_shader_module.get();
            shader_stage_create_info[i].pName = "main";
        }

        // setup vertex input state
        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
        vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_description.get_input_binding();
        vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_description.get_attribute_descriptions_count();
        vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_description.get_attribute_descriptions().data();

        // setup input assembly state
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {};
        input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

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
        VkRect2D scissor = { {0,0}, {swapchain.get_width(), swapchain.get_height()}};
        viewport_state_create_info.pScissors = &scissor;

        // setup rasterization state
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
        rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;       
        rasterization_state_create_info.lineWidth = 1.0f;

        // setup pipeline layout
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount = 1;
        layout_create_info.pSetLayouts = &descriptor_set.get_layout();
        layout_create_info.pushConstantRangeCount = 1;
        layout_create_info.pPushConstantRanges = &push_constants.get_range();
        vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &layout);

        // setup multisample state
        VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
        multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // setup color blend state
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
        color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.attachmentCount = 1;
        color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

        // finalize graphics pipeline
        VkGraphicsPipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.stageCount = shader_stage_create_info.size();
        pipeline_create_info.pStages = shader_stage_create_info.data();
        pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
        pipeline_create_info.pViewportState = &viewport_state_create_info;
        pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
        pipeline_create_info.pMultisampleState = &multisample_state_create_info;
        pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
        pipeline_create_info.layout = layout;
        pipeline_create_info.renderPass = renderpass.get();
        pipeline_create_info.subpass = 0;
        VkResult result = vkCreateGraphicsPipelines(logical, 0, 1, &pipeline_create_info, nullptr, &pipeline);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "graphics pipeline successfully created");
        }
        else {
            Log::log(ERROR, "failed to create graphics pipeline (VkResult=", result, ")");
        }
    }

    VkPipeline& get() { return pipeline; }

    VkPipelineLayout& get_layout() { return layout; }

    ~GraphicsPipeline() {
        Log::log(INFO, "destroying graphics pipeline");
        vkDestroyPipeline(logical, pipeline, nullptr);
        vkDestroyPipelineLayout(logical, layout, nullptr);
    }

private:
    VkPipeline pipeline = nullptr;
    VkPipelineLayout layout = nullptr;
    VkDevice logical = nullptr;
    VkViewport viewport = {};
    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
};

class ComputePipeline {
public:

    // constructor with push constants
    ComputePipeline(Device& device, ShaderModule& compute_shader_module, PushConstants& push_constants, DescriptorSet& descriptor_set) {
        this->logical = device.get_logical();
        this->set = &descriptor_set;
        this->constants = &push_constants;

        // setup pipeline layout        
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount = 1; // = number of descriptor sets
        layout_create_info.pSetLayouts = &descriptor_set.get_layout();
        layout_create_info.pushConstantRangeCount = 1;
        layout_create_info.pPushConstantRanges = &push_constants.get_range();
        layout_create_info.pNext = NULL;
        VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &layout);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "created pipeline layout for compute pipeline (handle: ", layout, ")");
        }
        else {
            Log::log(ERROR, "failed to create compute pipeline layout (VkResult=", result, ")");
        }

        // setup shader stage
        VkPipelineShaderStageCreateInfo shader_stage_create_info = {};
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.pNext = NULL;
        shader_stage_create_info.flags = NULL;
        shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shader_stage_create_info.module = compute_shader_module.get();
        shader_stage_create_info.pName = "main";

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
            Log::log(INFO, "compute pipeline successfully created (handle: ", pipeline, ")");
        }
        else {
            Log::log(ERROR, "failed to create compute pipeline (VkResult=", result, ")");
        }
    }

    // constructor without push constants
    ComputePipeline(Device& device, ShaderModule& compute_shader_module, DescriptorSet& descriptor_set) {
        this->logical = device.get_logical();
        this->set = &descriptor_set;

        // setup pipeline layout        
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount = 1; // = number of descriptor sets
        layout_create_info.pSetLayouts = &descriptor_set.get_layout();
        layout_create_info.pushConstantRangeCount = 0;
        layout_create_info.pNext = NULL;
        VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &layout);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "created pipeline layout for compute pipeline (handle: ", layout, ")");
        }
        else {
            Log::log(ERROR, "failed to create compute pipeline layout (VkResult=", result, ")");
        }

        // setup shader stage
        VkPipelineShaderStageCreateInfo shader_stage_create_info = {};
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.pNext = NULL;
        shader_stage_create_info.flags = NULL;
        shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shader_stage_create_info.module = compute_shader_module.get();
        shader_stage_create_info.pName = "main";

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
            Log::log(INFO, "compute pipeline successfully created (handle: ", pipeline, ")");
        }
        else {
            Log::log(ERROR, "failed to create compute pipeline (VkResult=", result, ")");
        }
    }

    void destroy() {
        if (pipeline != nullptr) {
            Log::log(INFO, "destroying compute pipeline");
            vkDestroyPipeline(logical, pipeline, nullptr);
            pipeline = nullptr;
        }
        if (layout != nullptr) {
            Log::log(INFO, "destroying pipeline layout");
            vkDestroyPipelineLayout(logical, layout, nullptr);
            layout = nullptr;
        }
    }

    ~ComputePipeline() {
        destroy();
    }

    VkPipeline& get() { return pipeline; }

    VkPipelineLayout& get_layout() { return layout; }

    DescriptorSet* get_set() { return set; }

    PushConstants* get_constants() { return constants; }

private:
    VkPipeline pipeline = nullptr;
    VkPipelineLayout layout = nullptr;
    VkDevice logical = nullptr;
    DescriptorSet* set = nullptr;
    PushConstants* constants = nullptr;

};

class TransferPipeline {
public:
    TransferPipeline(Device& device, RenderPass& renderpass, PushConstants& push_constants, DescriptorSet& descriptor_set) {
        this->logical = device.get_logical();

        // Create a pipeline layout for the transfer pipeline
        descriptor_set_layout = descriptor_set.get_layout();
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;;
        pipeline_layout_create_info.pushConstantRangeCount = 1;
        pipeline_layout_create_info.pPushConstantRanges = &push_constants.get_range();

        vkCreatePipelineLayout(logical, &pipeline_layout_create_info, nullptr, &layout);

        // Finalize the transfer pipeline
        VkPipeline pipeline = nullptr;
        VkGraphicsPipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.layout = layout;
        pipeline_create_info.renderPass = renderpass.get();
        pipeline_create_info.subpass = 0;

        VkResult result = vkCreateGraphicsPipelines(logical, 0, 1, &pipeline_create_info, nullptr, &pipeline);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "transfer pipeline successfully created");
        }
        else {
            Log::log(ERROR, "failed to create transfer pipeline (VkResult=", result, ")");
        }
    }

    VkPipeline& get() { return pipeline; }

    ~TransferPipeline() {
        Log::log(INFO, "destroying transfer pipeline");
        vkDestroyPipeline(logical, pipeline, nullptr);
        vkDestroyPipelineLayout(logical, layout, nullptr);
    }

private:
    VkPipeline pipeline = nullptr;
    VkPipelineLayout layout = nullptr;
    VkDevice logical = nullptr;
    VkDescriptorSetLayout descriptor_set_layout = nullptr;
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

    VkResult wait(uint64_t timeout_nanosec = 1000000) {
        return vkWaitForFences(logical, 1, &fence, VK_TRUE, timeout_nanosec);
    }

    VkFence& get() { return fence; }
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

    VkResult wait(uint64_t timeout_nanosec = 1000000000) {
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

    VkSemaphore& get() { return semaphore; }

private:
    VkSemaphore semaphore = nullptr;
    VkDevice logical = nullptr;
    VkSemaphoreType type;
    VkSemaphoreWaitInfo wait_info = {};
    VkSemaphoreSignalInfo signal_info = {};
};

class Event {
public:
    Event(Device& device) {
        this->logical = device.get_logical();
        VkEventCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
        vkCreateEvent(logical, &create_info, nullptr, &event);
    }

    ~Event() {
        vkDestroyEvent(logical, event, nullptr);
    }

    bool signaled() {
        return vkGetEventStatus(logical, event) == VK_EVENT_SET;
    }

    VkResult set() {
        return vkSetEvent(logical, event);
    }

    VkResult reset() {
        return vkResetEvent(logical, event);
    }

    void signal(CommandBuffer& command_buffer, VkDependencyFlags flags = VK_DEPENDENCY_VIEW_LOCAL_BIT); // defined later, outside class, because it depends on CommandBuffer::get(), which has to be defined first

    VkEvent& get() { return event; }

private:
    VkEvent event = nullptr;
    VkDevice logical = nullptr;
    VkDependencyInfo dependency_info = {};
};

class CommandBuffer {
public:
    // default constructor
    CommandBuffer() {}

    // parametric constructor
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
            Log::log(INFO, "[OLD COMMAND BUFFER DESTROYED]");
        }

        // setup command buffer
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = pool.get();
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = 1;
        VkResult result = vkAllocateCommandBuffers(logical, &allocate_info, &buffer);
        if (result == VK_SUCCESS) {
            Log::log(INFO, "successfully allocated command buffer (handle: ", buffer, ")");
        }
        else {
            Log::log(WARNING, "in CommandBuffer constructor: memory allocation failed (VkResult=", result, ")!");
        }
        
        begin_recording();
    }

    void destroy() {
        if (buffer != nullptr) {
            vkFreeCommandBuffers(logical, pool, 1, &buffer);
            Log::log(INFO, "[COMMAND BUFFER DESTROYED]");
            buffer = nullptr;
        }
    }
    // destructor
    ~CommandBuffer() {
        destroy();
    }

    void reset(VkCommandBufferResetFlags flags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) {
        VkResult result = vkResetCommandBuffer(buffer, flags);
        if (result == VK_SUCCESS) {
            Log::log(DEBUG, "successfully reset command buffer");
        }
        else {
            Log::log(WARNING, "failed to reset command buffer (handle: ", buffer, ", VkResult = ", result, ")");
        }
        begin_recording();
    }

    void set_event(Event& event, VkPipelineStageFlags& stage_mask) {
        vkCmdSetEvent(buffer, event.get(), stage_mask);
    }

    void reset_event(Event& event, VkPipelineStageFlags& stage_mask) {
        vkCmdResetEvent(buffer, event.get(), stage_mask);
    }

    void wait_event(Event& event, VkPipelineStageFlags& src_stage_mask, VkPipelineStageFlags& dst_stage_mask) {
        vkCmdWaitEvents(buffer, 1, &event.get(), src_stage_mask, dst_stage_mask, 0, nullptr, 0, nullptr, 0, nullptr);
    }

    void bind_pipeline(GraphicsPipeline& pipeline) {
        if (usage != QueueFamily::GRAPHICS) {
            Log::log(ERROR, "invalid usage of CommandBuffer::bind_pipeline(): this command buffer doesn't support graphics");
        }
        if (pipeline.get() != nullptr) {
            vkCmdBindPipeline(buffer, bind_point, pipeline.get());
        }
        else {
            Log::log(ERROR, "CommandBuffer::bind_pipeline() has invalid pipeline argument");
        }
    }

    void bind_pipeline(ComputePipeline& pipeline) {
        if (usage != QueueFamily::COMPUTE) {
            Log::log(ERROR, "invalid usage of CommandBuffer::bind_pipeline(): this command buffer doesn't support compute");
        }
        if (pipeline.get() != nullptr) {
            Log::log(DEBUG, "binding pipeline ", pipeline.get(), " to bindpoint type ", bind_point, " at command buffer ", buffer);
            vkCmdBindPipeline(buffer, bind_point, pipeline.get());
        }
        else {
            Log::log(ERROR, "CommandBuffer::bind_pipeline() has invalid pipeline argument");
        }
        pipeline_layout = pipeline.get_layout();
    }

    void bind_descriptor_set(DescriptorSet& set) {
        if (pipeline_layout == nullptr) {
            Log::log(ERROR, "invalid usage of CommandBuffer::bind_descriptor_set(): please use CommandBuffer::bind_pipeline() first!");
        }
        Log::log(DEBUG, "binding descriptor sets to command buffer, bindpoint ", bind_point);
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

    template<typename T>
    void copy_buffer(Buffer<T>& src_buffer, Buffer<T>& dst_buffer, uint32_t& size_bytes, uint32_t src_offset=0, uint32_t dst_offset=0) {
        copy_region.srcOffset = src_offset;
        copy_region.dstOffset = dst_offset;
        copy_region.size = size_bytes;
        vkCmdCopyBuffer(buffer, src_buffer.get(), dst_buffer.get(), 1, &copy_region);
    }

    void draw(uint32_t& vertex_count, uint32_t instance_count=1, uint32_t first_vertex=0, uint32_t first_instance=0) {
        vkCmdDraw(buffer, vertex_count, instance_count, first_vertex, first_instance);
    }

    void dispatch(uint32_t rows, uint32_t cols = 1, uint32_t depth = 1, uint32_t workgroup_size = 32) {
        // dispatch for compute
        if (usage == QueueFamily::COMPUTE) {
            const uint32_t workgroups_x = (rows + workgroup_size - 1) / workgroup_size;
            const uint32_t workgroups_y = (cols + workgroup_size - 1) / workgroup_size;
            const uint32_t workgroups_z = (depth + workgroup_size - 1) / workgroup_size;
            vkCmdDispatch(buffer, workgroups_x, workgroups_y, workgroups_z);
        }
        else {
            Log::warning("invalid call of method CommandBuffer::dispatch, only allowed for usage type QueueFamily::COMPUTE");
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

    void next_subpass() {
        vkCmdNextSubpass(buffer, VK_SUBPASS_CONTENTS_INLINE);
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

    /// shorthand for bind_pipeline -> push_constants -> dispatch -> submit;
    void compute(ComputePipeline& pipeline, uint32_t items_x, uint32_t items_y = 1, uint32_t items_z = 1, uint32_t workgroup_size = 32, bool fenced = true) {
        bind_pipeline(pipeline);
        bind_descriptor_set(*pipeline.get_set());
        if (pipeline.get_constants() != nullptr) {
            push_constants(*pipeline.get_constants());
        }
        dispatch(items_x, items_y, items_z, workgroup_size);
        if (fenced) {
            Fence fence(*device,false);
            submit(fence);
            while (!fence.signaled()) {
                fence.wait(10000);
            };
        }
        else {
            submit();
        }
        reset();
    }

protected:

    // start command buffer recording state
    void begin_recording() {
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pNext = NULL;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // specifies that each recording of the command buffer will only be submitted once, and the command buffer will be reset and recorded again between each submission
        begin_info.pInheritanceInfo = nullptr; // pointer to a VkCommandBufferInheritanceInfo struct; only relevant for secondary command buffers
        VkResult result = vkBeginCommandBuffer(buffer, &begin_info);
        if (result == VK_SUCCESS) {
            Log::log(DEBUG, "beginning command buffer recording state");
        }
        else {
            Log::log(WARNING, "failed to begin command buffer recording state (VkResult = ", result, ")");
        }
    }

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
    VkBufferCopy copy_region = {};
    VkRenderingInfo rendering_info = {};
    VkRenderPassBeginInfo renderpass_begin_info = {};
    VkSubpassBeginInfo subpass_begin_info = {};
    VkSubmitInfo submit_info = {};
    VkCommandPool pool = nullptr;
};

// definition outside class (because it depends on CommandBuffer::get(), which needs to be defined first)
void Event::signal(CommandBuffer& command_buffer, VkDependencyFlags flags) {
    dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependency_info.pNext = NULL;
    dependency_info.dependencyFlags = flags;
    dependency_info.memoryBarrierCount = 0;
    dependency_info.pMemoryBarriers = nullptr;
    dependency_info.imageMemoryBarrierCount = 0;
    dependency_info.pImageMemoryBarriers = nullptr;
    vkCmdSetEvent2(command_buffer.get(), event, &dependency_info);
}

// shared instance and device manager as singleton class
class VkManager {
public:
    static VkManager* make_singleton(std::vector<const char*>& instance_layer_names,
                                                     std::vector<const char*>& instance_extension_names,
                                                     std::vector<const char*> device_extension_names,
                                                     char* application_name = "",
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
        Log::log(DEBUG, "creating new graphics command pool");
        command_pool_graphics = new CommandPool(*device, QueueFamily::GRAPHICS);
        Log::log(DEBUG, "creating new compute command pool");
        command_pool_compute = new CommandPool(*device, QueueFamily::COMPUTE);
        Log::log(DEBUG, "creating new transfer command pool");
        command_pool_transfer = new CommandPool(*device, QueueFamily::TRANSFER);
    }

    // private custom destructor method
    static void destroy_singleton() {
        if (shared != nullptr) {
            Log::log(DEBUG, "singleton manager destructor invoked");
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