#ifndef VKCONTEXT_H
#define VKCONTEXT_H

#include "log.h"
#include <cstdint>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <utility>
#include <vector>
#include <vulkan_core.h>
#include <boost/type_traits.hpp>

enum QueueFamily {
    GRAPHICS,
    COMPUTE,
    TRANSFER,
    UNDEFINED
};

enum BufferUsage {
    VERTEX,
    STORAGE,
    UNIFORM,
    INDEX
};

class Instance {
public:

    Instance() {};

    ~Instance() {
        if (instance != nullptr) {
            delete[] enabled_layer_names_ptr;
            delete[] enabled_extension_names_ptr;
            vkDestroyInstance(instance, nullptr);
        }
    }
    void init_application(const char* name = "Vulkan Application", uint32_t major_version = 1, uint32_t minor_version = 0, uint32_t patch_version = 0) {
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = name;
        application_info.applicationVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    }

    void init_engine(const char* name = "", uint32_t major_version = 0, uint32_t minor_version = 0, uint32_t patch_version = 0) {
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pEngineName = name;
        application_info.engineVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    }

    void init_api_version(uint32_t version = VK_API_VERSION_1_2) {
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.apiVersion = version;
    }

    void init_layers(const std::vector<const char*> enabled_layer_names) {
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        uint32_t layer_count = enabled_layer_names.size();
        enabled_layer_names_ptr = new const char* [layer_count];
        for (uint32_t i = 0; i < layer_count; ++i) {
            enabled_layer_names_ptr[i] = enabled_layer_names[i];
        }
        instance_create_info.enabledLayerCount = layer_count;
        instance_create_info.ppEnabledLayerNames = enabled_layer_names_ptr;
        delete[] enabled_layer_names_ptr;
    }

    void init_extensions(const std::vector<const char*> enabled_extension_names) {
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        uint32_t extension_count = enabled_extension_names.size();
        enabled_extension_names_ptr = new const char* [extension_count];
        for (uint32_t i = 0; i < extension_count; ++i) {
            enabled_extension_names_ptr[i] = enabled_extension_names[i];
        }
        instance_create_info.enabledExtensionCount = extension_count;
        instance_create_info.ppEnabledExtensionNames = enabled_extension_names_ptr;
    }


    void create(VkInstanceCreateFlags flags = 0) {
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pNext = nullptr;
        instance_create_info.flags = flags;
        instance_create_info.pApplicationInfo = &application_info;

        if (instance_create_info.enabledLayerCount == 0) {
            Log::log(LOG_LEVEL_WARNING, "in method 'Instance::create()': no layers enabled for Vulkan instance");
        }

        if (instance_create_info.enabledExtensionCount == 0) {
            Log::log(LOG_LEVEL_WARNING, "in method 'Instance::create()': no extensions enabled for Vulkan instance; make sure none are required!");
        }

        VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
        if (result != VK_SUCCESS) {
            Log::log(LOG_LEVEL_ERROR, "Failed to create Vulkan Instance (VkResult=", result, ")");
        }
        Log::info("Vulkan instance successfully created.");
    }

    VkInstance get() const {
        return instance;
    }


private:
    VkInstance instance;
    VkApplicationInfo application_info = {};
    VkInstanceCreateInfo instance_create_info = {};
    const char** enabled_extension_names_ptr;
    const char** enabled_layer_names_ptr;
};

class Queue {
public:
    Queue(){}
    ~Queue(){};
    void wait_idle() const {vkQueueWaitIdle(queue);}
    uint32_t family_index = UINT32_MAX;
    uint32_t index = UINT32_MAX;
    VkQueue queue = nullptr;
    QueueFamily usage = QueueFamily::UNDEFINED;
    float priority = 1.0f;
};

class Device {
public:
    Device(){};

    Device(Instance instance, uint32_t id = 0, VkPhysicalDeviceFeatures enabled_features = {}, const std::vector<const char*> enabled_extensions = {}) {
        // confirm valid instance
        if (instance.get() == nullptr) {
            Log::error("Invalid call to Device constructor: create Vulkan instance first!");
        }

        // search for devices with Vulkan support
        uint32_t num_devices = 0;
        vkEnumeratePhysicalDevices(instance.get(), &num_devices, NULL);
        if (num_devices == 0) {
            Log::warning("No device(s) with Vulkan support found!");
            physical = VK_NULL_HANDLE;
            logical = VK_NULL_HANDLE;
            return;
        }
        else {
            // list available devices
            std::vector<VkPhysicalDevice> devices(num_devices);
            vkEnumeratePhysicalDevices(instance.get(), &num_devices, devices.data());
            Log::info("available physical devices with Vulkan support:");
            for (uint32_t i = 0; i < num_devices; i++) {
                vkGetPhysicalDeviceProperties(devices[i], &properties);
                Log::log(LOG_LEVEL_INFO, " -- name: ", properties.deviceName, ", ID: ", properties.deviceID, ", type: ", properties.deviceType);
            }
            // select physical device and write properties into member variable
            uint32_t selection = std::min(id, num_devices - 1);
            physical = devices[selection];
            Log::log(LOG_LEVEL_INFO, "Selected physical device ID: ", selection);
        }

        // get device properties
        vkGetPhysicalDeviceProperties(physical, &properties);
        vkGetPhysicalDeviceProperties2(physical, &properties2);

        // Queue creation
        uint32_t num_queue_families;
        std::vector<VkQueueFamilyProperties> queue_families;
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &num_queue_families, queue_families.data());
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        // Iterate over the queue families to find appropriate queue indices
        for (uint32_t i = 0; i < num_queue_families; ++i) {
            const VkQueueFamilyProperties& queue_family = queue_families[i];

            // Check for graphics queue support
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                if (graphics_queue.index == UINT32_MAX) {
                    graphics_queue.index = 0;
                    graphics_queue.family_index = i;
                    graphics_queue.priority = 1.0f;
                    graphics_queue.usage = QueueFamily::GRAPHICS;
                    queue_create_infos.push_back({});
                    queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queue_create_infos[i].queueFamilyIndex = i;
                    queue_create_infos[i].queueCount = 1;
                    queue_create_infos[i].pQueuePriorities = &graphics_queue.priority;
                    continue;
                }
            }

            // Check for compute queue support
            if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                if (compute_queue.index == UINT32_MAX) {
                    compute_queue.index = 0;
                    compute_queue.family_index = i;
                    compute_queue.priority = 1.0f;
                    compute_queue.usage = QueueFamily::COMPUTE;
                    queue_create_infos.push_back({});
                    queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queue_create_infos[i].queueFamilyIndex = i;
                    queue_create_infos[i].queueCount = 1;
                    queue_create_infos[i].pQueuePriorities = &compute_queue.priority;
                    continue;
                }
            }

            // Check for transfer queue support
            if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                if (transfer_queue.index == UINT32_MAX) {
                    transfer_queue.index = 0;
                    transfer_queue.family_index = i;
                    transfer_queue.priority = 1.0f;
                    transfer_queue.usage = QueueFamily::TRANSFER;
                    queue_create_infos.push_back({});
                    queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queue_create_infos[i].queueFamilyIndex = i;
                    queue_create_infos[i].queueCount = 1;
                    queue_create_infos[i].pQueuePriorities = &transfer_queue.priority;
                    continue;
                }
            }
        }

        // Create logical device
        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.ppEnabledExtensionNames = enabled_extensions.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        device_create_info.pEnabledFeatures = &enabled_features;

        if (vkCreateDevice(physical, &device_create_info, nullptr, &logical) != VK_SUCCESS) {
            Log::log(LOG_LEVEL_ERROR, "Failed to create Vulkan logical device on physical device ", physical);
        }

        // Acquire queue handles for this logical device
        if (graphics_queue.index != UINT32_MAX) {
            vkGetDeviceQueue(logical, graphics_queue.family_index, graphics_queue.index, &(graphics_queue.queue));
        }
        if (compute_queue.index != UINT32_MAX) {
            vkGetDeviceQueue(logical, compute_queue.family_index, compute_queue.index, &(compute_queue.queue));
        }
        if (transfer_queue.index != UINT32_MAX) {
            vkGetDeviceQueue(logical, transfer_queue.family_index, transfer_queue.index, &(transfer_queue.queue));
        }
    }

    ~Device() {
        vkDeviceWaitIdle(logical);
        vkDestroyDevice(logical, nullptr);
    }

    VkPhysicalDeviceProperties properties = {};
    VkPhysicalDeviceProperties2 properties2 = {};
    VkPhysicalDevice physical = nullptr;
    VkDevice logical = nullptr;
    Queue graphics_queue, compute_queue, transfer_queue;
};

class RenderPass {
public:
    RenderPass() {};

    RenderPass(Device device, VkFormat format, QueueFamily usage) {
        this->device = device;
        // setup attachment description
        VkAttachmentDescription attachment_descr = {};
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
        VkAttachmentReference attachment_ref = {};
        attachment_ref.attachment = 0;
        if (usage == QueueFamily::GRAPHICS) {
            attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        else {
            attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
        }

        // setup subpass
        VkSubpassDescription subpass = {};
        if (usage == QueueFamily::GRAPHICS) {
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        }
        else {
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        }
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachment_ref;

        // setup render pass details
        VkRenderPassCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &attachment_descr;
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        vkCreateRenderPass(device.logical, &create_info, 0, &renderpass);
    }

    ~RenderPass() {
        vkDestroyRenderPass(device.logical, renderpass, nullptr);
    }

    VkRenderPass renderpass;

private:
    Device device;
};

class Swapchain {
public:
    Swapchain() {};

    Swapchain(Device device, VkSurfaceKHR surface, VkImageUsageFlags usage, RenderPass renderpass, uint32_t min_image_count = 3, VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR) {
        this->device = device;
        this->renderpass = renderpass;
        VkBool32 supports_present = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device.physical, device.graphics_queue.family_index, surface, &supports_present);
        if (!supports_present) {
            Log::error("graphics queue doesn't support present!");
        }

        // get surface capabilities
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical, surface, &surface_capabilities);
        if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
            surface_capabilities.currentExtent.width = surface_capabilities.minImageExtent.width;
        }
        if (surface_capabilities.currentExtent.height == 0xFFFFFFFF) {
            surface_capabilities.currentExtent.height = surface_capabilities.minImageExtent.height;
        }
        width = surface_capabilities.currentExtent.width;
        height = surface_capabilities.currentExtent.height;

        // get available formats
        uint32_t num_formats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device.physical, surface, &num_formats, 0);
        std::vector<VkSurfaceFormatKHR> available_formats(num_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device.physical, surface, &num_formats, available_formats.data());
        if (num_formats == 0) {
            Log::error("no surface formats available!");
        }
        uint32_t selected_format = 0; // change if needed
        format = available_formats[selected_format].format;
        VkColorSpaceKHR color_space = available_formats[selected_format].colorSpace;

        // setup swapchain details
        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = surface;
        create_info.minImageCount = min_image_count;
        create_info.imageFormat = format;
        create_info.imageColorSpace = color_space;
        create_info.imageExtent = surface_capabilities.currentExtent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = usage;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;

        // finalize swapchain
        vkCreateSwapchainKHR(device.logical, &create_info, nullptr, &swapchain);

        // create images
        vkGetSwapchainImagesKHR(device.logical, swapchain, &num_images, 0);
        image.resize(num_images);
        vkGetSwapchainImagesKHR(device.logical, swapchain, &num_images, image.data());

        // create image views
        image_view.resize(num_images);
        for (uint32_t i = 0; i < num_images; i++) {
            VkImageViewCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = image[i];
            create_info.viewType = view_type;
            create_info.format = format;
            create_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
            create_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            vkCreateImageView(device.logical, &create_info, nullptr, &image_view[i]);
        }

        // create framebuffers
        framebuffer.resize(num_images);
        for (uint32_t i = 0; i < num_images; i++) {
            VkFramebufferCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            create_info.renderPass = renderpass.renderpass;
            create_info.attachmentCount = 1;
            create_info.pAttachments = &image_view[i];
            create_info.width = width;
            create_info.height = height;
            create_info.layers = 1;
            vkCreateFramebuffer(device.logical, &create_info, 0, &framebuffer[i]);
        }
    }

    ~Swapchain() {
        for (uint32_t i = 0; i < num_images; i++) {
            vkDestroyFramebuffer(device.logical, framebuffer[i], nullptr);
            vkDestroyImageView(device.logical, image_view[i], nullptr);
        }
        framebuffer.clear();
        vkDestroySwapchainKHR(device.logical, swapchain, nullptr);
    }

    uint32_t num_images = 0;
    std::vector<VkImage> image;
    std::vector<VkImageView> image_view;
    std::vector<VkFramebuffer> framebuffer;
    VkSwapchainKHR swapchain;
    uint32_t width;
    uint32_t height;
    VkFormat format;
private:
    Device device;
    RenderPass renderpass;
};

class RenderAttachment {
public:
    RenderAttachment(){}
    RenderAttachment(VkImageView image_view, VkImageLayout image_layout, VkAttachmentLoadOp load_op, VkAttachmentStoreOp store_op, VkClearValue clear_value) {
        attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachment.pNext = NULL;
        attachment.imageView = image_view;
        attachment.imageLayout = image_layout;
        attachment.resolveMode = VK_RESOLVE_MODE_NONE;
        attachment.loadOp = load_op;
        attachment.storeOp = store_op;
        attachment.clearValue = clear_value;
    }
    VkRenderingAttachmentInfo attachment = {};
private:
};

class FrameBuffer {
public:
    FrameBuffer(Device device, Swapchain swapchain, RenderPass renderpass);
    ~FrameBuffer();
    std::vector<VkFramebuffer> buffer;
private:
    Device device;
};

class CommandPool {
public:
    CommandPool() {}

    CommandPool(Device device, QueueFamily usage) {
        this->device = device;
        this->usage = usage;

        // setup command pool
        VkCommandPoolCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        if (usage == QueueFamily::GRAPHICS) {
            create_info.queueFamilyIndex = device.graphics_queue.family_index;
        }
        else if (usage == QueueFamily::COMPUTE) {
            create_info.queueFamilyIndex = device.compute_queue.family_index;
        }
        else if (usage == QueueFamily::TRANSFER) {
            create_info.queueFamilyIndex = device.transfer_queue.family_index;
        }
        else {
            Log::error("in CommandPool constructor: invalid QueueFamily argument!");
        }
        vkCreateCommandPool(device.logical, &create_info, nullptr, &pool);
    }

    ~CommandPool() {
        vkDestroyCommandPool(device.logical, pool, nullptr);
    }

    void trim() {
        vkTrimCommandPool(device.logical, pool, NULL);
    }

    VkResult reset(VkCommandPoolResetFlags flags = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) {
        return vkResetCommandPool(device.logical, pool, flags);
    }

    VkCommandPool pool;
    QueueFamily usage;
    Device device;

private:
};

class VertexDescription {
public:
    VertexDescription() {}

    VertexDescription(uint32_t dimensions) {
        this->dimensions = dimensions;
        // setup attribute description for location coordinates
        VkVertexInputAttributeDescription location_attribute_description = {};
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
        if (has_color) {
            Log::warning("a color attribute has already been added to this vertex description");
            return;
        }
        // setup color attribute
        VkVertexInputAttributeDescription color_attribute_description = {};
        color_attribute_description.binding = 0;
        color_attribute_description.location = 1;
        color_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
        color_attribute_description.offset = 0;

        // add to attribute list
        attribute_descriptions.push_back(color_attribute_description);

        // update stride distance (in bytes)
        input_binding.stride += sizeof(float) * 3; // add one float per color
    }

    std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() const {
        return attribute_descriptions;
    }

    VkVertexInputBindingDescription get_input_binding() const {
        return input_binding;
    }

    uint32_t get_attribute_descriptions_count() const {
        return attribute_descriptions.size();
    }

    uint64_t get_size() const {
        return input_binding.stride;
    }

private:
    uint32_t dimensions = 0;
    bool has_color = false;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    VkVertexInputBindingDescription input_binding = {};
};

class GraphicsPipeline {
public:
    GraphicsPipeline() {}

    GraphicsPipeline(Device device, RenderPass renderpass, Swapchain swapchain, VertexDescription vertex_description, VkShaderModule vertex_shader_module, VkShaderModule fragment_shader_module, uint32_t push_constants_count) {
        this->device = device;

        std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_info;

        // setup shader stages
        {

            if (vertex_shader_module != nullptr) {
                shader_stage_create_info.push_back({});
                shader_stage_create_info[shader_stage_create_info.size() - 1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stage_create_info[shader_stage_create_info.size() - 1].stage = VK_SHADER_STAGE_VERTEX_BIT;
                shader_stage_create_info[shader_stage_create_info.size() - 1].module = vertex_shader_module;
                shader_stage_create_info[shader_stage_create_info.size() - 1].pName = "main";
            }

            if (fragment_shader_module != nullptr) {
                shader_stage_create_info.push_back({});
                shader_stage_create_info[shader_stage_create_info.size() - 1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stage_create_info[shader_stage_create_info.size() - 1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shader_stage_create_info[shader_stage_create_info.size() - 1].module = fragment_shader_module;
                shader_stage_create_info[shader_stage_create_info.size() - 1].pName = "main";
            }
        }

        // setup vertex input state
        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
        vertex_input_state_create_info.pVertexBindingDescriptions = &(vertex_description.get_input_binding());
        vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_description.get_attribute_descriptions_count();
        vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_description.get_attribute_descriptions().data();

        // setup input assembly state
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
        input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // setup viewport state
        VkPipelineViewportStateCreateInfo viewport_state = {};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        VkViewport viewport = { 0.0f, 0.0f, (float)swapchain.width, (float)swapchain.height };
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        VkRect2D scissor = { {0,0}, {swapchain.width, swapchain.height} };
        viewport_state.pScissors = &scissor;

        // setup rasterization state
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
        rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.lineWidth = 1.0f;

        // setup push constants memory
        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_ALL;
        push_constant_range.offset = 0;
        push_constant_range.size = push_constants_count * 4;

        // setup pipeline layout
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.pushConstantRangeCount = push_constants_count;
        layout_create_info.pPushConstantRanges = &push_constant_range;
        vkCreatePipelineLayout(device.logical, &layout_create_info, nullptr, &layout);

        // setup multisample state
        VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
        multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // setup color blend state
        VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blend_state = {};
        color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state.attachmentCount = 1;
        color_blend_state.pAttachments = &color_blend_attachment_state;

        // finalize graphics pipeline
        VkGraphicsPipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.stageCount = shader_stage_create_info.size();
        pipeline_create_info.pStages = shader_stage_create_info.data();
        pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_state;
        pipeline_create_info.pViewportState = &viewport_state;
        pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
        pipeline_create_info.pMultisampleState = &multisample_state_create_info;
        pipeline_create_info.pColorBlendState = &color_blend_state;
        pipeline_create_info.layout = layout;
        pipeline_create_info.renderPass = renderpass.renderpass;
        pipeline_create_info.subpass = 0;
        vkCreateGraphicsPipelines(device.logical, 0, 1, &pipeline_create_info, nullptr, &pipeline);
    }

    ~GraphicsPipeline() {
        vkDestroyPipeline(device.logical, pipeline, nullptr);
        vkDestroyPipelineLayout(device.logical, layout, nullptr);
    }

    VkPipeline pipeline;
    VkPipelineLayout layout;
private:
    Device device;
};

class ComputePipeline {
public:
    ComputePipeline() {}

    ComputePipeline(Device device, VkShaderModule compute_shader_module, uint32_t push_constants_count) {
        this->device = device;

        // setup push constants memory
        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_ALL;
        push_constant_range.offset = 0;
        push_constant_range.size = push_constants_count * 4;

        // setup pipeline layout
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.pushConstantRangeCount = push_constants_count;
        layout_create_info.pPushConstantRanges = &push_constant_range;
        vkCreatePipelineLayout(device.logical, &layout_create_info, nullptr, &layout);

        // setup shader stage
        VkPipelineShaderStageCreateInfo shader_stage_create_info = {};
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shader_stage_create_info.module = compute_shader_module;
        shader_stage_create_info.pName = "main";

        // finalize compute pipeline
        VkComputePipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_create_info.pNext = NULL;
        pipeline_create_info.flags = 0;
        pipeline_create_info.stage = shader_stage_create_info;
        pipeline_create_info.layout = layout;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        vkCreateComputePipelines(device.logical, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline);
    }

    ~ComputePipeline() {
        vkDestroyPipeline(device.logical, pipeline, nullptr);
        vkDestroyPipelineLayout(device.logical, layout, nullptr);
    }

    VkPipeline pipeline = nullptr;
    VkPipelineLayout layout = nullptr;

private:
    Device device;
};

template<typename T>
class Buffer {
public:
    Buffer(){}

    Buffer(Device device, BufferUsage usage, uint32_t num_elements, VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        this->device = device;
        this->num_elements = num_elements;
        this->size_bytes = num_elements * sizeof(T);

        // create buffer
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = size_bytes;
        if (usage == BufferUsage::VERTEX) {
            buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        else if (usage == BufferUsage::INDEX) {
            buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        else if (usage == BufferUsage::STORAGE) {
            buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
        else if (usage == BufferUsage::UNIFORM) {
            buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        else {
            Log::log(LOG_LEVEL_ERROR, "in method Buffer::Buffer(): invalid BufferUsage argument");
        }
        vkCreateBuffer(device.logical, &buffer_create_info, nullptr, &buffer);

        // get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device.logical, buffer, &memory_requirements);

        // get memory type index
        VkPhysicalDeviceMemoryProperties device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties(device.physical, &device_memory_properties);
        uint32_t type_index = UINT32_MAX;
        uint32_t type_filter = memory_properties;
        ;
        for (uint32_t i = 0; i < device_memory_properties.memoryTypeCount; i++) {
            // check if required memory type is allowed
            if (type_filter & (1 << i)) {
                // check if required properties are satisfied
                if ((device_memory_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) {
                    type_index = i;
                    break;
                }
            }
        }
        if (type_index == UINT32_MAX) {
            Log::error("in constructor Buffer::Buffer(): no suitable memory type is unavailable");
        }

        // allocate memory
        VkMemoryAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = type_index;
        vkAllocateMemory(device.logical, &allocate_info, nullptr, &memory);

        // bind memory to buffer
        VkDeviceSize memory_offset = 0;
        vkBindBufferMemory(device.logical, buffer, memory, memory_offset);
    }

    ~Buffer() {
        vkDestroyBuffer(device.logical, buffer, nullptr);
        vkFreeMemory(device.logical, memory, nullptr);
    }

    void map_memory(std::vector<T> buffer_data) {
        size_t vector_elements = buffer_data.size();
        size_t required = size_bytes / sizeof(T);
        if (vector_elements != required) {
            Log::log(LOG_LEVEL_WARNING, "in method Buffer::map_memory(): the passed vector has ", vector_elements,
                " elements whilst the target buffer is supposed to have ", num_elements, " data elements");
        }
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* data;
        vkMapMemory(device.logical, memory, offset, size_bytes, flags, &data);
        memcpy(data, buffer_data.data(), std::min(size_bytes, vector_elements*sizeof(T)));
        vkUnmapMemory(device.logical, memory);
    }

    void set(uint32_t flattened_index, T value) {
        if (index >= num_elements) {
            Log::log(LOG_LEVEL_ERROR, "in method Buffer::set(): index out of bounds: ", index);
            return;
        }
        VkDeviceSize offset = flattened_index * sizeof(T);
        VkMemoryMapFlags flags = 0;
        void* data;
        vkMapMemory(device.logical, memory, offset, sizeof(T), flags, &data);
        memcpy(data, &value, sizeof(T));
        vkUnmapMemory(device.logical, memory);
    }

    uint32_t get_num_elements() const {
        return num_elements;
    }

    VkBuffer buffer = nullptr;
    VkDeviceMemory memory = nullptr;
private:
    uint32_t num_elements=0;
    Device device;
    uint64_t size_bytes=0;
};

class Fence {
public:
    Fence() {}

    Fence(Device device, bool signaled = false) {
        this->device = device;
        VkFenceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.pNext = NULL;
        if (signaled) {
            create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }
        else {
            create_info.flags = 0;
        }
        vkCreateFence(device.logical, &create_info, nullptr, &fence);
    }

    ~Fence() {
        vkDestroyFence(device.logical, fence, nullptr);
    }

    bool active() {
        return vkGetFenceStatus(device.logical, fence) == VK_SUCCESS;
    }

    VkResult reset() {
        return vkResetFences(device.logical, 1, &fence);
    }

    VkResult wait(uint64_t timeout_nanosec = 1000000000) {
        return vkWaitForFences(device.logical, 1, &fence, VK_TRUE, timeout_nanosec);
    }

    VkFence fence;
private:
    Device device;
};

class Semaphore {
public:
    Semaphore() {}

    Semaphore(Device device, VkSemaphoreType type = VK_SEMAPHORE_TYPE_BINARY, uint64_t initial_value = 0) {
        this->device = device;
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
        vkCreateSemaphore(device.logical, &create_info, nullptr, &semaphore);
    }

    ~Semaphore() {
        vkDestroySemaphore(device.logical, semaphore, nullptr);
    }

    VkResult wait(uint64_t timeout_nanosec = 1000000000) {
        VkSemaphoreWaitInfo wait_info = {};
        wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        wait_info.pNext = NULL;
        wait_info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &semaphore;
        return vkWaitSemaphores(device.logical, &wait_info, timeout_nanosec);
    }

    uint64_t counter() {
        uint64_t value; 
        vkGetSemaphoreCounterValue(device.logical, semaphore, &value);
        return value;
    }

    void signal(uint64_t value) {
        VkSemaphoreSignalInfo signal_info = {};
        signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signal_info.pNext = NULL;
        signal_info.semaphore = semaphore;
        signal_info.value = value;
        vkSignalSemaphore(device.logical, &signal_info);
    }

    VkSemaphore semaphore;

private:
    Device device;
    VkSemaphoreType type;
};

class Event {
public:
    Event() {}

    Event(Device device) {
        this->device = device;
        VkEventCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
        vkCreateEvent(device.logical, &create_info, nullptr, &event);
    }

    ~Event() {
        vkDestroyEvent(device.logical, event, nullptr);
    }

    bool signaled() {
        return vkGetEventStatus(device.logical, event) == VK_EVENT_SET;
    }

    VkResult set() {
        return vkSetEvent(device.logical, event);
    }

    VkResult reset() {
        return vkResetEvent(device.logical, event);
    }

    void signal(VkCommandBuffer command_buffer, VkDependencyFlags flags = VK_DEPENDENCY_VIEW_LOCAL_BIT) {
        VkDependencyInfo dependency_info = {};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.pNext = NULL;
        dependency_info.dependencyFlags = flags;
        dependency_info.memoryBarrierCount = 0;
        dependency_info.pMemoryBarriers = nullptr;
        dependency_info.imageMemoryBarrierCount = 0;
        dependency_info.pImageMemoryBarriers = nullptr;
        vkCmdSetEvent2(command_buffer, event, &dependency_info);
    }

    VkEvent event;
private:
    Device device;
};

class CommandBuffer {
public:
    CommandBuffer(){}

    CommandBuffer(Device device, QueueFamily usage, CommandPool pool) {
        this->device = device;
        this->usage = usage;
        this->pool = pool;

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
            Log::error("in CommandPool::CommandBuffer constructor: invalid QueueFamily argument!");
        }
        // setup command buffer
        VkCommandBufferAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = pool.pool;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = 1;
        VkResult result = vkAllocateCommandBuffers(device.logical, &allocate_info, &buffer);
        if (result != VK_SUCCESS) {
            Log::log(LOG_LEVEL_WARNING, "in CommandBuffer constructor: memory allocation failed (VkResult=", result, ")!");
        }

        // start command buffer recording state
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pNext = NULL;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // specifies that each recording of the command buffer will only be submitted once, and the command buffer will be reset and recorded again between each submission
        begin_info.pInheritanceInfo = nullptr; // pointer to a VkCommandBufferInheritanceInfo struct; only relevant for secondary command buffers
        vkBeginCommandBuffer(buffer, &begin_info);
    }

    ~CommandBuffer() {
        vkFreeCommandBuffers(device.logical, pool.pool, 1, &buffer);
    }

    template<typename T>
    void bind_data_buffer(Buffer<T> data_buffer) {
        VkDeviceSize offset = 0;
        if (data_buffer.get_usage() == BufferUsage::VERTEX){
            vkCmdBindVertexBuffers(this->buffer, 0, 1, &data_buffer.buffer, &offset);
        }
        else if (data_buffer.get_usage() == BufferUsage::INDEX) {
            vkCmdBindIndexBuffer(this->buffer, data_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        }
    }

    VkResult reset(VkCommandBufferResetFlags flags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) {
        vkResetCommandBuffer(buffer, flags);
    }

    void set_event(Event event, VkPipelineStageFlags stage_mask) {
        vkCmdSetEvent(buffer, event.event, stage_mask);
    }

    void reset_event(Event event, VkPipelineStageFlags stage_mask) {
        vkCmdResetEvent(buffer, event.event, stage_mask);
    }

    void wait_event(Event event, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask) {vkCmdWaitEvents(buffer, 1, &(event.event), src_stage_mask, dst_stage_mask, 0, nullptr, 0, nullptr, 0, nullptr);}

    void bind_pipeline(GraphicsPipeline pipeline) {
        if (pipeline.pipeline != nullptr) {
            vkCmdBindPipeline(buffer, bind_point, pipeline.pipeline);
        }
    }

    void bind_pipeline(ComputePipeline pipeline) {
        if (pipeline.pipeline != nullptr) {
            vkCmdBindPipeline(buffer, bind_point, pipeline.pipeline);
        }
    }

    template<typename T>
    void push_constants(std::vector<T>& data, uint32_t offset=0) {
        uint32_t size = data.size();
        if (boost::is_floating_point<T>::value && T != float) {
            for (T& value : data) {
                value = (float)value;
            }
        }
        else if (boost::is_integral<T>::value && T != uint32_t) {
            for (T& value : data) {
                value = (uint32_t)value;
            }
        }
        else if (boost::is_boolean<T>::value && T != bool32_t) {
            for (T& value : data) {
                value = (bool32_t)value;
            }
        }
        else {
            Log::log(LOG_LEVEL_WARNING, "in method CommandBuffer::push_constants(): data vector is non-integer, non-floating_point and non-boolean; please make sure it's a 32bit type");
        }
        vkCmdPushConstants(buffer, pipeline_layout, VK_SHADER_STAGE_ALL, offset, size, data.data());
    }

    template<typename T>
    void copy_buffer(Buffer<T> src_buffer, Buffer<T> dst_buffer, uint32_t size_bytes, uint32_t src_offset=0, uint32_t dst_offset=0) {
        VkBufferCopy copy_region = {};
        copy_region.srcOffset = src_offset;
        copy_region.dstOffset = dst_offset;
        copy_region.size = size_bytes;
        vkCmdCopyBuffer(buffer, src_buffer.buffer, dst_buffer.buffer, 1, &copy_region);
    }

    void draw(uint32_t vertex_count, uint32_t instance_count=1, uint32_t first_vertex=0, uint32_t first_instance=0) {
        vkCmdDraw(buffer, vertex_count, instance_count, first_vertex, first_instance);
    }

    void dispatch(uint32_t workgroups_x = 4, uint32_t workgroups_y = 4, uint32_t workgroups_z = 1) {
        // dispatch for compute
        if (usage == QueueFamily::COMPUTE) {
            vkCmdDispatch(buffer, workgroups_x, workgroups_y, workgroups_z);
        }
        else {
            Log::warning("invalid call of method CommandBuffer::dispatch, only allowed for usage type QueueFamily::COMPUTE");
        }
    }

    void begin_render(VkOffset2D offset, VkExtent2D extent, VkRenderingFlags flags, std::vector<VkRenderingAttachmentInfo> color_attachments, VkRenderingAttachmentInfo depth_attachment, VkRenderingAttachmentInfo stencil_attachment) {
        VkRenderingInfo rendering_info = {};
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

    void begin_renderpass(RenderPass renderpass, VkOffset2D offset, VkExtent2D extent, std::vector<VkClearValue> clear_value) {
        VkRenderPassBeginInfo renderpass_begin_info = {};
        renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.pNext = NULL;
        renderpass_begin_info.renderPass = renderpass.renderpass;
        renderpass_begin_info.renderArea = { offset, extent }; // VkRect2D
        renderpass_begin_info.clearValueCount = clear_value.size();
        renderpass_begin_info.pClearValues = clear_value.data();

        VkSubpassBeginInfo subpass_begin_info = {};
        subpass_begin_info.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
        subpass_begin_info.pNext = NULL;
        subpass_begin_info.contents = VK_SUBPASS_CONTENTS_INLINE;

        vkCmdBeginRenderPass2(buffer, &renderpass_begin_info, &subpass_begin_info);
    }

    void end_renderpass() { vkCmdEndRenderPass(buffer); }

    void next_subpass() { vkCmdNextSubpass(buffer, VK_SUBPASS_CONTENTS_INLINE); }

    void submit(VkPipeline pipeline, VkFence fence = VK_NULL_HANDLE) {

        // stop command buffer recording state (thus triggering executable state)
        vkEndCommandBuffer(buffer);

        // submit to queue (triggers command buffer pending state)
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pCommandBuffers = &buffer;
        submit_info.commandBufferCount = 1;

        VkQueue queue;
        if (usage == QueueFamily::GRAPHICS) { queue = device.graphics_queue.queue; }
        if (usage == QueueFamily::COMPUTE) { queue = device.compute_queue.queue; }
        if (usage == QueueFamily::TRANSFER) { queue = device.transfer_queue.queue; }

        vkQueueSubmit(queue, 1, &submit_info, fence);
    }

    VkCommandBuffer buffer;
    QueueFamily usage;
    VkPipelineBindPoint bind_point;
    VkPipelineLayout pipeline_layout;
    Device device;
    CommandPool pool;
 
private:
};


// standalone helper functions that are not part of any class:

VkShaderModule shader_module(Device device, const char* shader_code) {
    // setup create info
    VkShaderModuleCreateInfo shader_module_create_info = {};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.pCode = (uint32_t*)shader_code;
    shader_module_create_info.codeSize = strlen(shader_code) * sizeof(uint32_t);

    // create module
    VkShaderModule module;
    vkCreateShaderModule(device.logical, &shader_module_create_info, nullptr, &module);
    return module;
}

VkShaderModule shader_module_from_file(Device device, const char* filename) {
    // read shader file
    VkShaderModule result = {};
    FILE* file = fopen(filename, "rb");
    if (!file) {
        Log::log(LOG_LEVEL_ERROR, "shader file not found: ", filename);
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t* buffer = new uint8_t[file_size];
    fread(buffer, 1, file_size, file);

    // setup create info
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = file_size;
    create_info.pCode = (uint32_t*)buffer;

    // create module
    vkCreateShaderModule(device.logical, &create_info, nullptr, &result);

    delete[] buffer;
    fclose(file);
    return result;
}

#endif