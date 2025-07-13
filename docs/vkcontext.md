# Vulkan Context Library (`vkcontext.h`)

## Summary
The `vkcontext.h` library provides high-level abstraction for managing Vulkan objects and workflows.
It simplifies the creation and management of Vulkan instances, devices, surfaces, swapchains, pipelines,
and other essential Vulkan components. The library is designed to streamline Vulkan development by offering
reusable classes and methods for common Vulkan operations, while maintaining flexibility for advanced use cases.
This library was built on a Win64 machine, but it's intended to be cross-platform.

_Note: testing is mostly complete for CGU compute. Handling of graphics still needs some fine-tuning (TODO)._

---

## Features
- **Instance and Device Management**: Simplifies the creation of Vulkan instances and logical devices.
- **Surface and Swapchain Management**: Handles platform-specific surface creation and swapchain setup.
- **Pipeline and RenderPass Management**: Provides tools for creating graphics and compute pipelines, as well as render passes.
- **Command Buffers and Synchronization**: Includes utilities for command buffer allocation, recording, and synchronization primitives like fences, semaphores, and events.
- **Memory and Resource Management**: Offers abstractions for buffers, images, and descriptor sets.
- **Cross-Platform Support**: Supports multiple platforms, including Windows, Linux, Android, and macOS.
- . . . (and more)

---

## Classes and Methods

### Instance Management

The `Instance` class manages the Vulkan instance, which is the entry point for all Vulkan operations.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Instance()`                        | Default constructor that initializes the Vulkan application and engine info.   |
| `Instance(Instance&& other)`        | Move constructor to transfer ownership of the Vulkan instance.                 |
| `Instance& operator=(Instance&& other)`| Move assignment operator to transfer ownership of the Vulkan instance.      |
| `~Instance()`                       | Destructor that cleans up the Vulkan instance.                                 |
| `void set_application(...)`         | Sets the application name and version.                                         |
| `void set_engine(...)`              | Sets the engine name and version.                                              |
| `void set_api_version(...)`         | Sets the Vulkan API version.                                                   |
| `void log_available_layers()`       | Logs the available Vulkan instance layers.                                     |
| `void enable_layers(...)`           | Enables specific Vulkan instance layers										   |
| `void log_available_extensions()`   | Logs the available Vulkan instance extensions.                                 |
| `void enable_extensions(...)`       | Enables specific Vulkan instance extensions.                                   |
| `void create(...)`                  | Creates (finalizes) the Vulkan instance (after setting its attributes)         |
| `VkInstance get()`                  | Returns the Vulkan instance handle.                                            |

---

### Device Management
The `Device` class manages the physical and logical Vulkan devices.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Device(Instance& instance, ...)`   | Constructs a logical device for a selected physical device.                    |
| `Device(Device&& other)`            | Move constructor to transfer ownership of the Vulkan device.                   |
| `Device& operator=(Device&& other)` | Move assignment operator to transfer ownership of the Vulkan device.           |
| `VkDevice& get_logical()`           | Returns the logical device handle.                                             |
| `VkPhysicalDevice& get_physical()`  | Returns the physical device handle.                                            |
| `VkQueue& get_graphics_queue()`     | Returns the graphics queue handle.                                             |
| `VkQueue& get_compute_queue()`      | Returns the compute queue handle.                                              |
| `VkQueue& get_transfer_queue()`     | Returns the transfer queue handle.                                             |
| `uint32_t get_graphics_queue_family_index()` | Returns the graphics queue family index.                              |
| `uint32_t get_compute_queue_family_index()` | Returns the compute queue family index.                                |
| `uint32_t get_transfer_queue_family_index()` | Returns the transfer queue family index.                              |
| `... get_properties()` | Returns the properties struct of the physical device.                                       |
| `... get_properties2()` | Returns the properties2 struct of the physical device.                                     |
| `... get_memory_properties()` | Returns the memory properties of the selected physical device.                       |
| `... get_features()` | Returns the enabled features struct of the selected physical device.                          |
| `... get_synchronization_features()` | Returns the synchronization features struct of the selected physical device.  |

---
### Image Management
The `Image` class abstracts Vulkan image creation and management.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Image(Device& device, ...)`        | Constructs a Vulkan image with specified parameters.                           |
| `Image(Image&& other)`              | Move constructor to transfer ownership of the Vulkan image.                    |
| `Image& operator=(Image&& other)`   | Move assignment operator to transfer ownership of the Vulkan image.            |
| `VkImage get() const`               | Returns the Vulkan image handle.                                               |
| `VkFormat get_format() const`       | Returns the format of the Vulkan image.                                        |
| `VkExtent3D get_extent() const`     | Returns the extent of the Vulkan image.                                        |
| `VkImageLayout get_layout() const`  | Returns the current layout of the Vulkan image.                                |
| `VkDeviceMemory get_memory() const` | Returns the device memory handle associated with the image.                    |
| `void set_layout(VkImageLayout layout)` | Sets the layout of the Vulkan image.                                       |

The `ImageView` class provides a view into a Vulkan image, allowing for different formats and subresources.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `ImageView(Device& device, Image& image, ...)` | Constructs a Vulkan image view for the specified image.             |
| `ImageView(ImageView&& other)`      | Move constructor to transfer ownership of the Vulkan image view.               |
| `ImageView& operator=(ImageView&& other)` | Move assignment operator to transfer ownership of the Vulkan image view. |
| `VkImageView get() const`           | Returns the Vulkan image view handle.                                          |
___
### Surface Management
The `Surface` class abstracts platform-specific Vulkan surface creation.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Surface(Instance& instance, ...)`  | Creates a Vulkan surface for the specified platform.                           |
| `Surface(Surface&& other)`          | Move constructor to transfer ownership of the Vulkan surface.                  |
| `Surface& operator=(Surface&& other)` | Move assignment operator to transfer ownership of the Vulkan surface.        |
| `VkSurfaceKHR get() const`          | Returns the Vulkan surface handle.                                             |
| `bool is_valid() const`             | Checks if the surface handle is valid.                                         |
| `const bool get_physical_device_support(Device& device, QueueFamily queue_family)`| Checks present support of the surface for the specified queue family. |
| `... get_capabilities(Device& device)` | Retrieves a struct of the surface's capabilities.                           |
| `std::vector<VkSurfaceFormatKHR> get_formats(Device& device)` | Retrieves the supported surface formats.             |
| `std::vector<VkPresentModeKHR> get_present_modes(Device& device)` | Retrieves the supported presentation modes.      |

The `SurfaceFormat` class represents a format and color space for a Vulkan surface.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `SurfaceFormat(VkFormat format, VkColorSpaceKHR color_space)` | Constructs a surface format with the specified format and color space. |
| `SurfaceFormat(SurfaceFormat&& other)` | Move constructor to transfer ownership of the surface format.               |
| `SurfaceFormat& operator=(SurfaceFormat&& other)` | Move assignment operator to transfer ownership of the surface format. |
| `VkColorSpaceKHR get_color_space() const` | Returns the color space of the surface format.                           |
| `VkFormat get_format() const`       | Returns the format of the surface.                                             |
| `... get()`                         | Returns a Vulkan struct of the surface format.                                 |
| `void set_color_space(VkColorSpaceKHR newcolor_space)` | Sets a (new) color space for the surface format.            |
| `void set_format(VkFormat newformat)` | Sets a (new) format for the surface format.                                  |

---

### Swapchain Management
The `Swapchain` class manages the Vulkan swapchain and its associated resources.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Swapchain(Device& device, ...)`    | Constructs a swapchain for the specified surface and device.                   |
| `void create_framebuffers(...)`     | Creates framebuffers for the swapchain images.                                 |
| `void acquire_next_image(...)`      | Acquires the next image from the swapchain.                                    |
| `VkResult present_rendered_image()` | Presents the rendered image to the swapchain (optionally with wait semaphore(s)).|
| `void recreate()`                   | Recreates the swapchain with the current surface and device settings.          |
| `uint32_t get_width() const`        | Returns the extent width.                                                      |
| `uint32_t get_height() const`       | Returns the extent height.                                                     |
| `VkExtent2D get_extent() const`     | Returns the extent of the swapchain.                                           |
| `VkSwapchainKHR get() const`        | Returns the Vulkan swapchain handle.                                           |
| `VkImageView get_color_image_view(uint32_t index)` | Returns the color image view at the specified index.            |
| `VkImageView get_framebuffer_image_view(uint32_t index)` | Returns the framebuffer image view at the specified index.|
| `VkFramebuffer get_framebuffer(uint32_t index)` | Returns the framebuffer at the specified index.                    |
| `VkImage get_image(uint32_t index)` | Returns the swapchain image at the specified index.                            |
| `VkImage get_current_image() const` | Returns the current swapchain image.                                           |
| `uint32_t get_image_count() const` | Returns the number of images in the swapchain.                                  |
| `uint32_t get_current_image_index() const` | Returns the index of the current swapchain image.                       |

---

### RenderPass Management
The `RenderPass` class defines the structure and dependencies of rendering operations.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `RenderPass(Device& device, ...)`   | Constructs a render pass for the specified device.                             |
| `RenderPass(RenderPass&& other)`    | Move constructor to transfer ownership of the render pass.                     |
| `RenderPass& operator=(RenderPass&& other)` | Move assignment operator to transfer ownership of the render pass.     |
| `uint32_t add_attachment(...)` | Adds an attachment description to the render pass; returns its index                |
| `uint32_t add_subpass_dependency_(...)` | Adds a subpass to the render pass.                                         |
| `VkRenderPass get()`               | Returns the Vulkan handle of the render pass.                                   |
| `void finalize()`                   | Finalizes the render pass after all attachments and subpasses are added.       |
| `uint32_t get_attachment_count() const` | Returns the number of attachments in the render pass.                      |
| `uint32_t get_subpass_count() const` | Returns the number of subpasses in the render pass.                           |
| `uint32_t get_multisample_count() const` | Returns the multisample count of the render pass.                         |
| `bool has_depth_stencil() const` | Checks if the render pass has a depth/stencil attachment.                         |
| `... get_attachment_descriptions()` | Returns a std::vector of the attachment descriptions of the render pass.       |
| `const AttachmentType get_attachment_type(uint32_t index) const` | Returns the type of an attachment at the specified index. |

The `SubPass` class represents a subpass within a render pass, defining how attachments are used.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `SubPass(RenderPass& renderpass)`   | Default constructor for a subpass.                                             |
| `void add_attachment_reference(uint32_t attachment_index)` | Adds a reference to an attachment from the pool of attachment descriptions (owned by the main RenderPass object) |
| `uint32_t finalize(...)` | Finalizes the subpass and adds it to the parend RenderPass; returns its index within the RenderPass |

---

### Pipeline Management

The `GraphicsPipeline` and `ComputePipeline` classes manage Vulkan pipelines.

class `GraphicsPipeline`

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `GraphicsPipeline(Device& device, ...)` | Constructs a graphics pipeline (with renderpass, subpass, swapchain, vertex shader, etc).|
| `VkPipeline& get()`                 | Returns the Vulkan graphics pipeline handle.                                   |
| `VkPipelineLayout& get_layout()`    | Returns the pipeline layout handle.                                            |
| `const VkViewport& get_viewport() const` | Returns the viewport struct                                               |

class `ComputePipeline`

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `ComputePipeline(Device& device, ...)` | Constructs a compute pipeline.                                              |
| `VkPipeline& get()`                 | Returns the Vulkan compute pipeline handle.                                    |
| `VkPipelineLayout& get_layout()`    | Returns the pipeline layout handle.                                            |
| `DescriptorSet* get_set() const`    | Returns a pointer to the descriptor set associated with the pipeline.          |
| `PushConstants* get_constants()`    | Returns a pointer to the push constants range associated with the pipeline.    |
| `uint32_t get_workgroup_size_x()`   | Returns the x-dimensional workgroup size the pipepine was created with.        |
| `uint32_t get_workgroup_size_y()`   | Returns the y-dimensional workgroup size the pipepine was created with.        |
| `uint32_t get_workgroup_size_z()`   | Returns the z-dimensional workgroup size the pipepine was created with.        |

`DeviceMemoryBarrier` objects are generic memory barriers for synchronization between different pipeline stages

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `DeviceMemoryBarrier(...)`          | Parametric constructor for a device memory barrier with the specified stage flags and access flags. |


`BufferMemoryBarrier` objects synchronize buffers between pipeline stages

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `BufferMemoryBarrier(...)`          | Parametric constructor for a buffer memory barrier with the specified stage flags and access flags. |

`ImageMemoryBarrier` objects synchronize image memory between pipeline stages

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `ImageMemoryBarrier(...)`           | Parametric constructor for an image memory barrier with the specified stage flags, access flags and layout changes. |


---

### Shaders

`ShaderModule` objects manage shaders (=GLSL code snippets for execution on the GPU).

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `ShaderModule(const Device& device, const unsigned char* binary, size_t size_bytes)` | constructor for binary data   |
| `ShaderModule(Device& device, const char* foldername, const char* filename)` | constructor for file import           |
| `ShaderModule(Device& device, const std::string& foldername, const std::string& filename)` | constructor for file import |
| `ShaderModule(ShaderModule&& other)` | move constructor															   |
| `ShaderModule& operator=(ShaderModule&& other)` | move assignment													   |
| `VkShaderModule& get() const`		  | returns the Vulkan handle of the shader module								   |

___

### Push Constants

`PushConstants` objects handle a package ("range") of host-defined constants for frequent GPU-side access from within a shader

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `PushConstants()`					  | constructor for an empty push constants object								   |
| `PushConstants(std::vector<T>& values)` | constructor with built-in assignment of a vector of values of equal type T |
| `PushConstants(Args... args)`       | constructor with built-in assignment of arguments of mixed types               |
| `uint32_t add_values(T value)`      | add a new constant of type T to the end of the push constants range; returns the bytes offset of the affected writing location|
| `uint32_t add_values(T value, size_t offset)`| (over-)writes a value at the specified byte offset within the push constants range |
| `uint32_t add_values(std::initializer_list<T> values)`| writes a list of values of equal type T to the current end of the push constants range; returns the byte offset of the last written element |
| `uint32_t add_values(std::vector<T> values)`| writes a vector of values of equal type T to the current end of the push constants range; returns the byte offset of the last written element |
| `const uint32_t* get_data() const`  | returns a pointer to the start of the push constants range                     |
| `const VkPushConstantRange& get_range() const`| returns the Vulkan handle of the push constants range                |
| `size_t get_size()`				  | returns the currently used size in bytes									   |
| `size_t get_total_capacity() const` | returns the total reserved capacity (used + free) of the push constants range  |
| `size_t get_free_capacity() const`  | returns the remaining (unused) capacity (in bytes) available without memory reallocation  |

___

### Command Pool / Command Buffer

The `CommandPool` class manages Vulkan command pools, which are used to allocate command buffers. Use separate pools for each queue family.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `CommandPool(Device& device, ...)`  | Constructs a command pool for the specified queue family.                      |
| `void trim()`                       | Trims the command pool, releasing unused memory.                               |
| `VkResult reset()`                  | Resets the command pool, releasing all command buffers.                        |
| `VkCommandPool get()`               | Returns the Vulkan command pool handle.                                        |
| `QueueFamily get_usage() const`     | Returns the queue family associated with this pool                             |

The `CommandBuffer` class manages Vulkan command buffers (used to record commands for execution on the GPU).

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `CommandBuffer(Device& device, ...)`| Constructs a command buffer for the queue family associated with the pool.     |
| `CommandBuffer(CommandBuffer&& other) noexcept`| Move constructor.                                                   |
| `CommandBuffer& operator=(CommandBuffer&& other) noexcept`| Move assignment.                                         |
| `Event set_event(...)`			  | Set event on the command buffer with the specified memory barriers and flags. Use NULLOPT for any barrier types that aren't needed.|
| `void reset_event(const Event& event, ...)`| Reset a command buffer event.                                           |
| `void wait_event(const Event& event) const`| Wait for the specified event to signal.                                 |
| `void reset()`                      | Resets the command buffer.                                                     |
| `void bind_pipeline(...)`           | Binds a graphics or compute pipeline to the command buffer.                    |
| `void bind_descriptor_set(const DescriptorSet& set)` | Binds a descriptor set to the command buffer (the command buffer has to be constructed for graphics or compute queue family!)|
| `void bind_constants(PushConstants& constants) const`| Binds a push constants range to the command buffer.           |
| `... copy_buffer(...)`              | For copy operation between a source and destination buffer (Staging->Device, Device->Staging, Device->Device).|
| `void add_barrier(...)`             | Add a device/buffer/image memory barrier to the command buffer.                |
| `void add_barriers(...)`            | Add vectors of multiple barriers at once. Use NULLOPT for barrier types that aren't needed.|
| `void transition_image_layout()`    | Transition image layout for the specified image.                               |
| `void draw(...)`                    | Records a draw command for graphics.                                           |
| `void dispatch(...)`                | Records a dispatch command for compute.                                        |
| `void begin_render(...)`            |                                                                                |
| `void begin_renderpass(...)`        |                                                                                |
| `void end_renderpass() const`       |                                                                                |
| `void next_subpass(...)`            |                                                                                |
| `void submit(...)`                  | Ends recording and submits the command buffer to the queue (optionally with Fence).|
| `VkCommandBuffer& get()`            | Returns the Vulkan handle of the command buffer                                |
| `void compute(...)`                 | shorthand for: bind compute pipeline -> bind descriptor set -> push constants -> dispatch -> add buffer memory barriers (optionally) -> end recording -> submit (note: a fence will only be used if fence_timeout_nanosec != 0); the boolean direct_submit can be set to false in case multiple dispatches need to be added before a final submit |
| `void begin_recording()`            | Start command buffer recording state. This is already done once by default after a CommandBuffer object is created.|


---

### Synchronization Primitives

The library provides classes for synchronization, including `Fence`, `Semaphore`, and `Event`.

`Fence` objects handle synchonization between GPU and CPU, allowing the CPU to wait for GPU operations to complete.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Fence(Device& device, ...)`        | Constructs a fence.                                                            |
| `Fence(Fence&& other)`              | Move constructor to transfer ownership of the fence.                           |
| `Fence& operator=(Fence&& other)`   | Move assignment operator to transfer ownership of the fence.                   |
| `bool signaled()`                   | Checks if the fence is signaled.                                               |
| `VkResult reset()`                  | Resets the fence.                                                              |
| `VkResult wait(...)`                | Waits for the fence to be signaled.                                            |
| `VkFence get()`                     | Returns the Vulkan fence handle.                                               |

`Semaphore` objects handle synchronization on the GPU side (synchronize operations between different queue submissions on the GPU).

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Semaphore(Device& device, ...)`    | Constructs a semaphore.                                                        |
| `Semaphore(Semaphore&& other)`      | Move constructor to transfer ownership of the semaphore.                       |
| `Semaphore& operator=(Semaphore&& other)` | Move assignment operator to transfer ownership of the semaphore.         |
| `VkResult wait(...)`                | Waits for the semaphore to be signaled.                                        |
| `uint64_t counter()`                | Returns the current counter value of the semaphore.                            |
| `void signal(...)`                  | Signals the semaphore.                                                         |
| `VkSemaphore get()`                 | Returns the Vulkan semaphore handle.                                           |
|

`Event` objects handle fine-grained synchronization of operations within a single command buffer or queue.

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Event(const Device& device)`       | Constructs an event.                                                           |
| `Event(Event&& other)`              | Move constructor to transfer ownership of the event.                           |
| `Event& operator=(Event&& other)`   | Move assignment operator to transfer ownership of the event.                   |
| `VkResult reset()`                  | Resets the event to the unsignaled state.                                      |
| `VkResult set()`                    | Sets the event to the signaled state.                                          |
| `bool signaled()`                   | Checks if the event is signaled.                                               |
| `VkEvent get()`                     | Returns the Vulkan event handle.                                               |
| `... get_dependency_info()`         | Returns a struct with dependency information for the event.                    |

---

### Descriptor Management

class `DescriptorPool`

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `DescriptorPool(const Device& device, ....)` | Parametric constructor.                                               |
| `DescriptorPool(DescriptorPool&& other) noexcept` | Move constructor.                                                |
| `DescriptorPool& operator=(DescriptorPool&& other) noexcept` | Move assignment.                                      |
| `VkDescriptorPool get() const`      | Returns the Vulkan handle of the pool.                                         |
| `... get_sets() const`              | Returns a vector of the sets assigned to this pool.                            |
| `uint32_t get_max_set() const`      | Returns the max number of sets the pool is configured to hold.                 |
| `uint32_t get_current_sets_count() const` | Returns the number of sets currently allocated to this pool.             |
| `void release_all_sets()`           | Releases all descriptor sets from the pool.                                    |
| `void release_set(const DescriptorSet& set)` | Released a single set from the pool. Returns the remaining number of sets.|
| `uint32_t allocate_set(DescriptorSet& set)` | Allocates a new descriptor set to the pool and returns its index       |

class `DescriptorSet`

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `DescriptorSet(Device& device)`     | Constructs a descriptor set.                                                   |
| `DescriptorSet(DescriptorSet&& other) noexcept` | move constructor												   |
| `DescriptorSet& operator=(DescriptorSet&& other) noexcept` | move assignment                                         |
| `uint32_t bind_buffer(...)`         | Binds a buffer to the descriptor set. Returns the binding index.               |
| `void replace_buffer(...)`          | Replaces the buffer at the specified binding (usage on finalized sets is allowed).|
| `uint32_t bind_image(...)`          | Binds an image view to the descriptor set. Returns the binding index.          |
| `void replace_image(...)`           | Replaces the image at the specified binding (usage on finalized sets is allowed).|
| `void update()`                     | Updates the descriptor set with the current bindings.                          |
| `void finalize_layout()`		      | Finalizes the set layout after all bindings are added.                         |
| `VkDescriptorSet get() const`       | Returns the Vulkan handle of the set.                                          |
| `... get_layout() const`            | Returns the Vulkan handle of the set layout.                                   |
| `... get_buffer_bindings() const`   | Returns a vector of buffer binding info structs                                |
| `... get_image_bndings() const`     | Returns a vector of image binding info structs                                 |

class `VertexDescriptions`

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `VertexDescriptions()`			  | constructor																	   |
| `void add_attribute(...)`			  |																				   |
| `uint32_t add_binding(...)`         |																				   |
| `... get_attribute_descriptions() const`|																			   |
| `... get_input_bindings() const`    |																				   |

---
### Buffers

class `Buffer`

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Buffer(Device& device, ...)`       | Constructs a buffer.                                                           |
| `Buffer(const Buffer<T>& other)`	  | copy constructor															   |
| `Buffer& operator=(const Buffer<T>& other)`| copy assignment													       |
| `Buffer(Buffer<T>&& other) noexcept`| move constructor															   |
| `Buffer& operator=(Buffer<T>&& other) noexcept`| move assignment													   |
| `void write(...)`                   | Writes data to the buffer (with overloads for writing from a std::vector<T>, a std::array<T>, a std::initializer_list<T> or another Buffer<T> |
| `void flush(...)`                   | flush host writes to host memory domain (only necessary if the memory allocation doesn't have the flag VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
| `std::vector<T> read(...)`          | Reads (copies) data from the buffer into a std::vector<T>                      |
| `void write_element(uint32_t element_index, T value)` | writes a single element to the buffer                        |
| `T read_element(uint32_t index)`    | Retrieves a single element from the buffer.                                    |
| `void set_all(...)`                 | assigns the same value to a contiguous sequence of n buffer elements, starting at a specified offset; n=0 -> assign all |
| `uint32_t get_elements() const`     | returns the total number of elements in the buffer                             |
| `uint64_t get_size_bytes() const`   | returns the buffer size in bytes                                               |
| `VkDeviceMemory get_memory() const` | returns the Vulkan handle to the device memory of the buffer                   |
| `VkBuffer get() const`              | returns the Vulkan handle of the underlying VkBuffer object                    |
| `VkFlags get_memory_property_flags() const` | returns the flags of the buffer's memory properties                    |

---
### Sampler

class `Sampler` for texture sampling

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|
| `Sampler(...)`                      | parametric constructor														   |
| `const VkSampler& get`              | returns the handle to the underlying Vulkan sampler object                     |

___

### Shared Vulkan Manager

The `VulkanManager` class creates a singleton object to conviently manage instance, device and command pools 

| **Method**                          | **Description**                                                                |
|-------------------------------------|--------------------------------------------------------------------------------|

___
### Cross-Platform Support
The library supports multiple platforms, including:
- **Windows**: Uses `VK_USE_PLATFORM_WIN32_KHR`.
- **Linux**: Uses `VK_USE_PLATFORM_XCB_KHR`.
- **Android**: Uses `VK_USE_PLATFORM_ANDROID_KHR`.
- **macOS**: Uses `VK_USE_PLATFORM_METAL_EXT`.