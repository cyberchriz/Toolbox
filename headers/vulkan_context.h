#pragma once
#include <iostream>
#include <string.h>
#include <vector>
#include <vulkan.h>
#include "log.h"

namespace VulkanContext {

    class Instance {
    public:
        // public methods
        void init_application(const char* name = "", uint32_t major_version = 0, uint32_t minor_version = 0, uint32_t patch_version = 0);
        void init_engine(const char* name = "", uint32_t major_version = 0, uint32_t minor_version = 0, uint32_t patch_version = 0);
        void init_api_version(uint32_t version = VK_API_VERSION_1_2) {application_info.apiVersion = version; }
        void init_layers(const std::vector<const char*>& enabled_layer_names);
        void init_extensions(const std::vector<const char*>& enabled_extension_names);
        void create(const void* pNext = nullptr, VkInstanceCreateFlags flags = 0);
        VkInstance get() const { return instance; }
        Instance();
    private:
        ~Instance();
        VkInstance instance = nullptr;
        VkApplicationInfo application_info = {};
        VkInstanceCreateInfo instance_create_info = {};
    };

    enum QueueFamily {
        GRAPHICS,
        COMPUTE,
        TRANSFER,
        UNDEFINED
    };

    class Queue {
    public:
        Queue(){}
        uint32_t family_index = UINT32_MAX;
        uint32_t index = UINT32_MAX;
        VkQueue queue = nullptr;
        QueueFamily usage = QueueFamily::UNDEFINED;
        float priority = 1.0f;
        void wait_idle() const { vkQueueWaitIdle(queue); }
    private:
        ~Queue() {};
    };

    class Device {
    public:
        // deleting the default constructor
        Device() = delete;
        // parametric constructor
        Device(Instance instance, uint32_t id = 0, VkPhysicalDeviceFeatures enabled_features = {}, const std::vector<const char*>& enabled_extensions = {});
        // public member variables
        VkPhysicalDeviceProperties properties = {};
        VkPhysicalDeviceProperties2 properties2 = {};
        VkPhysicalDevice physical = nullptr;
        VkDevice logical = nullptr;
        Queue graphics_queue, compute_queue, transfer_queue;
    private:
        // Destructor
        ~Device();
    };

    class RenderPass {
        RenderPass() = delete;
        RenderPass(Device device, VkFormat format, QueueFamily usage = QueueFamily::GRAPHICS);
        VkRenderPass renderpass;
    private:
        ~RenderPass();
        Device device;
    };

    class Swapchain {
    public:
        Swapchain() = delete;
        Swapchain(Device device, VkSurfaceKHR surface, VkImageUsageFlags usage, RenderPass renderpass, uint32_t min_image_count = 3, VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR);
        uint32_t num_images = 0;
        std::vector<VkImage> image;
        std::vector<VkImageView> image_view;
        std::vector<VkFramebuffer> framebuffer;
        VkSwapchainKHR swapchain;
        uint32_t width;
        uint32_t height;
        VkFormat format;
    private:
        ~Swapchain();
        Device device;
    };

    class RenderAttachment {
    public:
        RenderAttachment() = delete;
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
        FrameBuffer() = delete;
        FrameBuffer(Device device, Swapchain swapchain, RenderPass renderpass);
        std::vector<VkFramebuffer> buffer;
    private:
        ~FrameBuffer();
        Device device;
    };

    class CommandBuffer {
    public:
        CommandBuffer() = delete;
        CommandBuffer(Device device, QueueFamily usage, CommandPool pool);
        void bind_vertex_buffer(VertexBuffer vertex_buffer);
        void bind_index_buffer(IndexBuffer index_buffer);
        VkResult reset(VkCommandBufferResetFlags flags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) { vkResetCommandBuffer(buffer, flags); }
        void set_event(Event event, VkPipelineStageFlags stage_mask);
        void reset_event(Event event, VkPipelineStageFlags stage_mask);
        void wait_event(Event event, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask) {vkCmdWaitEvents(buffer, 1, &(event.event), src_stage_mask, dst_stage_mask, 0, nullptr, 0, nullptr, 0, nullptr);}
        void bind_pipeline(GraphicsPipeline pipeline);
        void bind_pipeline(ComputePipeline pipeline);
        void dispatch(uint32_t workgroups_x = 4, uint32_t workgroups_y = 4, uint32_t workgroups_z = 1);
        void begin_render(VkOffset2D offset, VkExtent2D extent, VkRenderingFlags flags, std::vector<VkRenderingAttachmentInfo> color_attachments, VkRenderingAttachmentInfo depth_attachment, VkRenderingAttachmentInfo stencil_attachment);
        void begin_renderpass(RenderPass renderpass, VkOffset2D offset, VkExtent2D extent, std::vector<VkClearValue> clear_value);
        void end_renderpass() { vkCmdEndRenderPass(buffer); }
        void next_subpass() { vkCmdNextSubpass(buffer, VK_SUBPASS_CONTENTS_INLINE); }
        void submit(VkPipeline pipeline, VkFence fence = VK_NULL_HANDLE);
        VkCommandBuffer buffer;
        QueueFamily usage;
        VkPipelineBindPoint bind_point;
        Device device;
        VkCommandPool pool;
    private:
        ~CommandBuffer();
    };

    class CommandPool {
    public:
        CommandPool() = delete;
        CommandPool(Device device, QueueFamily usage = QueueFamily::GRAPHICS);
        void trim() { vkTrimCommandPool(device.logical, pool, NULL); }
        VkResult reset(VkCommandPoolResetFlags flags = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) { return vkResetCommandPool(device.logical, pool, flags); }
        VkCommandPool pool;
        QueueFamily usage;
        Device device;
    private:
        ~CommandPool();
    };

    class GraphicsPipeline {
    public:
        GraphicsPipeline() = delete;
        GraphicsPipeline(Device device, RenderPass renderpass, Swapchain swapchain, VertexDescription vertex_description, VkShaderModule vertex_shader_module, VkShaderModule fragment_shader_module);
        VkPipeline pipeline;
        VkPipelineLayout layout;
    private:
        ~GraphicsPipeline();
        Device device;
    };

    class ComputePipeline {
    public:
        ComputePipeline() = delete;
        ComputePipeline(Device device, VkShaderModule compute_shader_module);
        VkPipeline pipeline;
        VkPipelineLayout layout;
    private:
        ~ComputePipeline();
        Device device;
    };

    class VertexDescription {
        VertexDescription() = delete;
        VertexDescription(uint32_t dimensions);
        void add_color_attribute();
        std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() const { return attribute_descriptions; }
        VkVertexInputBindingDescription get_input_binding() const { return input_binding; }
        uint32_t get_attribute_description_count() const { return attribute_descriptions.size(); }
        uint64_t get_size() const { return input_binding.stride; }
    private:
        ~VertexDescription(){}
        uint32_t dimensions = 0;
        bool has_color = false;
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        VkVertexInputBindingDescription input_binding = {};
    };

    class VertexBuffer {
    public:
        // public methods
        VertexBuffer() = delete;
        VertexBuffer(Device device, VertexDescription vertex_description, uint32_t num_vertices, VkMemoryPropertyFlags memory_properties);
        uint32_t get_num_vertices() const { return num_vertices; }

        // public member variables
        void map_memory();
        VkBuffer buffer;
        VkDeviceMemory memory;
        std::vector<float> vertex_data;
    private:
        ~VertexBuffer();
        uint32_t num_vertices;
        Device device;
        uint64_t size; // size in bytes
    };

    class IndexBuffer {
    public:
        // public methods
        IndexBuffer() = delete;
        IndexBuffer(Device device, uint32_t num_indices, VkMemoryPropertyFlags memory_properties);
        uint32_t get_num_indices() const { return num_indices; }
        
        // public member variables
        VkBuffer buffer;
        VkDeviceMemory memory;
        void map_memory();
        std::vector<uint32_t> index_data;
    private:
        ~IndexBuffer();
        uint32_t num_indices;
        Device device;
        uint64_t size; // size in bytes
    };

    class Fence {
    public:
        Fence() = delete;
        Fence(Device device, bool signaled=false);
        bool active() { return vkGetFenceStatus(device.logical, fence) == VK_SUCCESS; }
        VkResult reset() { return vkResetFences(device.logical, 1, &fence); }
        VkResult wait(uint64_t timeout_nanosec=1000000000) {return vkWaitForFences(device.logical, 1, &fence, VK_TRUE, timeout_nanosec); }
        VkFence fence;
    private:
        Device device;
        ~Fence();
    };

    class Semaphore {
    public:
        Semaphore() = delete;
        Semaphore(Device device, VkSemaphoreType type = VK_SEMAPHORE_TYPE_BINARY, uint64_t initial_value = 0);
        VkResult wait(uint64_t timeout_nanosec = 1000000000);
        uint64_t counter() { uint64_t value;  vkGetSemaphoreCounterValue(device.logical, semaphore, &value); return value; }
        void signal(uint64_t value);
        VkSemaphore semaphore;
    private:
        Device device;
        VkSemaphoreType type;
        ~Semaphore();
    };

    class Event {
    public:
        Event() = delete;
        Event(Device device);
        bool signaled() { return vkGetEventStatus(device.logical, event) == VK_EVENT_SET; }
        VkResult set() { return vkSetEvent(device.logical, event); }
        VkResult reset() { return vkResetEvent(device.logical, event); }
        void signal(CommandBuffer command_buffer, VkDependencyFlags flags = VK_DEPENDENCY_VIEW_LOCAL_BIT);
        VkEvent event;
    private:
        Device device;
        ~Event();
    };
    // standalone helper functions that are not part of any class:

    VkShaderModule shader_module(Device device, const char* shader_code);
    VkShaderModule shader_module_from_file(Device device, const char* filename);
    uint32_t get_memory_type_index(Device device, uint32_t type_filter, VkMemoryPropertyFlags memory_properties);






    // =============================================================================================
    // DEFINITIONS:
    // =============================================================================================

    enum QueueFamily {
        GRAPHICS,
        COMPUTE,
        TRANSFER,
        UNDEFINED
    };

    struct Queue {
        uint32_t family_index = UINT32_MAX;
        uint32_t index = UINT32_MAX;
        VkQueue queue = nullptr;
        QueueFamily usage = QueueFamily::UNDEFINED;
        float priority = 1.0f;
    };

    Instance::Instance() {
        // set default parameters
        application_info = {};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = "Vulkan Application";
        application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        application_info.apiVersion = VK_API_VERSION_1_2;
        instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    Instance::~Instance() {
        vkDestroyInstance(instance, nullptr);
    }

    void Instance::init_application(const char* name, uint32_t major_version, uint32_t minor_version, uint32_t patch_version) {
        application_info.pApplicationName = name;
        application_info.applicationVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    }

    void Instance::init_engine(const char* name, uint32_t major_version, uint32_t minor_version, uint32_t patch_version) {
        application_info.pEngineName = name;
        application_info.engineVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
    }

    void Instance::init_layers(const std::vector<const char*>& enabled_layer_names) {
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size());
        instance_create_info.ppEnabledLayerNames = enabled_layer_names.data();
    }

    void Instance::init_extensions(const std::vector<const char*>& enabled_extension_names) {
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
        instance_create_info.ppEnabledExtensionNames = enabled_extension_names.data();
    }

    void Instance::create(const void* pNext, VkInstanceCreateFlags flags) {
        instance_create_info.pNext = pNext;
        instance_create_info.flags = flags;
        instance_create_info.pApplicationInfo = &application_info;

        if (instance_create_info.enabledLayerCount == 0) {
            Log::warning("in method 'Instance::create()': no layers enabled for Vulkan instance");
        }

        if (instance_create_info.enabledExtensionCount == 0) {
            Log::warning("in method 'Instance::create()': no extensions enabled for Vulkan instance; make sure none are required!");
        }

        VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
        if (result != VK_SUCCESS) {
            Log::log(LOG_LEVEL_ERROR, "Failed to create Vulkan Instance (VkResult=", result, ")");
        }
        Log::info("Vulkan instance successfully created.");
    }

    Device::Device(Instance instance, uint32_t id, VkPhysicalDeviceFeatures enabled_features, const std::vector<const char*>& enabled_extensions) {
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

    Device::~Device() {
        vkDeviceWaitIdle(logical);
        vkDestroyDevice(logical, nullptr);
    }

    Swapchain::Swapchain(Device device, VkSurfaceKHR surface, VkImageUsageFlags usage, RenderPass renderpass, uint32_t min_image_count, VkImageViewType view_type, VkPresentModeKHR present_mode) {
        this->device = device;
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
        VkSwapchainCreateInfoKHR create_info;
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

    Swapchain::~Swapchain() {
        for (uint32_t i = 0; i < num_images; i++) {
            vkDestroyFramebuffer(device.logical, framebuffer[i], nullptr);
            vkDestroyImageView(device.logical, image_view[i], nullptr);
        }
        framebuffer.clear();
        vkDestroySwapchainKHR(device.logical, swapchain, nullptr);
    }

    RenderPass::RenderPass(Device device, VkFormat format, QueueFamily usage) {
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
        VkRenderPassCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &attachment_descr;
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        vkCreateRenderPass(device.logical, &create_info, 0, &renderpass);
    }

    RenderPass::~RenderPass() {
        vkDestroyRenderPass(device.logical, renderpass, nullptr);
    }

    CommandPool::CommandPool(Device device, QueueFamily usage) {
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

    CommandPool::~CommandPool() {
        vkDestroyCommandPool(device.logical, pool, nullptr);
    }

    CommandBuffer::CommandBuffer(Device device, QueueFamily usage, CommandPool pool) {
        this->device = device;
        this->usage = usage;
        this->pool = pool.pool;

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

    CommandBuffer::~CommandBuffer() {
        vkFreeCommandBuffers(device.logical, pool.pool, 1, &buffer);
    }

    void CommandBuffer::bind_vertex_buffer(VertexBuffer vertex_buffer) {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(this->buffer, 0, 1, &vertex_buffer.buffer, &offset);
    }

    void CommandBuffer::bind_index_buffer(IndexBuffer index_buffer) {
        vkCmdBindIndexBuffer(this->buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void CommandBuffer::set_event(Event event, VkPipelineStageFlags stage_mask) {
        vkCmdSetEvent(buffer, event.event, stage_mask);
    }

    void CommandBuffer::reset_event(Event event, VkPipelineStageFlags stage_mask) {
        vkCmdResetEvent(buffer, event.event, stage_mask);
    }

    void CommandBuffer::bind_pipeline(GraphicsPipeline pipeline) {
        if (pipeline.pipeline != nullptr) {
            vkCmdBindPipeline(buffer, bind_point, pipeline.pipeline);
        }
    }

    void CommandBuffer::bind_pipeline(ComputePipeline pipeline) {
        if (pipeline.pipeline != nullptr) {
            vkCmdBindPipeline(buffer, bind_point, pipeline.pipeline);
        }
    }

    void CommandBuffer::dispatch(uint32_t workgroups_x, uint32_t workgroups_y, uint32_t workgroups_z) {
        // dispatch for compute
        if (usage == QueueFamily::COMPUTE) {
            vkCmdDispatch(buffer, workgroups_x, workgroups_y, workgroups_z);
        }
        else {
            Log::warning("invalid call of method CommandBuffer::dispatch, only allowed for usage type QueueFamily::COMPUTE");
        }
    }

    void CommandBuffer::begin_render(VkOffset2D offset, VkExtent2D extent, VkRenderingFlags flags, std::vector<VkRenderingAttachmentInfo> color_attachments, VkRenderingAttachmentInfo depth_attachment, VkRenderingAttachmentInfo stencil_attachment) {
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

    void CommandBuffer::begin_renderpass(RenderPass renderpass, VkOffset2D offset, VkExtent2D extent, std::vector<VkClearValue> clear_value) {
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

    void CommandBuffer::submit(VkPipeline pipeline, VkFence fence) {

        // stop command buffer recording state (thus triggering executable state)
        vkEndCommandBuffer(buffer);

        // submit to queue (triggers command buffer pending state)
        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pCommandBuffers = &buffer;
        submit_info.commandBufferCount = 1;

        VkQueue queue;
        if (usage == QueueFamily::GRAPHICS) { queue = device.graphics_queue.queue; }
        if (usage == QueueFamily::COMPUTE) { queue = device.compute_queue.queue; }
        if (usage == QueueFamily::TRANSFER) { queue = device.transfer_queue.queue; }

        vkQueueSubmit(queue, 1, &submit_info, fence);
    }

    VertexDescription::VertexDescription(uint32_t dimensions) {
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

    void VertexDescription::add_color_attribute() {
        if (has_color) {
            Log::warning("a color attribute has already been added to this vertex description");
            return;
        }
        // setup color attribute
        VkVertexInputAttributeDescription color_attribute_description = {};
        color_attribute_description.binding = 0;
        color_attribute_description.location = 1;
        color_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
        color_attribute_description.offset = size;
        
        // add to attribute list
        attribute_descriptions.push_back(color_attribute_description);
 
        // update stride distance (in bytes)
        input_binding.stride += sizeof(float) * 3; // add one float per color
    }

    GraphicsPipeline::GraphicsPipeline(Device device, RenderPass renderpass, Swapchain swapchain, VertexDescription vertex_description, VkShaderModule vertex_shader_module, VkShaderModule fragment_shader_module){
        this->device = device;

        std::vector<VkPipelineStageCreateInfo> pipeline_stage;

        // setup shader stages
        {

            if (vertex_shader_module != nullptr) {
                pipeline_stage.push_back({});
                pipeline_stage[pipeline_stage.size() - 1].sType = VK_STRUCTURE_TYPE_PIPELINE_pipeline_stage_CREATE_INFO;
                pipeline_stage[pipeline_stage.size() - 1].stage = VK_pipeline_stage_VERTEX_BIT;
                pipeline_stage[pipeline_stage.size() - 1].module = vertex_shader_module;
                pipeline_stage[pipeline_stage.size() - 1].pName = "main";
            }

            if (fragment_shader_module != nullptr) {
                pipeline_stage.push_back({});
                pipeline_stage[pipeline_stage.size() - 1].sType = VK_STRUCTURE_TYPE_PIPELINE_pipeline_stage_CREATE_INFO;
                pipeline_stage[pipeline_stage.size() - 1].stage = VK_pipeline_stage_FRAGMENT_BIT;
                pipeline_stage[pipeline_stage.size() - 1].module = fragment_shader_module;
                pipeline_stage[pipeline_stage.size() - 1].pName = "main";
            }
        }

        // setup vertex input state
        {   
            VkVertexInputStateCreateInfo vertex_input_state = {};
            vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state.vertexBindingDescriptionCount = 1;
            vertex_input_state.pVertexBindingDescriptions = &(vertex_description.get_vertex_input_binding());
            vertex_input_state.vertexAttributeDescriptionCount = vertex_description.get_attribute_count();
            vertex_input_state.pVertexAttributeDescriptions = vertex_description.get_attribute_descriptions().data();
        }

        // setup input assembly state
        {
            VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
            input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }

        // setup viewport state
        {
            VkPipelineViewportStateCreateInfo viewport_state = {};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            VkViewport viewport = { 0.0f, 0.0f, (float)swapchain.width, (float)swapchain.height };
            viewport_state.pViewports = &viewport;
            viewport_state.scissorCount = 1;
            VkRect2D scissor = { {0,0}, {swapchain.width, swapchain.height} };
            viewport_state.pScissors = &scissor;
        }

        // setup rasterization state
        {
            VkPipelineRasterizationStateCreateInfo rasterization_state = {};
            rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state.lineWidth = 1.0f;
        }

        // setup pipeline layout
        {
            VkPipelineLayoutCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            vkCreatePipelineLayout(device.logical, &create_info, nullptr, &layout);
        }

        // setup multisample state
        {
            VkPipelineMultisampleStateCreateInfo multisample_state = {};
            multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        }

        // setup color blend state
        {
            VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
            color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment_state.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo color_blend_state = {};
            color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state.attachmentCount = 1;
            color_blend_state.pAttachment = &color_blend_attachment_state;
        }

        // finalize graphics pipeline
        {
            VkPipelineCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            create_info.stageCount = pipeline_stage.size();
            create_info.pStages = pipeline_stage.data();
            create_info.pVertexInputState = &vertex_input_state;
            create_info.pInputAssemblyState = &input_assembly_state;
            create_info.pViewportState = &viewport_state;
            create_info.pRasterizationState = &rasterization_state;
            create_info.pMultisampleState = &multisample_state;
            create_info.pColorBlendState = &color_blend_state;
            create_info.layout = layout;
            create_info.renderPass = renderpass.renderpass;
            create_info.subpass = 0;
            vkCreateGraphicsPipelines(device.logical, 0, 1, &create_info, nullptr, &pipeline);
        }
    }

    GraphicsPipeline::~GraphicsPipeline() {
        vkDestroyPipeline(device.logical, pipeline, nullptr);
        vkDestroyPipelineLayout(device.logical, layout, nullptr);
    }

    ComputePipeline::ComputePipeline(Device device, VkShaderModule compute_shader_module) {
        this->device = device;

        // setup pipeline layout
        {
            VkPipelineLayouCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            vkCreatePipelineLayout(device.logical, &create_info, nullptr, &layout);
        }

        // setup shader stage
        {
            VkPipelineShaderStageCreateInfo shader_stage_create_info = {};
            shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_SHADER;
            shader_stage_create_info.module = compute_shader_module;
            shader_stage_create_info.pName = "main";
        }
        
        // setup layout
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        vkCreatePipelineLayout(device.logical, &layout_create_info, nullptr, &layout);

        
        // finalize compute pipeline
        {
            VkComputePipelineCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            create_info.pNext = NULL;
            create_info.flags = 0;
            create_info.stage = shader_stage_create_info;
            create_info.layout = layout;
            create_info.basePipelineHandle = VK_NULL_HANDLE;
            vkCreateComputePipelines(device.logical, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
        }
    }

    ComputePipeline::~ComputePipeline() {
        vkDestroyPipeline(device.logical, pipeline, nullptr);
        vkDestroyPipelineLayout(device.logical, layout, nullptr);
    }

    uint32_t get_memory_type_index(Device device, uint32_t type_filter, VkMemoryPropertyFlags memory_properties) {
        VkPhysicalDeviceMemoryProperties device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties(device.physical, &device_memory_properties);

        for (uint32_t i = 0; i < device_memory_properties.memoryTypeCount; i++) {
            // check if required memory type is allowed
            if (type_filter & (1 << i)) {
                // check if required properties are satisfied
                if (device_memory_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties){
                    return i;
                }
            }
        }
        Log::error("in function 'get_memory_type_index': suitable memory type is unavailable");
        return UINT32_MAX;
    }

    VertexBuffer::VertexBuffer(Device device, VertexDescription vertex_description, uint32_t num_vertices, VkMemoryPropertyFlags memory_properties) {
        this->device = device;
        this->num_vertices = num_vertices;
        this->size = vertex_description.get_size() * num_vertices;
        vertex_data.resize(size / sizeof(float));

        // create buffer
        VkBufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = size;
        create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vkCreateBuffer(device.logical, &create_info, nullptr, &buffer->buffer);

        // get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device.logical, buffer->buffer, &memory_requirements);

        // allocate memory
        VkMemoryAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = get_memory_type_index(device, memory_requirements.memoryTypeBits, memory_properties);
        vkAllocateMemory(device.logical, &allocate_info, nullptr, &buffer->memory);

        // bind memory to buffer
        VkDeviceSize memory_offset = 0;
        vkBindBufferMemory(device.logical, buffer->buffer, buffer->memory, memory_offset);
    }

    VertexBuffer::~VertexBuffer() {
        vkDestroyBuffer(device.logical, buffer->buffer, nullptr);
        vkFreeMemory(device.logical, buffer->memory, nullptr);
    }

    void VertexBuffer::map_memory(){
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* data;
        vkMapMemory(device.logical, buffer.memory, offset, size, flags, &data);
        memcpy(data, vertex_data.data(), size);
    }

    IndexBuffer::IndexBuffer(Device device, uint32_t num_indices, VkMemoryPropertyFlags memory_properties) {
        this->device = device;
        this->num_indices = num_indices;
        this->size = num_indices * sizeof(uint32_t);
        index_data.resize(num_indices);

        // create buffer
        VkBufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = size;
        create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        vkCreateBuffer(device.logical, &create_info, nullptr, &buffer->buffer);

        // get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device.logical, buffer->buffer, &memory_requirements);

        // allocate memory
        VkMemoryAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = get_memory_type_index(device, memory_requirements.memoryTypeBits, memory_properties);
        vkAllocateMemory(device.logical, &allocate_info, nullptr, &buffer->memory);

        // bind memory to buffer
        VkDeviceSize memory_offset = 0;
        vkBindBufferMemory(device.logical, buffer->buffer, buffer->memory, memory_offset);
    }

    IndexBuffer::~IndexBuffer() {
        vkDestroyBuffer(device.logical, buffer->buffer, nullptr);
        vkFreeMemory(device.logical, buffer->memory, nullptr);
    }

    void IndexBuffer::map_memory() {
        VkDeviceSize offset = 0;
        VkMemoryMapFlags flags = 0;
        void* data;
        vkMapMemory(device.logical, buffer.memory, offset, size, flags, &data);
        memcpy(data, index_data.data(), size);
    }

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
            Log::error("shader file not found: " + filename);
        }
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        uint8_t* buffer = new uint8_t[file_size];
        fread(buffer, 1, file_size, file);

        // setup create info
        VkShaderModuleCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = file_size;
        create_info.pCode = (uint32_t*)buffer;

        // create module
        vkCreateShaderModule(device.logical, &create_info, nullptr, &result);
        
        delete[] buffer;
        fclose(file);
        return result;
    }

    Fence::Fence(Device device, bool signaled){
        this->device = device;
        VkFenceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.pNext = NULL;
        if (signaled){
            create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }
        else {
            create_info.flags = 0;
        }
        vkCreateFence(device.logical, &create_info, nullptr, &fence);
    }

    Fence::~Fence() {
        vkDestroyFence(device.logical, fence, nullptr);
    }

    Semaphore::Semaphore(Device device, VkSemaphoreType type, uint64_t initial_value) {
        this->device = device;
        this->type = type;

        VkSemaphoreTypeCreateInfo type_create_info = {};
        type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        type_create_info.pNext = NULL;
        type_create_info.semaphoreType = type;
        type_create_info.initialValue = type==VK_SEMAPHORE_TYPE_BINARY ? 0 : initial_value;

        VkSemaphoreCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        create_info.pNext = &type_create_info;
        create_info.flags = 0;
        vkCreateSemaphore(device.logical, &create_info, nullptr, &semaphore);
    }

    VkResult Semaphore::wait(uint64_t timeout_nanosec) {
        VkSemaphoreWaitInfo wait_info = {};
        wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        wait_info.pNext = NULL;
        wait_info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &semaphore;
        return vkWaitSemaphores(device.logical, &wait_info, timeout_nanosec);
    }

    void Semaphore::signal(uint64_t value) {
        VkSemaphoreSignalInfo signal_info = {};
        signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signal_info.pNext = NULL;
        signal_info.semaphore = semaphore;
        signal_info.value = value;
        vkSignalSemaphore(device.logical, &signal_info);
    }

    Semaphore::~Semaphore() {
        vkDestroySemaphore(device.logical, semaphore, nullptr);
    }

    Event::Event(Device device) {
        this->device = device;
        VkEventCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
        vkCreateEvent(device.logical, &create_info, nullptr, &event);
    }

    Event::~Event() {
        vkDestroyEvent(device.logical, event, nullptr);
    }

    void Event::signal(CommandBuffer command_buffer, VkDependencyFlags flags) {
        VkDependencyInfo dependency_info = {};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.pNext = NULL;
        dependency_info.dependencyFlags = flags;
        dependency_info.memoryBarrierCount = 0;
        dependency_info.pMemoryBarriers = nullptr;
        dependency_info.imageMemoryBarrierCount = 0;
        dependency_info.pImageMemoryBarriers = nullptr;
        vkCmdSetEvent2(command_buffer.bind_index_buffer, event, &dependency_info);
    }

}; // end of namespace VulkanContext

// namespace alias
namespace Vk = VulkanContext;