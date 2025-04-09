# Vulkan Context Library (`vkcontext.h`)

## Summary
The `vkcontext.h` library provides a high-level abstraction for managing Vulkan objects and workflows. It simplifies the creation and management of Vulkan instances, devices, surfaces, swapchains, pipelines, and other essential Vulkan components. The library is designed to streamline Vulkan development by offering reusable classes and methods for common Vulkan operations, while maintaining flexibility for advanced use cases.

---

## Features
- **Instance and Device Management**: Simplifies the creation of Vulkan instances and logical devices.
- **Surface and Swapchain Management**: Handles platform-specific surface creation and swapchain setup.
- **Pipeline and RenderPass Management**: Provides tools for creating graphics and compute pipelines, as well as render passes.
- **Command Buffers and Synchronization**: Includes utilities for command buffer allocation, recording, and synchronization primitives like fences, semaphores, and events.
- **Memory and Resource Management**: Offers abstractions for buffers, images, and descriptor sets.
- **Cross-Platform Support**: Supports multiple platforms, including Windows, Linux, Android, and macOS.

---

## Classes and Methods

### Instance Management
The `Instance` class manages the Vulkan instance, which is the entry point for all Vulkan operations.

| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `Instance()`                        | Default constructor that initializes the Vulkan application and engine info.    |
| `Instance(Instance&& other)`        | Move constructor to transfer ownership of the Vulkan instance.                  |
| `void set_application(...)`         | Sets the application name and version.                                         |
| `void set_engine(...)`              | Sets the engine name and version.                                              |
| `void set_api_version(...)`         | Sets the Vulkan API version.                                                   |
| `void log_available_layers()`       | Logs the available Vulkan instance layers.                                     |
| `void enable_layers(...)`           | Enables specific Vulkan instance layers.                                       |
| `void log_available_extensions()`   | Logs the available Vulkan instance extensions.                                 |
| `void enable_extensions(...)`       | Enables specific Vulkan instance extensions.                                   |
| `void create(...)`                  | Creates the Vulkan instance.                                                   |
| `VkInstance get() const`            | Returns the Vulkan instance handle.                                            |

---

### Device Management
The `Device` class manages the physical and logical Vulkan devices.

| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `Device(Instance& instance, ...)`   | Constructs a logical device for a selected physical device.                    |
| `Device(Device&& other)`            | Move constructor to transfer ownership of the Vulkan device.                   |
| `VkDevice& get_logical()`           | Returns the logical device handle.                                             |
| `VkPhysicalDevice& get_physical()`  | Returns the physical device handle.                                            |
| `VkQueue& get_graphics_queue()`     | Returns the graphics queue handle.                                             |
| `VkQueue& get_compute_queue()`      | Returns the compute queue handle.                                              |
| `VkQueue& get_transfer_queue()`     | Returns the transfer queue handle.                                             |
| `uint32_t get_graphics_queue_family_index()` | Returns the graphics queue family index.                                      |
| `VkPhysicalDeviceProperties& get_properties()` | Returns the properties of the physical device.                              |

---

### Surface Management
The `Surface` class abstracts platform-specific Vulkan surface creation.

| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `Surface(Instance& instance, ...)`  | Creates a Vulkan surface for the specified platform.                           |
| `VkSurfaceKHR get() const`          | Returns the Vulkan surface handle.                                             |
| `bool is_valid() const`             | Checks if the surface handle is valid.                                         |
| `VkSurfaceCapabilitiesKHR get_capabilities(Device& device)` | Retrieves the surface capabilities.                                           |
| `std::vector<VkSurfaceFormatKHR> get_formats(Device& device)` | Retrieves the supported surface formats.                                     |
| `std::vector<VkPresentModeKHR> get_present_modes(Device& device)` | Retrieves the supported presentation modes.                                  |

---

### Swapchain Management
The `Swapchain` class manages the Vulkan swapchain and its associated resources.

| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `Swapchain(Device& device, ...)`    | Constructs a swapchain for the specified surface and device.                   |
| `void create_framebuffers(...)`     | Creates framebuffers for the swapchain images.                                 |
| `void acquire_next_image(...)`      | Acquires the next image from the swapchain.                                    |
| `VkResult present_rendered_image()` | Presents the rendered image to the swapchain.                                  |
| `VkSwapchainKHR get() const`        | Returns the Vulkan swapchain handle.                                           |
| `VkImage get_image(uint32_t index)` | Returns the swapchain image at the specified index.                            |

---

### RenderPass Management
The `RenderPass` class defines the structure and dependencies of rendering operations.

| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `RenderPass(Device& device, ...)`   | Constructs a render pass for the specified device.                             |
| `RenderAttachment add_attachment(...)` | Adds an attachment description to the render pass.                            |
| `uint32_t add_subpass(...)`         | Adds a subpass to the render pass.                                             |
| `VkRenderPass& get()`               | Returns the Vulkan render pass handle.                                         |

---

### Pipeline Management
The `GraphicsPipeline` and `ComputePipeline` classes manage Vulkan pipelines.

#### GraphicsPipeline
| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `GraphicsPipeline(Device& device, ...)` | Constructs a graphics pipeline.                                              |
| `VkPipeline& get()`                 | Returns the Vulkan graphics pipeline handle.                                   |
| `VkPipelineLayout& get_layout()`    | Returns the pipeline layout handle.                                            |

#### ComputePipeline
| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `ComputePipeline(Device& device, ...)` | Constructs a compute pipeline.                                               |
| `VkPipeline& get()`                 | Returns the Vulkan compute pipeline handle.                                    |
| `VkPipelineLayout& get_layout()`    | Returns the pipeline layout handle.                                            |

---

### Command Buffer Management
The `CommandBuffer` class manages Vulkan command buffers.

| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `CommandBuffer(Device& device, ...)` | Constructs a command buffer for the specified queue family.                   |
| `void reset()`                      | Resets the command buffer.                                                     |
| `void bind_pipeline(...)`           | Binds a pipeline to the command buffer.                                        |
| `void draw(...)`                    | Records a draw command.                                                        |
| `void dispatch(...)`                | Records a compute dispatch command.                                            |
| `void submit(...)`                  | Submits the command buffer to the queue.                                       |

---

### Synchronization Primitives
The library provides classes for synchronization, including `Fence`, `Semaphore`, and `Event`.

#### Fence
| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `Fence(Device& device, ...)`        | Constructs a fence.                                                            |
| `bool signaled()`                   | Checks if the fence is signaled.                                               |
| `VkResult reset()`                  | Resets the fence.                                                              |
| `VkResult wait(...)`                | Waits for the fence to be signaled.                                            |

#### Semaphore
| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `Semaphore(Device& device, ...)`    | Constructs a semaphore.                                                        |
| `VkResult wait(...)`                | Waits for the semaphore to be signaled.                                        |
| `void signal(...)`                  | Signals the semaphore.                                                         |

#### Event
| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `Event(Device& device)`             | Constructs an event.                                                           |
| `bool signaled()`                   | Checks if the event is signaled.                                               |

---

### Descriptor and Buffer Management
The library provides abstractions for managing Vulkan buffers, descriptor sets, and descriptor pools.

#### DescriptorSet
| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `DescriptorSet(Device& device)`     | Constructs a descriptor set.                                                   |
| `uint32_t bind_buffer(...)`         | Binds a buffer to the descriptor set.                                          |
| `uint32_t bind_image(...)`          | Binds an image view to the descriptor set.                                     |
| `void update()`                     | Updates the descriptor set with the current bindings.                          |

#### Buffer
| **Method**                          | **Description**                                                                 |
|-------------------------------------|---------------------------------------------------------------------------------|
| `Buffer(Device& device, ...)`       | Constructs a buffer.                                                           |
| `void write(...)`                   | Writes data to the buffer.                                                     |
| `std::vector<T> read(...)`          | Reads data from the buffer.                                                    |
| `T get(uint32_t index)`             | Retrieves a single element from the buffer.                                    |

---

## Cross-Platform Support
The library supports multiple platforms, including:
- **Windows**: Uses `VK_USE_PLATFORM_WIN32_KHR`.
- **Linux**: Uses `VK_USE_PLATFORM_XCB_KHR`.
- **Android**: Uses `VK_USE_PLATFORM_ANDROID_KHR`.
- **macOS**: Uses `VK_USE_PLATFORM_METAL_EXT`.

---

## License
This library is provided under the MIT License. See the `LICENSE` file for details.