#ifndef VKCONTEXT_H

// Macros & Preprocessor Definitions
#define VKCONTEXT_H
#define NOMINMAX
#define NULLOPT std::nullopt
#define MAX_DESCRIPTOR_SET_COUNT 50 // max number of descriptor sets within the shared singleton descriptor pool
#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

// include headers
#include <array>
#include <atomic>
#include <cmath>
#include <corecrt.h>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <initializer_list>
#include <iostream>
#include <log.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <third_party/tiny_obj_loader.h>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <vkdebug.h>
#include <vulkan/vulkan.h>

// Platform-Specific Headers
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <windows.h>            // required for HWND, HINSTANCE
#include <vulkan/vulkan_win32.h>

#elif defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#include <android/asset_manager.h>
#include <android/native_window.h> // required for ANativeWindow
#include <vulkan/vulkan_android.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "vulkan_renderer", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "vulkan_renderer", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "vulkan_renderer", __VA_ARGS__))

#elif defined(__linux__) || defined(__unix__)
#define VK_USE_PLATFORM_XLIB_KHR
#include <unistd.h>
#include <vulkan/vulkan_xlib.h>
#include <X11/Xlib.h>


#else
#error "Unsupported platform: No Vulkan WSI platform defined."
#endif // platform-specific headers

// Vulkan SDK 1.3 includes VK_KHR_synchronization2 extension
#if (VK_HEADER_VERSION >= 204)
#define VK_HEADER_VERSION_1_3
#endif

// Macros for Vulkan functions
#define GET_INSTANCE_PROC_ADDR(name) \
    name = (PFN_vk##name)vkGetInstanceProcAddr(instance, "vk" #name);

#define GET_DEVICE_PROC_ADDR(name) \
    name = (PFN_vk##name)vkGetDeviceProcAddr(device, "vk" #name);

// Vulkan-specific constants and types
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t DEFAULT_API_MAJOR_VERSION = 1;
constexpr uint32_t DEFAULT_API_MINOR_VERSION = 3;
constexpr uint32_t DEFAULT_API_PATCH_VERSION = 0;
constexpr uint32_t DEFAULT_DEVICE = 0;

// default instances layers for VulkanManager class
std::vector<const char*> DEFAULT_INSTANCE_LAYERS = {
#ifdef _DEBUG
	"VK_LAYER_KHRONOS_validation"
#endif
};

// default instance extensions for the VulkanManager class
std::vector<const char*> DEFAULT_INSTANCE_EXTENSIONS = {
	"VK_KHR_get_physical_device_properties2"
#ifdef _DEBUG
	,"VK_EXT_debug_utils"
#endif
};

// default device extensions for the VulkanManager class
std::vector<const char*> DEFAULT_DEVICE_EXTENSIONS = {
	"VK_KHR_synchronization2",
	"VK_EXT_descriptor_indexing",
	"VK_EXT_shader_atomic_float",
	"VK_KHR_storage_buffer_storage_class",
	"VK_KHR_uniform_buffer_standard_layout",
	"VK_KHR_shader_non_semantic_info",
	"VK_KHR_push_descriptor",
	"VK_KHR_shader_float16_int8",
	"VK_KHR_swapchain",
	"VK_KHR_timeline_semaphore"
	//"VK_KHR_shader_int64",
	//"VK_KHR_shader_float64",
	//"VK_EXT_shader_atomic_float16_add"
};

// default enabled device features for the VulkanManager class
VkPhysicalDeviceFeatures DEFAULT_DEVICE_FEATURES = {
	VK_TRUE,   /* robustBufferAccess                       */
	VK_FALSE,  /* fullDrawIndexUint32                      */
	VK_FALSE,  /* imageCubeArray                           */
	VK_FALSE,  /* independentBlend                         */
	VK_FALSE,  /* geometryShader                           */
	VK_FALSE,  /* tessellationShader                       */
	VK_FALSE,  /* sampleRateShading                        */
	VK_FALSE,  /* dualSrcBlend                             */
	VK_FALSE,  /* logicOp                                  */
	VK_FALSE,  /* multiDrawIndirect                        */
	VK_FALSE,  /* drawIndirectFirstInstance                */
	VK_FALSE,  /* depthClamp                               */
	VK_FALSE,  /* depthBiasClamp                           */
	VK_FALSE,  /* fillModeNonSolid                         */
	VK_FALSE,  /* depthBounds                              */
	VK_FALSE,  /* wideLines                                */
	VK_FALSE,  /* largePoints                              */
	VK_FALSE,  /* alphaToOne                               */
	VK_FALSE,  /* multiViewport                            */
	VK_FALSE,  /* samplerAnisotropy                        */
	VK_FALSE,  /* textureCompressionETC2                   */
	VK_FALSE,  /* textureCompressionASTC_LDR               */
	VK_FALSE,  /* textureCompressionBC                     */
	VK_FALSE,  /* occlusionQueryPrecise                    */
	VK_FALSE,  /* pipelineStatisticsQuery                  */
	VK_FALSE,  /* vertexPipelineStoresAndAtomics           */
	VK_FALSE,  /* fragmentStoresAndAtomics                 */
	VK_FALSE,  /* shaderTessellationAndGeometryPointSize   */
	VK_FALSE,  /* shaderImageGatherExtended                */
	VK_FALSE,  /* shaderStorageImageExtendedFormats        */
	VK_FALSE,  /* shaderStorageImageMultisample            */
	VK_FALSE,  /* shaderStorageImageReadWithoutFormat      */
	VK_FALSE,  /* shaderStorageImageWriteWithoutFormat     */
	VK_TRUE,   /* shaderUniformBufferArrayDynamicIndexing  */
	VK_FALSE,  /* shaderSampledImageArrayDynamicIndexing   */
	VK_TRUE,   /* shaderStorageBufferArrayDynamicIndexing  */
	VK_FALSE,  /* shaderStorageImageArrayDynamicIndexing   */
	VK_FALSE,  /* shaderClipDistance                       */
	VK_FALSE,  /* shaderCullDistance                       */
	VK_TRUE,   /* shaderFloat64                            */
	VK_TRUE,   /* shaderInt64                              */
	VK_TRUE,   /* shaderInt16                              */
	VK_FALSE,  /* shaderResourceResidency                  */
	VK_FALSE,  /* shaderResourceMinLod                     */
	VK_TRUE,   /* sparseBinding                            */
	VK_TRUE,   /* sparseResidencyBuffer                    */
	VK_FALSE,  /* sparseResidencyImage2D                   */
	VK_FALSE,  /* sparseResidencyImage3D                   */
	VK_FALSE,  /* sparseResidency2Samples                  */
	VK_FALSE,  /* sparseResidency4Samples                  */
	VK_FALSE,  /* sparseResidency8Samples                  */
	VK_FALSE,  /* sparseResidency16Samples                 */
	VK_FALSE,  /* sparseResidencyAliased                   */
	VK_FALSE,  /* variableMultisampleRate                  */
	VK_FALSE   /* inheritedQueries                         */
};

// default pool size per descriptor type (used by the VulkanManager class)
std::vector<VkDescriptorPoolSize> DEFAULT_POOL_SIZE = {
	{VK_DESCRIPTOR_TYPE_SAMPLER, 5},
	{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3},
	{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 5},
	{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5},
	{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 3},
	{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 3},
	{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
	{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 50},
	{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3},
	// {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0},
	// {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 0},
	// {VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 0},
	// {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0},
	// {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 0},
	// {VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM, 0},
	// {VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, 0},
	// {VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 0},
	// {VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV, 0}
};

// Default camera values
const float CAMERA_YAW = -90.0f;
const float CAMERA_PITCH = 0.0f;
const float CAMERA_SPEED = 2.5f;
const float CAMERA_SENSITIVITY = 0.1f;
const float CAMERA_FOV = 45.0f;

// preferred formats
const std::vector<VkFormat> DEPTH_FORMAT_CANDIDATES = {
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D16_UNORM
};

const std::vector<VkFormat> COLOR_FORMAT_CANDIDATES = {
	VK_FORMAT_R8G8B8A8_SRGB,
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_B8G8R8A8_SRGB,
	VK_FORMAT_B8G8R8A8_UNORM,
	VK_FORMAT_R16G16B16A16_SFLOAT
}; // surface format and color attachment format MUST match! therefore these preferences are used for the selection of BOTH

const std::vector<VkFormat> INPUT_FORMAT_CANDIDATES = {
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R8G8B8A8_UNORM
};

const std::vector<VkFormat> RESOLVE_FORMAT_CANDIDATES = COLOR_FORMAT_CANDIDATES;

const std::vector<VkFormat> PRESERVE_FORMAT_CANDIDATES = COLOR_FORMAT_CANDIDATES;

// forward declarations
class Camera;
class CommandPool;
class CommandBuffer;
class DescriptorSet;
class Device;
class Event;
class ImageView;
class Image;
class ShaderModule;
class SubPass;
class Surface;
class RenderPass;
struct Vertex;

// +=================================+   
// | Global Enums                    |
// +=================================+
enum BufferType {
	VERTEX_BUFFER,
	STORAGE_BUFFER,
	UNIFORM_BUFFER,
	INDEX_BUFFER,
	TRANSFER_BUFFER
};

enum DescriptorType {
	UNIFORM_BUFFER_DESCRIPTOR,
	STORAGE_BUFFER_DESCRIPTOR,
	STORAGE_IMAGE_DESCRIPTOR,
	SAMPLED_IMAGE_DESCRIPTOR,
	COMBINED_IMAGE_SAMPLER_DESCRIPTOR
};

enum QueueFamily {
	GRAPHICS_QUEUE,
	COMPUTE_QUEUE,
	TRANSFER_QUEUE,
	UNKNOWN_QUEUE
};

enum AttachmentType {
	INPUT_TYPE,
	COLOR_TYPE,
	DEPTH_TYPE,
	RESOLVE_TYPE,
	PRESERVE_TYPE
};

enum TaskType {
	TASK_TYPE_COMPUTE,
	TASK_TYPE_GRAPHICS,
	TASK_TYPE_TRANSFER
};

enum CameraMovement {
	CAMERA_FORWARD,
	CAMERA_BACKWARD,
	CAMERA_LEFT,
	CAMERA_RIGHT,
	CAMERA_UP,
	CAMERA_DOWN
};

// +=================================+   
// | Helper Functions                |
// +=================================+
std::string vkresult_to_string(VkResult result);			// helper function to convert VkResult values to human-readable strings
VkDescriptorType get_descriptor_type(DescriptorType type);	// helper method to convert DescriptorType to VkDescriptorType
namespace std { template<> struct hash<Vertex>; };

// This function gets the directory of the currently running executable
std::filesystem::path get_executable_directory() {
#ifdef _WIN32
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path();
#else
	char buffer[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (len != -1) {
		buffer[len] = '\0';
		return std::filesystem::path(buffer).parent_path();
	}
	Log::error("Could not determine executable path.");
#endif
}

// +=================================+   
// | Instance                        |
// +=================================+

/// class for managing a Vulkan Instance,
/// which is a wrapper for all other Vulkan objects
class Instance {
public:
	Instance();										// default constructor
	Instance(Instance&& other) noexcept;			// move constructor
	Instance& operator=(Instance&& other) noexcept;	// move assignment
	Instance(const Instance&) = delete;				// copy constructor: deleted
	Instance& operator=(const Instance&) = delete;	// copy assignment: deleted
	~Instance();									// destructor

	// set application name and version
	void set_application(const char* name = "Vulkan Application", uint32_t major_version = 1, uint32_t minor_version = 0, uint32_t patch_version = 0);

	// initialize Vulkan engine and version
	void set_engine(const char* name = "", uint32_t major_version = 0, uint32_t minor_version = 0, uint32_t patch_version = 0);

	// initialize Vulkan API version
	void set_api_version(uint32_t major_version = 0, uint32_t minor_version = 0, uint32_t patch_version = 0);

	// log names of available instance layers
	void log_available_layers();

	// add Vulkan layers
	void enable_layers(const std::vector<const char*>& layer_names);
	void enable_layers(const char* layer_name);

	// log names of available Instance extensions
	void log_available_extensions() const;

	// add Vulkan extensions
	void enable_extensions(const std::vector<const char*>& extension_names);
	void enable_extensions(const char* extension_name);

	// create Vulkan instance
	void create(VkInstanceCreateFlags flags = 0, const void* pNext = nullptr);

	// get handle to the Vulkan instance
	VkInstance get() const;

protected:
	VkInstance instance = nullptr;
	VkApplicationInfo application_info = {};
	std::vector<const char*> extensions = {};
	std::vector<const char*> layers = {};
};

// +=================================+   
// | Device                          |
// +=================================+

// class for managing physical and logical GPU devices
class Device {
public:
	// default constructor: deleted
	Device() = delete;

	// parametric constructor
	Device(const Instance& instance, const VkPhysicalDeviceFeatures& enabled_features = {}, const std::vector<const char*>& enabled_extension_names = {}, uint32_t preferred_id = 0);

	// move constructor
	Device(Device&& other) noexcept;

	// destructor
	~Device();

	// move assignment
	Device& operator=(Device&& other) noexcept;

	// copy constructor & copy assignment: deleted
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;

	// select a suitable format the matches the requirements of both the surface and the physical device
	VkSurfaceFormatKHR select_surface_format(Surface& surface);

	// getters
	VkDevice get_logical() const;
	VkPhysicalDevice get_physical() const;
	VkQueue get_graphics_queue() const;
	VkQueue get_compute_queue() const;
	VkQueue get_transfer_queue() const;
	uint32_t get_graphics_queue_family_index() const;
	uint32_t get_compute_queue_family_index() const;
	uint32_t get_transfer_queue_family_index() const;
	const VkPhysicalDeviceProperties& get_properties() const;
	const VkPhysicalDeviceProperties2& get_properties2() const;
	const std::vector<const char*>& get_extensions() const;
	const std::vector<const char*>& get_device_extension_names() const;
	const uint32_t get_index() const;
	const uint32_t get_id() const;
	const VkPhysicalDeviceMemoryProperties& get_memory_properties();
	const VkPhysicalDeviceFeatures2& get_features() const;
	const VkPhysicalDeviceSynchronization2Features& get_synchronization_features() const;
	const VkPhysicalDeviceTimelineSemaphoreFeatures& get_timeline_sempahore_features() const;
	VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

protected:
	void destroy(); // helper method to release resources
	void move_resources(Device& other); // helper method for move constructor and move assignment

	VkPhysicalDevice physical = nullptr;
	VkDevice logical = nullptr;
	uint32_t index = 0;
	uint32_t id = 0;
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
	std::vector<const char*> extensions = {};
	std::vector<const char*> device_extension_names = {};
	VkPhysicalDeviceMemoryProperties memory_properties = {};
	VkPhysicalDeviceFeatures2 enabled_features2 = {}; // Vulkan 1.1+ feature set, can be extended with pNext
	VkPhysicalDeviceSynchronization2Features synchronization2_features = {}; // Vulkan 1.3+ feature set for synchronization2
	VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = {};
};

// +=================================+   
// | Image                           |
// +=================================+
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
	);

	// move constructor
	Image(Image&& other) noexcept;

	// move assignment
	Image& operator=(Image&& other) noexcept;

	// deleted copy & copy assignment constructors
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	// destructor
	~Image();

	// getters
	VkImage get() const;
	VkFormat get_format() const;
	VkExtent3D get_extent() const;
	VkImageLayout get_layout() const;
	VkDeviceMemory get_memory() const;

	// setters
	void set_layout(VkImageLayout new_layout);

protected:
	void destroy();
	uint32_t find_memory_type(Device& device, VkMemoryPropertyFlags properties, uint32_t type_filter); // helper method to find a suitable memory type

	VkDevice logical = VK_NULL_HANDLE;
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkExtent3D extent = {};
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED; // Track current layout
};

// +=================================+   
// | RenderPass                      |
// +=================================+

// a render pass defines the structure and dependencies of graphics rendering operations
class RenderPass {
	friend class SubPass;
public:
	// delete default constructor
	RenderPass() = delete;

	// parametric constructor
	RenderPass(Device& device, uint32_t multisample_count = 1);

	// move constructor
	RenderPass(RenderPass&& other) noexcept;

	// move assignment constructor
	RenderPass& operator=(RenderPass&& other) noexcept;

	// delete copy constructors
	RenderPass(const RenderPass&) = delete;
	RenderPass& operator=(const RenderPass&) = delete;

	// destructor
	~RenderPass();

	// adds an attachment description (=owned by this main RenderPass) and returns its index
	uint32_t add_attachment(
		AttachmentType type,
		VkImageLayout initial_layout,
		VkImageLayout final_layout,
		VkAttachmentLoadOp load_op,
		VkAttachmentStoreOp store_op,
		VkAttachmentLoadOp stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VkAttachmentStoreOp stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE
	);

	// adds a color attachment description (=owned by this main RenderPass) and returns its index
	uint32_t add_color_attachment(
		VkSurfaceFormatKHR surface_format,
		VkImageLayout initial_layout,
		VkImageLayout final_layout,
		VkAttachmentLoadOp load_op,
		VkAttachmentStoreOp store_op,
		VkAttachmentLoadOp stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VkAttachmentStoreOp stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE
	);

	// add subpass dependency
	VkSubpassDependency add_subpass_dependency(
		uint32_t source,
		uint32_t destination,
		VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VkAccessFlags src_access_mask = 0,
		VkAccessFlags dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	);

	void finalize();

	// getters
	VkRenderPass get() const;
	uint32_t get_multisample_count() const;
	uint32_t get_attachment_count() const;
	uint32_t get_subpass_count() const;
	bool has_depth_stencil() const;
	const std::vector<VkAttachmentDescription>& get_attachment_descriptions();
	const AttachmentType get_attachment_type(uint32_t index) const;


protected:
	void destroy();
	void move_resources(RenderPass& other);

	VkRenderPass renderpass = VK_NULL_HANDLE;
	Device* device = nullptr;
	VkDevice logical = VK_NULL_HANDLE;
	uint32_t multisample_count = 1;
	bool depth_stencil_flag = false;
	std::vector<VkAttachmentDescription> attachment_description;
	std::vector<AttachmentType> attachment_type;
	std::vector<VkSubpassDescription> subpass_description;
	std::vector<VkSubpassDependency> subpass_dependency;
};

// +=================================+   
// | SubPass                         |
// +=================================+
class SubPass {
public:
	SubPass() = delete;
	SubPass(RenderPass& renderpass);

	// deleted copy constructors
	SubPass(const SubPass&) = delete;
	SubPass& operator=(const SubPass&) = delete;

	// add a reference to an attachment from the pool of attachment descriptions owned by the main RenderPass
	void add_attachment_reference(uint32_t attachment_index);

	uint32_t get_index() const;

	// finalize the subpass and add it to the parent RenderPass;
	// returns the index of the subpass within the parent RenderPass
	uint32_t finalize(VkSubpassDescriptionFlags flags = 0, VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS);

protected:
	RenderPass* renderpass = nullptr; // pointer to the parent RenderPass
	std::vector<VkAttachmentReference> color_attachment_reference = {};
	std::unique_ptr<std::vector<VkAttachmentReference>> input_attachment_reference = nullptr;
	std::unique_ptr<std::vector<uint32_t>> preserve_attachment_reference = nullptr;
	std::unique_ptr<VkAttachmentReference> depth_attachment_reference = nullptr;
	std::unique_ptr<VkAttachmentReference> resolve_attachment_reference = nullptr;
	uint32_t index = 0;
	bool finalized = false; // this flag is updated by the parent RenderPass class (as a friend class) after the subpass has been added
};

// +=================================+   
// | Surface                         |
// +=================================+

// Platform Agnostic Surface Class
class Surface {
public:
	// Deleted default constructor (Surface always needs an instance and platform info)
	Surface() = delete;

	// Platform-Specific Constructors
#ifdef VK_USE_PLATFORM_WIN32_KHR
	Surface(const Instance& instance, HINSTANCE hinstance, HWND hwnd);
	Surface(const Instance& instance, GLFWwindow* window);
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	Surface(Instance& instance, ANativeWindow* window);
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
	Surface(Instance& instance, xcb_connection_t* connection, xcb_window_t window);
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
	Surface(Instance& instance, void* caMetalLayer);
#endif

	// Move constructor
	Surface(Surface&& other) noexcept;

	// Move assignment
	Surface& operator=(Surface&& other) noexcept;

	// Deleted copy operations
	Surface(const Surface&) = delete;
	Surface& operator=(const Surface&) = delete;

	// Destructor
	~Surface();

	// getters
	VkSurfaceKHR get() const;
	const bool is_valid() const;
	const bool get_physical_device_support(Device& device, QueueFamily queue_family) const;
	VkSurfaceCapabilitiesKHR get_capabilities(Device& device) const;
	std::vector<VkSurfaceFormatKHR> get_formats(Device& device) const;
	std::vector<VkPresentModeKHR> get_present_modes(Device& device) const;

protected:
	void destroy();
	VkInstance instance_handle = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
};

// +=================================+   
// | Fence                           |
// +=================================+

// for synchronization between GPU and CPU
class Fence {
public:
	// deleted default constructor
	Fence() = delete;

	// parametric constructor
	Fence(Device& device, bool signaled = false);

	// move constructor
	Fence(Fence&& other) noexcept;

	// move assignment
	Fence& operator=(Fence&& other) noexcept;

	// deleted copy constructor and assignment
	Fence(const Fence&) = delete;
	Fence& operator=(const Fence&) = delete;

	// destructor
	~Fence();

	// query fence status
	bool signaled(std::string caller_function = "UNKNOWN") const;

	// reset the fence to unsignaled state
	VkResult reset(std::string caller_function = "UNKNOWN") const;

	// wait for the fence to be signaled
	VkResult wait(std::string caller_function = "UNKNOWN", uint64_t timeout_nanosec = UINT64_MAX) const;

	// return the fence's Vulkan handle
	VkFence get() const;

private:
	VkFence fence = VK_NULL_HANDLE;
	VkDevice logical = VK_NULL_HANDLE;
};

// +=================================+   
// | Semaphore                       |
// +=================================+

// for synchronization on the GPU (between command buffers)
class Semaphore {
public:
	// default constructor: deleted
	Semaphore() = delete;

	// constructor for a binary semaphore
	explicit Semaphore(const Device& device, VkPipelineStageFlagBits2 wait_dst_stage_mask, VkPipelineStageFlagBits2 signal_dst_stage_mask);

	// Constructor for a timeline semaphore
	Semaphore(const Device& device, uint64_t initial_value, VkPipelineStageFlagBits2 wait_dst_stage_mask, VkPipelineStageFlagBits2 signal_dst_stage_mask);

	// destructor
	~Semaphore();

	// move constructor
	Semaphore(Semaphore&& other) noexcept;

	// move assignment
	Semaphore& operator=(Semaphore&& other) noexcept;

	// deleted copy constructor and assignment
	Semaphore(const Semaphore&) = delete;
	Semaphore& operator=(const Semaphore&) = delete;

	// wait for a timeline semaphore to reach the counter value as currently stored in the counter member variable (CPU call)
	// note: use with caution! this is a blocking CPU call and defeats the purpose of semaphores (=GPU/GPU synchronization) in most scenarios!
	// better approach: use wait semaphore on queue submit;
	// this method may still be useful when e.g. the semaphore is expected to already have signaled and to confirm before releasing resources
	VkResult wait(uint64_t timeout_nanosec);

	// wait for a timeline semaphore to reach a specific counter value (CPU call)
	// note: use with caution! this is a blocking CPU call and defeats the purpose of semaphores (=GPU/GPU synchronization) in most scenarios!
	// better approach: use wait semaphore on queue submit;
	// this method may still be useful when e.g. the semaphore is expected to already have signaled and to confirm before releasing resources
	VkResult wait(uint64_t wait_value, uint64_t timeout_nanosec);

	// query a timeline semaphore counter value (CPU call)
	uint64_t get_counter() const;

	// query the current value of the counter (member variable)
	uint64_t get_counter_var() const;

	// signal a timeline semaphore with a specified value (CPU call)
	void signal(uint64_t value) const;

	// flush current counter value (member variable) to semaphore signal (CPU call)
	void signal() const;

	// increment counter (member variable (atomic))
	uint64_t increment_counter();

	// set counter member variable atomically to a specific value
	void set_counter_var(uint64_t value);

	// getters
	VkSemaphore get() const;
	VkSemaphoreType get_type() const;
	const VkSemaphore* get_ptr() const;
	VkPipelineStageFlags2 get_wait_dst_stage_mask() const;
	VkPipelineStageFlags2 get_signal_dst_stage_mask() const;

private:
	VkSemaphore semaphore = VK_NULL_HANDLE;
	VkSemaphoreType type;
	VkDevice logical = VK_NULL_HANDLE;
	VkPipelineStageFlagBits2 wait_dst_stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // default value
	VkPipelineStageFlagBits2 signal_dst_stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // default value
	std::atomic<uint64_t> counter_value{ 0 }; // only used for timeline semaphores
};

// +=================================+   
// | Event                           |
// +=================================+

// events are used for synchronization between the CPU and GPU
class Event {
public:
	// constructor
	Event() = delete;
	Event(Device& device);

	// destructor
	~Event();

	// move constructor
	Event(Event&& other) noexcept;

	// move assignment
	Event& operator=(Event&& other) noexcept;

	// deleted copy constructor and assignment
	Event(const Event&) = delete;
	Event& operator=(const Event&) = delete;

	void reset();

	// set the event to signaled state
	void set() const;

	// query event status
	bool signaled() const;

	// getters
	const VkEvent& get() const;
	const VkDependencyInfo& get_dependency_info() const;
	VkDependencyInfo* get_dependency_info_ptr();

protected:
	VkDependencyInfo dependency_info = {};
	VkEvent event = VK_NULL_HANDLE;
	VkDevice logical = VK_NULL_HANDLE;

};

// +=================================+   
// | CommandPool                     |
// +=================================+
class CommandPool {
public:
	// deleted default constructor
	CommandPool() = delete;

	// parametric constructor
	CommandPool(Device& device, QueueFamily usage);

	// destructor
	~CommandPool();

	// deleted copy constructor and assignment
	CommandPool(const CommandPool&) = delete;
	CommandPool& operator=(const CommandPool&) = delete;

	// trim the command pool (release unused memory back to the system)
	void trim() const;

	// reset the command pool (release all resources allocated from the pool back to the pool)
	VkResult reset(VkCommandPoolResetFlags flags = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) const;

	// getters
	VkCommandPool get() const;
	QueueFamily get_usage() const;

private:
	VkCommandPool pool;
	VkDevice logical = VK_NULL_HANDLE;
	QueueFamily usage;
};

// +=================================+   
// | VertexDescriptions              |
// +=================================+
class VertexDescriptions {
public:
	VertexDescriptions();

	~VertexDescriptions();

	void add_attribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset = 0);

	// adds a binding to the vertex descriptions and returns the index of the new binding
	uint32_t add_binding(uint32_t stride, VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX);

	// getters
	const std::vector<VkVertexInputAttributeDescription>& get_attribute_descriptions() const;
	const std::vector<VkVertexInputBindingDescription>& get_input_bindings() const;

protected:
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {};
	std::vector<VkVertexInputBindingDescription> binding_descriptions = {};
};

// +=================================+   
// | ShaderModule                    |
// +=================================+
class ShaderModule {
public:
	// deleted default constructor
	ShaderModule() = delete;

	// constructor with binary data
	ShaderModule(Device& device, const unsigned char* binary, size_t size_bytes);

	// Constructor with folder & filename as C-style string literals
	ShaderModule(Device& device, const char* foldername, const char* filename);

	// constructor with folder & filename as std::string
	ShaderModule(Device& device, const std::string& foldername, const std::string& filename);

	// move constructor
	ShaderModule(ShaderModule&& other) noexcept;

	// move assignment
	ShaderModule& operator=(ShaderModule&& other) noexcept;

	// Deleted copy constructor and assignment
	ShaderModule(const ShaderModule&) = delete;
	ShaderModule& operator=(const ShaderModule&) = delete;

	// getters
	const VkShaderModule& get() const;

	// destructor
	~ShaderModule();

private:
	VkShaderModule module = VK_NULL_HANDLE;
	VkDevice logical = VK_NULL_HANDLE;
};

// +=================================+   
// | PushConstants                   |
// +=================================+
class PushConstants {
public:

	// default constructor
	PushConstants();

	// constructor with values as std::vector<T>
	template<typename T>
	PushConstants(const std::vector<T>& values);

	// constructor with values of mixed types as variadic template arguments
	template<typename... Args> PushConstants(Args... args);

	// destructor
	~PushConstants();

	// add a new value to the end of the push constants range;
	// the size of data type T must(!) be a multiple of 4;
	// returns the memory location offset of the new value within the push constants range
	template<typename T> size_t add_values(T value);

	// add a new value at the specified offset position of the push constants range;
	// overwrites the existing value at that position;
	// the size of data type T must(!) be a multiple of 4;
	// use this method with caution, as it may lead to undefined behavior if the offset (in bytes!) is not aligned correctly
	// or if data type T is different from the previously added data type (which may lead to data corruption)
	template<typename T> void add_values(T value, size_t offset);

	// add multiple new values to the push constants range as a std::initializer_list;
	// the size of data type T must(!) be a multiple of 4;
	// returns the byte offset of the last written element within the push constants range
	template<typename T> size_t add_values(std::initializer_list<T> values);

	// add multiple new values to the push constants range as a std::vector<T>;
	// the size of data type T must(!) be a multiple of 4;
	// returns the memory location byte offset of the new values within the push constants range
	template<typename T> size_t add_values(const std::vector<T>& new_data);

	// replace all previous constants starting from offset 0;
	// return the memory location byte offset of the end of the newly written range
	template<typename... Args> size_t replace_values(size_t begin_offset, Args... args);

	// free range by setting its size to zero (this doesn't affect the capacity!)
	void free();

	// getters
	const uint32_t* get_data() const;
	size_t get_size() const;
	size_t get_total_capacity() const;			// = total size of the occupied push constants range in bytes
	size_t get_free_capacity() const;			// = free space in bytes without reallocation

protected:
	template<typename T> constexpr size_t get_std430_alignment() const; // helper method
	template<typename T> size_t get_padded_offset();
	static constexpr float_t reserve = 0.5;    // reserve space for future growth (>=50% of current size)
	static constexpr size_t min_capacity = 32; // min capacity in bytes (should be a multiple of 4)
	uint32_t* data = nullptr;
	size_t size = 0;
	size_t capacity = min_capacity;
};

// +=================================+   
// | Buffer<T>                       |
// +=================================+

// buffer class for vertex data, index data, storage data, uniform data, etc.
template<typename T>
class Buffer {
public:

	// deleted default constructor:
	// non-parametric buffer construction not allowed;
	// all buffers must be created with a specific size and usage
	Buffer() = delete;

	// parametric constructor
	Buffer(Device& device, BufferType type, uint32_t elements, VkMemoryPropertyFlags memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Deleted copy constructor and copy assignment operator
	Buffer(const Buffer<T>& other) = delete;
	Buffer& operator=(const Buffer<T>& other) = delete;

	// Move constructor
	Buffer(Buffer<T>&& other) noexcept;

	// Move assignment
	Buffer& operator=(Buffer<T>&& other) noexcept;

	// copy data elements from a std::vector to a host visible buffer
	// (set copied elements to 0 to copy all)
	void write(const std::vector<T>& source_vector, uint32_t copied_elements = 0, uint32_t source_offset_elements = 0, uint32_t target_offset_elements = 0);

	// copy data elements from a std::array to a host visible buffer
	void write(const T* source_array, uint32_t copied_elements, uint32_t source_offset_elements = 0, uint32_t target_offset_elements = 0);

	// copy data elements from a std::initializer_list to a host visible buffer
	void write(const std::initializer_list<T> list, uint32_t target_offset_elements = 0);

	// copy data elements from one host visible buffer to another
	// (set copied elements to 0 to copy all);
	// the source buffer must be host visible;
	// both buffers must be of the same type
	void write(const Buffer<T>& sourcebuffer, uint32_t copied_elements = 0, uint32_t source_offset_elements = 0, uint32_t target_offset_elements = 0);


	// flush host writes to host memory domain
	// this is only necessary if the memory allocation doesn't have the flag VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	// (can be made available to the device memory domain by using a pipeline barrier with the VK_ACCESS_HOST_WRITE_BIT access type flag)
	void flush(uint64_t offset = 0, uint64_t size = VK_WHOLE_SIZE);

	// returns a continuous data sequence from a host visible data buffer as a std::vector<T>
	// (set read_elements to 0 to read all)
	std::vector<T> read(uint32_t read_elements = 0, uint32_t source_offset_elements = 0);

	// returns a single element from a host visible data buffer
	T read_element(uint32_t element_index) const;

	// assigns a single element of a host visible data buffer
	void write_element(uint32_t element_index, T value);

	// assigns the same value to continuous sequence of buffer elements
	// (set write_elements to 0 to assign all)
	void set_all(T value, uint32_t offset_elements = 0, uint32_t write_elements = 0);

	// getters
	uint32_t get_elements() const;
	uint64_t get_size_bytes() const;
	VkDeviceMemory get_memory() const;
	VkBuffer get() const;
	VkMemoryPropertyFlags get_memory_property_flags() const;
	bool host_visible() const;

	// destructor
	~Buffer();

protected:
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	uint32_t elements = 0;
	VkDevice logical = VK_NULL_HANDLE;
	VkMemoryPropertyFlags memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint64_t size_bytes = 0;
	bool is_host_visible = false;
};

// +=================================+   
// | Vertex                          |
// +=================================+
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coord;
	glm::vec4 color;
	uint32_t material_index;
	bool operator==(const Vertex& other) const;
};

// +=================================+   
// | Material                        |
// +=================================+
struct Material {
	std::string name;

	glm::vec3 ambient;	// ambient color of the material
	glm::vec3 diffuse;	// diffuse color, often the primary color of the object
	glm::vec3 specular;	// specular color, for highlights
	glm::vec3 transmittance; // how much light passes through the object (for transparant objects)
	glm::vec3 emission;	// color of light emitted by the material

	float shininess;	// specular exponent, controlling the size and sharpness of highlights
	float ior;			// index of refraction
	float dissolve;		// 1.0 = fully opaque, 0.0 = fully transparent
	int   illum;		// illumination model
	float roughness;

	// Texture map filenames
	std::string ambient_texname;
	std::string diffuse_texname;
	std::string specular_texname;
	std::string bump_texname;
	std::string displacement_texname;
	std::string alpha_texname;
	std::string reflection_texname;
};

struct MaterialStd430 {
	glm::vec3 ambient;			float padding1 = 1.0f; // Add padding to align the next vector to a 16-byte boundary.
	glm::vec3 diffuse;			float padding2 = 1.0f;
	glm::vec3 specular;			float padding3 = 1.0f;
	glm::vec3 transmittance;	float padding4 = 1.0f;
	glm::vec3 emission;			float padding5 = 1.0f;
	float shininess;
	float ior;
	float dissolve;
	int   illum;
	float roughness;

	// Delete default constructor to prevent uninitialized objects
	MaterialStd430() = delete;

	// Constructor to convert from the application-side Material struct
	// to shader-compliant std430 alignment
	MaterialStd430(const Material& material) :
		ambient(material.ambient),
		diffuse(material.diffuse),
		specular(material.specular),
		transmittance(material.transmittance),
		emission(material.emission),
		shininess(material.shininess),
		ior(material.ior),
		dissolve(material.dissolve),
		illum(material.illum),
		roughness(material.roughness)
	{
	}
};

// +=================================+   
// | SubMesh                         |
// +=================================+
struct SubMesh {
	uint32_t material_index;
	uint32_t first_index;
	uint32_t index_count;
	uint32_t vertex_offset;
};

// +=================================+   
// | Mesh                            |
// +=================================+
class Mesh {
public:
	// constructor
	Mesh() = delete;

	// parametric constructor
	Mesh(Device& device, const std::string& obj_relative_path, Semaphore* semaphore = nullptr);

	// destructor
	~Mesh();

	// getters
	const Buffer<Vertex>& get_vertex_buffer() const;
	const Buffer<uint32_t>& get_index_buffer() const;
	const Buffer<MaterialStd430>& get_material_buffer() const;
	uint32_t get_index_count() const;
	uint32_t get_vertex_count() const;
	const VertexDescriptions& get_vertex_descriptions() const;
	VkIndexType get_index_type() const;
	const std::vector<SubMesh>& get_submeshes() const;
private:
	Device& device;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Material> materials;
	std::vector<MaterialStd430> materials_std430;
	std::vector<SubMesh> submeshes;

	std::unique_ptr<Buffer<Vertex>> vertex_buffer;
	std::unique_ptr<Buffer<uint32_t>> index_buffer;
	std::unique_ptr<Buffer<MaterialStd430>> material_buffer;

	uint32_t index_count = 0;
	uint32_t vertex_count = 0;
	uint32_t material_count = 0;
	VertexDescriptions vertex_descriptions;
	VkIndexType index_type;
};

// +=================================+   
// | Entity                          |
// +=================================+
class Entity {
public:
	// deleted default constructor
	Entity() = delete;

	// parametric constructor
	Entity(Mesh& mesh);

	// destructor
	~Entity() = default;

	// Calculates the model matrix, applying the transformations in the correct order: scale, rotate, then translate.
	glm::mat4 get_model_matrix();

	// Public setter methods for transformations.
	void set_position(const glm::vec3& position);
	void set_rotation(const glm::vec3& rotation);
	void set_scale(const glm::vec3& scale);

	// Public getter method for the mesh.
	Mesh& get_mesh() const;

protected:
	// Transformation properties.
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	Mesh* mesh;
};


// +=================================+   
// | Camera                          |
// +=================================+

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors, and Matrices for use in rendering.
class Camera {
public:
	// Constructor with vectors
	Camera(glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f), float camera_yaw = CAMERA_YAW, float camera_pitch = CAMERA_PITCH);

	// Constructor with scalar values
	Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float camera_yaw, float camera_pitch);

	// Getters for matrices
	glm::mat4 get_view_matrix();
	glm::mat4 get_projection_matrix();

	// Setters for camera attributes
	void set_position(const glm::vec3& new_position);
	void set_world_up(const glm::vec3& new_world_up);
	void set_yaw(float new_yaw);
	void set_pitch(float new_pitch);
	void set_movement_speed(float new_speed);
	void set_mouse_sensitivity(float new_sensitivity);
	void set_fov(float new_fov);
	void set_aspect_ratio(float new_aspect_ratio);
	void set_near_plane(float new_near_plane);
	void set_far_plane(float new_far_plane);

	// Processes input
	void process_keyboard(CameraMovement direction, float delta_time);
	void process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch = true);
	void process_mouse_scroll(float y_offset);

private:
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	// Euler Angles
	float yaw;
	float pitch;

	// Camera options
	float movement_speed;
	float mouse_sensitivity;
	float fov;

	// Projection options
	float aspect_ratio;
	float near_plane;
	float far_plane;
};


// +=================================+   
// | Sample                          |
// +=================================+

// Sampler class for texture sampling
class Sampler {
public:
	// deleted default constructor
	Sampler() = delete;

	// parametric constructor
	Sampler(
		Device& device,
		VkFilter magFilter = VK_FILTER_LINEAR,
		VkFilter minFilter = VK_FILTER_LINEAR,
		VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VkBool32 anisotropyEnable = VK_TRUE,
		float maxAnisotropy = 16.0f
	);

	// destructor
	~Sampler();

	// getters
	const VkSampler get() const;
	const VkSampler* get_ptr() const;

protected:
	VkSampler sampler = nullptr;
	VkDevice logical = VK_NULL_HANDLE;
	VkSamplerCreateInfo sampler_create_info = {};
};

// +=================================+   
// | DescriptorBindingInfo           |
// +=================================+
struct DescriptorBindingInfo {
	uint32_t binding_index = 0;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceSize offset = 0;
	VkDeviceSize range = VK_WHOLE_SIZE;
	VkImageView image_view = VK_NULL_HANDLE;
	VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkSampler sampler = VK_NULL_HANDLE;
	VkBufferView buffer_view = VK_NULL_HANDLE;
	VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bool updated = false; // whether the descriptor set has been updated with the information of this binding info instance
};

// +=================================+   
// | DescriptorSetLayout             |
// +=================================+
class DescriptorSetLayout {
public:
	// delete default constructor
	DescriptorSetLayout() = delete;

	// constructor
	DescriptorSetLayout(Device& device);

	// destructor
	~DescriptorSetLayout();

	// deleted copy constructor and copy assignment operator
	DescriptorSetLayout(const DescriptorSetLayout& other) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout& other) = delete;

	// add binding for a buffer or image
	uint32_t add_binding(DescriptorType type, VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL, VkDescriptorBindingFlags binding_flags = 0);

	// add multiple bindings of the same type at once
	void add_bindings(uint32_t count, DescriptorType type, VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL);

	// finalizes the descriptor set layout and creates the descriptor set
	void finalize();

	// getters
	uint32_t get_bindings_count() const;
	const VkDescriptorSetLayoutBinding get_binding(uint32_t binding_index) const;
	const VkDescriptorSetLayout get() const;
	const VkDescriptorSetLayout* get_ptr() const;

private:
	VkDevice logical = VK_NULL_HANDLE;
	VkDescriptorSetLayout layout = nullptr;
	std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
	std::vector<VkDescriptorBindingFlags> binding_flags;
	bool finalized = false;
};

// +=================================+   
// | ImageView                       |
// +=================================+
class ImageView {
public:
	// constructor
	ImageView() = delete;
	ImageView(Device& device, Image& image, VkImageViewType view_type, VkImageAspectFlags aspect_flags, uint32_t base_mip_level = 0, uint32_t level_count = 1, uint32_t base_array_layer = 0, uint32_t layer_count = 1);

	// move constructor
	ImageView(ImageView&& other) noexcept;

	// move assignment
	ImageView& operator=(ImageView&& other) noexcept;

	// delete copy & copy assignment constructors
	ImageView(const ImageView&) = delete;
	ImageView& operator=(const ImageView&) = delete;

	// destructor
	~ImageView();

	// getters
	VkImageView get() const;

protected:
	void destroy();
	VkDevice logical = VK_NULL_HANDLE;
	VkImageView image_view = VK_NULL_HANDLE;
};

// +=================================+   
// | BufferView                      |
// +=================================+

// class for creating a view into a data buffer
template<typename T>
class BufferView {
public:
	// deleted default constructor
	BufferView() = delete;

	// parametric constructor
	BufferView(Device& device, Buffer<T>& buffer, VkFormat format);

	// move constructor
	BufferView(BufferView&& other) noexcept;

	// move assignment
	BufferView& operator=(BufferView&& other) noexcept;

	// deleted copy constructors
	BufferView(const BufferView&) = delete;
	BufferView& operator=(const BufferView&) = delete;

	// getters
	VkBufferView get() const;

	// destructor
	~BufferView();

protected:
	void destroy();
	VkDevice logical = VK_NULL_HANDLE;
	VkBufferView buffer_view = VK_NULL_HANDLE;
	Buffer<T>& buffer_ref;
	VkFormat format;
};

// +=================================+   
// | DepthBuffer                     |
// +=================================+

class DepthBuffer {
public:

	DepthBuffer(Device& device, VkImageType image_type, VkExtent3D extent, VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	~DepthBuffer();

	// Deleted copy constructor / copy assignment operator
	DepthBuffer(const DepthBuffer&) = delete;
	DepthBuffer& operator=(const DepthBuffer&) = delete;

	// move constructor / move assignment
	DepthBuffer(DepthBuffer&& other) noexcept;
	DepthBuffer& operator=(DepthBuffer&& other) noexcept;

	// Getters
	const ImageView& get_image_view() const;
	const Image& get_image() const;
	VkFormat get_format() const;
	VkDeviceMemory get_memory() const;

private:
	VkDevice logical = VK_NULL_HANDLE;
	std::unique_ptr<Image> image = nullptr;
	std::unique_ptr<ImageView> image_view = nullptr;
	VkDeviceMemory memory;
	VkFormat format;
};

// +=================================+   
// | Swapchain                       |
// +=================================+
class Swapchain {
public:
	// constructor
	Swapchain() = delete;
	Swapchain(
		Device& device,
		RenderPass& renderpass,
		Surface& surface,
		VkSurfaceFormatKHR surface_format,
		VkImageUsageFlags image_usage,
		DepthBuffer& depth_buffer,
		const std::optional<std::vector<VkImageView>>& other_image_views = NULLOPT,
		VkExtent2D extent = { 1920, 1080 },
		uint32_t min_image_count = 3,
		VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D,
		VkPresentModeKHR present_mode_preference = VK_PRESENT_MODE_FIFO_KHR
	);

	// destructor
	~Swapchain();

	uint32_t acquire_next_image(const Semaphore& image_available_semaphore, const std::optional<Fence>& fence = NULLOPT, uint64_t timeout = UINT64_MAX);

	// present the rendered image to the graphics queue (method overload without semaphores)
	VkResult present_rendered_image();

	// present the rendered image to the graphics queue (method overload with single semaphore)
	VkResult present_rendered_image(const Semaphore& wait_semaphore);

	// present the rendered image to the graphics queue (method overload with multiple semaphores)
	VkResult present_rendered_image(const std::vector<Semaphore>& wait_semaphores);

	void recreate();

	// getters
	uint32_t get_width() const;
	uint32_t get_height() const;
	VkExtent2D get_extent() const;
	VkSwapchainKHR get() const;
	VkImage get_current_image() const;
	uint32_t get_current_image_index() const;
	uint32_t get_image_count() const;
	VkSurfaceFormatKHR get_surface_format() const;
	VkImageView get_color_image_view(uint32_t index) const;
	VkImageView get_framebuffer_image_view(uint32_t index) const;
	VkFramebuffer get_framebuffer(uint32_t index) const;
	VkImage get_image(uint32_t index) const;

protected:
	void destroy();
	uint32_t num_images = 0;
	std::vector<VkImage> image;
	std::vector<VkImageView> color_image_views;
	std::vector<VkImageView> framebuffer_image_views;
	std::vector<VkFramebuffer> framebuffer;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	VkExtent2D extent = { 1920, 1080 };
	VkSurfaceFormatKHR surface_format;
	VkImageUsageFlags image_usage;
	VkColorSpaceKHR color_space;
	VkDevice logical = VK_NULL_HANDLE;
	Device* device = nullptr;					// pointer to the device that created this Swapchain (the device object must outlive this Swapchain; the device itself is NOT OWNED BY THIS CLASS !)
	RenderPass* renderpass = nullptr;
	Surface* surface = nullptr;					// pointer to the surface that is associated with this Swapchain (the surface object must outlive this Swapchain; the surface itself is NOT OWNED BY THIS CLASS !)
	DepthBuffer* depth_buffer = nullptr;
	const std::optional<std::vector<VkImageView>>& other_image_views;
	uint32_t current_image_index = 0;
	VkImageViewType view_type;
	VkPresentModeKHR present_mode_preference = VK_PRESENT_MODE_FIFO_KHR;
};

// +=================================+   
// | DescriptorPool                  |
// +=================================+

// DescriptorPool manages descriptor sets and their memory allocation
class DescriptorPool {
	friend class DescriptorSet;
public:
	// constructors
	DescriptorPool() = delete;
	DescriptorPool(Device& device, const uint32_t max_sets = 20, const std::vector<VkDescriptorPoolSize>& pool_sizes = { {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20} });

	// destructor
	~DescriptorPool();

	// move constructor
	DescriptorPool(DescriptorPool&& other) noexcept;

	// move assignment
	DescriptorPool& operator=(DescriptorPool&& other) noexcept;

	// deleted copy constructor and assignment
	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator=(const DescriptorPool&) = delete;

	// getters
	VkDescriptorPool get() const;
	const std::vector<VkDescriptorSet>& get_sets() const;
	uint32_t get_max_sets() const;
	uint32_t get_current_sets_count() const;

	// release all descriptor sets from the pool
	void release_all_sets();

	uint32_t release_set(VkDescriptorSet set);

	uint32_t allocate_set(VkDescriptorSet& descriptor_set, DescriptorSetLayout& descriptor_set_layout);

private:
	VkDescriptorPool pool = VK_NULL_HANDLE;
	VkDevice logical = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> sets;
	uint32_t max_sets = 0;
};

// +=================================+   
// | DescriptorSet                   |
// +=================================+

// DescriptorSets hold binding information for shader resources
class DescriptorSet {
	friend class DescriptorPool;
public:
	// delete default constructor
	DescriptorSet() = delete;

	// parametric constructor
	DescriptorSet(Device& device, DescriptorSetLayout& layout, DescriptorPool& pool);

	// destructor
	~DescriptorSet();

	// move constructor
	DescriptorSet(DescriptorSet&& other) noexcept;

	// move assignment
	DescriptorSet& operator=(DescriptorSet&& other) noexcept;

	// deleted copy constructor and assignment
	DescriptorSet(const DescriptorSet&) = delete;
	DescriptorSet& operator=(const DescriptorSet&) = delete;

	// binds a buffer to the descriptor set
	template<typename T>
	void bind_buffer(uint32_t binding_index, const Buffer<T>& buffer, VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL);

	// bind an image to the descriptor set
	void bind_image(
		uint32_t binding_index,
		const ImageView& image_view,
		DescriptorType type,
		const Sampler& sampler,
		VkImageLayout image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL
	);

	// binds a sampler
	void bind_sampler(uint32_t binding_index, const Sampler& sampler, VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL);

	// binds a storage image
	void bind_storage_image(
		uint32_t binding_index,
		const ImageView& image_view,
		VkImageLayout image_layout = VK_IMAGE_LAYOUT_GENERAL,
		VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL
	);

	// binds a texel buffer view (uniform or storage)
	template<typename T>
	void bind_texel_buffer(uint32_t binding_index, const BufferView<T>& buffer_view, VkShaderStageFlagBits shader_stage_flags = VK_SHADER_STAGE_ALL);

	// updates the descriptor set with the current bindings
	void write();

	// getters
	VkDescriptorSet get() const;
	DescriptorSetLayout& get_layout() const;
	const std::vector<DescriptorBindingInfo>& get_binding_info() const;

private:
	DescriptorSetLayout& layout;
	DescriptorPool& pool;
	VkDevice logical = nullptr;
	VkDescriptorSet set = nullptr;
	std::vector<DescriptorBindingInfo> binding_info;
};

// +=================================+
// | ColorBlendState                 |
// +=================================+

struct ColorBlendState {
	ColorBlendState(
		VkBlendFactor src_color_blend_factor,
		VkBlendFactor dst_color_blend_factor,
		VkBlendOp color_blend_op,
		VkBlendFactor src_alpha_blend_factor,
		VkBlendFactor dst_alpha_blend_factor,
		VkBlendOp alpha_blend_op,
		float_t blend_constants[4],
		VkPipelineColorBlendStateCreateFlags flags = 0,
		VkLogicOp logic_op = VK_LOGIC_OP_CLEAR
	) : attachment_state(
		VK_TRUE,	// blendEnable
		src_color_blend_factor,
		dst_color_blend_factor,
		color_blend_op,
		src_alpha_blend_factor,
		dst_alpha_blend_factor,
		alpha_blend_op,
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT // colorWriteMask
	),
		logic_op(logic_op),
		flags(flags) {
		for (uint32_t i = 0; i < 4; i++) {
			this->blend_constants.push_back(blend_constants[i]);
		}
	}

	const VkPipelineColorBlendAttachmentState& get_attachment() const { return attachment_state; };
	const VkLogicOp get_logic_op() const { return logic_op; };
	const VkPipelineColorBlendStateCreateFlags get_flags() const { return flags; }
	const std::vector<float_t>& get_blend_constants() const { return blend_constants; }
private:
	VkLogicOp logic_op;
	std::vector<float_t> blend_constants;
	VkPipelineColorBlendStateCreateFlags flags;
	VkPipelineColorBlendAttachmentState attachment_state = {};
};

// +=================================+
// | GraphicsPipelineLayout          |
// +=================================+
// This class encapsulates the VkPipelineLayout handle. It is responsible for
// creating the pipeline layout based on provided descriptor set layouts and
// push constant ranges.
class GraphicsPipelineLayout {
public:
	// Deleted default constructor to prevent creation without parameters.
	GraphicsPipelineLayout() = delete;

	// Constructors for different combinations of descriptor sets and push constants.
	GraphicsPipelineLayout(Device& device, const DescriptorSetLayout& set_layout, const PushConstants& constants);
	GraphicsPipelineLayout(Device& device, const std::vector<DescriptorSetLayout>& set_layouts, const std::vector<PushConstants>& constants);
	GraphicsPipelineLayout(Device& device, const DescriptorSetLayout& set_layout, const std::vector<PushConstants>& constants);
	GraphicsPipelineLayout(Device& device, const std::vector<DescriptorSetLayout>& set_layouts, const PushConstants& constants);

	// Destructor to clean up the Vulkan handle.
	~GraphicsPipelineLayout();

	// Deleted copy constructor and copy assignment operator to prevent
	// accidental copying of Vulkan handles.
	GraphicsPipelineLayout(const GraphicsPipelineLayout& other) = delete;
	GraphicsPipelineLayout& operator=(const GraphicsPipelineLayout& other) = delete;

	// Move constructor for efficient resource transfer.
	GraphicsPipelineLayout(GraphicsPipelineLayout&& other) noexcept;

	// Move assignment operator for efficient resource transfer.
	GraphicsPipelineLayout& operator=(GraphicsPipelineLayout&& other) noexcept;

	// Getter to retrieve the underlying VkPipelineLayout handle.
	VkPipelineLayout get() const;

protected:
	VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
	VkDevice logical = VK_NULL_HANDLE;
};

// +=================================+   
// | GraphicsPipeline                |
// +=================================+
class GraphicsPipeline {
public:
	// constructor
	GraphicsPipeline() = delete;
	GraphicsPipeline(
		Device& device,
		const RenderPass& renderpass,
		uint32_t subpass_index,
		const GraphicsPipelineLayout& pipeline_layout,
		const ShaderModule& vertex_shader,
		const std::vector<VertexDescriptions>& vertex_descriptions,
		const std::optional<ColorBlendState>& color_blend_state = NULLOPT,
		const std::optional<ShaderModule>& fragment_shader = NULLOPT,
		const std::optional<ShaderModule>& hull_shader = NULLOPT,
		const std::optional<ShaderModule>& domain_shader = NULLOPT,
		uint32_t tessellation_patch_control_points = 3,
		VkPipelineDepthStencilStateCreateFlagBits depth_stencil_flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT
	);

	// destructor
	~GraphicsPipeline();

	// deleted copy constructor and copy assignment operator
	GraphicsPipeline(const GraphicsPipeline& other) = delete;
	GraphicsPipeline& operator=(const GraphicsPipeline& other) = delete;

	// move constructor
	GraphicsPipeline(GraphicsPipeline&& other) noexcept;

	// move assignment
	GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

	// getters
	VkPipeline get() const;
	const VkPipelineLayout& get_layout() const;

protected:
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipeline_layout;
	VkDevice logical = VK_NULL_HANDLE;
};

// +=================================+   
// | ComputePipeline                 |
// +=================================+
class ComputePipeline {
public:
	// deleted default constructor
	ComputePipeline() = delete;

	// parametric constructor
	ComputePipeline(
		const Device& device,
		const ShaderModule& compute_shader_module,
		uint32_t push_constants_range_size,
		DescriptorSetLayout& descriptor_set_layout,
		uint32_t workgroup_size_x,
		uint32_t workgroup_size_y = 1,
		uint32_t workgroup_size_z = 1,
		std::vector<uint32_t> addon_specialization_constants = {}
	);

	// destructor
	~ComputePipeline();

	// deleted copy constructor and copy assignment operator
	ComputePipeline(const ComputePipeline& other) = delete;
	ComputePipeline& operator=(const ComputePipeline& other) = delete;

	// move constructor
	ComputePipeline(ComputePipeline&& other) noexcept;

	// move assignment
	ComputePipeline& operator=(ComputePipeline&& other) noexcept;

	// getters
	VkPipeline get() const;
	VkPipelineLayout get_layout() const;
	const DescriptorSetLayout& get_descriptor_set_layout() const;

	uint32_t get_workgroup_size_x() const;
	uint32_t get_workgroup_size_y() const;
	uint32_t get_workgroup_size_z() const;

private:
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkDevice logical = VK_NULL_HANDLE;
	DescriptorSetLayout* descriptor_set_layout = nullptr;
	uint32_t workgroup_size_x = 0;
	uint32_t workgroup_size_y = 0;
	uint32_t workgroup_size_z = 0;
	std::vector<uint32_t> specialization_data;
	std::vector<VkSpecializationMapEntry> specialization_map_entries;
};

// +=================================+   
// | CommandBuffer                   |
// +=================================+

// command buffer for recording commands;
// used for graphics, compute and transfer operations
class CommandBuffer {
public:
	// deleted default constructor
	CommandBuffer() = delete;

	// parametric constructor
	CommandBuffer(Device& device, const CommandPool& pool);

	// move constructor
	CommandBuffer(CommandBuffer&& other) noexcept;

	// move assignment
	CommandBuffer& operator=(CommandBuffer&& other) noexcept;

	// copy constructor & copy assignment (deleted; copying command buffers is not allowed)
	CommandBuffer(const CommandBuffer&) = delete;
	CommandBuffer& operator=(const CommandBuffer&) = delete;

	// destructor
	~CommandBuffer();

	// handle events
	Event set_event(
		const std::optional<std::vector<VkMemoryBarrier2>>& device_memory_barriers,
		const std::optional<std::vector<VkBufferMemoryBarrier2>>& buffer_memory_barriers,
		const std::optional<std::vector<VkImageMemoryBarrier2>>& image_memory_barriers,
		VkDependencyFlags flags) const;
	void reset_event(const Event& event, VkPipelineStageFlags stage_mask) const;
	void wait_event(const Event& event) const;

	// bind pipeline to command buffer
	void bind_pipeline(ComputePipeline& pipeline);
	void bind_pipeline(GraphicsPipeline& pipeline);

	// bind push constants to command buffer
	void bind_push_constants(PushConstants& constants, ComputePipeline& pipeline) const;
	void bind_push_constants(PushConstants& constants, GraphicsPipeline& pipeline) const;

	// bind descriptor set to command buffer
	void bind_descriptor_set(DescriptorSet& set, ComputePipeline& pipeline) const;
	void bind_descriptor_set(DescriptorSet& set, GraphicsPipeline& pipeline);

	// bind mesh model to command buffer
	void bind_mesh(Mesh& mesh);

	// copy data between two buffers;
	// this method has automatic boundary checks and shrinks the copy region to fit if the size_bytes argument is too large
	template<typename T>
	void copy_buffer(const Buffer<T>& src_buffer, Buffer<T>& dst_buffer, uint64_t size_bytes = UINT64_MAX, uint64_t src_offset_bytes = 0, uint64_t dst_offset_bytes = 0);

	void add_device_memory_barrier(
		VkPipelineStageFlags2 source_stage_flags,
		VkAccessFlags2 source_access_flags,
		VkPipelineStageFlags2 target_stage_flags,
		VkAccessFlags2 target_access_flags
	);

	template<typename T>
	void add_buffer_memory_barrier(
		Buffer<T>& buffer,
		VkAccessFlags2 src_access_flags = VK_ACCESS_2_SHADER_WRITE_BIT,
		VkAccessFlags2 dst_access_flags = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
		VkPipelineStageFlags2 src_stage_flags = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		VkPipelineStageFlags2 dst_stage_flags = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		uint32_t src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
		uint32_t dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED
	);

	void add_image_memory_barrier(
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
	);

	void record_barriers();

	// transition image layout
	void transition_image_layout(
		Image& image,
		VkImageLayout new_layout,
		VkImageAspectFlags aspect_mask,
		VkPipelineStageFlags2 src_stage,
		VkAccessFlags2 src_access,
		VkPipelineStageFlags2 dst_stage,
		VkAccessFlags2 dst_access
	);

	// transition image layout, overloaded method for common transitions (=simplified usage)
	void transition_image_layout(
		Image& image,
		VkImageLayout new_layout,
		VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT
	);

	// record draw commands
	void draw(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) const;
	void draw_indexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t first_instance = 0) const;

	// command buffer state control
	void dispatch(const ComputePipeline& pipeline, uint32_t global_size_x, uint32_t global_size_y = 1, uint32_t global_size_z = 1) const; // dispatch for compute
	void begin_render(VkOffset2D offset, VkExtent2D extent, VkRenderingFlags flags, std::vector<VkRenderingAttachmentInfo>& color_attachments, VkRenderingAttachmentInfo& depth_attachment, VkRenderingAttachmentInfo& stencil_attachment) const;
	void begin_renderpass(RenderPass& renderpass, VkFramebuffer framebuffer, VkOffset2D offset, VkExtent2D extent, std::vector<VkClearValue>& clear_value) const;
	void end_renderpass() const;
	void next_subpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
	VkResult begin_recording() const; // start command buffer recording state
	void end_recording() const; // end command buffer recording state (thus triggering executable state)
	VkResult reset(VkCommandBufferResetFlags flags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	// getters
	VkCommandBuffer& get();
	QueueFamily get_usage() const;
	VkQueue get_queue() const;

private:
	VkCommandBuffer buffer = nullptr;
	QueueFamily usage = QueueFamily::UNKNOWN_QUEUE;
	VkDevice logical = nullptr;
	Device* device = nullptr;
	VkQueue queue = nullptr;
	VkRenderingInfo rendering_info = {};
	VkCommandPool pool = VK_NULL_HANDLE;
	std::vector<VkMemoryBarrier2> device_memory_barriers;
	std::vector<VkBufferMemoryBarrier2> buffer_memory_barriers;
	std::vector<VkImageMemoryBarrier2> image_memory_barriers;
};

// +=================================+   
// | Task                            |
// +=================================+

// resources container for a compute or graphics task,
// managed by the VulkanManager class to ensure the lifetime
// of the resources exceeds the scope of any functions which
// record commands related to the given task
class Task {
	friend class VulkanManager;
public:
	// constructor
	Task() = delete;
	Task(Device& device, CommandPool& command_pool, DescriptorPool& descriptor_pool, std::string calling_function = "UNKNOWN");

	// destructor
	~Task();

	// move constructor
	Task(Task&& other) noexcept;

	// create temporary buffers (=which can be deleted after task execution has finished);
	// returns a reference to the new Buffer<T> object
	template<typename T> Buffer<T>& add_temp_buffer(
		uint32_t elements,
		BufferType usage = BufferType::STORAGE_BUFFER,
		VkMemoryPropertyFlags memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	// create a temporary descriptor set layout, owned by this task;
	// returns a reference to the new DescriptorSetLayout object;
	// a task is designed to own only one descriptor set layout;
	// if a layout already exists, it will be replaced and a warning is issued;
	// if you need multiple layouts, please create them externally and provide them to the task using Task::add_descriptor_set(DescriptorSetLayout&);
	// also keep in mind that a descriptor set layout owned by the task is no longer valid beyond the lifetime of the task!
	DescriptorSetLayout& add_descriptor_set_layout();

	// add a descriptor set for the set layout owned by this compute task
	DescriptorSet& add_descriptor_set();

	// add a descriptor set for the referenced (=externally owned) set layout
	DescriptorSet& add_descriptor_set(DescriptorSetLayout& descriptor_set_layout);

	// add a compute shader module from SPIR-V binary data and returns its ID within this task
	uint32_t add_shader(const unsigned char* compute_shader_spirv_bin, size_t compute_shader_spirv_bytes);

	template<typename ...Args> PushConstants& add_constants(Args... args);

	ComputePipeline& add_pipeline(
		uint32_t shader_id,
		uint32_t push_constants_range_size,
		DescriptorSetLayout& set_layout,
		uint32_t workgroup_size_x,
		uint32_t workgroup_size_y = 1,
		uint32_t workgroup_size_z = 1,
		std::vector<uint32_t> addon_specialization_constants = {}
	);

	ComputePipeline& add_pipeline(
		uint32_t shader_id,
		DescriptorSetLayout& set_layout,
		uint32_t workgroup_size_x,
		uint32_t workgroup_size_y = 1,
		uint32_t workgroup_size_z = 1,
		std::vector<uint32_t> addon_specialization_constants = {}
	);

	ComputePipeline& add_pipeline(
		uint32_t shader_id,
		uint32_t push_constants_range_size,
		uint32_t workgroup_size_x,
		uint32_t workgroup_size_y = 1,
		uint32_t workgroup_size_z = 1,
		std::vector<uint32_t> addon_specialization_constants = {}
	);

	ComputePipeline& add_pipeline(
		uint32_t shader_id,
		uint32_t workgroup_size_x,
		uint32_t workgroup_size_y = 1,
		uint32_t workgroup_size_z = 1,
		std::vector<uint32_t> addon_specialization_constants = {}
	);

	GraphicsPipeline& add_pipeline(
		const RenderPass& renderpass,
		uint32_t subpass_index,
		const GraphicsPipelineLayout& pipeline_layout,
		const ShaderModule& vertex_shader,
		const std::vector<VertexDescriptions>& vertex_descriptions,
		const std::optional<ColorBlendState>& color_blend_state = NULLOPT,
		const std::optional<ShaderModule>& fragment_shader = NULLOPT,
		const std::optional<ShaderModule>& hull_shader = NULLOPT,
		const std::optional<ShaderModule>& domain_shader = NULLOPT,
		uint32_t tessellation_patch_control_points = 3,
		VkPipelineDepthStencilStateCreateFlagBits depth_stencil_flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT
	);

	// creates and adds a temporary timeline semaphore owned by this task;
	// this resource can then be used as a wait or signal semaphore
	Semaphore& add_temp_timeline_semaphore(
		uint64_t initial_value = 0,
		VkPipelineStageFlagBits2 wait_dst_stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		VkPipelineStageFlagBits2 signal_dst_stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	);

	// creates and adds a temporary binary semaphore owned by this task;
	// this resource can then be used as a wait or signal semaphore
	Semaphore& add_temp_binary_semaphore(
		VkPipelineStageFlagBits2 wait_dst_stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		VkPipelineStageFlagBits2 signal_dst_stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
	);

	// adds a reference to a binary wait semaphore (which will be used during the submit call);
	// used to add externally owned semaphores (i.e. not owned by this task);
	// returns the total number of binary wait semaphores which are currently observed by this task
	uint32_t add_binary_wait_semaphore(Semaphore& semaphore);

	// adds a reference to a binary signal semaphore (which will be used during the submit call);
	// used to add externally owned semaphores (i.e. not owned by this task);
	// returns the total number of signal semaphores which are currently affected by this task
	uint32_t add_binary_signal_semaphore(Semaphore& semaphore);

	// adds a single binary semaphore which functions as a wait semaphore at the beginning of the command buffer execution,
	// then finally it changes back to the signaled state at the end of execution;
	// the semaphore MUST be expected to initially reach the signaled state (or already be in the signaled state),
	// otherwise a deadlock will occur (waiting for the signaled state indefinitely) !!!
	// the function expects a reference to an externally owned binary semaphore (i.e. not owned by this task)
	void binary_wait_and_signal(Semaphore& semaphore);

	// adds a single timeline semaphore which functions as a wait semaphore at the beginning of the command buffer execution,
	// then it signals the specified signal_value at the end of execution;
	// this function expects a reference to an externally owned timeline semaphore (i.e. not owned by this task)
	void timeline_sync(Semaphore& semaphore, uint64_t wait_value, uint64_t signal_value);

	// adds a single semaphore which functions as a wait semaphore at the beginning of the command buffer execution with its current counter value,
	// then this counter gets incremented (atomically) by one and is used for the signaled state at the end of execution;
	// returns the new signal value
	uint64_t timeline_sync(Semaphore& semaphore);

	// adds a reference to a timeline wait semaphore (which will be used during the submit call);
	// used to add externally owned semaphores (i.e. not owned by this task);
	// returns the total number of timeline wait semaphores which are currently observed by this task
	uint32_t add_timeline_wait_semaphore(Semaphore& semaphore, uint64_t wait_value);

	// adds a reference to a timeline signal semaphore (which will be used during the submit call);
	// used to add externally owned semaphores (i.e. not owned by this task);
	// returns the total number of timeline signal semaphores which are currently affected by this task
	uint32_t add_timeline_signal_semaphore(Semaphore& semaphore, uint64_t signal_value);

	// reset task (delete all previous resources);
	// returns false if the task is protected or still busy
	bool reset();

	// submit command buffer to appropriate queue on device;
	VkResult submit(uint64_t wait_after_submit_nanosec = 0);

	// getters
	std::string get_calling_function() const;
	bool available();
	DescriptorSet& get_set() const;
	CommandBuffer& get_command_buffer();
	Fence& get_fence();
	PushConstants& get_constants();

private:
	std::unique_ptr<CommandBuffer> command_buffer = nullptr;
	std::unique_ptr<Fence> fence = nullptr;
	std::unique_ptr<PushConstants> constants = nullptr;
	std::unique_ptr<DescriptorSetLayout> set_layout = nullptr;
	std::unique_ptr<DescriptorSet> set = nullptr;
	CommandPool* command_pool = nullptr;
	DescriptorPool* descriptor_pool = nullptr;
	Device* device = nullptr;
	std::vector<std::unique_ptr<ComputePipeline>> compute_pipelines;
	std::vector<std::unique_ptr<GraphicsPipeline>> graphics_pipelines;
	std::vector<std::unique_ptr<ShaderModule>> shaders;
	std::vector<std::variant<
		std::unique_ptr<Buffer<float_t>>,
		std::unique_ptr<Buffer<double_t>>,
		std::unique_ptr<Buffer<int8_t>>,
		std::unique_ptr<Buffer<int16_t>>,
		std::unique_ptr<Buffer<int32_t>>,
		std::unique_ptr<Buffer<int64_t>>,
		std::unique_ptr<Buffer<uint8_t>>,
		std::unique_ptr<Buffer<uint16_t>>,
		std::unique_ptr<Buffer<uint32_t>>,
		std::unique_ptr<Buffer<uint64_t>>,
		std::unique_ptr<Buffer<glm::vec2>>,
		std::unique_ptr<Buffer<glm::vec3>>,
		std::unique_ptr<Buffer<glm::vec4>>,
		std::unique_ptr<Buffer<glm::mat4>>,
		std::unique_ptr<Buffer<Vertex>>,
		std::unique_ptr<Buffer<MaterialStd430>>
		>> temp_buffers;
	std::vector<std::unique_ptr<Semaphore>> temp_semaphores;
	std::vector<VkSemaphore> binary_wait_semaphores;
	std::vector<VkSemaphore> timeline_wait_semaphores;
	std::vector<VkSemaphore> binary_signal_semaphores;
	std::vector<VkSemaphore> timeline_signal_semaphores;
	std::vector<Semaphore*> timeline_signal_semaphores_ptrs; // note: this class typically does not own the Semaphores that are being pointed to! Clearing this vector is okay, but this class shouldn't call 'delete' on these pointers!
	std::vector<uint64_t> timeline_semaphore_wait_values;
	std::vector<uint64_t> timeline_semaphore_signal_values;
	std::vector<VkPipelineStageFlags2> binary_wait_semaphores_dst_stage_masks;
	std::vector<VkPipelineStageFlags2> binary_signal_semaphores_dst_stage_masks;
	std::vector<VkPipelineStageFlags2> timeline_wait_semaphores_dst_stage_masks;
	std::vector<VkPipelineStageFlags2> timeline_signal_semaphores_dst_stage_masks;
	std::string calling_function = "NONE (=TASK IS AVAILABLE)";
	bool protection_flag = true; // if true, the task cannot be reset or deleted at least until submit
	static uint64_t num_created;
	static uint64_t num_destroyed;
	TaskType task_type;
};

// +=================================+   
// | VulkanManager                   |
// +=================================+

// shared manager for instance, device and command pools singleton class
class VulkanManager {
public:
	// create a singleton with default device features
	static VulkanManager& get_singleton();

	// destructor
	~VulkanManager();

	// a singleton class should not be copied or moved
	VulkanManager(const VulkanManager&) = delete;
	VulkanManager& operator=(const VulkanManager&) = delete;
	VulkanManager(VulkanManager&&) = delete;
	VulkanManager& operator=(VulkanManager&&) = delete;

	// get an available (=idle) task which can be used for new resources
	static Task& get_compute_task(std::string calling_function = "");
	static Task& get_graphics_task(std::string calling_function = "");
	static Task& get_transfer_task(std::string calling_function = "");
	static void log_tasks();

	static Surface& get_surface(uint32_t initial_width = 1920, uint32_t initial_height = 1080, const char* window_title = "Vulkan Window", GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);

	// provides a raw pointer to a timeline semaphore owned by the VulkanManager singleton;
	// this can be helpful to make sure the semaphore's lifetime exceeds local function scopes;
	Semaphore* get_timeline_semaphore(uint64_t initial_value, VkPipelineStageFlagBits2 wait_dst_stage_mask, VkPipelineStageFlagBits2 signal_dst_stage_mask);

	// getters
	static Device& get_device();
	static const Device* get_device_ptr();
	static const Instance& get_instance();
	static CommandPool& get_command_pool_graphics();
	static CommandPool& get_command_pool_compute();
	static CommandPool& get_command_pool_transfer();
	static DescriptorPool& get_descriptor_pool();
	static const VkPhysicalDeviceFeatures& get_enabled_device_features();

private:
	// shared members
	static std::unique_ptr<Instance> instance;
	static std::unique_ptr<Device> device;
	static std::unique_ptr<VulkanManager> singleton;
	static std::vector<const char*> shared_instance_layer_names;
	static std::vector<const char*> shared_instance_extension_names;
	static std::vector<const char*> shared_device_extension_names;
	static VkPhysicalDeviceFeatures shared_enabled_device_features;
	static uint32_t shared_default_device_id;
	static uint32_t shared_api_major_version;
	static uint32_t shared_api_minor_version;
	static uint32_t shared_api_patch_version;
	static std::unique_ptr<CommandPool> shared_command_pool_compute;
	static std::unique_ptr<CommandPool> shared_command_pool_graphics;
	static std::unique_ptr<CommandPool> shared_command_pool_transfer;
	static std::unique_ptr<DescriptorPool> shared_descriptor_pool;
	static std::vector<std::unique_ptr<Task>> tasks;
	static std::vector<std::unique_ptr<Semaphore>> timeline_semaphores;
	static std::unique_ptr<Surface> surface;
	static GLFWwindow* glfw_window;

	VulkanManager(); // private constructor
};






// ===============================================================================================================================================================================
// DEFINITIONS:




// +=================================+   
// | Helper Functions                |
// +=================================+

// helper method to convert DescriptorType to VkDescriptorType
VkDescriptorType get_descriptor_type(DescriptorType type) {
	switch (type) {
	case DescriptorType::STORAGE_BUFFER_DESCRIPTOR: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case DescriptorType::UNIFORM_BUFFER_DESCRIPTOR: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case DescriptorType::SAMPLED_IMAGE_DESCRIPTOR:  return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case DescriptorType::STORAGE_IMAGE_DESCRIPTOR:  return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case DescriptorType::COMBINED_IMAGE_SAMPLER_DESCRIPTOR: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	default: Log::warning("In DescriptorSet::get_descriptor_type(type): Invalid descriptor type '", type, "'."); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
}

// +=================================+   
// | Instance                        |
// +=================================+

// default constructor
Instance::Instance() {
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	// set defaults for application name, engine name and API version,
	// i.e. in case the user doesn't set them explicitly
	application_info.pApplicationName = "Vulkan Application";
	application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.pEngineName = "Generic";
	application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
}

// move constructor
Instance::Instance(Instance&& other) noexcept {
	this->instance = std::exchange(other.instance, nullptr);
	this->application_info = std::move(other.application_info);
	this->extensions = std::move(other.extensions);
	this->layers = std::move(other.layers);
	// Reset 'other' instance
	other.instance = nullptr;
}

// move assignment
Instance& Instance::operator=(Instance&& other) noexcept {
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

// destructor
Instance::~Instance() {
	if (instance != nullptr) {
		vkDestroyInstance(instance, nullptr);
		instance = nullptr;
		Log::info("[INSTANCE DESTROYED]");
	}
}

// set application name and version
void Instance::Instance::set_application(const char* name, uint32_t major_version, uint32_t minor_version, uint32_t patch_version) {
	application_info.pApplicationName = name;
	application_info.applicationVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
}

// initialize Vulkan engine and version
void Instance::set_engine(const char* name, uint32_t major_version, uint32_t minor_version, uint32_t patch_version) {
	application_info.pEngineName = name;
	application_info.engineVersion = VK_MAKE_VERSION(major_version, minor_version, patch_version);
}

// initialize Vulkan API version
void Instance::set_api_version(uint32_t major_version, uint32_t minor_version, uint32_t patch_version) {
	application_info.apiVersion = VK_MAKE_API_VERSION(0, major_version, minor_version, patch_version);
}

// log names of available instance layers
void Instance::log_available_layers() {
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

// add Vulkan layers for the Instance
void Instance::enable_layers(const std::vector<const char*>& layer_names) {
	for (const char* name : layer_names) {
		layers.push_back(name);
	}
}
void Instance::enable_layers(const char* layer_name) {
	layers.push_back(layer_name);
}

// log names of available Instance extensions
void Instance::log_available_extensions() const {
	// log available extensions
	if (Log::get_level() >= LogLevel::LEVEL_INFO) {
		uint32_t count;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> available_extensions(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, available_extensions.data());
		Log::info(count, " instance extensions available");
		for (uint32_t i = 0; i < count; i++) {
			Log::debug("(", i + 1, ") ", available_extensions[i].extensionName);
		}
	}
}

// add Vulkan extensions for the Instance
void Instance::enable_extensions(const std::vector<const char*>& extension_names) {
	for (const char* name : extension_names) {
		extensions.push_back(name);
	}
}
void Instance::enable_extensions(const char* extension_name) {
	extensions.push_back(extension_name);
}

// create Vulkan instance
void Instance::create(VkInstanceCreateFlags flags, const void* pNext) {
	// destroy any previous instance
	if (instance != nullptr) {
		vkDestroyInstance(instance, nullptr);
		instance = nullptr;
		Log::info("[OLD INSTANCE DESTROYED]");
	}

	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
	instance_create_info.ppEnabledLayerNames = layers.data();
	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instance_create_info.ppEnabledExtensionNames = extensions.data();
	instance_create_info.pNext = pNext;
	instance_create_info.flags = flags;
	instance_create_info.pApplicationInfo = &application_info;

	VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
	if (result == VK_SUCCESS) {
		Log::info("Vulkan instance successfully created.");
	}
	else {
		Log::error("Failed to create Vulkan Instance (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
}

// get handle to the Vulkan instance
VkInstance Instance::get() const {
	return instance;
}


// +=================================+   
// | Device                          |
// +=================================+

// parametric constructor
Device::Device(const Instance& instance, const VkPhysicalDeviceFeatures& enabled_features, const std::vector<const char*>& enabled_extension_names, uint32_t preferred_id) : id(preferred_id) {

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
	this->index = 0;
	physical = devices[this->index];

	Log::info("available physical devices with Vulkan support:");

	for (uint32_t i = 0; i < num_devices; i++) {
		vkGetPhysicalDeviceProperties(devices[i], &properties);
		if (i == this->index) {
			this->id = properties.deviceID;
		}
		Log::info("(", i, ") ", properties.deviceName, ", deviceID ", properties.deviceID, ", vendorID ", properties.vendorID,
			", type ", properties.deviceType, ", API version ", properties.apiVersion, ", driver version ", properties.driverVersion);
		// chose specific device (instead of default index 0) if passed id matches
		if (id == properties.deviceID) {
			physical = devices[i];
			this->id = properties.deviceID;
			this->index = i;
		}
	}
	Log::info("Selected physical device ", this->index, " with ID ", this->id);

	// store properties for selected device
	vkGetPhysicalDeviceProperties(physical, &properties);
	properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties2.pNext = nullptr;
	vkGetPhysicalDeviceProperties2(physical, &properties2);

	// get available extensions for the selected device
	uint32_t available_extension_count;
	vkEnumerateDeviceExtensionProperties(physical, nullptr, &available_extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions(available_extension_count);
	vkEnumerateDeviceExtensionProperties(physical, nullptr, &available_extension_count, available_extensions.data());

	// log available extensions
	if (Log::get_level() >= LogLevel::LEVEL_INFO) {
		Log::debug(available_extension_count, " device extensions available");
		for (uint32_t i = 0; i < available_extension_count; i++) {
			Log::debug("(", i + 1, ") ", available_extensions[i].extensionName);
		}
	}

	// Filter requested extensions against available extensions
	std::vector<const char*> supported_extensions;
	this->extensions = enabled_extension_names; // Store the original requested extensions
	bool use_synchronization2 = false;
	for (const auto& requested_extension : enabled_extension_names) {
		bool found = false;
		for (const auto& available_extension : available_extensions) {
			if (strcmp(requested_extension, available_extension.extensionName) == 0) {
				if (strcmp(requested_extension, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0) {
					use_synchronization2 = true; // Enable synchronization2 if requested
				}
				found = true;
				break;
			}
		}
		if (found) {
			supported_extensions.push_back(requested_extension);
		}
		else {
			Log::warning("Device extension '", requested_extension, "' is not supported on this device and will be removed from the request.");
		}
	}
	device_extension_names = supported_extensions; // Update the member with only supported extensions

	// prepare device features (member variable)
	enabled_features2 = {};
	enabled_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

	// Check if timeline semaphore extension is supported
	bool use_timeline_semaphore = false;
	for (const auto& extension : device_extension_names) {
		if (strcmp(extension, "VK_KHR_timeline_semaphore") == 0) {
			use_timeline_semaphore = true;
			break;
		}
	}

	// Set up pNext chain for device features
	void* pNext_ptr = nullptr;

	if (use_synchronization2) {
		synchronization2_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		synchronization2_features.pNext = nullptr; // Set pNext to null for now
		synchronization2_features.synchronization2 = VK_TRUE;
		pNext_ptr = &synchronization2_features;
	}

	if (use_timeline_semaphore) {
		timeline_semaphore_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
		timeline_semaphore_features.pNext = pNext_ptr; // Chain it to the previous struct
		timeline_semaphore_features.timelineSemaphore = VK_TRUE;
		pNext_ptr = &timeline_semaphore_features; // Update pNext_ptr to point to the new head of the chain
	}

	enabled_features2.pNext = pNext_ptr;
	enabled_features2.features = enabled_features;

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
	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext = &enabled_features2;
	device_create_info.flags = 0; // reserved for future use
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extension_names.size());
	device_create_info.ppEnabledExtensionNames = device_extension_names.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.pEnabledFeatures = NULL;
	result = vkCreateDevice(physical, &device_create_info, nullptr, &logical);
	if (result == VK_SUCCESS) {
		Log::info("successfully created logical device (handle: ", logical, ")");
	}
	else {
		Log::error("Failed to create Vulkan logical device (VkResult=", result, ", ", vkresult_to_string(result), ")");
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

// move constructor
Device::Device(Device&& other) noexcept {
	move_resources(other);
}

// move assignment
Device& Device::operator=(Device&& other) noexcept {
	if (this != &other) {
		destroy(); // release resource from 'this'
		move_resources(other);
		Log::info("Device resources from 'other' moved to 'this'");
	}
	return *this;
}
// destructor
Device::~Device() {
	destroy();
}

// getter functions
// (note: member variables can be returned by reference,
// because the instance is usually a high-level object,
// expected to stay alive as long as needed)
VkDevice Device::get_logical() const { return logical; }
VkPhysicalDevice Device::get_physical() const { return physical; }
VkQueue Device::get_graphics_queue() const { return graphics_queue; }
VkQueue Device::get_compute_queue() const { return compute_queue; }
VkQueue Device::get_transfer_queue() const { return transfer_queue; }
uint32_t Device::get_graphics_queue_family_index() const { return graphics_queue_family_index; }
uint32_t Device::get_compute_queue_family_index() const { return compute_queue_family_index; }
uint32_t Device::get_transfer_queue_family_index() const { return transfer_queue_family_index; }
const VkPhysicalDeviceProperties& Device::get_properties() const { return properties; }
const VkPhysicalDeviceProperties2& Device::get_properties2() const { return properties2; }
const std::vector<const char*>& Device::get_extensions() const { return extensions; }
const std::vector<const char*>& Device::get_device_extension_names() const { return device_extension_names; }
const uint32_t Device::get_index() const { return this->index; }
const uint32_t Device::get_id() const { return this->id; }
const VkPhysicalDeviceFeatures2& Device::get_features() const { return enabled_features2; }
const VkPhysicalDeviceSynchronization2Features& Device::get_synchronization_features() const { return synchronization2_features; }
const VkPhysicalDeviceTimelineSemaphoreFeatures& Device::get_timeline_sempahore_features() const { return timeline_semaphore_features; }

VkFormat Device::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			Log::debug("Found VkFormat supported by the physical device: ", format, ", with linear tiling.");
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			Log::debug("Found VkFormat supported by the physical device: ", format, ", with optimal tiling.");
			return format;
		}
		else {
			Log::error("In Device::find_supported_format(): failed to find suitable format based on the provided candidate list.");
		}
	}
}

// This method selects and returns a surface format,
// negotiating between the requirements of physical device, surface, and candidate preferences
VkSurfaceFormatKHR Device::select_surface_format(Surface& surface) {
	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface.get(), &format_count, nullptr);
	if (format_count == 0) {
		Log::error("Critical error in method Device::select_surface_format(): no formats available!");
	}

	std::vector<VkSurfaceFormatKHR> available_formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface.get(), &format_count, available_formats.data());

	// Prioritize a direct match with the preferred formats.
	// We also check for the preferred color space (SRGB).
	// Because the formats for color attachements and surface must be identical, COLOR_FORMAT_CANDIDATES is used here
	for (const auto& candidates : COLOR_FORMAT_CANDIDATES) {
		for (const auto& available_format : available_formats) {
			if (available_format.format == candidates && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return available_format;
			}
		}
	}
	Log::error("Critical error in method Device::select_surface_format(): no available format matches the requirements of surface, physical device and preferred candidates");
}

const VkPhysicalDeviceMemoryProperties& Device::get_memory_properties() {
	static bool properties_queried = false;
	if (!properties_queried) {
		vkGetPhysicalDeviceMemoryProperties(physical, &memory_properties);
		properties_queried = true;
	}
	return memory_properties;
}

// helper method to release resources
void Device::destroy() {
	// destroy logical device
	if (logical != nullptr) {
		vkDeviceWaitIdle(logical);
		vkDestroyDevice(logical, nullptr);
		logical = nullptr;
		Log::info("[LOGICAL DEVICE DESTROYED]");
	}
}

// helper method for move constructor and move assignment
void Device::move_resources(Device& other) {
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
	this->enabled_features2 = std::move(other.enabled_features2);
	this->synchronization2_features = std::move(other.synchronization2_features);
}

// +=================================+   
// | Image                           |
// +=================================+

Image::Image(
	Device& device,
	VkImageType type,
	VkFormat format,
	VkExtent3D extent,
	uint32_t mip_levels,
	uint32_t array_layers,
	VkSampleCountFlagBits samples,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags memory_properties,
	VkImageLayout initial_layout
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
		Log::error("in constructor Image::Image(...): failed to create image (VkResult=", result, ", ", vkresult_to_string(result), ")");
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
		Log::error("in constructor Image::Image(...): Failed to allocate image memory (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}

	result = vkBindImageMemory(logical, image, memory, 0);
	if (result != VK_SUCCESS) {
		vkFreeMemory(logical, memory, nullptr);
		vkDestroyImage(logical, image, nullptr);
		memory = VK_NULL_HANDLE;
		image = VK_NULL_HANDLE;
		Log::error("in constructor Image::Image(...): Failed to bind image memory (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	Log::debug("in constructor Image::Image(...): image created successfully (handle: ", image, ")");
}

// move constructor
Image::Image(Image&& other) noexcept :
	logical(std::exchange(other.logical, VK_NULL_HANDLE)),
	image(std::exchange(other.image, VK_NULL_HANDLE)),
	memory(std::exchange(other.memory, VK_NULL_HANDLE)),
	format(other.format),
	extent(other.extent),
	layout(other.layout) {
}

// move assignment
Image& Image::operator=(Image&& other) noexcept {
	if (this != &other) {
		destroy();
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		image = std::exchange(other.image, VK_NULL_HANDLE);
		memory = std::exchange(other.memory, VK_NULL_HANDLE);
		format = other.format;
		extent = other.extent;
		layout = other.layout;
	}
	return *this;
}

// destructor
Image::~Image() {
	destroy();
}

// getters
VkImage Image::get() const { return image; }
VkFormat Image::get_format() const { return format; }
VkExtent3D Image::get_extent() const { return extent; }
VkImageLayout Image::get_layout() const { return layout; }
VkDeviceMemory Image::get_memory() const { return memory; }

// setters
void Image::set_layout(VkImageLayout new_layout) { layout = new_layout; }

// helper method for releasing resources
void Image::destroy() {
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

// helper method to find a suitable memory type
uint32_t Image::find_memory_type(Device& device, VkMemoryPropertyFlags properties, uint32_t type_filter) {
	const auto& mem_properties = device.get_memory_properties();
	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return UINT32_MAX;
}


// +=================================+   
// | RenderPass                      |
// +=================================+

// a render pass defines the structure and dependencies of graphics rendering operations

// parametric constructor
RenderPass::RenderPass(Device& device, uint32_t multisample_count) : device(&device), logical(device.get_logical()), multisample_count(multisample_count) {}

// move constructor
RenderPass::RenderPass(RenderPass&& other) noexcept {
	move_resources(other);
}

// move assignment constructor
RenderPass& RenderPass::operator=(RenderPass&& other) noexcept {
	if (this != &other) {
		// Release resources held by the current object (if any)
		this->destroy();

		// Move the state from 'other' to 'this'
		move_resources(other);
	}
	return *this;
}

// destructor
RenderPass::~RenderPass() {
	destroy();
}

// adds an attachment description (=owned by this main RenderPass) and returns its index
uint32_t RenderPass::add_attachment(
	AttachmentType type,
	VkImageLayout initial_layout,
	VkImageLayout final_layout,
	VkAttachmentLoadOp load_op,
	VkAttachmentStoreOp store_op,
	VkAttachmentLoadOp stencil_load_op,
	VkAttachmentStoreOp stencil_store_op) {

	uint32_t id = static_cast<uint32_t>(attachment_description.size());
	attachment_type.push_back(type);
	attachment_description.resize(id + 1);
	attachment_description[id] = {};
	attachment_description[id].flags = 0; // or: VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT
	attachment_description[id].initialLayout = initial_layout;
	attachment_description[id].finalLayout = final_layout;
	attachment_description[id].loadOp = load_op;
	attachment_description[id].storeOp = store_op;
	attachment_description[id].stencilLoadOp = stencil_load_op;
	attachment_description[id].stencilStoreOp = stencil_store_op;

	switch (type) {
	case AttachmentType::COLOR_TYPE: {
		Log::error("invalid usage of method RenderPass::add_attachment with AttachmentType::COLOR_TYPE -> use method RenderPass::add_color_attachment() instead!");
		break;
	}
	case AttachmentType::DEPTH_TYPE: {
		attachment_description[id].samples = static_cast<VkSampleCountFlagBits>(multisample_count);
		attachment_description[id].format = device->find_supported_format(DEPTH_FORMAT_CANDIDATES, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		depth_stencil_flag = true;
		break;
	}
	case AttachmentType::INPUT_TYPE: {
		attachment_description[id].samples = VK_SAMPLE_COUNT_1_BIT; // Input attachments are usually single-sampled
		attachment_description[id].format = device->find_supported_format(INPUT_FORMAT_CANDIDATES, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		break;
	}
	case AttachmentType::PRESERVE_TYPE: {
		attachment_description[id].samples = VK_SAMPLE_COUNT_1_BIT; // preserve attachments are usually single-sampled
		attachment_description[id].format = device->find_supported_format(PRESERVE_FORMAT_CANDIDATES, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		break;
	}
	case AttachmentType::RESOLVE_TYPE: {
		attachment_description[id].samples = VK_SAMPLE_COUNT_1_BIT; // resolve attachments are usually single-sampled
		attachment_description[id].format = device->find_supported_format(RESOLVE_FORMAT_CANDIDATES, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		break;
	}
	default: {
		Log::error("method RenderPass::add_attachment() has been called with invalid RenderAttachment::Type type argument (", type, ")");
	}
	}

	return id;
}

// adds a color attachment description (=owned by this main RenderPass) and returns its index
uint32_t RenderPass::add_color_attachment(
	VkSurfaceFormatKHR surface_format,
	VkImageLayout initial_layout,
	VkImageLayout final_layout,
	VkAttachmentLoadOp load_op,
	VkAttachmentStoreOp store_op,
	VkAttachmentLoadOp stencil_load_op,
	VkAttachmentStoreOp stencil_store_op
) {
	uint32_t id = static_cast<uint32_t>(attachment_description.size());
	attachment_type.push_back(AttachmentType::COLOR_TYPE);
	attachment_description.resize(id + 1);
	attachment_description[id] = {};
	attachment_description[id].flags = 0; // or: VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT
	attachment_description[id].format = surface_format.format;
	attachment_description[id].samples = static_cast<VkSampleCountFlagBits>(multisample_count);
	attachment_description[id].initialLayout = initial_layout;
	attachment_description[id].finalLayout = final_layout;
	attachment_description[id].loadOp = load_op;
	attachment_description[id].storeOp = store_op;
	attachment_description[id].stencilLoadOp = stencil_load_op;
	attachment_description[id].stencilStoreOp = stencil_store_op;
	return id;
}

VkSubpassDependency RenderPass::add_subpass_dependency(
	uint32_t source,
	uint32_t destination,
	VkPipelineStageFlags src_stage_mask,
	VkPipelineStageFlags dst_stage_mask,
	VkAccessFlags src_access_mask,
	VkAccessFlags dst_access_mask
) {
	uint32_t id = static_cast<uint32_t>(subpass_dependency.size());
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

void RenderPass::finalize() {
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

// getter functions
VkRenderPass RenderPass::get() const { return renderpass; }
uint32_t RenderPass::get_multisample_count() const { return multisample_count; }
uint32_t RenderPass::get_attachment_count() const { return static_cast<uint32_t>(attachment_description.size()); }
uint32_t RenderPass::get_subpass_count() const { return static_cast<uint32_t>(subpass_description.size()); }
bool RenderPass::has_depth_stencil() const { return depth_stencil_flag; }
const std::vector<VkAttachmentDescription>& RenderPass::get_attachment_descriptions() { return attachment_description; }
const AttachmentType RenderPass::get_attachment_type(uint32_t index) const { return attachment_type[index]; }

// helper method for releasing resources
void RenderPass::destroy() {
	if (renderpass != nullptr) {
		vkDestroyRenderPass(logical, renderpass, nullptr);
		renderpass = nullptr;
	}
}

// helper method to move resources for the move constructor and move assignment
void RenderPass::move_resources(RenderPass& other) {
	logical = std::exchange(other.logical, VK_NULL_HANDLE);
	device = std::exchange(other.device, nullptr);
	renderpass = std::exchange(other.renderpass, VK_NULL_HANDLE);
	multisample_count = std::move(other.multisample_count);
	attachment_description = std::move(other.attachment_description);
	subpass_description = std::move(other.subpass_description);
}

// +=================================+   
// | SubPass                         |
// +=================================+

// parametric constructor
SubPass::SubPass(RenderPass& renderpass) : renderpass(&renderpass) {}

// add a reference to an attachment from the pool of attachment descriptions owned by the main RenderPass
void SubPass::add_attachment_reference(uint32_t attachment_index) {
	if (finalized) {
		Log::warning("in method SubPass::add_attachment(): this subpass has already been finalized, as it's already been added to a parent RenderPass; hence no more attachments can be added at this point");
		return;
	}
	else {
		switch (renderpass->get_attachment_type(attachment_index)) {
		case AttachmentType::COLOR_TYPE: {
			uint32_t id = static_cast<uint32_t>(color_attachment_reference.size());
			color_attachment_reference.resize(id + 1);
			color_attachment_reference[id].attachment = attachment_index;
			color_attachment_reference[id].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		}
		case AttachmentType::DEPTH_TYPE: {
			if (!depth_attachment_reference) {
				depth_attachment_reference = std::make_unique<VkAttachmentReference>();
			}
			depth_attachment_reference->attachment = attachment_index;
			depth_attachment_reference->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		}
		case AttachmentType::INPUT_TYPE: {
			if (!input_attachment_reference) {
				input_attachment_reference = std::make_unique<std::vector<VkAttachmentReference>>();
			}
			uint32_t id = static_cast<uint32_t>(input_attachment_reference->size());
			input_attachment_reference->resize(id + 1);
			(*input_attachment_reference)[id].attachment = attachment_index;
			(*input_attachment_reference)[id].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		}
		case AttachmentType::PRESERVE_TYPE: {
			if (!preserve_attachment_reference) {
				preserve_attachment_reference = std::make_unique<std::vector<uint32_t>>();
			}
			preserve_attachment_reference->push_back(attachment_index);
			break;
		}
		case AttachmentType::RESOLVE_TYPE: {
			if (!resolve_attachment_reference) {
				resolve_attachment_reference = std::make_unique<VkAttachmentReference>();
			}
			resolve_attachment_reference->attachment = attachment_index;
			resolve_attachment_reference->layout = VK_IMAGE_LAYOUT_GENERAL; // 'general' supports all types of device access, unless specified otherwise
			break;
		}
		default: {
			Log::warning("in method SubPass::add_attachment(): RenderAttachment has invalid type; failed to add the attachment to the subpass");
		}
		}
	}
}

// finalize the subpass and add it to the parent RenderPass;
// returns the index of the subpass within the parent RenderPass
uint32_t SubPass::finalize(VkSubpassDescriptionFlags flags, VkPipelineBindPoint bind_point) {
	if (finalized) {
		Log::warning("in method Subpass::finalize() this subpass has already been finalized and added to a parent RenderPass");
		return UINT32_MAX;
	}
	uint32_t id = static_cast<uint32_t>(renderpass->subpass_description.size());
	renderpass->subpass_description.resize(id + 1);
	renderpass->subpass_description[id].flags = flags;
	renderpass->subpass_description[id].pipelineBindPoint = bind_point;

	renderpass->subpass_description[id].inputAttachmentCount = input_attachment_reference ? static_cast<uint32_t>(input_attachment_reference->size()) : 0;
	renderpass->subpass_description[id].pInputAttachments = input_attachment_reference ? input_attachment_reference->data() : nullptr;

	renderpass->subpass_description[id].colorAttachmentCount = static_cast<uint32_t>(color_attachment_reference.size());
	renderpass->subpass_description[id].pColorAttachments = color_attachment_reference.empty() ? nullptr : color_attachment_reference.data();

	renderpass->subpass_description[id].pResolveAttachments = resolve_attachment_reference ? resolve_attachment_reference.get() : nullptr;

	renderpass->subpass_description[id].pDepthStencilAttachment = depth_attachment_reference ? depth_attachment_reference.get() : nullptr;
	renderpass->depth_stencil_flag = depth_attachment_reference->layout != VK_IMAGE_LAYOUT_UNDEFINED;

	renderpass->subpass_description[id].preserveAttachmentCount = preserve_attachment_reference ? static_cast<uint32_t>(preserve_attachment_reference->size()) : 0;
	renderpass->subpass_description[id].pPreserveAttachments = preserve_attachment_reference ? preserve_attachment_reference->data() : nullptr;

	finalized = true;
	this->index = id;
	return id;
}

uint32_t SubPass::get_index() const {
	return this->index;
}

// +=================================+   
// | Surface                         |
// +=================================+

// Platform Agnostic Surface Class


// Constructor for Windows
#ifdef VK_USE_PLATFORM_WIN32_KHR
Surface::Surface(const Instance& instance, HINSTANCE hinstance, HWND hwnd) : instance_handle(instance.get()) {
	if (!hinstance) {
		Log::error("Surface creation failed: Provided Win32 HINSTANCE is NULL.");
	}
	if (!hwnd) {
		Log::error("Surface creation failed: Provided Win32 HWND is NULL.");
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
		Log::error("Failed to create Win32 surface (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	else {
		Log::info("Win32 Vulkan surface created successfully (handle: ", surface, ")");
	}
}

// Constructor for Windows, using GLFW
Surface::Surface(const Instance& instance, GLFWwindow* window) : instance_handle(instance.get()) {
	VkResult result = glfwCreateWindowSurface(instance_handle, window, nullptr, &surface);
	if (result == VK_SUCCESS) {
		Log::info("GLFW surface created successfully (handle: ", surface, ")");
	}
	else {
		Log::error("Failed to create window surface with GLFW! Result = ", vkresult_to_string(result));
	}
}
#endif

// constructor for Android
#ifdef VK_USE_PLATFORM_ANDROID_KHR
Surface::Surface(Instance& instance, ANativeWindow* window) {
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
		Log::error("Failed to create Android surface (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	else {
		Log::info("Android Vulkan surface created successfully (handle: ", surface, ")");
	}
}
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
Surface::Surface(Instance& instance, xcb_connection_t* connection, xcb_window_t window)
	: instance_handle(instance.get()), surface(VK_NULL_HANDLE) {
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
		Log::error("Failed to create XCB surface (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	else {
		Log::info("XCB Vulkan surface created successfully (handle: ", surface, ")");
	}
}
#endif

// constructor for MacOS
#ifdef VK_USE_PLATFORM_METAL_EXT
	// Using void* to pass CAMetalLayer* to avoid Objective-C type in header
Surface::Surface(Instance& instance, void* caMetalLayer) {
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
		Log::error("Failed to create Metal surface (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	else {
		Log::info("Metal Vulkan surface created successfully (handle: ", surface, ")");
	}
}
#endif


// Move constructor
Surface::Surface(Surface&& other) noexcept {
	instance_handle = std::exchange(other.instance_handle, nullptr);
	surface = std::exchange(other.surface, VK_NULL_HANDLE);
}

// Move assignment
Surface& Surface::operator=(Surface&& other) noexcept {
	if (this != &other) {
		destroy(); // Clean up existing resource before move
		instance_handle = std::exchange(other.instance_handle, nullptr);
		surface = std::exchange(other.surface, VK_NULL_HANDLE);
	}
	return *this;
}


// Destructor
Surface::~Surface() {
	destroy();
}

// Get the underlying VkSurfaceKHR handle
VkSurfaceKHR Surface::get() const {
	return surface;
}

// Check if the surface handle is valid
const bool Surface::is_valid() const {
	return surface != VK_NULL_HANDLE;
}

// Check if a specific queue family on a physical device supports presentation to this surface
const bool Surface::get_physical_device_support(Device& device, QueueFamily queue_family) const {
	if (surface == VK_NULL_HANDLE || device.get_physical() == VK_NULL_HANDLE) {
		Log::warning("Attempted to check surface support with null surface or physical device.");
		return VK_FALSE;
	}
	VkBool32 present_support = VK_FALSE;
	VkResult result;
	switch (queue_family) {
	case QueueFamily::COMPUTE_QUEUE:
		result = vkGetPhysicalDeviceSurfaceSupportKHR(device.get_physical(), device.get_compute_queue_family_index(), surface, &present_support);
		break;
	case QueueFamily::GRAPHICS_QUEUE:
		result = vkGetPhysicalDeviceSurfaceSupportKHR(device.get_physical(), device.get_graphics_queue_family_index(), surface, &present_support);
		break;
	case QueueFamily::TRANSFER_QUEUE:
		result = vkGetPhysicalDeviceSurfaceSupportKHR(device.get_physical(), device.get_transfer_queue_family_index(), surface, &present_support);
		break;
	default:
		result = VK_ERROR_UNKNOWN;
	}
	if (result != VK_SUCCESS) {
		Log::warning("Failed to query physical device surface support, i.e. invalid device or unsupported surface extension (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	return bool(present_support);
}

// Get the capabilities of the surface for a specific physical device
VkSurfaceCapabilitiesKHR Surface::get_capabilities(Device& device) const {
	VkSurfaceCapabilitiesKHR capabilities = {};
	if (surface == VK_NULL_HANDLE || device.get_physical() == VK_NULL_HANDLE) {
		Log::warning("Attempted to get surface capabilities with null surface or physical device.");
		return capabilities; // Return empty struct
	}
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_physical(), surface, &capabilities);
	if (result != VK_SUCCESS) {
		Log::error("Failed to query physical device surface capabilities (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	return capabilities;
}

// Get the supported surface formats for a specific physical device
std::vector<VkSurfaceFormatKHR> Surface::get_formats(Device& device) const {
	std::vector<VkSurfaceFormatKHR> formats;
	if (surface == VK_NULL_HANDLE || device.get_physical() == VK_NULL_HANDLE) {
		Log::warning("Attempted to get surface formats with null surface or physical device.");
		return formats; // Return empty vector
	}
	uint32_t formatCount = 0;
	// Query count first
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_physical(), surface, &formatCount, nullptr);
	if (result != VK_SUCCESS || formatCount == 0) {
		Log::warning("Failed to query physical device surface format count or none available (VkResult=", result, ", ", vkresult_to_string(result), ")");
		return formats; // Return empty vector
	}

	formats.resize(formatCount);
	// Query actual formats
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_physical(), surface, &formatCount, formats.data());
	if (result != VK_SUCCESS) {
		Log::error("Failed to query physical device surface formats (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	return formats;
}

// Get the supported presentation modes for a specific physical device
std::vector<VkPresentModeKHR> Surface::get_present_modes(Device& device) const {
	std::vector<VkPresentModeKHR> presentModes;
	if (surface == VK_NULL_HANDLE || device.get_physical() == VK_NULL_HANDLE) {
		Log::warning("Attempted to get surface present modes with null surface or physical device.");
		return presentModes; // Return empty vector
	}
	uint32_t presentModeCount = 0;
	// Query count first
	VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_physical(), surface, &presentModeCount, nullptr);
	if (result != VK_SUCCESS || presentModeCount == 0) {
		Log::warning("Failed to query physical device surface present mode count or none available (VkResult=", result, ", ", vkresult_to_string(result), ")");
		return presentModes; // Return empty vector
	}

	presentModes.resize(presentModeCount);
	// Query actual modes
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_physical(), surface, &presentModeCount, presentModes.data());
	if (result != VK_SUCCESS) {
		Log::error("Failed to query physical device surface present modes (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	return presentModes;
}

// Helper to destroy the surface (called by destructor and move assignment)
void Surface::destroy() {
	if (surface != VK_NULL_HANDLE && instance_handle != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(instance_handle, surface, nullptr);
		Log::info("[SURFACE DESTROYED] (handle: ", surface, ")");
	}
	surface = VK_NULL_HANDLE;
}

// +=================================+   
// | Fence                           |
// +=================================+

// for synchronization between GPU and CPU

// parametric constructor
Fence::Fence(Device& device, bool signaled) : logical(device.get_logical()) {
	VkFenceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
	vkCreateFence(device.get_logical(), &create_info, nullptr, &fence);
}

// move constructor
Fence::Fence(Fence&& other) noexcept :
	fence(std::exchange(other.fence, VK_NULL_HANDLE)),
	logical(std::exchange(other.logical, VK_NULL_HANDLE)) {
}

// move assignment
Fence& Fence::operator=(Fence&& other) noexcept {
	if (this != &other) {
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		fence = std::exchange(other.fence, VK_NULL_HANDLE);
	}
	return *this;
}

// destructor
Fence::~Fence() {
	vkDestroyFence(logical, fence, nullptr);
}

// query fence status
bool Fence::signaled(std::string caller_function) const {
	VkResult result = vkGetFenceStatus(logical, fence);
	if (result == VK_SUCCESS) {
		Log::debug("The fence with Vulkan handle ", fence, " has reached the signaled state.");
		return true; // Fence is signaled
	}
	else if (result == VK_NOT_READY) {
		Log::debug("in method Fence::signaled() for calling function ", caller_function, ": fence (handle: ", fence, " is unsignaled or not ready");
		return false; // Fence is not signaled
	}
	else if (result == VK_ERROR_DEVICE_LOST) {
		Log::error("in method Fence::signaled() for calling function ", caller_function, ": device lost! Fence handle: ", fence);
	}
	else {
		Log::warning("in method Fence::signaled() for calling function ", caller_function, ": unknown error(VkResult = ", result, ", ", vkresult_to_string(result), "); Fence handle: ", fence);
	}
	return false; // Fence is not signaled or an error occurred
}

// reset the fence to unsignaled state
VkResult Fence::reset(std::string caller_function) const {
	Log::debug("Fence::reset(): resetting fence (handle: ", fence, ") for calling function ", caller_function);
	return vkResetFences(logical, 1, &fence);
}

// wait for the fence to be signaled
VkResult Fence::wait(std::string caller_function, uint64_t timeout_nanosec) const {
	VkResult result = vkWaitForFences(logical, 1, &fence, VK_TRUE, timeout_nanosec);
	if (result == VK_SUCCESS) {
		return result; // Fence is signaled
	}
	else if (result == VK_TIMEOUT) {
		Log::warning("in method Fence::wait(): wait for fence (fence handle: ", fence, ", caller function : ", caller_function, ") has timed out!");
	}
	else if (result == VK_ERROR_DEVICE_LOST) {
		Log::error("in method Fence::wait(): device lost! (fence handle: ", fence, ", caller function : ", caller_function, ")");
	}
	else {
		Log::warning("in method Fence::wait(): unknown error (fence handle: ", fence, ", caller function : ", caller_function, ", VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
	return result;
}

VkFence Fence::get() const { return fence; }


// +=================================+   
// | Semaphore                       |
// +=================================+

// for synchronization on the GPU (between command buffers)

// constructor for a binary semaphore
Semaphore::Semaphore(const Device& device, VkPipelineStageFlagBits2 wait_dst_stage_mask, VkPipelineStageFlagBits2 signal_dst_stage_mask) :
	logical(device.get_logical()),
	wait_dst_stage_mask(wait_dst_stage_mask),
	signal_dst_stage_mask(signal_dst_stage_mask) {

	this->type = VK_SEMAPHORE_TYPE_BINARY;

	VkSemaphoreCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	create_info.flags = 0;

	VkResult result = vkCreateSemaphore(device.get_logical(), &create_info, nullptr, &semaphore);
	if (result != VK_SUCCESS) {
		Log::warning("Semaphore constructor (for binary semaphore) has failed with VkResult = ", result, ", ", vkresult_to_string(result));
	}

}

// Constructor for a timeline semaphore
Semaphore::Semaphore(const Device& device, uint64_t initial_value, VkPipelineStageFlagBits2 wait_dst_stage_mask, VkPipelineStageFlagBits2 signal_dst_stage_mask) :
	logical(device.get_logical()),
	wait_dst_stage_mask(wait_dst_stage_mask),
	signal_dst_stage_mask(signal_dst_stage_mask) {

	this->type = VK_SEMAPHORE_TYPE_TIMELINE;

	VkSemaphoreTypeCreateInfo type_create_info = {};
	type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	type_create_info.semaphoreType = type;
	type_create_info.initialValue = initial_value;

	VkSemaphoreCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	create_info.pNext = &type_create_info;
	create_info.flags = 0;

	VkResult result = vkCreateSemaphore(device.get_logical(), &create_info, nullptr, &semaphore);
	if (result != VK_SUCCESS) {
		Log::warning("Semaphore constructor (for timeline semaphore) has failed with VkResult = ", result, ", ", vkresult_to_string(result));
	}
}

// destructor
Semaphore::~Semaphore() {
	vkDestroySemaphore(logical, semaphore, nullptr);
	semaphore = VK_NULL_HANDLE;
}

// move constructor
Semaphore::Semaphore(Semaphore&& other) noexcept :
	semaphore(std::exchange(other.semaphore, VK_NULL_HANDLE)),
	logical(std::exchange(other.logical, VK_NULL_HANDLE)),
	type(other.get_type()) {
}

// move assignment
Semaphore& Semaphore::operator=(Semaphore&& other) noexcept {
	if (this != &other) {
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		semaphore = std::exchange(other.semaphore, VK_NULL_HANDLE);
		type = other.get_type();
	}
	return *this;
}

// wait for a timeline semaphore to reach the counter value as currently stored in the counter member variable (CPU call)
// note: use with caution! this is a blocking CPU call and defeats the purpose of semaphores (=GPU/GPU synchronization) in most scenarios!
// better approach: use wait semaphore on queue submit;
// this method may still be useful when e.g. the semaphore is expected to already have signaled and to confirm before releasing resources
VkResult Semaphore::wait(uint64_t timeout_nanosec) {
	if (this->type != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("Semaphore::wait() should only be used with timeline semaphores");
		return VK_ERROR_UNKNOWN;
	}
	VkSemaphoreWaitInfo wait_info = {};
	wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	wait_info.pNext = NULL;
	wait_info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
	wait_info.semaphoreCount = 1;
	wait_info.pSemaphores = &semaphore;
	uint64_t wait_value = this->get_counter_var();
	wait_info.pValues = &wait_value;
	VkResult result = vkWaitSemaphores(logical, &wait_info, timeout_nanosec);
	if (result != VK_SUCCESS) {
		Log::warning("in method Semaphore::wait(): the semaphore (", semaphore, ") has not signaled within the requested max waiting period of ",
			timeout_nanosec, " nano-seconds; VkResult = ", result, "(", vkresult_to_string(result), ")");
	}
	return result;
}

// wait for a timeline semaphore to reach a specific counter value (CPU call)
// note: use with caution! this is a blocking CPU call and defeats the purpose of semaphores (=GPU/GPU synchronization) in most scenarios!
// better approach: use wait semaphore on queue submit;
// this method may still be useful when e.g. the semaphore is expected to already have signaled and to confirm before releasing resources
VkResult Semaphore::wait(uint64_t wait_value, uint64_t timeout_nanosec) {
	if (this->type != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("Semaphore::wait() should only be used with timeline semaphores");
		return VK_ERROR_UNKNOWN;
	}
	VkSemaphoreWaitInfo wait_info = {};
	wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	wait_info.pNext = NULL;
	wait_info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
	wait_info.semaphoreCount = 1;
	wait_info.pSemaphores = &semaphore;
	wait_info.pValues = &wait_value;
	VkResult result = vkWaitSemaphores(logical, &wait_info, timeout_nanosec);
	if (result != VK_SUCCESS) {
		Log::warning("in method Semaphore::wait(): the semaphore (", semaphore, ") has not signaled within the requested max waiting period of ",
			timeout_nanosec, " nano-seconds; VkResult = ", result, "(", vkresult_to_string(result), ")");
	}
	return result;
}

// query a timeline semaphore counter value (CPU call)
uint64_t Semaphore::get_counter() const {
	if (this->type != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Semaphore::get_counter() should only be used with timeline semaphores!");
		return UINT64_MAX;
	}
	uint64_t value;
	vkGetSemaphoreCounterValue(logical, semaphore, &value);
	return value;
}

// query the current value of the counter (member variable)
uint64_t Semaphore::get_counter_var() const {
	return counter_value.load(std::memory_order_relaxed);
}

// signal a timeline semaphore with a specified value (CPU call)
void Semaphore::signal(uint64_t value) const {
	if (this->type != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Semaphore::signal() should only be used with timeline semaphores!");
		return;
	}
	VkSemaphoreSignalInfo signal_info = {};
	signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	signal_info.pNext = NULL;
	signal_info.semaphore = semaphore;
	signal_info.value = value;
	vkSignalSemaphore(logical, &signal_info);
}

// flush current counter value (member variable) to semaphore signal (CPU call)
void Semaphore::signal() const {
	if (this->type != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Semaphore::signal() should only be used with timeline semaphores!");
		return;
	}
	VkSemaphoreSignalInfo signal_info = {};
	signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	signal_info.pNext = NULL;
	signal_info.semaphore = semaphore;
	signal_info.value = counter_value.load();
	vkSignalSemaphore(logical, &signal_info);
}

// setters

// increment counter (member variable (atomic))
uint64_t Semaphore::increment_counter() {
	if (this->type != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Semaphore::increment_counter() should only be used with timeline semaphores!");
		return UINT64_MAX;
	}
	// atomic increment by 1; return new value after increment
	return counter_value.fetch_add(1, std::memory_order_acq_rel) + 1;
}

// set counter member variable atomically to a specific value
void Semaphore::set_counter_var(uint64_t value) {
	if (this->type != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Semaphore::set_counter() should only be used with timeline semaphores!");
		return;
	}
	counter_value.store(value, std::memory_order_relaxed);
}

// getters
VkSemaphore Semaphore::get() const { return semaphore; }
VkSemaphoreType Semaphore::get_type() const { return type; }
const VkSemaphore* Semaphore::get_ptr() const { return &semaphore; }
VkPipelineStageFlags2 Semaphore::get_wait_dst_stage_mask() const { return static_cast<VkPipelineStageFlags2>(wait_dst_stage_mask); }
VkPipelineStageFlags2 Semaphore::get_signal_dst_stage_mask() const { return static_cast<VkPipelineStageFlags2>(signal_dst_stage_mask); }


// +=================================+   
// | Event                           |
// +=================================+

Event::Event(Device& device) : logical(device.get_logical()) {
	// If the VK_KHR_portability_subset extension is enabled, and VkPhysicalDevicePortabilitySubsetFeaturesKHR::events is VK_FALSE,
	// then the implementation does not support events, and vkCreateEvent must not be used !!!
	VkEventCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT;
	vkCreateEvent(logical, &create_info, nullptr, &event);
}

// destructor
Event::~Event() {
	vkDestroyEvent(logical, event, nullptr);
}

// move constructor
Event::Event(Event&& other) noexcept :
	event(std::exchange(other.event, VK_NULL_HANDLE)),
	logical(std::exchange(other.logical, VK_NULL_HANDLE)) {
}

// move assignment
Event& Event::operator=(Event&& other) noexcept {
	if (this != &other) {
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		event = std::exchange(other.event, VK_NULL_HANDLE);
	}
	return *this;
}

void Event::reset() {
	vkResetEvent(logical, event);
}

// set the event to signaled state
void Event::set() const {
	vkSetEvent(logical, event);
}

// query event status
bool Event::signaled() const {
	return vkGetEventStatus(logical, event) == VK_EVENT_SET;
}

// getters
const VkEvent& Event::get() const { return event; }
const VkDependencyInfo& Event::get_dependency_info() const { return dependency_info; }
VkDependencyInfo* Event::get_dependency_info_ptr() { return &dependency_info; }

// +=================================+   
// | CommandPool                     |
// +=================================+

// parametric constructor
CommandPool::CommandPool(Device& device, QueueFamily usage) : logical(device.get_logical()), usage(usage) {

	// setup command pool
	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (usage == QueueFamily::GRAPHICS_QUEUE) {
		create_info.queueFamilyIndex = device.get_graphics_queue_family_index();
	}
	else if (usage == QueueFamily::COMPUTE_QUEUE) {
		create_info.queueFamilyIndex = device.get_compute_queue_family_index();
	}
	else if (usage == QueueFamily::TRANSFER_QUEUE) {
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
		Log::error("failed to create command pool (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
}

// destructor
CommandPool::~CommandPool() {
	if (pool != nullptr) {
		// destroy command pool
		Log::info("CommandPool destructor: destroying command pool with handle ", pool);
		vkDestroyCommandPool(logical, pool, nullptr);
		pool = nullptr;
	}
}

// trim the command pool (release unused memory back to the system)
void CommandPool::trim() const {
	vkTrimCommandPool(logical, pool, NULL);
}

// reset the command pool (release all resources allocated from the pool back to the pool)
VkResult CommandPool::reset(VkCommandPoolResetFlags flags) const {
	return vkResetCommandPool(logical, pool, flags);
}

// getters
VkCommandPool CommandPool::get() const { return pool; }
QueueFamily CommandPool::get_usage() const { return usage; }

// +=================================+   
// | VertexDescriptions              |
// +=================================+

// constructor
VertexDescriptions::VertexDescriptions() {}

// destructor
VertexDescriptions::~VertexDescriptions() {}

void VertexDescriptions::add_attribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset) {
	uint32_t id = static_cast<uint32_t>(attribute_descriptions.size());
	attribute_descriptions.resize(id + 1);
	attribute_descriptions[id].binding = binding;
	attribute_descriptions[id].location = location;
	attribute_descriptions[id].format = format;
	attribute_descriptions[id].offset = offset;
}

// adds a binding to the vertex descriptions and returns the index of the new binding
uint32_t VertexDescriptions::add_binding(uint32_t stride, VkVertexInputRate input_rate) {
	uint32_t id = static_cast<uint32_t>(binding_descriptions.size());
	binding_descriptions.resize(id + 1);
	binding_descriptions[id].binding = id;
	binding_descriptions[id].inputRate = input_rate;
	binding_descriptions[id].stride = stride;
	return id;
}

// getters
const std::vector<VkVertexInputAttributeDescription>& VertexDescriptions::get_attribute_descriptions() const { return attribute_descriptions; }
const std::vector<VkVertexInputBindingDescription>& VertexDescriptions::get_input_bindings() const { return binding_descriptions; }

// +=================================+   
// | ShaderModule                    |
// +=================================+

// constructor with binary data
ShaderModule::ShaderModule(Device& device, const unsigned char* binary, size_t size_bytes) :
	logical(device.get_logical()) {
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
		Log::error("failed to create shader module (VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
}

// Constructor with folder & filename as C-style string literals
ShaderModule::ShaderModule(Device& device, const char* foldername, const char* filename) {
	ShaderModule(device, std::string(foldername), std::string(filename));
}

// constructor with folder & filename as std::string
ShaderModule::ShaderModule(Device& device, const std::string& foldername, const std::string& filename) : logical(device.get_logical()) {

	std::string file_path;
	if (foldername.back() != '/') {
		file_path = foldername + '/' + filename;
	}
	else {
		file_path = foldername + filename;
	}
	long file_size = 0;
	FILE* file = nullptr;
	errno_t err = fopen_s(&file, file_path.c_str(), "rb");
	if (err != 0) {
		// Handle the error (e.g., log it or throw an exception)
		Log::error("Failed to open shader file: ", file_path);
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
			Log::error("failed to create shader module (VkResult = ", result, ", ", vkresult_to_string(result), ")");
		}
		delete[] buffer;
		fclose(file);
	}
}

// move constructor
ShaderModule::ShaderModule(ShaderModule&& other) noexcept {
	this->logical = std::exchange(other.logical, VK_NULL_HANDLE);
	this->module = std::exchange(other.module, VK_NULL_HANDLE);
	if (module != VK_NULL_HANDLE) {
		Log::info("shader module moved (handle: ", module, ")");
	}
}

// move assignment
ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept {
	if (this != &other) {
		if (module != nullptr) {
			Log::info("move assignment operation: destroying previous shader module (handle: ", module, ")");
			vkDestroyShaderModule(logical, module, nullptr);
			module = nullptr;
		}
		module = std::exchange(other.module, VK_NULL_HANDLE);
		this->logical = std::exchange(other.logical, VK_NULL_HANDLE);
		if (module != nullptr) {
			Log::info("shader module moved to 'this' (handle: ", module, ")");
		}
	}
	return *this;
}

// destructor
ShaderModule::~ShaderModule() {
	if (module != nullptr) {
		vkDestroyShaderModule(logical, module, nullptr);
	}
}

// getters
const VkShaderModule& ShaderModule::get() const { return module; }



// +=================================+   
// | PushConstants                   |
// +=================================+

// default constructor
PushConstants::PushConstants() {
	this->size = 0;
	data = new uint32_t[min_capacity / 4];
}

// constructor with values as std::vector<T>
template<typename T>
PushConstants::PushConstants(const std::vector<T>& values) {
	this->size = 0;
	data = new uint32_t[min_capacity / 4];
	add_values(values);
}

// constructor with values of mixed types as variadic template arguments
template<typename... Args>
PushConstants::PushConstants(Args... args) {
	this->size = 0;
	data = new uint32_t[min_capacity / 4];
	(add_values(args), ...); // fold expression to call the add_values method for each argument
}

// destructor
PushConstants::~PushConstants() {
	if (data != nullptr) {
		delete[] data;
		data = nullptr;
	}
}

// add a new value to the end of the push constants range;
// returns the memory location offset of the new value within the push constants range
template<typename T>
size_t PushConstants::add_values(T value) {

	// --- std430 ALIGNMENT LOGIC ---

	// First, we calculate the required alignment for the new data type T based on std430 rules.
	// For types like glm::vec3, this will be 16 bytes.
	size_t alignment = get_std430_alignment<T>();

	// Then, we calculate the padding needed to ensure the current offset is a multiple of the required alignment.
	size_t padding = (alignment - (this->size % alignment)) % alignment;
	size_t padded_offset = this->size + padding;

	// The new total size will be the padded offset plus the size of the new data.
	size_t new_size = padded_offset + sizeof(T);
	size_t old_size = this->size;

	// Reallocate memory if existing capacity is insufficient.
	if (this->capacity < new_size) {
		this->capacity = size_t(std::max(float_t(min_capacity), float_t(4 * ceil(0.25f * new_size * (1.0f + reserve)))));
		uint32_t* new_allocation = new uint32_t[this->capacity / 4];
		if (data != nullptr) {
			memcpy(new_allocation, data, this->size);
			delete[] data;
		}
		data = new_allocation;
	}

	// copy value to the end of the data array at the new padded offset
	memcpy(data + padded_offset / 4, &value, sizeof(T));
	this->size = new_size;
	return padded_offset;
}

// add a new value at the specified offset position of the push constants range;
// overwrites the existing value at that position;
template<typename T>
void PushConstants::add_values(T value, size_t offset) {
	// validate offset
	if (offset > this->size) {
		Log::error("in method PushConstants::add_values(T value, size_t offset): offset exceeds current push constant range size");
		return;
	}
	// Check if the provided offset is correctly aligned for the given type.
	size_t alignment = get_std430_alignment<T>();
	if (offset % alignment) {
		Log::warning("in method PushConstants::add_values(T value, size_t offset): offset is not correctly aligned for this data type. Data will likely be corrupted!");
	}

	// copy value at the specified offset position
	memcpy(data + offset / 4, &value, sizeof(T));
}

// add multiple new values to the push constants range as a std::initializer_list;
// returns the byte offset of the last written element within the push constants range
template<typename T>
size_t PushConstants::add_values(std::initializer_list<T> values) {
	size_t current_offset = this->size;
	for (const T& i : values) {
		current_offset = this->add_values(i);
	}
	return current_offset;
}

// add multiple new values to the push constants range as a std::vector<T>;
// returns the memory location byte offset of the new values within the push constants range
template<typename T>
size_t PushConstants::add_values(const std::vector<T>& new_data) {
	size_t current_offset = this->size;
	for (const T& i : new_data) {
		current_offset = this->add_values(i);
	}
	return current_offset;
}

// replace all previous constants starting from offset 0;
// return the memory location byte offset of the end of the newly written range
template<typename... Args>
size_t PushConstants::replace_values(size_t begin_offset, Args... args) {
	size_t current_offset = begin_offset;
	// Use a fold expression to apply the add_values method for each argument,
	// ensuring the proper offset is passed.
	auto add_and_track_offset = [&](auto&& arg) {
		this->add_values(arg, current_offset);
		current_offset += get_std430_alignment<decltype(arg)>() * (sizeof(arg) / get_std430_alignment<decltype(arg)>() + (sizeof(arg) % get_std430_alignment<decltype(arg)>() != 0));
		};
	(add_and_track_offset(args), ...);
	return current_offset;
}

// free range by setting its size to zero (this doesn't affect the capacity!)
void PushConstants::free() { this->size = 0; }

// getters
const uint32_t* PushConstants::get_data() const { return data; }
size_t PushConstants::get_size() const { return this->size; }
size_t PushConstants::get_total_capacity() const { return this->capacity; }
size_t PushConstants::get_free_capacity() const { return this->capacity - this->size; }

// Helper function to get the std430 alignment for a given type.
// This is the core of the alignment logic.
template<typename T>
constexpr size_t PushConstants::get_std430_alignment() const {
	if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int> || std::is_same_v<T, uint32_t>) {
		return 4;
	}
	else if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, double_t> || std::is_same_v<T, uint64_t>) {
		return 8;
	}
	else if constexpr (std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4>) {
		return 16;
	}
	else if constexpr (std::is_same_v<T, glm::mat4>) {
		return 16;
	}
	else {
		// Default to a 4-byte alignment for unknown types.
		// This is a warning as it may not be correct for complex types.
		Log::warning("In method PushConstants::get_std430_alignment(): unknown type. Using 4-byte alignment.");
		return 4;
	}
}

// Helper function to calculate the padded offset for a new value.
template<typename T>
size_t PushConstants::get_padded_offset() {
	size_t alignment = get_std430_alignment<T>();
	// Calculate the padding needed to align the current size
	size_t padding = (alignment - (this->size % alignment)) % alignment;
	return this->size + padding;
}

// +=================================+   
// | Buffer<T>                       |
// +=================================+

// buffer class for vertex data, index data, storage data, uniform data, etc.

// parametric constructor
template<typename T>
Buffer<T>::Buffer(Device& device, BufferType type, uint32_t elements, VkMemoryPropertyFlags memory_property_flags) :
	logical(device.get_logical()),
	memory_property_flags(memory_property_flags),
	elements(elements),
	size_bytes(elements * sizeof(T)) {

	if (sizeof(T) < 4) {
		Log::warning("in Buffer::Buffer(): data type 'T' has less than 4 bytes, which is not recommended for alignment reasons.");
	}

	is_host_visible = memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	// translate BufferType enum argument
	// note: all usage types can participate in transfer operations by default; this feature may not always be needed (e.g. with uniform buffers), but more likely than not, no relevant performance hit is to be expected anyways
	VkBufferUsageFlags vk_buffer_usage;
	switch (type) {
	case BufferType::VERTEX_BUFFER:   vk_buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
	case BufferType::INDEX_BUFFER:    vk_buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
	case BufferType::STORAGE_BUFFER:  vk_buffer_usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
	case BufferType::UNIFORM_BUFFER:  vk_buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
	case BufferType::TRANSFER_BUFFER: vk_buffer_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
	default: Log::error("in method Buffer::Buffer(): invalid BufferType argument: ", type);
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
		Log::error("failed to create data buffer, VkResult=", result, ", ", vkresult_to_string(result));
	}

	// get buffer memory requirements
	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(logical, buffer, &memory_requirements);

	// find suitable memory type index
	uint32_t type_index = UINT32_MAX;
	Log::info("in Buffer::Buffer() constructor: searching for buffer memory types (requested: ", memory_property_flags, ")");
	const auto& mem_properties = device.get_memory_properties();
	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
		Log::debug("memory type ", i, ": ", mem_properties.memoryTypes[i].propertyFlags);
		if ((memory_requirements.memoryTypeBits & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & memory_property_flags) == memory_property_flags) {
			type_index = i;
			Log::info("[SUCCESS]");
		}
	}
	if (type_index == UINT32_MAX) {
		Log::error("in Buffer::Buffer() constructor:: no suitable memory type found. Aborting.");
	}

	// allocate memory
	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = memory_requirements.size;
	allocate_info.memoryTypeIndex = type_index;
	result = vkAllocateMemory(logical, &allocate_info, nullptr, &memory);
	if (result != VK_SUCCESS) {
		Log::error("in Buffer::Buffer() constructor: failed to allocate buffer memory, VkResult=", result, ", ", vkresult_to_string(result));
	}

	// bind memory to buffer
	result = vkBindBufferMemory(logical, buffer, memory, 0);
	if (result != VK_SUCCESS) {
		Log::error("in Buffer::Buffer() constructor: failed to bind buffer memory, VkResult=", result, ", ", vkresult_to_string(result));
	}
}

// Move constructor
template<typename T>
Buffer<T>::Buffer(Buffer<T>&& other) noexcept
	: buffer(std::exchange(other.buffer, VK_NULL_HANDLE)),
	memory(std::exchange(other.memory, VK_NULL_HANDLE)),
	elements(other.elements),
	logical(std::exchange(other.logical, VK_NULL_HANDLE)),
	size_bytes(other.size_bytes),
	is_host_visible(other.is_host_visible),
	memory_property_flags(other.memory_property_flags) {

	if (buffer != VK_NULL_HANDLE) {
		Log::info("Buffer moved (new owner), handle: ", buffer);
	}
	else {
		Log::warning("Buffer move constructor called, but the 'other' buffer handle is VK_NULL_HANDLE");
	}
}

// Move assignment
template<typename T>
Buffer<T>& Buffer<T>::operator=(Buffer<T>&& other) noexcept {
	if (this != &other) {
		// reference members (device) cannot be reassigned!
		// We must assume that 'this' already has a valid 'device' reference.

		// Release existing resources owned by 'this' object
		if (memory != VK_NULL_HANDLE) {
			vkFreeMemory(logical, memory, nullptr);
			memory = VK_NULL_HANDLE;
		}
		if (buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(logical, buffer, nullptr);
			buffer = VK_NULL_HANDLE;
		}

		// Transfer ownership of resources from 'other' to 'this'
		this->buffer = other.buffer;
		this->memory = other.memory;
		this->elements = other.elements;
		this->size_bytes = other.size_bytes;
		this->is_host_visible = other.is_host_visible;
		this->memory_property_flags = other.memory_property_flags;

		// Invalidate the other object
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

// destructor
template<typename T>
Buffer<T>::~Buffer() {
	if (memory != VK_NULL_HANDLE && memory != VkDeviceMemory(0xdddddddddddddddd)) {
		Log::debug("in Buffer<T> destructor: freeing buffer memory (memory handle: ", memory, ")");
		vkFreeMemory(logical, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
	if (buffer != VK_NULL_HANDLE && buffer != VkBuffer(0xdddddddddddddddd)) {
		Log::debug("in Buffer<T> destructor: destroying buffer (buffer handle: ", buffer, ")");
		vkDestroyBuffer(logical, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}
}

// copy data elements from a std::vector to a host visible buffer
// (set copied elements to 0 to copy all)
template<typename T>
void Buffer<T>::write(const std::vector<T>& source_vector, uint32_t copied_elements, uint32_t source_offset_elements, uint32_t target_offset_elements) {
	if (!is_host_visible) {
		Log::error("Buffer::write() called on non-host-visible buffer");
	}
	uint32_t vector_elements = source_vector.size();
	uint64_t vector_size_bytes;
	if (copied_elements == 0) {
		vector_size_bytes = (vector_elements - source_offset_elements) * sizeof(T);
	}
	else {
		vector_size_bytes = copied_elements * sizeof(T);
	}
	VkDeviceSize target_offset_bytes = target_offset_elements * sizeof(T);
	if (target_offset_bytes + vector_size_bytes > this->size_bytes) {
		Log::warning("Buffer::write attempting to write past buffer bounds. Clipping copy region size to fit.");
		vector_size_bytes = (this->size_bytes > target_offset_bytes) ? this->size_bytes - target_offset_bytes : 0;
	}
	if (vector_size_bytes == 0) {
		Log::debug("in Buffer<T>::write(): requested copy region has size ", vector_size_bytes, " bytes, i.e.nothing to copy");
		return;
	}
	Log::debug("writing from a vector<T> to buffer ", buffer, " (memory: ", memory, "), ", vector_elements, " vector elements (", vector_size_bytes, " bytes, with ", source_offset_elements, " elements offset in source and ", target_offset_elements, " elements offset in target");
	void* data;
	vkMapMemory(logical, memory, target_offset_bytes, vector_size_bytes, VkMemoryMapFlags(0), &data);
	memcpy(data, source_vector.data() + source_offset_elements, vector_size_bytes);
	vkUnmapMemory(logical, memory);
}

// copy data elements from a std::array to a host visible buffer
template<typename T>
void Buffer<T>::write(const T* source_array, uint32_t copied_elements, uint32_t source_offset_elements, uint32_t target_offset_elements) {
	if (!is_host_visible) {
		Log::error("Buffer::write() called on non-host-visible buffer");
	}
	size_t array_size_bytes = copied_elements * sizeof(T);
	VkDeviceSize target_offset_bytes = target_offset_elements * sizeof(T);
	if (target_offset_bytes + array_size_bytes > this->size_bytes) {
		Log::warning("Buffer::write attempting to write past buffer bounds. Clipping copy region size to fit.");
		array_size_bytes = (this->size_bytes > target_offset_bytes) ? this->size_bytes - target_offset_bytes : 0;
	}
	if (array_size_bytes <= 0) {
		Log::debug("in Buffer<T>::write(): requested copy region has size ", array_size_bytes, " bytes, i.e.nothing to copy");
		return;
	}
	void* data;
	vkMapMemory(logical, memory, target_offset_bytes, array_size_bytes, VkMemoryMapFlags(0), &data);
	memcpy(data, source_array + source_offset_elements, array_size_bytes);
	vkUnmapMemory(logical, memory);
}

// copy data elements from a std::initializer_list to a host visible buffer
template<typename T>
void Buffer<T>::write(const std::initializer_list<T> list, uint32_t target_offset_elements) {
	if (!is_host_visible) {
		Log::error("Buffer::write() called on non-host-visible buffer");
	}
	size_t list_size_bytes = list.size() * sizeof(T);
	VkDeviceSize target_offset_bytes = target_offset_elements * sizeof(T);
	if (target_offset_bytes + list_size_bytes > this->size_bytes) {
		Log::warning("Buffer::write attempting to write past buffer bounds. Clipping copy region size to fit.");
		list_size_bytes = (this->size_bytes > target_offset_bytes) ? this->size_bytes - target_offset_bytes : 0;
	}
	if (list_size_bytes <= 0) {
		Log::debug("in Buffer<T>::write(): requested copy region has size ", list_size_bytes, " bytes, i.e.nothing to copy");
		return;
	}
	Log::debug("writing from a std::initializer_list<T> to buffer ", buffer, " (memory: ", memory, "), ", list.size(), " list elements (", list_size_bytes, " bytes, with ", target_offset_elements, " elements offset in target");
	void* data;
	vkMapMemory(logical, memory, target_offset_bytes, list_size_bytes, VkMemoryMapFlags(0), &data);
	memcpy(data, list.begin(), list_size_bytes);
	vkUnmapMemory(logical, memory);
}

// copy data elements from one host visible buffer to another
// (set copied elements to 0 to copy all);
// the source buffer must be host visible;
// both buffers must be of the same type
template<typename T>
void Buffer<T>::write(const Buffer<T>& sourcebuffer, uint32_t copied_elements, uint32_t source_offset_elements, uint32_t target_offset_elements) {
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


// flush host writes to host memory domain
// this is only necessary if the memory allocation doesn't have the flag VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
// (can be made available to the device memory domain by using a pipeline barrier with the VK_ACCESS_HOST_WRITE_BIT access type flag)
template<typename T>
void Buffer<T>::flush(uint64_t offset, uint64_t size) {
	void* source = nullptr;
	vkMapMemory(logical, this->memory, offset, size, VkMemoryMapFlags(0), &source);

	VkMappedMemoryRange range;
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.pNext = NULL;
	range.memory = this->memory;
	range.offset = offset;
	range.size = size;

	VkResult result = vkFlushMappedMemoryRanges(logical, 1, &range);
	if (result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			Log::warning("Buffer<T>::flush() failed: out of host memory !");
		}
		else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			Log::warning("Buffer<T>::flush() failed: out of device memory !");
		}
	}

	vkUnmapMemory(logical, this->memory);
}

// returns a continuous data sequence from a host visible data buffer as a std::vector<T>
// (set read_elements to 0 to read all)
template<typename T>
std::vector<T> Buffer<T>::read(uint32_t read_elements, uint32_t source_offset_elements) {
	if (!is_host_visible) {
		Log::error("Buffer<T>::read() called on non-host-visible buffer");
	}
	uint32_t source_elements;
	if (read_elements == 0) {
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
template<typename T>
T Buffer<T>::read_element(uint32_t element_index) const {
	if (!is_host_visible) {
		Log::error("Buffer::read_element(uint32_t element_index) called on non-host-visible buffer");
	}
	if (element_index >= this->elements) {
		Log::error("in method Buffer::get(): element index ", element_index, " is out of bounds (allowed indices: 0-", this->elements - 1, ")");
	}
	void* data = nullptr;
	T element = static_cast<T>(0);
	VkResult result = vkMapMemory(logical, memory, element_index * sizeof(T), sizeof(T), VkMemoryMapFlags(0), &data);
	if (result != VK_SUCCESS) {
		Log::error("in method Buffer<T>::get(uint32_t element_index): failed to map buffer memory (VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
	memcpy(&element, data, sizeof(T));
	vkUnmapMemory(logical, memory);
	return element;
}

// assigns a single element of a host visible data buffer
template<typename T>
void Buffer<T>::write_element(uint32_t element_index, T value) {
	if (!is_host_visible) {
		Log::error("method Buffer<T>::set() called on non-host-visible buffer.");
	}
	if (element_index >= this->elements) {
		Log::error("in method Buffer::set(): element index ", element_index, " is out of bounds (allowed indices: 0-", this->elements - 1, ")");
	}
	void* data = nullptr;
	VkResult result = vkMapMemory(logical, memory, element_index * sizeof(T), sizeof(T), VkMemoryMapFlags(0), &data);
	if (result != VK_SUCCESS) {
		Log::error("in method Buffer<T>::set(uint32_t element_index, T value): failed to map buffer memory (VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
	memcpy(data, &value, sizeof(T));
	vkUnmapMemory(logical, memory);
}

// assigns the same value to continuous sequence of buffer elements
// (set write_elements to 0 to assign all)
template<typename T>
void Buffer<T>::set_all(T value, uint32_t offset_elements, uint32_t write_elements) {
	if (!is_host_visible) {
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
template<typename T> uint32_t Buffer<T>::get_elements() const { return this->elements; }
template<typename T> uint64_t Buffer<T>::get_size_bytes() const { return size_bytes; }
template<typename T> VkDeviceMemory Buffer<T>::get_memory() const { return memory; }
template<typename T> VkBuffer Buffer<T>::get() const { return buffer; }
template<typename T> VkMemoryPropertyFlags Buffer<T>::get_memory_property_flags() const { return memory_property_flags; }
template<typename T> bool Buffer<T>::host_visible() const { return is_host_visible; }


// +=================================+   
// | Vertex                          |
// +=================================+

// helper method to provide a hash function for std::unordered_map (required for the Mesh class)
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return
				((hash<glm::vec3>()(vertex.position) ^
					(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.tex_coord) << 1);
		}
	};
}

bool Vertex::operator==(const Vertex& other) const {
	return position == other.position && normal == other.normal && tex_coord == other.tex_coord;
}

// +=================================+   
// | Mesh                            |
// +=================================+

// parametric constructor
Mesh::Mesh(Device& device, const std::string& obj_relative_path, Semaphore* timeline_semaphore) : device(device) {

	// Dynamically construct the full absolute path to the OBJ file.
	// This assumes the `obj_filename` argument is a path relative to the executable's directory.
	std::filesystem::path project_root = get_executable_directory() / "..";

	// Construct the full path to the OBJ file from the project root.
	std::filesystem::path full_obj_path = project_root / obj_relative_path;

	// Get the directory of the OBJ file to use as the material search path.
	std::string mtl_path = full_obj_path.parent_path().string();

	// Load mesh data with tinyobjloader
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = mtl_path;
	tinyobj::ObjReader reader;

	// Use the absolute path for parsing the file.
	if (!reader.ParseFromFile(full_obj_path.string(), reader_config)) {
		if (!reader.Error().empty()) {
			Log::error("TinyObjReader: %s", reader.Error().c_str());
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		Log::warning("TinyObjReader: %s", reader.Warning().c_str());
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	std::unordered_map<Vertex, uint32_t> unique_vertices{};

	// Group faces by material
	std::map<int, std::vector<tinyobj::index_t>> faces_by_material;
	for (const auto& shape : shapes) {
		size_t index_offset = 0;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
			int material_id = shape.mesh.material_ids[f];
			faces_by_material[material_id].push_back(shape.mesh.indices[index_offset + 0]);
			faces_by_material[material_id].push_back(shape.mesh.indices[index_offset + 1]);
			faces_by_material[material_id].push_back(shape.mesh.indices[index_offset + 2]);
			index_offset += 3;
		}
	}

	// Handle OBJ files with no material information:
	// tinyobjloader defaults material_id to -1 when no material is present.
	// If the OBJ file has no materials at all, we create a default material and treat the
	// entire mesh as a single submesh with that material.
	if (faces_by_material.size() == 1 && faces_by_material.count(-1)) {
		Log::info("No materials found in OBJ file. Creating a default material and a single submesh.");

		// Remove the entry with key -1 and re-insert under a valid material index 0.
		std::vector<tinyobj::index_t> all_faces = faces_by_material[-1];
		faces_by_material.erase(-1);
		faces_by_material[0] = all_faces;

		// Populate a default material.
		Material default_material{};
		default_material.diffuse = { 1.0f, 1.0f, 1.0f }; // white
		this->materials.push_back(default_material);
		this->materials_std430.push_back(MaterialStd430(default_material));
	}

	// Populate vertices, indices, and submeshes
	uint32_t current_index_count = 0;
	uint32_t current_vertex_count = 0;
	for (const auto& pair : faces_by_material) {
		uint32_t material_index = pair.first;
		const auto& faces = pair.second;

		SubMesh submesh{};
		submesh.material_index = material_index;
		submesh.first_index = current_index_count;
		submesh.vertex_offset = current_vertex_count;

		uint32_t submesh_index_count = 0;
		for (const auto& index : faces) {
			Vertex vertex{};
			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};
			if (attrib.texcoords.size() > 0) {
				vertex.tex_coord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			if (!attrib.colors.empty()) {
				vertex.color = {
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2],
					1.0f // Assign a default alpha value (the .obj format doesn't natively define an alpha channel for vertex colors)
				};
			}

			if (unique_vertices.count(vertex) == 0) {
				unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(unique_vertices[vertex]);
			submesh_index_count++;
		}

		submesh.index_count = submesh_index_count;
		submeshes.push_back(submesh);

		current_index_count += submesh_index_count;
		current_vertex_count = static_cast<uint32_t>(vertices.size());
	}

	// Populate materials
	for (const auto& mat : materials) {
		Material material{};
		material.ambient = { mat.ambient[0], mat.ambient[1], mat.ambient[2] };
		material.diffuse = { mat.diffuse[0], mat.diffuse[1], mat.diffuse[2] };
		material.specular = { mat.specular[0], mat.specular[1], mat.specular[2] };
		material.transmittance = { mat.transmittance[0], mat.transmittance[1], mat.transmittance[2] };
		material.emission = { mat.emission[0], mat.emission[1], mat.emission[2] };
		material.shininess = mat.shininess;
		material.ior = mat.ior;
		material.dissolve = mat.dissolve;
		material.illum = mat.illum;
		this->materials.push_back(material);
		this->materials_std430.push_back(MaterialStd430(material));
	}

	this->index_count = static_cast<uint32_t>(indices.size());
	this->vertex_count = static_cast<uint32_t>(vertices.size());
	this->material_count = static_cast<uint32_t>(materials_std430.size());
	this->index_type = VK_INDEX_TYPE_UINT32;

	// Populate vertex descriptions
	const uint32_t binding = vertex_descriptions.add_binding(sizeof(Vertex));
	vertex_descriptions.add_attribute(0, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
	if (!attrib.normals.empty()) {
		vertex_descriptions.add_attribute(1, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
	}
	if (!attrib.texcoords.empty()) {
		vertex_descriptions.add_attribute(2, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, tex_coord));
	}
	if (!attrib.colors.empty()) {
		vertex_descriptions.add_attribute(3, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color));
	}

	// acquire an idle task
	VulkanManager& manager = VulkanManager::get_singleton();
	Task& task = manager.get_graphics_task(__FUNCTION__);

	// write vertex, index and material data to staging buffers (owned by the task to ensure their lifetime exceeds this local scope)
	Log::info("Loading mesh with ", this->vertex_count, " vertices and ", this->index_count, " indices (write to staging buffer, then transfer to device_local memory).");
	auto& vertex_staging_buffer = task.add_temp_buffer<Vertex>(vertex_count, VERTEX_BUFFER, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	auto& index_staging_buffer = task.add_temp_buffer<uint32_t>(index_count, INDEX_BUFFER, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	auto& material_staging_buffer = task.add_temp_buffer<MaterialStd430>(material_count, STORAGE_BUFFER, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vertex_staging_buffer.write(vertices);
	index_staging_buffer.write(indices);
	material_staging_buffer.write(materials_std430);

	// initialize device_local buffers
	this->vertex_buffer = std::make_unique<Buffer<Vertex>>(device, VERTEX_BUFFER, vertex_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	this->index_buffer = std::make_unique<Buffer<uint32_t>>(device, INDEX_BUFFER, index_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	this->material_buffer = std::make_unique<Buffer<MaterialStd430>>(device, STORAGE_BUFFER, material_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// transfer from staging buffers to device_local memory
	CommandBuffer& cb = task.get_command_buffer();
	cb.begin_recording();
	cb.add_buffer_memory_barrier(vertex_staging_buffer, VK_ACCESS_2_HOST_WRITE_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_HOST_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
	cb.add_buffer_memory_barrier(index_staging_buffer, VK_ACCESS_2_HOST_WRITE_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_HOST_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
	cb.add_buffer_memory_barrier(material_staging_buffer, VK_ACCESS_2_HOST_WRITE_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_HOST_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT);
	cb.copy_buffer(vertex_staging_buffer, *vertex_buffer);
	cb.copy_buffer(index_staging_buffer, *index_buffer);
	cb.copy_buffer(material_staging_buffer, *material_buffer);
	cb.end_recording();
	if (timeline_semaphore) {
		if (timeline_semaphore->get_type() == VK_SEMAPHORE_TYPE_TIMELINE) {
			task.timeline_sync(*timeline_semaphore);
		}
		else {
			Log::warning("In Mesh constructor: invalid semaphore argument. Must be a timeline semaphore! Semaphore will not be used.");
		}
	}
	task.submit();
}

// destructor
Mesh::~Mesh() {}

// getters
const Buffer<Vertex>& Mesh::get_vertex_buffer() const { return *this->vertex_buffer; }
const Buffer<uint32_t>& Mesh::get_index_buffer() const { return *this->index_buffer; }
const Buffer<MaterialStd430>& Mesh::get_material_buffer() const { return *this->material_buffer; }
uint32_t Mesh::get_index_count() const { return this->index_count; }
uint32_t Mesh::get_vertex_count() const { return this->vertex_count; }
const VertexDescriptions& Mesh::get_vertex_descriptions() const { return vertex_descriptions; }
VkIndexType Mesh::get_index_type() const { return index_type; }
const std::vector<SubMesh>& Mesh::get_submeshes() const { return submeshes; }

// +=================================+   
// | Entity                          |
// +=================================+
// parametric constructor
Entity::Entity(Mesh& mesh) : mesh(&mesh) {};

// Calculates the model matrix, applying the transformations in the correct order: scale, rotate, then translate.
glm::mat4 Entity::get_model_matrix() {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale);
	return model;
}

// Public setter methods for transformations.
void Entity::set_position(const glm::vec3& position) { this->position = position; }
void Entity::set_rotation(const glm::vec3& rotation) { this->rotation = rotation; }
void Entity::set_scale(const glm::vec3& scale) { this->scale = scale; }

// Public getter method for the mesh.
Mesh& Entity::get_mesh() const { return *mesh; }

// +=================================+   
// | Camera                          |
// +=================================+

// Constructor with vectors
Camera::Camera(glm::vec3 camera_position, glm::vec3 camera_up, float camera_yaw, float camera_pitch) :
	front(glm::vec3(0.0f, 0.0f, -1.0f)),
	movement_speed(CAMERA_SPEED),
	mouse_sensitivity(CAMERA_SENSITIVITY),
	fov(CAMERA_FOV),
	aspect_ratio(4.0f / 3.0f),
	near_plane(0.1f),
	far_plane(100.0f) {
	position = camera_position;
	world_up = camera_up;
	yaw = camera_yaw;
	pitch = camera_pitch;
}

// Constructor with scalar values
Camera::Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float camera_yaw, float camera_pitch) :
	front(glm::vec3(0.0f, 0.0f, -1.0f)),
	movement_speed(CAMERA_SPEED),
	mouse_sensitivity(CAMERA_SENSITIVITY),
	fov(CAMERA_FOV),
	aspect_ratio(4.0f / 3.0f),
	near_plane(0.1f),
	far_plane(100.0f) {
	position = glm::vec3(pos_x, pos_y, pos_z);
	world_up = glm::vec3(up_x, up_y, up_z);
	yaw = camera_yaw;
	pitch = camera_pitch;
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::get_view_matrix() {
	// update the front vector
	glm::vec3 new_front;
	new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	new_front.y = sin(glm::radians(pitch));
	new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(new_front);

	// update right and up vector
	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(glm::cross(right, front));

	return glm::lookAt(position, position + front, up);
}

// Returns the projection matrix
glm::mat4 Camera::get_projection_matrix() {
	return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
}

// Sets the camera's position.
void Camera::set_position(const glm::vec3& new_position) {
	position = new_position;
}

// Sets the world up vector.
void Camera::set_world_up(const glm::vec3& new_world_up) {
	world_up = new_world_up;
}

// Sets the yaw angle.
void Camera::set_yaw(float new_yaw) {
	yaw = new_yaw;
}

// Sets the pitch angle.
void Camera::set_pitch(float new_pitch) {
	pitch = new_pitch;
	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}
}

// Sets the movement speed.
void Camera::set_movement_speed(float new_speed) {
	movement_speed = new_speed;
}

// Sets the mouse sensitivity.
void Camera::set_mouse_sensitivity(float new_sensitivity) {
	mouse_sensitivity = new_sensitivity;
}

// Sets the field of view.
void Camera::set_fov(float new_fov) {
	fov = new_fov;
	if (fov < 1.0f) {
		fov = 1.0f;
	}
	if (fov > 45.0f) {
		fov = 45.0f;
	}
}

// Sets the aspect ratio.
void Camera::set_aspect_ratio(float new_aspect_ratio) {
	aspect_ratio = new_aspect_ratio;
}

// Sets the near plane distance.
void Camera::set_near_plane(float new_near_plane) {
	near_plane = new_near_plane;
}

// Sets the far plane distance.
void Camera::set_far_plane(float new_far_plane) {
	far_plane = new_far_plane;
}

// Processes input received from any keyboard-like input system.
void Camera::process_keyboard(CameraMovement direction, float delta_time) {
	float velocity = movement_speed * delta_time;
	if (direction == CameraMovement::CAMERA_FORWARD)
		position += front * velocity;
	if (direction == CameraMovement::CAMERA_BACKWARD)
		position -= front * velocity;
	if (direction == CameraMovement::CAMERA_LEFT)
		position -= right * velocity;
	if (direction == CameraMovement::CAMERA_RIGHT)
		position += right * velocity;
	if (direction == CameraMovement::CAMERA_UP)
		position += world_up * velocity;
	if (direction == CameraMovement::CAMERA_DOWN)
		position -= world_up * velocity;
}

// Processes input received from a mouse input system.
void Camera::process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch) {
	x_offset *= mouse_sensitivity;
	y_offset *= mouse_sensitivity;

	yaw += x_offset;
	pitch += y_offset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrain_pitch) {
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}
}

// Processes input received from a mouse scroll-wheel event.
void Camera::process_mouse_scroll(float y_offset) {
	set_fov(fov - y_offset);
}

// +=================================+   
// | Sampler                         |
// +=================================+

// Sampler class for texture sampling

// constructor
Sampler::Sampler(
	Device& device,
	VkFilter magFilter,
	VkFilter minFilter,
	VkSamplerAddressMode addressMode,
	VkSamplerMipmapMode mipmapMode,
	VkBool32 anisotropyEnable,
	float maxAnisotropy
) : logical(device.get_logical()) {
	sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.pNext = nullptr;
	sampler_create_info.magFilter = magFilter;
	sampler_create_info.minFilter = minFilter;
	sampler_create_info.addressModeU = addressMode;
	sampler_create_info.addressModeV = addressMode;
	sampler_create_info.addressModeW = addressMode;
	sampler_create_info.anisotropyEnable = anisotropyEnable;
	sampler_create_info.maxAnisotropy = maxAnisotropy;
	sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_create_info.unnormalizedCoordinates = VK_FALSE;
	sampler_create_info.compareEnable = VK_FALSE;
	sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_create_info.mipmapMode = mipmapMode;

	if (vkCreateSampler(logical, &sampler_create_info, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

// destructor
Sampler::~Sampler() {
	if (sampler != nullptr) {
		vkDestroySampler(logical, sampler, nullptr);
		Log::info("destroyed image sampler (handle: ", sampler, ")");
		sampler = nullptr;
	}
}

// getters
const VkSampler Sampler::get() const { return sampler; }
const VkSampler* Sampler::get_ptr() const { return &sampler; }

// +=================================+   
// | DescriptorSetLayout             |
// +=================================+

// constructor
DescriptorSetLayout::DescriptorSetLayout(Device& device) : logical(device.get_logical()) {}

// destructor
DescriptorSetLayout::~DescriptorSetLayout() {
	if (layout != nullptr) {
		Log::debug("destroying descriptor set layout (handle: ", layout, ")");
		vkDestroyDescriptorSetLayout(logical, layout, nullptr);
		layout = nullptr;
		Log::info("[DESCRIPTORSET LAYOUT DESTROYED]");
	}
}

// add binding for a buffer or image
uint32_t DescriptorSetLayout::add_binding(DescriptorType type, VkShaderStageFlagBits shader_stage_flags, VkDescriptorBindingFlags binding_flags) {
	this->binding_flags.push_back(binding_flags);
	VkDescriptorSetLayoutBinding binding = {};
	uint32_t binding_index = static_cast<uint32_t>(layout_bindings.size());
	binding.binding = binding_index;
	binding.descriptorType = get_descriptor_type(type);
	binding.descriptorCount = 1;
	binding.stageFlags = shader_stage_flags;
	binding.pImmutableSamplers = nullptr;
	layout_bindings.push_back(binding);
	return binding_index;
}

// add multiple bindings of the same type at once
void DescriptorSetLayout::add_bindings(uint32_t count, DescriptorType type, VkShaderStageFlagBits shader_stage_flags) {
	for (uint32_t i = 0; i < count; i++) {
		add_binding(type, shader_stage_flags);
	}
}

// finalizes the descriptor set layout and creates the descriptor set
void DescriptorSetLayout::finalize() {
	if (!finalized) {
		VkDescriptorSetLayoutBindingFlagsCreateInfo layout_binding_flags_create_info = {};
		layout_binding_flags_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layout_binding_flags_create_info.pNext = nullptr;
		layout_binding_flags_create_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());
		layout_binding_flags_create_info.pBindingFlags = binding_flags.data();

		VkDescriptorSetLayoutCreateInfo layout_create_info = {};
		layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_create_info.pNext = &layout_binding_flags_create_info;
		layout_create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; // requires VK_EXT_descriptor_indexing
		layout_create_info.pBindings = layout_bindings.data();
		layout_create_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());
		VkResult result = vkCreateDescriptorSetLayout(logical, &layout_create_info, nullptr, &layout);
		if (result == VK_SUCCESS) {
			Log::info("descriptor set layout created (", layout_bindings.size(), " bindings, layout handle : ", layout, ")");
		}
		else {
			Log::error("DescriptorSetLayout::finalize() has failed (VkResult ", result, ", ", vkresult_to_string(result), ")");
		}
		finalized = true;
	}
}

// getters
uint32_t DescriptorSetLayout::get_bindings_count() const { return static_cast<uint32_t>(layout_bindings.size()); }
const VkDescriptorSetLayoutBinding DescriptorSetLayout::get_binding(uint32_t binding_index) const { return layout_bindings[binding_index]; }
const VkDescriptorSetLayout DescriptorSetLayout::get() const { return layout; }
const VkDescriptorSetLayout* DescriptorSetLayout::get_ptr() const { return &layout; }


// +=================================+   
// | ImageView                       |
// +=================================+

ImageView::ImageView(Device& device, Image& image, VkImageViewType view_type, VkImageAspectFlags aspect_flags, uint32_t base_mip_level, uint32_t level_count, uint32_t base_array_layer, uint32_t layer_count)
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
		Log::error("Failed to create image view (VkResult=", result, ", ", vkresult_to_string(result), ")");
		// Handle error
		return;
	}
	Log::info("ImageView created successfully (handle: ", image_view, ")");
}

// move constructor
ImageView::ImageView(ImageView&& other) noexcept :
	logical(std::exchange(other.logical, VK_NULL_HANDLE)),
	image_view(std::exchange(other.image_view, VK_NULL_HANDLE)) {
}

// move assignment
ImageView& ImageView::operator=(ImageView&& other) noexcept {
	if (this != &other) {
		destroy();
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		image_view = std::exchange(other.image_view, VK_NULL_HANDLE);
	}
	return *this;
}

// destructor
ImageView::~ImageView() {
	destroy();
}

// getters
VkImageView ImageView::get() const { return image_view; }

// helper method to release resources
void ImageView::destroy() {
	if (image_view != VK_NULL_HANDLE) {
		Log::info("Destroying image view (handle: ", image_view, ")");
		vkDestroyImageView(logical, image_view, nullptr);
		image_view = VK_NULL_HANDLE;
	}
}


// +=================================+   
// | BufferView                      |
// +=================================+

// class for creating a view into a data buffer

// parametric constructor
template<typename T>
BufferView<T>::BufferView(Device& device, Buffer<T>& buffer, VkFormat format)
	: logical(device.get_logical()), buffer_ref(buffer), format(format) {

	// Confirm valid device and buffer
	if (logical == VK_NULL_HANDLE) {
		Log::error("BufferView constructor called with an invalid logical device handle.");
		return;
	}
	if (buffer_ref.get() == VK_NULL_HANDLE) {
		Log::error("BufferView constructor called with an invalid buffer handle.");
		return;
	}

	// create the buffer view
	VkBufferViewCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	create_info.buffer = buffer_ref.get();
	create_info.format = format;
	create_info.offset = 0;
	create_info.range = buffer_ref.get_size_bytes();

	VkResult result = vkCreateBufferView(logical, &create_info, nullptr, &buffer_view);
	if (result != VK_SUCCESS) {
		Log::error("Failed to create Vulkan buffer view (VkResult=", result, ", ", vkresult_to_string(result), ")");
		buffer_view = VK_NULL_HANDLE; // Ensure handle is null on failure
	}
	else {
		Log::info("Successfully created buffer view (handle: ", buffer_view, ")");
	}
}

// move constructor
template<typename T>
BufferView<T>::BufferView(BufferView&& other) noexcept
	: logical(other.logical),
	buffer_ref(other.buffer_ref),
	format(other.format),
	buffer_view(std::exchange(other.buffer_view, VK_NULL_HANDLE)) {
	Log::info("BufferView moved (new owner), handle: ", buffer_view);
}

// move assignment
template<typename T>
BufferView<T>& BufferView<T>::operator=(BufferView<T>&& other) noexcept {
	if (this != &other) {
		// First, destroy resources owned by 'this'
		destroy();

		// Then, copy the immutable handles and move the owned handle
		this->logical = other.logical;
		this->buffer_ref = other.buffer_ref;
		this->format = other.format;
		this->buffer_view = std::exchange(other.buffer_view, VK_NULL_HANDLE);

		Log::info("BufferView move assigned (new owner), handle: ", buffer_view);
	}
	return *this;
}

// getter
template<typename T> VkBufferView BufferView<T>::get() const { return buffer_view; }

// destructor
template<typename T>
BufferView<T>::~BufferView() {
	destroy();
}

// helper method to release resources
template<typename T>
void BufferView<T>::destroy() {
	if (buffer_view != VK_NULL_HANDLE) {
		vkDestroyBufferView(logical, buffer_view, nullptr);
		buffer_view = VK_NULL_HANDLE;
		Log::info("[BUFFER VIEW DESTROYED]");
	}
}

// +=================================+   
// | DepthBuffer                     |
// +=================================+

// Constructor: Creates the depth image, allocates memory, and creates the image view.
DepthBuffer::DepthBuffer(Device& device, VkImageType image_type, VkExtent3D extent, VkImageUsageFlags usage) : logical(device.get_logical()) {

	// Find the best supported format for depth/stencil attachments.
	this->format = device.find_supported_format(DEPTH_FORMAT_CANDIDATES, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	// create the Image object
	image = std::make_unique<Image>(
		device,
		image_type,
		format,
		extent,
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	// get the memory handle from the image
	memory = image->get_memory();

	VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D; // default
	if (image_type == VkImageType::VK_IMAGE_TYPE_1D) { view_type = VK_IMAGE_VIEW_TYPE_1D; }
	else if (image_type == VkImageType::VK_IMAGE_TYPE_3D) { view_type = VK_IMAGE_VIEW_TYPE_3D; }

	// create the ImageView
	image_view = std::make_unique<ImageView>(device, *image, view_type, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1);
}

// Destructor: Cleans up the raw Vulkan handle. The image and image view
// wrappers will clean themselves up automatically.
DepthBuffer::~DepthBuffer() {
	// We only need to clean up the raw VkDeviceMemory handle, as the
	// Image and ImageView destructors will handle their respective
	// VkImage and VkImageView cleanup.
	if (memory != VK_NULL_HANDLE) {
		vkFreeMemory(logical, memory, nullptr);
	}
}

// Move Constructor
DepthBuffer::DepthBuffer(DepthBuffer&& other) noexcept
	: logical(other.logical),
	image(std::move(other.image)),
	image_view(std::move(other.image_view)),
	memory(other.memory),
	format(other.format) {
	// Set other's memory handle to null to prevent it from being freed
	// when the original object is destroyed.
	other.memory = VK_NULL_HANDLE;
}

// Move Assignment Operator
DepthBuffer& DepthBuffer::operator=(DepthBuffer&& other) noexcept {
	if (this != &other) {
		// First, clean up our own resources to avoid a memory leak.
		if (memory != VK_NULL_HANDLE) {
			vkFreeMemory(logical, memory, nullptr);
		}

		// Move the data from the other object.
		image = std::move(other.image);
		image_view = std::move(other.image_view);
		memory = other.memory;
		format = other.format;
		logical = other.logical;

		// Set the other object's memory handle to null.
		other.memory = VK_NULL_HANDLE;
	}
	return *this;
}

// getters
const ImageView& DepthBuffer::get_image_view() const { return *image_view; }
const Image& DepthBuffer::get_image() const { return *image; }
VkFormat DepthBuffer::get_format() const { return format; }
VkDeviceMemory DepthBuffer::get_memory() const { return memory; }

// +=================================+   
// | Swapchain                       |
// +=================================+

// constructor
Swapchain::Swapchain(
	Device& device,
	RenderPass& renderpass,
	Surface& surface,
	VkSurfaceFormatKHR surface_format,
	VkImageUsageFlags image_usage,
	DepthBuffer& depth_buffer,
	const std::optional<std::vector<VkImageView>>& other_image_views,
	VkExtent2D extent,
	uint32_t min_image_count,
	VkImageViewType view_type,
	VkPresentModeKHR present_mode_preference) :

	// init list
	logical(device.get_logical()),
	device(&device),
	renderpass(&renderpass),
	surface(&surface),
	surface_format(surface_format),
	surface_capabilities(surface.get_capabilities(device)),
	image_usage(image_usage),
	depth_buffer(&depth_buffer),
	other_image_views(other_image_views),
	extent(extent),
	view_type(view_type),
	present_mode_preference(present_mode_preference) {

	// store member variables according to constructor arguments
	if (!surface.get_physical_device_support(device, QueueFamily::GRAPHICS_QUEUE)) {
		Log::error("invalid Swapchain call: present not supported on the selected physical device and graphics queue family!");
	}

	// chose present mode
	std::vector<VkPresentModeKHR> available_present_modes = surface.get_present_modes(device);
	if (available_present_modes.empty()) {
		Log::error("Swapchain creation failed; no suitable present modes available for the surface");
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
		Log::warning("in Swapchain constructor: preferred present mode not available -> falling back to FIFO as default");
	}

	// adjust image count
	uint32_t image_count = min_image_count;
	if (image_count > surface_capabilities.maxImageCount) {
		image_count = surface_capabilities.maxImageCount;
		Log::warning("in Swapchain constructor: min image count exceeds max image count of surface capabilities -> reduced to ", image_count);
	}
	if (image_count < surface_capabilities.minImageCount) {
		image_count = surface_capabilities.minImageCount;
		Log::warning("in Swapchain constructor: surface capabilities require min image count of >=", image_count, " -> adjusted accordingly");
	}

	// adjust/limit image extent to surface capabilities
	this->extent.width = std::min(extent.width, surface_capabilities.maxImageExtent.width);
	this->extent.width = std::max(this->extent.width, surface_capabilities.minImageExtent.width);
	this->extent.height = std::min(extent.height, surface_capabilities.maxImageExtent.height);
	this->extent.height = std::max(this->extent.height, surface_capabilities.minImageExtent.height);

	// setup Swapchain details
	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface.get();
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1; // Use > 1 for stereoscopic rendering
	create_info.imageUsage = image_usage; // e.g., VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0; // Optional for exclusive
	create_info.pQueueFamilyIndices = nullptr; // Optional for exclusive
	create_info.preTransform = surface_capabilities.currentTransform; // e.g. VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.clipped = VK_TRUE; // Allow clipping occluded pixels
	create_info.oldSwapchain = VK_NULL_HANDLE; // Required for recreation, null for initial creation
	create_info.presentMode = selected_present_mode;

	// finalize Swapchain
	VkResult result = vkCreateSwapchainKHR(logical, &create_info, nullptr, &swapchain);
	if (result != VK_SUCCESS) {
		Log::error("Failed to create Swapchain (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	else {
		Log::debug("Swapchain created successfully.");
	}

	// get images
	vkGetSwapchainImagesKHR(logical, swapchain, &num_images, nullptr);
	image.resize(num_images);
	vkGetSwapchainImagesKHR(logical, swapchain, &num_images, image.data());

	// create image views for Swapchain images
	color_image_views.resize(num_images);
	for (uint32_t i = 0; i < num_images; i++) {
		VkImageViewCreateInfo view_info = {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = image[i];
		view_info.viewType = view_type;
		view_info.format = surface_format.format;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		result = vkCreateImageView(logical, &view_info, nullptr, &color_image_views[i]);
		if (result != VK_SUCCESS) {
			Log::error("Failed to create Swapchain image view ", i);
		}
	}
	Log::info("Swapchain created with ", num_images, " images/views.");

	// create framebuffers
	framebuffer.resize(num_images);
	uint32_t expected_attachments = 2u; // guarantee a minimum of 2 attachments (for color and depth)
	if (other_image_views.has_value()) {
		expected_attachments += static_cast<uint32_t>(other_image_views.value().size());
	}

	for (uint32_t i = 0; i < num_images; i++) {
		std::vector<VkImageView> attachments;
		attachments.reserve(expected_attachments);
		attachments.push_back(color_image_views[i]);
		attachments.push_back(depth_buffer.get_image_view().get());

		// add other attachments
		if (other_image_views.has_value()) {
			attachments.insert(attachments.end(), other_image_views.value().begin(), other_image_views.value().end());
		}

		VkFramebufferCreateInfo framebuffer_create_info = {};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = renderpass.get();
		framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_create_info.pAttachments = attachments.data();
		framebuffer_create_info.width = extent.width;
		framebuffer_create_info.height = extent.height;
		framebuffer_create_info.layers = 1;

		VkResult result = vkCreateFramebuffer(logical, &framebuffer_create_info, nullptr, &framebuffer[i]);
		if (result != VK_SUCCESS) {
			Log::error("Failed to create framebuffer ", i, " (VkResult=", result, ", ", vkresult_to_string(result), ")");
		}
	}

	Log::info("Swapchain framebuffers created successfully.");
}

uint32_t Swapchain::acquire_next_image(const Semaphore& image_available_semaphore, const std::optional<Fence>& fence, uint64_t timeout) {
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
		Log::error("Failed to acquire Swapchain image (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	// else: Success
	return current_image_index;
}

// present the rendered image to the graphics queue (method overload without semaphores)
VkResult Swapchain::present_rendered_image() {
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
		Log::error("Failed to present Swapchain image (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	// else: Success

	return result;
}

// present the rendered image to the graphics queue (method overload with single semaphore)
VkResult Swapchain::present_rendered_image(const Semaphore& wait_semaphore) {
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = wait_semaphore.get_ptr();
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
		Log::error("Failed to present Swapchain image (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	// else: Success

	return result;
}

// present the rendered image to the graphics queue (method overload with multiple semaphores)
VkResult Swapchain::present_rendered_image(const std::vector<Semaphore>& wait_semaphores) {
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
	std::vector<VkSemaphore> wait_semaphore_handles(wait_semaphores.size());
	for (uint32_t i = 0; i < wait_semaphores.size(); i++) {
		wait_semaphore_handles[i] = wait_semaphores[i].get();
	}
	present_info.pWaitSemaphores = wait_semaphore_handles.data();
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
		Log::info("Swapchain suboptimal during present. Okay to continue, but should be recreated soon.");
	}
	else if (result != VK_SUCCESS) {
		Log::error("Failed to present Swapchain image (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
	// else: Success

	return result;
}

void Swapchain::recreate() {
	destroy();
	// Recreate Swapchain with the same parameters
	Swapchain new_swapchain(
		*device,
		*renderpass,
		*surface,
		surface_format,
		image_usage,
		*depth_buffer,
		*other_image_views,
		extent,
		num_images,
		view_type,
		present_mode_preference
	);
	swapchain = new_swapchain.get();
	image = new_swapchain.image;
	color_image_views = new_swapchain.color_image_views;
	framebuffer = new_swapchain.framebuffer;
	framebuffer_image_views = new_swapchain.framebuffer_image_views;
}

// getters
uint32_t Swapchain::get_width() const { return extent.width; }
uint32_t Swapchain::get_height() const { return extent.height; }
VkExtent2D Swapchain::get_extent() const { return extent; }
VkSwapchainKHR Swapchain::get() const { return swapchain; }
VkImage Swapchain::get_current_image() const { return image[current_image_index]; }
uint32_t Swapchain::get_current_image_index() const { return current_image_index; }
uint32_t Swapchain::get_image_count() const { return num_images; }
VkSurfaceFormatKHR Swapchain::get_surface_format() const { return surface_format; }

VkImageView Swapchain::get_color_image_view(uint32_t index) const {
	if (index < color_image_views.size()) {
		return color_image_views[index];
	}
	else {
		Log::error("Invalid index for color image view: ", index);
		return VK_NULL_HANDLE;
	}
}

VkImageView Swapchain::get_framebuffer_image_view(uint32_t index) const {
	if (index < framebuffer_image_views.size()) {
		return framebuffer_image_views[index];
	}
	else {
		Log::error("Invalid index for framebuffer image view: ", index);
		return VK_NULL_HANDLE;
	}
}

VkFramebuffer Swapchain::get_framebuffer(uint32_t index) const {
	if (index < framebuffer.size()) {
		return framebuffer[index];
	}
	else {
		Log::error("Invalid index for framebuffer: ", index);
		return VK_NULL_HANDLE;
	}
}

VkImage Swapchain::get_image(uint32_t index) const {
	if (index < image.size()) {
		return image[index];
	}
	else {
		Log::error("Invalid index for image: ", index);
		return VK_NULL_HANDLE;
	}
}

// destructor
Swapchain::~Swapchain() {
	destroy();
}

void Swapchain::destroy() {
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


// +=================================+   
// | DescriptorPool                  |
// +=================================+

// DescriptorPool manages descriptor sets and their memory allocation

// parametric constructor
DescriptorPool::DescriptorPool(Device& device, const uint32_t max_sets, const std::vector<VkDescriptorPoolSize>& pool_sizes) :
	logical(device.get_logical()),
	max_sets(max_sets) {

	// setup create info
	VkDescriptorPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.maxSets = max_sets;
	create_info.pPoolSizes = pool_sizes.data();
	create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	create_info.pNext = NULL;

	// Create the descriptor pool
	VkResult result = vkCreateDescriptorPool(logical, &create_info, nullptr, &pool);
	if (result == VK_SUCCESS) {
		Log::debug("successfully created descriptor pool (handle: ", pool, ")");
	}
	else {
		Log::error("failed to create descriptor pool (VkResult =  ", result, ", ", vkresult_to_string(result), ")");
	}
}

// destructor
DescriptorPool::~DescriptorPool() {
	if (pool != VK_NULL_HANDLE) {
		Log::debug("destroying descriptor pool (handle: ", pool, ")");
		release_all_sets();
		vkDestroyDescriptorPool(logical, pool, nullptr);
		pool = VK_NULL_HANDLE;
		Log::info("[DESCRIPTOR POOL DESTROYED]");
	}
}

// move constructor
DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept :
	pool(std::exchange(other.pool, VK_NULL_HANDLE)),
	logical(std::exchange(other.logical, VK_NULL_HANDLE)),
	sets(std::move(other.sets)),
	max_sets(other.max_sets) {
	if (pool != nullptr) {
		Log::info("descriptor pool moved (handle: ", pool, ")");
	}
}

// move assignment
DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept {
	if (this != &other) {
		// reference members (device) cannot be reassigned!
		// We must assume that 'this' already has a valid 'device' reference.
		if (pool != nullptr) {
			Log::info("move assignment operation: destroying previous descriptor pool (handle: ", pool, ")");
			vkDestroyDescriptorPool(logical, pool, nullptr);
			pool = nullptr;
		}
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		pool = std::exchange(other.pool, VK_NULL_HANDLE);
		sets = std::move(other.sets);
		max_sets = other.max_sets;
		if (pool != VK_NULL_HANDLE) {
			Log::info("descriptor pool moved to 'this' (handle: ", pool, ")");
		}
	}
	return *this;
}

// getters
VkDescriptorPool DescriptorPool::get() const { return pool; }
const std::vector<VkDescriptorSet>& DescriptorPool::get_sets() const { return sets; }
uint32_t DescriptorPool::get_max_sets() const { return max_sets; }
uint32_t DescriptorPool::get_current_sets_count() const { return uint32_t(sets.size()); }

// release all descriptor sets from the pool
void DescriptorPool::release_all_sets() {
	if (sets.empty()) { return; }
	VkResult result = vkFreeDescriptorSets(logical, pool, sets.size(), sets.data());
	if (result == VK_SUCCESS) {
		Log::debug("all descriptor sets removed from pool, memory allocation freed");
	}
	else {
		Log::warning("failed to remove descriptor sets from pool (VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
	sets.clear();
}

uint32_t DescriptorPool::release_set(VkDescriptorSet set) {
	if (sets.empty()) { return 0; }

	// remove from VkDescriptorSet vector of the pool
	for (uint32_t i = 0; i < sets.size(); i++) {
		if (sets[i] == set) {
			sets.erase(sets.begin() + i);
		}
	}

	// free set
	VkResult result = vkFreeDescriptorSets(logical, pool, 1, &set);
	if (result == VK_SUCCESS) {
		Log::debug("descriptor set removed from pool, memory allocation freed");
	}
	else {
		Log::warning("failed to remove descriptor set from pool (VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
	return sets.size();
}

uint32_t DescriptorPool::allocate_set(VkDescriptorSet& descriptor_set, DescriptorSetLayout& descriptor_set_layout) {
	if (sets.size() >= max_sets) {
		Log::error("in method DescriptorPool::allocate_set(): max number of sets for this pool is ", max_sets, " (as defined by the pool constructor) and has been reached; no more descriptor sets can be added");
	}
	VkDescriptorSetAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.descriptorPool = pool;
	allocate_info.descriptorSetCount = 1;
	allocate_info.pSetLayouts = descriptor_set_layout.get_ptr();
	VkResult result = vkAllocateDescriptorSets(logical, &allocate_info, &descriptor_set);
	if (result != VK_SUCCESS) {
		Log::error("failed to allocate descriptor set (VkResult ", result, ", ", vkresult_to_string(result), ")");
	}
	uint32_t index = static_cast<uint32_t>(sets.size());
	Log::debug("adding new descriptor set (set index = ", index, ") to descriptor pool (pool handle: ", pool, ")");
	sets.push_back(descriptor_set);
	return index;
}

// +=================================+   
// | DescriptorSet                   |
// +=================================+

// DescriptorSets hold binding information for shader resources

// parametric constructor
DescriptorSet::DescriptorSet(Device& device, DescriptorSetLayout& layout, DescriptorPool& pool) : logical(device.get_logical()), pool(pool), layout(layout) {
	if (layout.get() == nullptr) {
		Log::error("Invalid call of DescriptorSet constructor. The passed layout is NULL. Make sure to create a valid layout first!");
	}
	layout.finalize(); // =just to make sure; does nothing if the layout has already been finalized
	pool.allocate_set(this->set, layout);
	binding_info.resize(layout.get_bindings_count());
};

// destructor
DescriptorSet::~DescriptorSet() {
	this->pool.release_set(this->set);
}

// move constructor
DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
	: logical(std::exchange(other.logical, nullptr)),
	set(std::exchange(other.set, nullptr)),
	layout(other.layout),
	pool(other.pool),
	binding_info(std::move(other.binding_info)) {
}

// move assignment
DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept {
	if (this != &other) {
		logical = std::exchange(other.logical, nullptr);
		this->set = std::exchange(other.set, nullptr);
		binding_info = std::move(other.binding_info);
	}
	return *this;
}

// binds a buffer to the descriptor set
template<typename T>
void DescriptorSet::bind_buffer(uint32_t binding_index, const Buffer<T>& buffer, VkShaderStageFlagBits shader_stage_flags) {
	// confirm valid binding index
	uint32_t bindings_count = layout.get_bindings_count();
	if (binding_index >= bindings_count) {
		Log::error("in method DescriptorSet::bind_buffer(): argument for the binding index is invalid; value is ", binding_index,
			" but the descriptor set layout only has ", bindings_count, " bindings (indices 0-", bindings_count - 1, ").");
	}
	else {
		Log::debug("binding/replacing buffer at binding index ", binding_index, " with new buffer ", buffer.get(), " in descriptor set (handle: ", set, ")");
	}

	binding_info[binding_index].binding_index = binding_index;
	binding_info[binding_index].buffer = buffer.get();
	binding_info[binding_index].offset = 0;
	binding_info[binding_index].range = VK_WHOLE_SIZE;
	binding_info[binding_index].descriptor_type = layout.get_binding(binding_index).descriptorType;
	binding_info[binding_index].updated = false;
}

// bind an image to the descriptor set
void DescriptorSet::bind_image(uint32_t binding_index, const ImageView& image_view, DescriptorType type, const Sampler& sampler, VkImageLayout image_layout, VkShaderStageFlagBits shader_stage_flags) {
	// confirm valid binding index
	uint32_t bindings_count = layout.get_bindings_count();
	if (binding_index >= bindings_count) {
		Log::error("in method DescriptorSet::bind_image(): argument for the binding index is invalid; value is ", binding_index,
			" but the descriptor set layout only has ", bindings_count, " bindings (indices 0-", bindings_count - 1, ").");
	}

	// confirm matching descriptor type
	if (get_descriptor_type(type) != layout.get_binding(binding_index).descriptorType) {
		Log::warning("in method DescriptorSet::bind_image() for binding index ", binding_index, ": descriptor type mismatch with layout");
	}

	binding_info[binding_index].binding_index = binding_index;
	binding_info[binding_index].image_view = image_view.get();
	binding_info[binding_index].image_layout = image_layout;
	binding_info[binding_index].sampler = sampler.get();
	binding_info[binding_index].descriptor_type = get_descriptor_type(type);
	binding_info[binding_index].updated = false;
}

// binds a sampler
void DescriptorSet::bind_sampler(uint32_t binding_index, const Sampler& sampler, VkShaderStageFlagBits shader_stage_flags) {
	uint32_t bindings_count = layout.get_bindings_count();
	if (binding_index >= bindings_count) {
		Log::error("in method DescriptorSet::bind_sampler(): argument for the binding index is invalid; value is ", binding_index, " but the descriptor set layout only has ", bindings_count, " bindings (indices 0-", bindings_count - 1, ").");
	}

	if (layout.get_binding(binding_index).descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER && layout.get_binding(binding_index).descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
		Log::warning("in method DescriptorSet::bind_sampler() for binding index ", binding_index, ": descriptor type mismatch with layout; layout type is ", layout.get_binding(binding_index).descriptorType, ", expected VK_DESCRIPTOR_TYPE_SAMPLER or VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER.");
	}

	binding_info[binding_index].binding_index = binding_index;
	binding_info[binding_index].sampler = sampler.get();
	binding_info[binding_index].descriptor_type = layout.get_binding(binding_index).descriptorType;
	binding_info[binding_index].updated = false;
}

// binds a storage image
void DescriptorSet::bind_storage_image(uint32_t binding_index, const ImageView& image_view, VkImageLayout image_layout, VkShaderStageFlagBits shader_stage_flags) {
	uint32_t bindings_count = layout.get_bindings_count();
	if (binding_index >= bindings_count) {
		Log::error("in method DescriptorSet::bind_storage_image(): argument for the binding index is invalid; value is ", binding_index, " but the descriptor set layout only has ", bindings_count, " bindings (indices 0-", bindings_count - 1, ").");
	}

	if (layout.get_binding(binding_index).descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
		Log::warning("in method DescriptorSet::bind_storage_image() for binding index ", binding_index, ": descriptor type mismatch with layout; layout type is ", layout.get_binding(binding_index).descriptorType, ", expected VK_DESCRIPTOR_TYPE_STORAGE_IMAGE.");
	}

	binding_info[binding_index].binding_index = binding_index;
	binding_info[binding_index].image_view = image_view.get();
	binding_info[binding_index].image_layout = image_layout;
	binding_info[binding_index].descriptor_type = layout.get_binding(binding_index).descriptorType;
	binding_info[binding_index].updated = false;
}

// binds a texel buffer view (uniform or storage)
template<typename T>
void DescriptorSet::bind_texel_buffer(uint32_t binding_index, const BufferView<T>& buffer_view, VkShaderStageFlagBits shader_stage_flags) {
	uint32_t bindings_count = layout.get_bindings_count();
	if (binding_index >= bindings_count) {
		Log::error("in method DescriptorSet::bind_texel_buffer(): argument for the binding index is invalid; value is ", binding_index, " but the descriptor set layout only has ", bindings_count, " bindings (indices 0-", bindings_count - 1, ").");
	}

	if (layout.get_binding(binding_index).descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER && layout.get_binding(binding_index).descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
		Log::warning("in method DescriptorSet::bind_texel_buffer() for binding index ", binding_index, ": descriptor type mismatch with layout; layout type is ", layout.get_binding(binding_index).descriptorType, ", expected a texel buffer type.");
	}

	binding_info[binding_index].binding_index = binding_index;
	binding_info[binding_index].buffer_view = buffer_view.get();
	binding_info[binding_index].descriptor_type = layout.get_binding(binding_index).descriptorType;
	binding_info[binding_index].updated = false;
}

// updates the descriptor set with the current bindings
void DescriptorSet::write() {

	uint32_t bindings_count = layout.get_bindings_count();

	std::vector<VkWriteDescriptorSet> descriptor_writes;
	std::vector<VkDescriptorImageInfo> image_infos_storage;
	std::vector<VkDescriptorBufferInfo> buffer_infos_storage;

	descriptor_writes.reserve(bindings_count);
	image_infos_storage.reserve(bindings_count);
	buffer_infos_storage.reserve(bindings_count);

	for (uint32_t i = 0; i < bindings_count; i++) {
		// only write any new or replaced bindings
		if (!binding_info[i].updated) {

			VkWriteDescriptorSet descriptor_write{};
			descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write.dstSet = set;
			descriptor_write.dstBinding = binding_info[i].binding_index;
			descriptor_write.dstArrayElement = 0;
			descriptor_write.descriptorCount = 1;
			descriptor_write.descriptorType = layout.get_binding(i).descriptorType;

			// process image bindings
			if (binding_info[i].image_view != VK_NULL_HANDLE) {
				VkDescriptorImageInfo image_info = {};
				image_info.sampler = binding_info[i].sampler;
				image_info.imageView = binding_info[i].image_view;
				image_info.imageLayout = binding_info[i].image_layout;
				image_infos_storage.push_back(image_info);

				descriptor_write.pNext = nullptr;
				descriptor_write.pImageInfo = &image_infos_storage.back();
				descriptor_write.pBufferInfo = nullptr;
				descriptor_write.pTexelBufferView = nullptr;
				descriptor_writes.push_back(descriptor_write);
			}

			// process buffer bindings
			else if (binding_info[i].buffer != VK_NULL_HANDLE) {
				VkDescriptorBufferInfo buffer_info = {};
				buffer_info.buffer = binding_info[i].buffer;
				buffer_info.offset = binding_info[i].offset;
				buffer_info.range = binding_info[i].range;
				buffer_infos_storage.push_back(buffer_info);

				descriptor_write.pNext = nullptr;
				descriptor_write.pBufferInfo = &buffer_infos_storage.back();
				descriptor_write.pImageInfo = nullptr;
				descriptor_writes.push_back(descriptor_write);
			}

			binding_info[i].updated = true;
		}
	}

	// Perform the update if there's anything to write
	if (!descriptor_writes.empty()) {
		vkUpdateDescriptorSets(logical, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		Log::debug("DescriptorSet::update() called vkUpdateDescriptorSets for set ", set, " with ", descriptor_writes.size(), " writes.");
	}
	else {
		Log::debug("DescriptorSet::update() called for set ", set, ", but no bindings needed updating.");
	}
}

// getters
VkDescriptorSet DescriptorSet::get() const { return set; } // return the vulkan handle of the descriptor set
DescriptorSetLayout& DescriptorSet::get_layout() const { return layout; }
const std::vector<DescriptorBindingInfo>& DescriptorSet::get_binding_info() const { return binding_info; }

// +=================================+   
// | GraphicsPipelineLayout          |
// +=================================+

// Parametric constructor for one descriptor set layout and one push constant range.
GraphicsPipelineLayout::GraphicsPipelineLayout(Device& device, const DescriptorSetLayout& set_layout, const PushConstants& constants)
	: logical(device.get_logical()) {

	// Setup push constant range
	VkPushConstantRange push_constant_range = {};
	push_constant_range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	push_constant_range.offset = 0;
	push_constant_range.size = static_cast<uint32_t>(constants.get_size());

	// Setup pipeline layout create info
	VkPipelineLayoutCreateInfo layout_create_info = {};
	layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_create_info.setLayoutCount = 1;
	layout_create_info.pSetLayouts = set_layout.get_ptr();
	layout_create_info.pushConstantRangeCount = 1;
	layout_create_info.pPushConstantRanges = &push_constant_range;

	VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &pipeline_layout);
	if (result == VK_SUCCESS) {
		Log::info("created graphics pipeline layout (handle: ", pipeline_layout, ")");
	}
	else {
		Log::error("failed to create graphics pipeline layout (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
}

// Parametric constructor for multiple descriptor set layouts and multiple push constant ranges.
GraphicsPipelineLayout::GraphicsPipelineLayout(Device& device, const std::vector<DescriptorSetLayout>& set_layouts, const std::vector<PushConstants>& constants)
	: logical(device.get_logical()) {

	// Collect push constant ranges from the vector of PushConstants objects
	std::vector<VkPushConstantRange> push_constant_ranges_vec;
	for (uint32_t i = 0; i < constants.size(); i++) {
		VkPushConstantRange range = {};
		range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		range.offset = 0;
		range.size = static_cast<uint32_t>(constants[i].get_size());
		push_constant_ranges_vec.push_back(range);
	}

	// Collect descriptor set layouts from the vector of DescriptorSetLayout objects
	std::vector<VkDescriptorSetLayout> set_layouts_vec;
	for (uint32_t i = 0; i < set_layouts.size(); i++) {
		set_layouts_vec.push_back(set_layouts[i].get());
	}

	// Setup pipeline layout create info
	VkPipelineLayoutCreateInfo layout_create_info = {};
	layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_create_info.setLayoutCount = static_cast<uint32_t>(set_layouts_vec.size());
	layout_create_info.pSetLayouts = set_layouts_vec.data();
	layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges_vec.size());
	layout_create_info.pPushConstantRanges = push_constant_ranges_vec.data();

	VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &pipeline_layout);
	if (result == VK_SUCCESS) {
		Log::info("created graphics pipeline layout (handle: ", pipeline_layout, ")");
	}
	else {
		Log::error("failed to create graphics pipeline layout (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
}

// Parametric constructor for one descriptor set layout and multiple push constant ranges.
GraphicsPipelineLayout::GraphicsPipelineLayout(Device& device, const DescriptorSetLayout& set_layout, const std::vector<PushConstants>& constants)
	: logical(device.get_logical()) {

	// Collect push constant ranges from the vector of PushConstants objects
	std::vector<VkPushConstantRange> push_constant_ranges_vec;
	for (uint32_t i = 0; i < constants.size(); i++) {
		VkPushConstantRange range = {};
		range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		range.offset = 0;
		range.size = static_cast<uint32_t>(constants[i].get_size());
		push_constant_ranges_vec.push_back(range);
	}

	// Setup pipeline layout create info
	VkPipelineLayoutCreateInfo layout_create_info = {};
	layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_create_info.setLayoutCount = 1;
	layout_create_info.pSetLayouts = set_layout.get_ptr();
	layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges_vec.size());
	layout_create_info.pPushConstantRanges = push_constant_ranges_vec.data();

	VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &pipeline_layout);
	if (result == VK_SUCCESS) {
		Log::info("created graphics pipeline layout (handle: ", pipeline_layout, ")");
	}
	else {
		Log::error("failed to create graphics pipeline layout (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
}

// Parametric constructor for multiple descriptor set layouts and one push constant range.
GraphicsPipelineLayout::GraphicsPipelineLayout(Device& device, const std::vector<DescriptorSetLayout>& set_layouts, const PushConstants& constants)
	: logical(device.get_logical()) {

	// Setup push constant range
	VkPushConstantRange push_constant_range = {};
	push_constant_range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	push_constant_range.offset = 0;
	push_constant_range.size = static_cast<uint32_t>(constants.get_size());

	// Collect descriptor set layouts from the vector of DescriptorSetLayout objects
	std::vector<VkDescriptorSetLayout> set_layouts_vec;
	for (uint32_t i = 0; i < set_layouts.size(); i++) {
		set_layouts_vec.push_back(set_layouts[i].get());
	}

	// Setup pipeline layout create info
	VkPipelineLayoutCreateInfo layout_create_info = {};
	layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_create_info.setLayoutCount = static_cast<uint32_t>(set_layouts_vec.size());
	layout_create_info.pSetLayouts = set_layouts_vec.data();
	layout_create_info.pushConstantRangeCount = 1;
	layout_create_info.pPushConstantRanges = &push_constant_range;

	VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &pipeline_layout);
	if (result == VK_SUCCESS) {
		Log::info("created graphics pipeline layout (handle: ", pipeline_layout, ")");
	}
	else {
		Log::error("failed to create graphics pipeline layout (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
}

// Destructor
GraphicsPipelineLayout::~GraphicsPipelineLayout() {
	if (pipeline_layout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(logical, pipeline_layout, nullptr);
	}
}

// Move constructor
GraphicsPipelineLayout::GraphicsPipelineLayout(GraphicsPipelineLayout&& other) noexcept
	: pipeline_layout(std::exchange(other.pipeline_layout, VK_NULL_HANDLE)),
	logical(std::exchange(other.logical, VK_NULL_HANDLE)) {
}

// Move assignment operator
GraphicsPipelineLayout& GraphicsPipelineLayout::operator=(GraphicsPipelineLayout&& other) noexcept {
	if (this != &other) {
		if (pipeline_layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(logical, pipeline_layout, nullptr);
		}
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		pipeline_layout = std::exchange(other.pipeline_layout, VK_NULL_HANDLE);
	}
	return *this;
}

// Getter
VkPipelineLayout GraphicsPipelineLayout::get() const {
	return pipeline_layout;
}

// +=================================+   
// | GraphicsPipeline                |
// +=================================+

// parametric constructor
GraphicsPipeline::GraphicsPipeline(
	Device& device,
	const RenderPass& renderpass,
	uint32_t subpass_index,
	const GraphicsPipelineLayout& pipeline_layout,
	const ShaderModule& vertex_shader,
	const std::vector<VertexDescriptions>& vertex_descriptions,
	const std::optional<ColorBlendState>& color_blend_state,
	const std::optional<ShaderModule>& fragment_shader,
	const std::optional<ShaderModule>& hull_shader,
	const std::optional<ShaderModule>& domain_shader,
	uint32_t tessellation_patch_control_points,
	VkPipelineDepthStencilStateCreateFlagBits depth_stencil_flags
) :
	logical(device.get_logical()),
	pipeline_layout(pipeline_layout.get()) {

	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	// A vector to hold all active shader stages
	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_info;

	// Setup vertex shader stage
	if (vertex_shader.get() != nullptr) {
		VkPipelineShaderStageCreateInfo vertex_shader_info{};
		vertex_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_info.module = vertex_shader.get();
		vertex_shader_info.pName = "main";
		shader_stage_create_info.push_back(vertex_shader_info);
	}

	// Setup tessellation stages
	VkPipelineTessellationStateCreateInfo tessellation_state_create_info = {};
	if (hull_shader.has_value() && domain_shader.has_value() && hull_shader.value().get() != nullptr && domain_shader.value().get() != nullptr) {
		VkPipelineShaderStageCreateInfo hull_shader_info{};
		hull_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		hull_shader_info.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		hull_shader_info.module = hull_shader.value().get();
		hull_shader_info.pName = "main";
		shader_stage_create_info.push_back(hull_shader_info);

		VkPipelineShaderStageCreateInfo domain_shader_info{};
		domain_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		domain_shader_info.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		domain_shader_info.module = domain_shader.value().get();
		domain_shader_info.pName = "main";
		shader_stage_create_info.push_back(domain_shader_info);

		tessellation_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessellation_state_create_info.pNext = nullptr;
		tessellation_state_create_info.flags = 0; // reserved for future use
		tessellation_state_create_info.patchControlPoints = tessellation_patch_control_points;
	}

	// Setup fragment shader stage
	if (fragment_shader.has_value() && fragment_shader.value().get() != nullptr) {
		VkPipelineShaderStageCreateInfo fragment_shader_info{};
		fragment_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragment_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragment_shader_info.module = fragment_shader.value().get();
		fragment_shader_info.pName = "main";
		shader_stage_create_info.push_back(fragment_shader_info);
	}

	// Setup vertex input state
	std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions;
	std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions;
	for (uint32_t i = 0; i < vertex_descriptions.size(); i++) {
		for (uint32_t j = 0; j < vertex_descriptions[i].get_input_bindings().size(); j++) {
			vertex_binding_descriptions.push_back(vertex_descriptions[i].get_input_bindings()[j]);
		}
		for (uint32_t j = 0; j < vertex_descriptions[i].get_attribute_descriptions().size(); j++) {
			vertex_attribute_descriptions.push_back(vertex_descriptions[i].get_attribute_descriptions()[j]);
		}
	}
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
	vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_binding_descriptions.size());
	vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
	vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions.size());
	vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();


	// setup input assembly state
	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {};
	input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


	// setup dynamic state(s)
	std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
	dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.pNext = nullptr;
	dynamic_state_create_info.flags = 0;
	dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_state_create_info.pDynamicStates = dynamic_states.data();

	// setup viewport state
	VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.pNext = nullptr;
	viewport_state_create_info.flags = 0;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.scissorCount = 1;

	// setup rasterization state
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
	rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.pNext = nullptr;
	rasterization_state_create_info.flags = 0; // reserved for future use
	rasterization_state_create_info.depthClampEnable = VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE; // Changed to FALSE
	rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
	rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state_create_info.depthBiasEnable = VK_FALSE;
	rasterization_state_create_info.lineWidth = 1.0f;

	// setup multi-sample state
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
	multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.rasterizationSamples = static_cast<VkSampleCountFlagBits>(renderpass.get_multisample_count());

	// setup color blend state
	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
	color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	if (color_blend_state.has_value()) {
		color_blend_state_create_info.attachmentCount = 1;
		color_blend_state_create_info.pAttachments = &color_blend_state.value().get_attachment();
		color_blend_state_create_info.blendConstants[0] = color_blend_state.value().get_blend_constants()[0];
		color_blend_state_create_info.blendConstants[1] = color_blend_state.value().get_blend_constants()[1];
		color_blend_state_create_info.blendConstants[2] = color_blend_state.value().get_blend_constants()[2];
		color_blend_state_create_info.blendConstants[3] = color_blend_state.value().get_blend_constants()[3];
		color_blend_state_create_info.flags = color_blend_state.value().get_flags();
		color_blend_state_create_info.logicOp = color_blend_state.value().get_logic_op();
		color_blend_state_create_info.logicOpEnable = color_blend_state.value().get_logic_op() == VK_LOGIC_OP_CLEAR ? VK_FALSE : VK_TRUE;
		color_blend_state_create_info.pNext = nullptr;
	}
	else {
		color_blend_state_create_info.attachmentCount = 0;
		color_blend_state_create_info.pAttachments = nullptr;
		color_blend_state_create_info.blendConstants[0] = 1.0f;
		color_blend_state_create_info.blendConstants[1] = 1.0f;
		color_blend_state_create_info.blendConstants[2] = 1.0f;
		color_blend_state_create_info.blendConstants[3] = 1.0f;
		color_blend_state_create_info.flags = 0;
		color_blend_state_create_info.logicOp = VK_LOGIC_OP_CLEAR;
		color_blend_state_create_info.logicOpEnable = VK_FALSE;
		color_blend_state_create_info.pNext = nullptr;
	}

	// setup depth-stencil state
	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {};
	if (renderpass.has_depth_stencil()) {
		depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_create_info.pNext = nullptr;
		depth_stencil_state_create_info.flags = 0; // Use a simpler approach without EXT flags
		depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
		depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
		depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
		pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
	}
	else {
		pipeline_create_info.pDepthStencilState = nullptr;
	}

	// Finalize graphics pipeline create info
	pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stage_create_info.size());
	pipeline_create_info.pStages = shader_stage_create_info.data();
	pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
	pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
	pipeline_create_info.pTessellationState = (hull_shader.has_value() && domain_shader.has_value()) ? &tessellation_state_create_info : nullptr;
	pipeline_create_info.pViewportState = &viewport_state_create_info;
	pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
	pipeline_create_info.pMultisampleState = &multisample_state_create_info;
	pipeline_create_info.pDepthStencilState = renderpass.has_depth_stencil() ? &depth_stencil_state_create_info : nullptr;
	pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
	pipeline_create_info.pDynamicState = &dynamic_state_create_info;
	pipeline_create_info.layout = pipeline_layout.get();
	pipeline_create_info.renderPass = renderpass.get();
	pipeline_create_info.subpass = subpass_index;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex = -1;

	VkResult result = vkCreateGraphicsPipelines(logical, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline);
	if (result == VK_SUCCESS) {
		Log::info("graphics pipeline successfully created");
	}
	else {
		Log::error("failed to create graphics pipeline (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
}

// destructor
GraphicsPipeline::~GraphicsPipeline() {
	if (pipeline != VK_NULL_HANDLE) {
		Log::info("destroying graphics pipeline");
		vkDestroyPipeline(logical, pipeline, nullptr);
	}
}

// move constructor
GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept
	: pipeline(std::exchange(other.pipeline, VK_NULL_HANDLE)),
	logical(std::exchange(other.logical, VK_NULL_HANDLE)) {
	pipeline_layout = other.pipeline_layout;
}

// move assignment
GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept {
	if (this != &other) {
		if (pipeline != VK_NULL_HANDLE) {
			Log::info("move assignment operation: destroying previous graphics pipeline (handle: ", pipeline, ")");
			vkDestroyPipeline(logical, pipeline, nullptr);
			pipeline = VK_NULL_HANDLE;
		}
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		pipeline = std::exchange(other.pipeline, VK_NULL_HANDLE);
		pipeline_layout = std::exchange(other.pipeline_layout, VK_NULL_HANDLE);
	}
	return *this;
}

// getters
VkPipeline GraphicsPipeline::get() const { return pipeline; }
const VkPipelineLayout& GraphicsPipeline::get_layout() const { return pipeline_layout; }

// +=================================+   
// | ComputePipeline                 |
// +=================================+

// parametric constructor
ComputePipeline::ComputePipeline(
	const Device& device,
	const ShaderModule& compute_shader_module,
	uint32_t push_constants_range_size,
	DescriptorSetLayout& descriptor_set_layout,
	uint32_t workgroup_size_x,
	uint32_t workgroup_size_y,
	uint32_t workgroup_size_z,
	std::vector<uint32_t> addon_specialization_constants
) :
	descriptor_set_layout(&descriptor_set_layout),
	logical(device.get_logical()),
	workgroup_size_x(workgroup_size_x),
	workgroup_size_y(workgroup_size_y),
	workgroup_size_z(workgroup_size_z) {


	// setup specialization constants (workgroup sizes + optional constants)
	// indexing for GLSL shader: local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2;
	// indexing for optional addon constants starts from constant_id = 3
	specialization_data = { workgroup_size_x, workgroup_size_y, workgroup_size_z };
	specialization_data.insert(specialization_data.end(), addon_specialization_constants.begin(), addon_specialization_constants.end());

	size_t constants_count = specialization_data.size();
	uint32_t current_offset = 0;

	for (uint32_t i = 0; i < constants_count; i++) {
		VkSpecializationMapEntry next_entry = {};
		next_entry.constantID = i;
		next_entry.offset = current_offset;
		next_entry.size = sizeof(uint32_t);
		specialization_map_entries.push_back(next_entry);
		current_offset += sizeof(uint32_t);
	}

	VkSpecializationInfo specialization_info = {};
	specialization_info.mapEntryCount = constants_count;
	specialization_info.pMapEntries = specialization_map_entries.data();
	specialization_info.dataSize = constants_count * sizeof(uint32_t);
	specialization_info.pData = specialization_data.data();

	// setup push constants range
	VkPushConstantRange push_constant_range = {};
	push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = push_constants_range_size;

	// setup pipeline layout        
	VkPipelineLayoutCreateInfo layout_create_info = {};
	layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_create_info.setLayoutCount = 1; // = number of descriptor set layouts
	layout_create_info.pSetLayouts = this->descriptor_set_layout->get_ptr();
	layout_create_info.pushConstantRangeCount = 1;
	layout_create_info.pPushConstantRanges = &push_constant_range;

	layout_create_info.pNext = nullptr;
	VkResult result = vkCreatePipelineLayout(logical, &layout_create_info, nullptr, &layout);
	if (result == VK_SUCCESS) {
		Log::info("created pipeline layout for compute pipeline (handle: ", layout, ")");
	}
	else {
		Log::error("failed to create compute pipeline layout (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}

	// setup shader stage
	VkPipelineShaderStageCreateInfo shader_stage_create_info = {};
	shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_info.pNext = nullptr;
	shader_stage_create_info.flags = 0;
	shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader_stage_create_info.module = compute_shader_module.get();
	shader_stage_create_info.pName = "main";
	shader_stage_create_info.pSpecializationInfo = &specialization_info;

	// finalize compute pipeline
	VkComputePipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_create_info.pNext = nullptr;
	pipeline_create_info.flags = 0;
	pipeline_create_info.stage = shader_stage_create_info;
	pipeline_create_info.layout = layout;
	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	result = vkCreateComputePipelines(device.get_logical(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline);
	if (result == VK_SUCCESS) {
		Log::info("compute pipeline successfully created (handle: ", pipeline, ")");
	}
	else {
		Log::error("failed to create compute pipeline (VkResult=", result, ", ", vkresult_to_string(result), ")");
	}
}

// destructor
ComputePipeline::~ComputePipeline() {
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

// move constructor
ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept
	: pipeline(std::exchange(other.pipeline, VK_NULL_HANDLE)),
	layout(std::exchange(other.layout, VK_NULL_HANDLE)),
	logical(std::exchange(other.logical, VK_NULL_HANDLE)),
	descriptor_set_layout(std::move(other.descriptor_set_layout)),
	workgroup_size_x(other.workgroup_size_x),
	workgroup_size_y(other.workgroup_size_y),
	workgroup_size_z(other.workgroup_size_z),
	specialization_data(std::move(other.specialization_data)),
	specialization_map_entries(std::move(other.specialization_map_entries)) {
}

// move assignment
ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) noexcept {
	if (this != &other) {
		if (pipeline != VK_NULL_HANDLE) {
			Log::info("move assignment operation: destroying previous compute pipeline (handle: ", pipeline, ")");
			vkDestroyPipeline(logical, pipeline, nullptr);
			pipeline = VK_NULL_HANDLE;
		}
		if (layout != VK_NULL_HANDLE) {
			Log::info("move assignment operation: destroying previous pipeline layout (handle: ", layout, ")");
			vkDestroyPipelineLayout(logical, layout, nullptr);
			layout = VK_NULL_HANDLE;
		}
		logical = std::exchange(other.logical, VK_NULL_HANDLE);
		pipeline = std::exchange(other.pipeline, VK_NULL_HANDLE);
		layout = std::exchange(other.layout, VK_NULL_HANDLE);
		descriptor_set_layout = std::move(other.descriptor_set_layout); other.descriptor_set_layout = nullptr;
		workgroup_size_x = other.workgroup_size_x;
		workgroup_size_y = other.workgroup_size_y;
		workgroup_size_z = other.workgroup_size_z;
		specialization_data = std::move(other.specialization_data);
		specialization_map_entries = std::move(other.specialization_map_entries);
	}
	return *this;
}

// getters
VkPipeline ComputePipeline::get() const { return pipeline; }
VkPipelineLayout ComputePipeline::get_layout() const { return layout; }
const DescriptorSetLayout& ComputePipeline::get_descriptor_set_layout() const { return *descriptor_set_layout; }
uint32_t ComputePipeline::get_workgroup_size_x() const { return workgroup_size_x; }
uint32_t ComputePipeline::get_workgroup_size_y() const { return workgroup_size_y; }
uint32_t ComputePipeline::get_workgroup_size_z() const { return workgroup_size_z; }


// +=================================+   
// | CommandBuffer                   |
// +=================================+

// command buffer for recording commands;
// used for graphics, compute and transfer operations

// parametric constructor
CommandBuffer::CommandBuffer(Device& device, const CommandPool& pool) {
	this->device = &device;
	this->logical = device.get_logical();

	this->usage = pool.get_usage();
	switch (usage) {
	case QueueFamily::COMPUTE_QUEUE: queue = this->device->get_compute_queue(); break;
	case QueueFamily::GRAPHICS_QUEUE: queue = this->device->get_graphics_queue(); break;
	case QueueFamily::TRANSFER_QUEUE: queue = this->device->get_transfer_queue(); break;
	default:
		Log::error("Constructor for CommandBuffer has invalid QueueFamily argument.");
	}

	this->pool = pool.get();

	// destroy any previous command buffer
	if (buffer != nullptr) {
		vkFreeCommandBuffers(logical, this->pool, 1, &buffer);
		buffer = nullptr;
		Log::info("[OLD COMMAND BUFFER DESTROYED]");
	}

	// setup command buffer
	VkCommandBufferAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandPool = pool.get();
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandBufferCount = 1;
	VkResult result = vkAllocateCommandBuffers(logical, &allocate_info, &buffer);
	if (result == VK_SUCCESS) {
		Log::info("successfully allocated command buffer (handle: ", buffer, ")");
	}
	else {
		Log::warning("in CommandBuffer constructor: memory allocation failed (VkResult=", result, ", ", vkresult_to_string(result), ")!");
	}
}

// move constructor
CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept :
	buffer(std::exchange(other.buffer, nullptr)),
	logical(std::exchange(other.logical, nullptr)),
	device(std::exchange(other.device, nullptr)),
	queue(std::exchange(other.queue, nullptr)),
	pool(std::exchange(other.pool, nullptr)),
	usage(other.usage),
	device_memory_barriers(std::exchange(other.device_memory_barriers, {})),
	buffer_memory_barriers(std::exchange(other.buffer_memory_barriers, {})),
	image_memory_barriers(std::exchange(other.image_memory_barriers, {})) {
	Log::debug("command buffer moved from other(", &other, ") to this (", this, ")");
}

// move assignment
CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
	if (this != &other) {
		logical = std::exchange(other.logical, nullptr);
		buffer = std::exchange(other.buffer, nullptr);
		device = std::exchange(other.device, nullptr);
		queue = std::exchange(other.queue, nullptr);
		pool = std::exchange(other.pool, nullptr);
		usage = other.usage;
		device_memory_barriers = std::exchange(other.device_memory_barriers, {});
		buffer_memory_barriers = std::exchange(other.buffer_memory_barriers, {});

	}
	Log::debug("command buffer moved from other(", &other, ") to this (", this, ")");
	return *this;
}

// destructor
CommandBuffer::~CommandBuffer() {
	if (buffer != nullptr) {
		vkFreeCommandBuffers(logical, pool, 1, &buffer);
		Log::info("[COMMAND BUFFER DESTROYED]");
		buffer = nullptr;
	}
}

// set event on command buffer
Event CommandBuffer::set_event(
	const std::optional<std::vector<VkMemoryBarrier2>>& device_memory_barriers,
	const std::optional<std::vector<VkBufferMemoryBarrier2>>& buffer_memory_barriers,
	const std::optional<std::vector<VkImageMemoryBarrier2>>& image_memory_barriers,
	VkDependencyFlags flags) const {
	Event event = Event(*device);
	event.get_dependency_info_ptr()->sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	event.get_dependency_info_ptr()->pNext = NULL;
	event.get_dependency_info_ptr()->dependencyFlags = flags;
	event.get_dependency_info_ptr()->memoryBarrierCount = device_memory_barriers.has_value() ? static_cast<uint32_t>(device_memory_barriers.value().size()) : 0;
	event.get_dependency_info_ptr()->pMemoryBarriers = device_memory_barriers.has_value() ? device_memory_barriers.value().data() : nullptr;
	event.get_dependency_info_ptr()->bufferMemoryBarrierCount = buffer_memory_barriers.has_value() ? static_cast<uint32_t>(buffer_memory_barriers.value().size()) : 0;
	event.get_dependency_info_ptr()->pBufferMemoryBarriers = buffer_memory_barriers.has_value() ? buffer_memory_barriers.value().data() : nullptr;
	event.get_dependency_info_ptr()->imageMemoryBarrierCount = image_memory_barriers.has_value() ? static_cast<uint32_t>(image_memory_barriers.value().size()) : 0;
	event.get_dependency_info_ptr()->pImageMemoryBarriers = image_memory_barriers.has_value() ? image_memory_barriers.value().data() : nullptr;
	vkCmdSetEvent2(this->buffer, event.get(), &event.get_dependency_info());
	Log::debug("command buffer: recording event ", event.get());
	return event;
}

void CommandBuffer::reset_event(const Event& event, VkPipelineStageFlags stage_mask) const {
	Log::debug("command buffer", buffer, ": recording reset for event ", event.get());
	vkCmdResetEvent(buffer, event.get(), stage_mask);
}

void CommandBuffer::wait_event(const Event& event) const {
	Log::debug("command buffer", buffer, ": recording waiting for event ", event.get());
	vkCmdWaitEvents2(buffer, 1, &event.get(), &event.get_dependency_info());
}

void CommandBuffer::bind_pipeline(ComputePipeline& pipeline) {
	if (usage != QueueFamily::COMPUTE_QUEUE) {
		Log::error("invalid usage of CommandBuffer::bind_pipeline() on command buffer ", buffer, ": this command buffer doesn't support compute (queue family mismatch)");
	}
	if (pipeline.get() != nullptr) {
		Log::debug("command buffer ", buffer, ": recording pipeline binding to compute bindpoint type");
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.get());
	}
	else {
		Log::error("CommandBuffer::bind_pipeline() on command buffer ", buffer, "has invalid pipeline argument");
	}
}

void CommandBuffer::bind_pipeline(GraphicsPipeline& pipeline) {
	if (usage != QueueFamily::GRAPHICS_QUEUE) {
		Log::error("invalid usage of CommandBuffer::bind_pipeline(): this command buffer doesn't support graphics (queue family mismatch)");
	}
	if (pipeline.get() != nullptr) {
		Log::debug("command buffer ", buffer, ": recording pipeline binding for graphics pipeline ", pipeline.get());
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get());
	}
	else {
		Log::error("CommandBuffer::bind_pipeline() on command buffer ", buffer, " has invalid pipeline argument");
	}
}

// bind pipeline push constants to command buffer
void CommandBuffer::bind_push_constants(PushConstants& constants, ComputePipeline& pipeline) const {
	Log::debug("recording push constants binding on command buffer ", buffer);
	vkCmdPushConstants(
		buffer,
		pipeline.get_layout(),
		VK_SHADER_STAGE_COMPUTE_BIT,
		0,
		constants.get_size(),
		constants.get_data()
	);
}

// bind pipeline push constants to command buffer
void CommandBuffer::bind_push_constants(PushConstants& constants, GraphicsPipeline& pipeline) const {
	Log::debug("recording push constants binding on command buffer ", buffer);
	vkCmdPushConstants(
		buffer,
		pipeline.get_layout(),
		VK_SHADER_STAGE_ALL_GRAPHICS,
		0,
		constants.get_size(),
		constants.get_data()
	);
}

// bind descriptor set to command buffer
void CommandBuffer::bind_descriptor_set(DescriptorSet& set, ComputePipeline& pipeline) const {
	Log::debug("recording descriptor set (set: ", set.get(), ", compute pipeline: ", pipeline.get(), ") on command buffer ", buffer);
	VkDescriptorSet set_handle = set.get();
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.get_layout(), 0, 1, &set_handle, 0, nullptr);
}

// bind descriptor set to command buffer
void CommandBuffer::bind_descriptor_set(DescriptorSet& set, GraphicsPipeline& pipeline) {
	Log::debug("recording descriptor set (set: ", set.get(), ", graphics pipeline: ", pipeline.get(), ") on command buffer ", buffer);
	VkDescriptorSet set_handle = set.get();
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_layout(), 0, 1, &set_handle, 0, nullptr);
}

// bind a Mesh model to the command buffer
void CommandBuffer::bind_mesh(Mesh& mesh) {
	VkBuffer vertex_buffers[] = { mesh.get_vertex_buffer().get() }; // binding a single vertex buffer
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(buffer, 0, 1, vertex_buffers, offsets);

	// Bind the index buffer to the command buffer.
	// The last parameter is the index type (UINT16 or UINT32).
	// This function assumes the index type is stored within the Mesh object.
	vkCmdBindIndexBuffer(buffer, mesh.get_index_buffer().get(), 0, mesh.get_index_type());

	Log::info("Successfully bound mesh to command buffer");
}

// copy data between two buffers;
// this method has automatic boundary checks and shrinks the copy region to fit if the size_bytes argument is too large
template<typename T>
void CommandBuffer::copy_buffer(const Buffer<T>& src_buffer, Buffer<T>& dst_buffer, uint64_t size_bytes, uint64_t src_offset_bytes, uint64_t dst_offset_bytes) {
	// fit to boundaries
	VkBufferCopy copy_region = {};
	copy_region.srcOffset = src_offset_bytes;
	copy_region.dstOffset = dst_offset_bytes;
	copy_region.size = size_bytes;
	uint64_t source_size = src_buffer.get_size_bytes();
	copy_region.size = std::min(copy_region.size, source_size - src_offset_bytes);
	uint64_t target_size = dst_buffer.get_size_bytes();
	copy_region.size = std::min(copy_region.size, target_size - dst_offset_bytes);
	if (copy_region.size == 0) return; // nothing to copy
	Log::debug("command buffer ", buffer, ": recording buffer copy (", copy_region.size, " bytes) from buffer ", src_buffer.get(), " (source offset: ", src_offset_bytes, " bytes) to ", dst_buffer.get(), " (target offset: ", dst_offset_bytes, " bytes)");
	vkCmdCopyBuffer(buffer, src_buffer.get(), dst_buffer.get(), 1, &copy_region);
}

void CommandBuffer::add_device_memory_barrier(
	VkPipelineStageFlags2 source_stage_flags,
	VkAccessFlags2 source_access_flags,
	VkPipelineStageFlags2 target_stage_flags,
	VkAccessFlags2 target_access_flags) {

	VkMemoryBarrier2 device_barrier{};
	device_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
	device_barrier.pNext = nullptr;
	device_barrier.srcStageMask = source_stage_flags;
	device_barrier.srcAccessMask = source_access_flags;
	device_barrier.dstStageMask = target_stage_flags;
	device_barrier.dstAccessMask = target_access_flags;

	Log::debug("command buffer ", this->buffer, ": adding device memory barrier");
	device_memory_barriers.push_back(device_barrier);
}

template<typename T>
void CommandBuffer::add_buffer_memory_barrier(
	Buffer<T>& buffer,
	VkAccessFlags2 src_access_flags,
	VkAccessFlags2 dst_access_flags,
	VkPipelineStageFlags2 src_stage_flags,
	VkPipelineStageFlags2 dst_stage_flags,
	uint32_t src_queue_family_index,
	uint32_t dst_queue_family_index) {

	VkBufferMemoryBarrier2 buffer_barrier{};
	buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
	buffer_barrier.pNext = nullptr;
	buffer_barrier.srcStageMask = src_stage_flags;
	buffer_barrier.srcAccessMask = src_access_flags;
	buffer_barrier.dstStageMask = dst_stage_flags;
	buffer_barrier.dstAccessMask = dst_access_flags;
	buffer_barrier.srcQueueFamilyIndex = src_queue_family_index;
	buffer_barrier.dstQueueFamilyIndex = dst_queue_family_index;
	buffer_barrier.buffer = buffer.get();
	buffer_barrier.offset = 0;
	buffer_barrier.size = VK_WHOLE_SIZE;

	Log::debug("command buffer ", this->buffer, ": adding buffer memory barrier for buffer ", buffer.get());
	buffer_memory_barriers.push_back(buffer_barrier);
}

void CommandBuffer::add_image_memory_barrier(
	VkImage image,
	VkImageSubresourceRange subresource_range,
	VkPipelineStageFlags2 source_stage_flags,
	VkAccessFlags2 source_access_flags,
	VkPipelineStageFlags2 target_stage_flags,
	VkAccessFlags2 target_access_flags,
	VkImageLayout old_layout,
	VkImageLayout new_layout,
	uint32_t source_queue_family_index,
	uint32_t target_queue_family_index) {

	VkImageMemoryBarrier2 image_barrier{};
	image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	image_barrier.pNext = nullptr;
	image_barrier.srcStageMask = source_stage_flags;
	image_barrier.srcAccessMask = source_access_flags;
	image_barrier.dstStageMask = target_stage_flags;
	image_barrier.dstAccessMask = target_access_flags;
	image_barrier.oldLayout = old_layout;
	image_barrier.newLayout = new_layout;
	image_barrier.srcQueueFamilyIndex = source_queue_family_index;
	image_barrier.dstQueueFamilyIndex = target_queue_family_index;
	image_barrier.image = image;
	image_barrier.subresourceRange = subresource_range;

	Log::debug("command buffer ", this->buffer, ": adding image memory barrier");
	image_memory_barriers.push_back(image_barrier);
}

void CommandBuffer::record_barriers() {
	VkDependencyInfo dependency_info = {};
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.pNext = nullptr;
	dependency_info.memoryBarrierCount = device_memory_barriers.size();
	dependency_info.pMemoryBarriers = device_memory_barriers.data();
	dependency_info.bufferMemoryBarrierCount = buffer_memory_barriers.size();
	dependency_info.pBufferMemoryBarriers = buffer_memory_barriers.data();
	dependency_info.imageMemoryBarrierCount = image_memory_barriers.size();
	dependency_info.pImageMemoryBarriers = image_memory_barriers.data();

	Log::debug("command buffer ", buffer, ": recording barriers");
	vkCmdPipelineBarrier2(buffer, &dependency_info);

	device_memory_barriers.clear();
	buffer_memory_barriers.clear();
	image_memory_barriers.clear();
}

// Transition an image's layout from one state to another.
void CommandBuffer::transition_image_layout(
	Image& image,
	VkImageLayout new_layout,
	VkImageAspectFlags aspect_mask,
	VkPipelineStageFlags2 src_stage,
	VkAccessFlags2 src_access,
	VkPipelineStageFlags2 dst_stage,
	VkAccessFlags2 dst_access) {

	// Set up the image memory barrier
	VkImageMemoryBarrier2 image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	image_memory_barrier.oldLayout = image.get_layout();
	image_memory_barrier.newLayout = new_layout;
	image_memory_barrier.srcQueueFamilyIndex = device->get_graphics_queue_family_index();
	image_memory_barrier.dstQueueFamilyIndex = device->get_graphics_queue_family_index();

	// The image to transition
	image_memory_barrier.image = image.get();

	// Define the subresource range for the transition
	image_memory_barrier.subresourceRange.aspectMask = aspect_mask;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = 1;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;

	// Specify the stages and access types for synchronization
	image_memory_barrier.srcAccessMask = src_access;
	image_memory_barrier.dstAccessMask = dst_access;
	image_memory_barrier.srcStageMask = src_stage;
	image_memory_barrier.dstStageMask = dst_stage;

	// Set up the dependency info and execute the barrier
	VkDependencyInfo dependency_info = {};
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.memoryBarrierCount = 0;
	dependency_info.pMemoryBarriers = nullptr;
	dependency_info.bufferMemoryBarrierCount = 0;
	dependency_info.pBufferMemoryBarriers = nullptr;
	dependency_info.imageMemoryBarrierCount = 1;
	dependency_info.pImageMemoryBarriers = &image_memory_barrier;

	// Record the pipeline barrier command
	vkCmdPipelineBarrier2(buffer, &dependency_info);

	// Update the image's layout state to the new layout
	image.set_layout(new_layout);
}

void CommandBuffer::transition_image_layout(
	Image& image,
	VkImageLayout new_layout,
	VkImageAspectFlags aspect_mask) {

	VkImageLayout old_layout = image.get_layout();

	VkPipelineStageFlags2 src_stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
	VkAccessFlags2 src_access = VK_ACCESS_2_NONE;
	VkPipelineStageFlags2 dst_stage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
	VkAccessFlags2 dst_access = VK_ACCESS_2_NONE;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		src_stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		dst_access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		src_stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		src_access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		dst_stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		dst_access = VK_ACCESS_2_SHADER_READ_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		src_stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		dst_access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		src_stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		src_access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		dst_stage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
		dst_access = VK_ACCESS_2_NONE;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		src_stage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
		dst_stage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
		dst_access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else {
		// Fallback for unhandled transitions
		Log::warning("CommandBuffer::transition_image_layout: Unhandled layout transition, using conservative masks/stages.");
		src_stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		src_access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
		dst_stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		dst_access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
	}

	// Call the explicit transition function with the determined values
	transition_image_layout(image, new_layout, aspect_mask, src_stage, src_access, dst_stage, dst_access);
}

void CommandBuffer::draw(uint32_t index_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const {
	vkCmdDraw(buffer, index_count, instance_count, first_vertex, first_instance);
}


void CommandBuffer::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) const {
	vkCmdDrawIndexed(buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

// dispatch for compute
void CommandBuffer::dispatch(const ComputePipeline& pipeline, uint32_t global_size_x, uint32_t global_size_y, uint32_t global_size_z) const {
	uint32_t workgroups_x = (global_size_x + pipeline.get_workgroup_size_x() - 1) / pipeline.get_workgroup_size_x();
	uint32_t workgroups_y = (global_size_y + pipeline.get_workgroup_size_y() - 1) / pipeline.get_workgroup_size_y();
	uint32_t workgroups_z = (global_size_z + pipeline.get_workgroup_size_z() - 1) / pipeline.get_workgroup_size_z();
	Log::debug("command buffer ", buffer, ": recording dispatch for compute; ",
		"workgroups_x=", workgroups_x, "(*", pipeline.get_workgroup_size_x(), " elements), ",
		"workgroups_y=", workgroups_y, "(*", pipeline.get_workgroup_size_y(), " elements), ",
		"workgroups_z=", workgroups_z, "(*", pipeline.get_workgroup_size_z(), " elements)");
	vkCmdDispatch(buffer, workgroups_x, workgroups_y, workgroups_z);
}

void CommandBuffer::begin_render(VkOffset2D offset, VkExtent2D extent, VkRenderingFlags flags, std::vector<VkRenderingAttachmentInfo>& color_attachments, VkRenderingAttachmentInfo& depth_attachment, VkRenderingAttachmentInfo& stencil_attachment) const {
	VkRenderingInfo rendering_info = {};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.pNext = NULL;
	rendering_info.flags = flags;
	rendering_info.renderArea = { offset, extent }; // VkRect2D
	rendering_info.layerCount = 1;
	rendering_info.viewMask = 0; // =multiview disabled by default
	rendering_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
	rendering_info.pColorAttachments = color_attachments.data();
	rendering_info.pDepthAttachment = &depth_attachment;
	rendering_info.pStencilAttachment = &stencil_attachment;
	vkCmdBeginRendering(buffer, &rendering_info);
}

void CommandBuffer::begin_renderpass(RenderPass& renderpass, VkFramebuffer framebuffer, VkOffset2D offset, VkExtent2D extent, std::vector<VkClearValue>& clear_value) const {
	VkRenderPassBeginInfo renderpass_begin_info = {};
	renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpass_begin_info.pNext = NULL;
	renderpass_begin_info.renderPass = renderpass.get();
	renderpass_begin_info.framebuffer = framebuffer;
	renderpass_begin_info.renderArea = { offset, extent }; // VkRect2D
	renderpass_begin_info.clearValueCount = static_cast<uint32_t>(clear_value.size());
	renderpass_begin_info.pClearValues = clear_value.data();

	VkSubpassBeginInfo subpass_begin_info = {};
	subpass_begin_info.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
	subpass_begin_info.pNext = NULL;
	subpass_begin_info.contents = VK_SUBPASS_CONTENTS_INLINE;

	vkCmdBeginRenderPass2(buffer, &renderpass_begin_info, &subpass_begin_info);
}

void CommandBuffer::end_renderpass() const {
	vkCmdEndRenderPass(buffer);
}

void CommandBuffer::next_subpass(VkSubpassContents contents) const {
	vkCmdNextSubpass(buffer, contents);
}

// start command buffer recording state
VkResult CommandBuffer::begin_recording() const {
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin_info.pInheritanceInfo = nullptr; // pointer to a VkCommandBufferInheritanceInfo struct; only relevant for secondary command buffers
	VkResult result = vkBeginCommandBuffer(buffer, &begin_info);
	if (result == VK_SUCCESS) {
		Log::debug("beginning command buffer recording state");
	}
	else {
		Log::warning("failed to begin command buffer recording state (VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
	return result;
}

// end command buffer recording state
// (thus triggering executable state)
void CommandBuffer::end_recording() const {
	VkResult result = vkEndCommandBuffer(buffer);
	if (result != VK_SUCCESS) {
		switch (result) {
		case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: Log::warning("in CommandBuffer::end_recording(): VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR"); break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: Log::warning("in CommandBuffer::end_recording(): VK_ERROR_OUT_OF_DEVICE_MEMORY"); break;
		case VK_ERROR_OUT_OF_HOST_MEMORY: Log::warning("in CommandBuffer::end_recording(): VK_ERROR_OUT_OF_HOST_MEMORY"); break;
		case VK_ERROR_VALIDATION_FAILED_EXT: Log::warning("in CommandBuffer::end_recording(): VK_ERROR_VALIDATION_FAILED"); break;
		default: Log::warning("in CommandBuffer::end_recording(): VK_ERROR_UNKNOWN");
		}
	}
	else {
		Log::debug("ended command buffer recording state (command buffer is now executable)");
	}
}

VkResult CommandBuffer::reset(VkCommandBufferResetFlags flags) {
	VkResult result = vkResetCommandBuffer(buffer, flags);
	if (result == VK_SUCCESS) {
		Log::debug("successfully reset command buffer");
	}
	else {
		Log::warning("failed to reset command buffer (handle: ", buffer, ", VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
	return result;
}

// getters
VkCommandBuffer& CommandBuffer::get() { return buffer; }
QueueFamily CommandBuffer::get_usage() const { return usage; }
VkQueue CommandBuffer::get_queue() const { return queue; }


// +=================================+   
// | Task                            |
// +=================================+

// resources container for a compute or graphics task
// (the command_pool argument determines the task type),
// managed by the VulkanManager class to ensure the lifetime
// of the resources exceeds the scope of any functions which
// record commands related to the given task

Task::Task(Device& device, CommandPool& command_pool, DescriptorPool& descriptor_pool, std::string calling_function) : device(&device), command_pool(&command_pool), descriptor_pool(&descriptor_pool), calling_function(calling_function) {
	if (command_pool.get_usage() == QueueFamily::COMPUTE_QUEUE) {
		task_type = TaskType::TASK_TYPE_COMPUTE;
	}
	else if (command_pool.get_usage() == QueueFamily::GRAPHICS_QUEUE) {
		task_type = TaskType::TASK_TYPE_GRAPHICS;
	}
	else {
		task_type = TaskType::TASK_TYPE_TRANSFER;
	}
	num_created++;
}

// destructor
Task::~Task() {
	num_destroyed++;
}

// move constructor
Task::Task(Task&& other) noexcept :
	command_buffer(std::exchange(other.command_buffer, nullptr)),
	fence(std::exchange(other.fence, nullptr)),
	device(std::exchange(other.device, nullptr)),
	descriptor_pool(std::exchange(other.descriptor_pool, nullptr)),
	set_layout(std::exchange(other.set_layout, nullptr)),
	set(std::exchange(other.set, nullptr)),
	shaders(std::move(other.shaders)),
	compute_pipelines(std::move(other.compute_pipelines)),
	graphics_pipelines(std::move(other.graphics_pipelines)),
	constants(std::exchange(other.constants, nullptr)),
	temp_buffers(std::move(other.temp_buffers)) {
	task_type = other.task_type;
}

// create temporary buffers (=which can be deleted after task execution has finished);
// returns a reference to the new Buffer<T> object
template<typename T>
Buffer<T>& Task::add_temp_buffer(uint32_t elements, BufferType usage, VkMemoryPropertyFlags memory_property_flags) {

	Log::debug("Task::add_temp_buffer(): creating temporary buffer (owned by task ", this, ") of type ", typeid(T).name(), " with ", elements, " elements(", elements * sizeof(T), " bytes) for calling function ", calling_function);
	std::unique_ptr<Buffer<T>> new_buffer_ptr = std::make_unique<Buffer<T>>(*device, usage, elements, memory_property_flags);
	Buffer<T>& new_buffer_ref = *new_buffer_ptr;
	this->temp_buffers.push_back(std::move(new_buffer_ptr));
	return new_buffer_ref;
}

// create a temporary descriptor set layout, owned by this task;
// returns a reference to the new DescriptorSetLayout object;
// a compute task is designed to own only one descriptor set layout;
// if a layout already exists, it will be replaced and a warning is issued;
// if you need multiple layouts, please create them externally and provide them to the task using Task::add_descriptor_set(DescriptorSetLayout&);
// also keep in mind that a descriptor set layout owned by the task is no longer valid beyond the lifetime of the task!
DescriptorSetLayout& Task::add_descriptor_set_layout() {
	Log::debug("Task::add_descriptor_set_layout(): creating descriptor set layout (owned by task ", this, ") for calling function ", calling_function);
	if (this->set_layout != nullptr) {
		Log::warning("in method Task::add_descriptor_set_layout(): a single compute task is designed to own only one descriptor set layout; this task (", this, ") already has a descriptor set layout, therefore the previous layout will be replaced!");
		this->set_layout.reset();
	}
	this->set_layout = std::make_unique<DescriptorSetLayout>(*this->device);
	return *this->set_layout;
}

// add a descriptor set for the set layout owned by this compute task
DescriptorSet& Task::add_descriptor_set() {
	Log::debug("Task::add_descriptor_set(): creating descriptor set (owned by task ", this, ") for calling function ", calling_function);
	if (this->set_layout == nullptr) {
		Log::warning("method Task::add_descriptor_set() for task ", this, " has failed : please provide a referenced descriptor set layout as function argument or define a descriptor set layout owned by this task first(using Task::add_descriptor_set_layout())");
	}
	else {
		if (this->set != nullptr) {
			Log::warning("in method Task::add_descriptor_set(): a single compute task is designed to own only one descriptor set; this task (", this, ") already has a descriptor set, therefore the previous set will be replaced!");
			VkDescriptorSet set_handle = this->set->get();
			this->descriptor_pool->release_set(set_handle);
			this->set.reset();
		}
		this->set = std::make_unique<DescriptorSet>(*this->device, *this->set_layout, *this->descriptor_pool);
	}
	return *this->set;
}

// add a descriptor set for the referenced (=externally owned) set layout
DescriptorSet& Task::add_descriptor_set(DescriptorSetLayout& descriptor_set_layout) {
	Log::debug("Task::add_descriptor_set(): creating descriptor set (owned by task ", this, ") for calling function ", calling_function);
	// delete old descriptor set, if any
	if (set != nullptr) {
		Log::warning("in method Task::add_descriptor_set(): a single compute task is designed to own only one descriptor set; this task (", this, ") already has a descriptor set, therefore the previous set will be replaced!");
		VkDescriptorSet set_handle = this->set->get();
		this->descriptor_pool->release_set(set_handle);
		this->set.reset();
	}
	this->set = std::make_unique<DescriptorSet>(*this->device, descriptor_set_layout, *this->descriptor_pool);
	return *this->set;
}

// add a compute shader module from SPIR-V binary data and returns its ID within this task
uint32_t Task::add_shader(const unsigned char* shader_spirv_bin, size_t shader_spirv_bytes) {
	Log::debug("Task::add_shader(): creating shader module (owned by task ", this, ") for calling function ", calling_function);
	uint32_t shader_id = static_cast<uint32_t>(shaders.size());
	shaders.push_back(std::make_unique<ShaderModule>(*device, shader_spirv_bin, shader_spirv_bytes));
	return shader_id;
}

template<typename ...Args> PushConstants&
Task::add_constants(Args... args) {
	(this->get_constants().add_values(args), ...); // fold expression
	return *constants;
}

ComputePipeline& Task::add_pipeline(
	uint32_t shader_id,
	uint32_t push_constants_range_size,
	DescriptorSetLayout& set_layout,
	uint32_t workgroup_size_x,
	uint32_t workgroup_size_y,
	uint32_t workgroup_size_z,
	std::vector<uint32_t> addon_specialization_constants) {

	Log::debug("Task::add_pipeline(): creating compute pipeline (owned by task ", this, ") for calling function ", calling_function);
	uint32_t pipeline_id = static_cast<uint32_t>(compute_pipelines.size());
	compute_pipelines.push_back(std::make_unique<ComputePipeline>(*device, *shaders[shader_id], push_constants_range_size, set_layout, workgroup_size_x, workgroup_size_y, workgroup_size_z, addon_specialization_constants));
	return *compute_pipelines[pipeline_id];
}

ComputePipeline& Task::add_pipeline(
	uint32_t shader_id,
	DescriptorSetLayout& set_layout,
	uint32_t workgroup_size_x,
	uint32_t workgroup_size_y,
	uint32_t workgroup_size_z,
	std::vector<uint32_t> addon_specialization_constants) {

	return this->add_pipeline(shader_id, this->constants->get_size(), set_layout, workgroup_size_x, workgroup_size_y, workgroup_size_z, addon_specialization_constants);
}

ComputePipeline& Task::add_pipeline(uint32_t shader_id,
	uint32_t push_constants_range_size,
	uint32_t workgroup_size_x,
	uint32_t workgroup_size_y,
	uint32_t workgroup_size_z,
	std::vector<uint32_t> addon_specialization_constants) {

	if (this->set_layout == nullptr) {
		Log::error("invalid call of Task::add_pipeline(): either a descriptor set layout must have been added to the task or provided as function argument");
	}
	return this->add_pipeline(shader_id, push_constants_range_size, *this->set_layout, workgroup_size_x, workgroup_size_y, workgroup_size_z, addon_specialization_constants);
}

ComputePipeline& Task::add_pipeline(uint32_t shader_id,
	uint32_t workgroup_size_x,
	uint32_t workgroup_size_y,
	uint32_t workgroup_size_z,
	std::vector<uint32_t> addon_specialization_constants) {

	if (this->set_layout == nullptr) {
		Log::error("invalid call of Task::add_pipeline(): either a descriptor set layout must have been added to the task (", this, ") or provided as function argument");
	}
	return this->add_pipeline(shader_id, this->constants->get_size(), *this->set_layout, workgroup_size_x, workgroup_size_y, workgroup_size_z, addon_specialization_constants);
}

GraphicsPipeline& Task::add_pipeline(
	const RenderPass& renderpass,
	uint32_t subpass_index,
	const GraphicsPipelineLayout& pipeline_layout,
	const ShaderModule& vertex_shader,
	const std::vector<VertexDescriptions>& vertex_descriptions,
	const std::optional<ColorBlendState>& color_blend_state,
	const std::optional<ShaderModule>& fragment_shader,
	const std::optional<ShaderModule>& hull_shader,
	const std::optional<ShaderModule>& domain_shader,
	uint32_t tessellation_patch_control_points,
	VkPipelineDepthStencilStateCreateFlagBits depth_stencil_flags
) {

	uint32_t pipeline_id = static_cast<uint32_t>(graphics_pipelines.size());
	graphics_pipelines.push_back(std::make_unique<GraphicsPipeline>(
		*device,
		renderpass,
		subpass_index,
		pipeline_layout,
		vertex_shader,
		vertex_descriptions,
		color_blend_state,
		fragment_shader,
		hull_shader,
		domain_shader,
		tessellation_patch_control_points,
		depth_stencil_flags
	));
	return *graphics_pipelines[pipeline_id];
}

// creates and adds a temporary timeline semaphore owned by this task;
// this resource can then be used as a wait or signal semaphore
Semaphore& Task::add_temp_timeline_semaphore(uint64_t initial_value, VkPipelineStageFlagBits2 wait_dst_stage_mask, VkPipelineStageFlagBits2 signal_dst_stage_mask) {
	temp_semaphores.push_back(std::make_unique<Semaphore>(*device, initial_value, wait_dst_stage_mask, signal_dst_stage_mask));
	Log::debug("Task::add_temp_timeline_semaphore(): new timeline semaphore (owned by task ", this, ") created at memory location ", temp_semaphores.back(), " for calling function ", calling_function, "; initial value = ");
	return *temp_semaphores.back();
}

// creates and adds a temporary binary semaphore owned by this task;
// this resource can then be used as a wait or signal semaphore
Semaphore& Task::add_temp_binary_semaphore(VkPipelineStageFlagBits2 wait_dst_stage_mask, VkPipelineStageFlagBits2 signal_dst_stage_mask) {
	temp_semaphores.push_back(std::make_unique<Semaphore>(*device, wait_dst_stage_mask, signal_dst_stage_mask));
	Log::debug("Task::add_temp_binary_semaphore(): new binary semaphore (owned by task ", this, ") created at memory location ", temp_semaphores.back(), " for calling function ", calling_function);
	return *temp_semaphores.back();
}

// adds a reference to a binary wait semaphore (which will be used during the submit call);
// used to add externally owned semaphores (i.e. not owned by this task);
// returns the total number of binary wait semaphores which are currently observed by this task
uint32_t Task::add_binary_wait_semaphore(Semaphore& semaphore) {
	if (semaphore.get_type() != VK_SEMAPHORE_TYPE_BINARY) {
		Log::warning("method Task::add_wait_semaphore() for task ", this, " has failed : this overload expects a binary semaphore(VK_SEMAPHORE_TYPE_BINARY).");
		return static_cast<uint32_t>(binary_wait_semaphores.size());
	}
	Log::debug("Task::add_binary_wait_semaphore(): adding binary wait semaphore for task ", this, " (externally owned, memory location : ", &semaphore, ") for calling function ", calling_function);
	binary_wait_semaphores_dst_stage_masks.push_back(semaphore.get_wait_dst_stage_mask());
	binary_signal_semaphores_dst_stage_masks.push_back(semaphore.get_signal_dst_stage_mask());
	binary_wait_semaphores.push_back(semaphore.get());
	return static_cast<uint32_t>(binary_wait_semaphores.size());
}

// adds a reference to a binary signal semaphore (which will be used during the submit call);
// used to add externally owned semaphores (i.e. not owned by this task);
// returns the total number of signal semaphores which are currently affected by this task
uint32_t Task::add_binary_signal_semaphore(Semaphore& semaphore) {
	if (semaphore.get_type() != VK_SEMAPHORE_TYPE_BINARY) {
		Log::warning("method Task::add_signal_semaphore() for task ", this, " has failed : this overload expects a binary semaphore(VK_SEMAPHORE_TYPE_BINARY).");
		return static_cast<uint32_t>(binary_signal_semaphores.size());
	}
	Log::debug("Task::add_binary_signal_semaphore(): adding binary signal semaphore for task ", this, " (externally owned, memory location ", &semaphore, ") for calling function ", calling_function);
	binary_wait_semaphores_dst_stage_masks.push_back(semaphore.get_wait_dst_stage_mask());
	binary_signal_semaphores_dst_stage_masks.push_back(semaphore.get_signal_dst_stage_mask());
	binary_signal_semaphores.push_back(semaphore.get());
	return static_cast<uint32_t>(binary_signal_semaphores.size());
}

// adds a single binary semaphore which functions as a wait semaphore at the beginning of the command buffer execution,
// then finally it changes back to the signaled state at the end of execution;
// the semaphore MUST be expected to initially reach the signaled state (or already be in the signaled state),
// otherwise a deadlock will occur (waiting for the signaled state indefinitely) !!!
// the function expects a reference to an externally owned binary semaphore (i.e. not owned by this task)
void Task::binary_wait_and_signal(Semaphore& semaphore) {
	if (semaphore.get_type() != VK_SEMAPHORE_TYPE_BINARY) {
		Log::warning("method Task::add_combined_semaphore() for task ", this, " has failed : this overload expects a binary semaphore(VK_SEMAPHORE_TYPE_BINARY).");
	}
	Log::debug("Task::binary_wait_and_signal():");
	this->add_binary_wait_semaphore(semaphore);
	this->add_binary_signal_semaphore(semaphore);
}

// adds a single timeline semaphore which functions as a wait semaphore at the beginning of the command buffer execution,
// then it signals the specified signal_value at the end of execution;
// this function expects a reference to an externally owned timeline semaphore (i.e. not owned by this task)
void Task::timeline_sync(Semaphore& semaphore, uint64_t wait_value, uint64_t signal_value) {
	if (semaphore.get_type() != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Task::add_combined_semaphore() for task ", this, " has failed : this overload expects a timeline semaphore(VK_SEMAPHORE_TYPE_TIMELINE).");
	}
	Log::debug("Task::timeline_sync(): adding timeline wait and signal semaphore for task ", this, " (externally owned, memory location ", &semaphore, ") for calling function ", calling_function, " (wait value = ", wait_value, ", signal value = ", signal_value, ")");
	this->add_timeline_wait_semaphore(semaphore, wait_value);
	this->add_timeline_signal_semaphore(semaphore, signal_value);
}

// adds a single semaphore which functions as a wait semaphore at the beginning of the command buffer execution with its current counter value,
// then this counter gets incremented (atomically) by one and is used for the signaled state at the end of execution;
// returns the new signal value
uint64_t Task::timeline_sync(Semaphore& semaphore) {
	if (semaphore.get_type() != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Task::add_combined_semaphore() for task ", this, " has failed : this overload expects a timeline semaphore(VK_SEMAPHORE_TYPE_TIMELINE).");
	}
	Log::debug("Task::timeline_sync(): adding timeline wait and signal semaphore:");
	uint64_t old_counter_var = semaphore.get_counter_var();
	this->add_timeline_wait_semaphore(semaphore, old_counter_var);
	semaphore.increment_counter();
	uint64_t new_counter_var = semaphore.get_counter_var();;
	this->add_timeline_signal_semaphore(semaphore, new_counter_var);
	return new_counter_var;
}

// adds a reference to a timeline wait semaphore (which will be used during the submit call);
// used to add externally owned semaphores (i.e. not owned by this task);
// returns the total number of timeline wait semaphores which are currently observed by this task
uint32_t Task::add_timeline_wait_semaphore(Semaphore& semaphore, uint64_t wait_value) {
	if (semaphore.get_type() != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Task::add_wait_semaphore() for task ", this, " has failed : this overload expects a timeline semaphore(VK_SEMAPHORE_TYPE_TIMELINE).");
		return static_cast<uint32_t>(timeline_wait_semaphores.size());
	}
	Log::debug("Task::add_timeline_wait_semaphore(): adding timeline wait semaphore for task ", this, " (externally owned, memory location ", &semaphore, ") for calling function ", calling_function, " (wait value = ", wait_value, ")");
	timeline_wait_semaphores_dst_stage_masks.push_back(semaphore.get_wait_dst_stage_mask());
	timeline_wait_semaphores.push_back(semaphore.get());
	timeline_semaphore_wait_values.push_back(wait_value);
	return static_cast<uint32_t>(timeline_wait_semaphores.size());
}

// adds a reference to a timeline signal semaphore (which will be used during the submit call);
// used to add externally owned semaphores (i.e. not owned by this task);
// returns the total number of timeline signal semaphores which are currently affected by this task
uint32_t Task::add_timeline_signal_semaphore(Semaphore& semaphore, uint64_t signal_value) {
	if (semaphore.get_type() != VK_SEMAPHORE_TYPE_TIMELINE) {
		Log::warning("method Task::add_signal_semaphore() for task ", this, " has failed : this overload expects a timeline semaphore(VK_SEMAPHORE_TYPE_TIMELINE).");
		return static_cast<uint32_t>(timeline_signal_semaphores.size());
	}
	Log::debug("Task::add_timeline_signal_semaphore(): adding timeline signal semaphore for task ", this, " (externally owned, memory location ", &semaphore, ") for calling function ", calling_function, " (signal value = ", signal_value, ")");
	timeline_signal_semaphores_dst_stage_masks.push_back(semaphore.get_signal_dst_stage_mask());
	timeline_signal_semaphores.push_back(semaphore.get());
	timeline_semaphore_signal_values.push_back(signal_value);
	timeline_signal_semaphores_ptrs.push_back(&semaphore);
	return static_cast<uint32_t>(timeline_signal_semaphores.size());
}

// reset task (delete all previous resources);
// returns false if the task is protected or still busy
bool Task::reset() {
	if (fence && !fence->signaled(calling_function)) {
		Log::warning("Task::reset() has failed: the task (", this, ") for calling function ", calling_function, " is still busy(its fence isn't yet in the signaled state)");
		return false;
	}
	if (protection_flag) {
		Log::warning("Task::reset() has failed: the task (", this, ") for calling function ", calling_function, " has not yet been submitted and therefore is protected!");
		return false;
	}
	else {
		Log::debug("Task::reset(): resetting compute task ", this);
		// reset the command buffer to the initial state
		// (this is not the same as command_buffer.reset() ! the smart pointer remains valid!)
		if (command_buffer != nullptr) { command_buffer->reset(); }

		// reset fence to un-signaled state
		// (this is not the same as fence.reset() ! the smart pointer remains valid!)
		fence->reset(calling_function);

		// release and delete descriptor set (if any)
		if (set != nullptr) {
			VkDescriptorSet set_handle = set->get();
			descriptor_pool->release_set(set_handle);
			this->set.reset();
		}

		// delete the descriptor set layout (if any)
		if (set_layout != nullptr) {
			set_layout.reset();
		}

		// clear push constants range
		// (this doesn't affect the capacity or memory location; it simply resets the range.size to zero)
		if (constants) { constants->free(); }

		// clear temporary buffer(s)
		temp_buffers.clear();

		// clear pipeline(s)
		compute_pipelines.clear();
		graphics_pipelines.clear();

		// clear shader module(s)
		shaders.clear();

		// clear semaphores
		binary_wait_semaphores.clear();
		timeline_wait_semaphores.clear();
		binary_signal_semaphores.clear();
		timeline_signal_semaphores.clear();
		timeline_semaphore_wait_values.clear();
		timeline_semaphore_signal_values.clear();
		binary_wait_semaphores_dst_stage_masks.clear();
		binary_signal_semaphores_dst_stage_masks.clear();
		timeline_wait_semaphores_dst_stage_masks.clear();
		timeline_signal_semaphores_dst_stage_masks.clear();
		temp_semaphores.clear();
		timeline_signal_semaphores_ptrs.clear();

		// reset calling function
		calling_function = "NONE (=TASK IS AVAILABLE)";

		// mark as protected (at least until submit)
		protection_flag = true;

		Log::debug("resetting task ", this, " is complete -> task is ready to be used");
		return true;
	}
}

// submit command buffer to appropriate queue on device;
VkResult Task::submit(uint64_t wait_after_submit_nanosec) {

	if (!command_buffer) {
		protection_flag = false;
		return VK_SUCCESS;
	}

	Log::debug("Task::submit(): submitting task ", this, " for calling function ", calling_function, " (fence: ", fence->get(), ")");

	uint32_t device_index = device->get_index();

	// setup wait semaphores submit info
	std::vector<VkSemaphoreSubmitInfo> wait_semaphore_submit_infos;
	wait_semaphore_submit_infos.reserve(binary_wait_semaphores.size() + timeline_wait_semaphores.size());
	for (uint32_t i = 0; i < binary_wait_semaphores.size(); i++) {
		wait_semaphore_submit_infos.emplace_back(
			VkSemaphoreSubmitInfo{
				VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,	// VkStructureType			sType
				NULL,										// void*					pNext
				binary_wait_semaphores[i],					// VkSemaphore				semaphore
				0,											// uint64_t					value (ignored in case of binary semaphores)
				binary_wait_semaphores_dst_stage_masks[i],	// VkPipelineStageFlags2	stageMask
				device_index								// uint32_t					deviceIndex
			}
		);
	}
	for (uint32_t i = 0; i < timeline_wait_semaphores.size(); i++) {
		wait_semaphore_submit_infos.emplace_back(
			VkSemaphoreSubmitInfo{
				VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,	// VkStructureType			sType
				NULL,										// void*					pNext
				timeline_wait_semaphores[i],				// VkSemaphore				semaphore
				timeline_semaphore_wait_values[i],			// uint64_t					value (ignored in case of binary semaphores)
				timeline_wait_semaphores_dst_stage_masks[i],// VkPipelineStageFlags2	stageMask
				device_index								// uint32_t					deviceIndex
			}
		);
	}

	// setup signal semaphores submit info
	std::vector<VkSemaphoreSubmitInfo> signal_semaphore_submit_infos;
	signal_semaphore_submit_infos.reserve(binary_signal_semaphores.size() + timeline_signal_semaphores.size());
	for (uint32_t i = 0; i < binary_wait_semaphores.size(); i++) {
		signal_semaphore_submit_infos.emplace_back(
			VkSemaphoreSubmitInfo{
				VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,	// VkStructureType			sType
				NULL,										// void*					pNext
				binary_signal_semaphores[i],				// VkSemaphore				semaphore
				0,											// uint64_t					value (ignored in case of binary semaphores)
				binary_signal_semaphores_dst_stage_masks[i],// VkPipelineStageFlags2	stageMask
				device_index								// uint32_t					deviceIndex
			}
		);
	}
	for (uint32_t i = 0; i < timeline_signal_semaphores.size(); i++) {
		signal_semaphore_submit_infos.emplace_back(
			VkSemaphoreSubmitInfo{
				VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,	// VkStructureType			sType
				NULL,										// void*					pNext
				timeline_signal_semaphores[i],				// VkSemaphore				semaphore
				timeline_semaphore_signal_values[i],		// uint64_t					value (ignored in case of binary semaphores)
				timeline_signal_semaphores_dst_stage_masks[i],// VkPipelineStageFlags2	stageMask
				device_index								// uint32_t					deviceIndex
			}
		);
	}

	VkCommandBufferSubmitInfo command_buffer_submit_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,		// VkStructureType			sType
		NULL,												// void*					pNext
		command_buffer ? command_buffer->get() : nullptr,	// VkCommandBuffer			commandBuffer
		0													// uint32_t					deviceMask
	};

	// submit to queue (triggers command buffer pending state)
	VkQueue queue;
	switch (task_type) {
	case TaskType::TASK_TYPE_COMPUTE: queue = device->get_compute_queue(); break;
	case TaskType::TASK_TYPE_GRAPHICS: queue = device->get_graphics_queue(); break;
	case TaskType::TASK_TYPE_TRANSFER: queue = device->get_transfer_queue(); break;
	}
	VkSubmitInfo2 submit_info2 = {};
	submit_info2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submit_info2.pNext = NULL;
	submit_info2.flags = 0;
	submit_info2.waitSemaphoreInfoCount = static_cast<uint32_t>(wait_semaphore_submit_infos.size());
	submit_info2.pWaitSemaphoreInfos = wait_semaphore_submit_infos.empty() ? nullptr : wait_semaphore_submit_infos.data();
	submit_info2.commandBufferInfoCount = static_cast<uint32_t>(command_buffer != nullptr);
	submit_info2.pCommandBufferInfos = command_buffer ? &command_buffer_submit_info : nullptr;
	submit_info2.signalSemaphoreInfoCount = static_cast<uint32_t>(signal_semaphore_submit_infos.size());
	submit_info2.pSignalSemaphoreInfos = signal_semaphore_submit_infos.empty() ? nullptr : signal_semaphore_submit_infos.data();

	VkResult result = vkQueueSubmit2(
		queue,								// VkQueue queue
		1,									// uint32_t submitCount
		&submit_info2,						// const VkSubmitInfo2* pSubmits
		fence ? fence->get() : nullptr		// VKFence fence
	);

	if (result != VK_SUCCESS) {
		Log::warning("Task::submit(): failed to submit task ", this, " for caller function ", calling_function,
			", with ", binary_wait_semaphores.size(), " binary wait semaphores, ", timeline_wait_semaphores.size(), " timeline wait semaphores, ",
			binary_signal_semaphores.size(), " binary signal semaphores, ", timeline_signal_semaphores.size(), " timeline signal semaphores): failed to submit task(VkResult = ", result, ", ", vkresult_to_string(result), ")");
	}
	else {
		Log::debug("Task::submit(): successfully submitted task ", this, " for caller function ", calling_function,
			", with ", binary_wait_semaphores.size(), " binary wait semaphores, ", timeline_wait_semaphores.size(), " timeline wait semaphores, ",
			binary_signal_semaphores.size(), " binary signal semaphores, ", timeline_signal_semaphores.size(), " timeline signal semaphores)");
	}

	if (wait_after_submit_nanosec != 0) {
		if (fence) {
			Log::debug("Task::submit(): waiting for task ", this, " to become idle(waiting for fence) for calling function ", calling_function, " after submission");
			result = fence->wait(calling_function, wait_after_submit_nanosec);
			if (result != VK_SUCCESS) {
				Log::warning("in Task::submit() for caller function ", calling_function, ": waiting for task ", this, " to become idle(= fence has reached signaled state) after submission has failed(VkResult = ", result, ", ", vkresult_to_string(result), ")");
			}
			else {
				Log::debug("Task::submit(): task ", this, "has become idle(= fence has reached signaled state) for calling function ", calling_function, " after submission");
				protection_flag = false;
			}
		}
		else {
			Log::Timer::sleep(wait_after_submit_nanosec, LogLevel::LEVEL_DEBUG);
		}
	}
	else {
		protection_flag = false;
	}

	return result;
}

// getters

std::string Task::get_calling_function() const { return calling_function; }

bool Task::available() {
	// 1. check protection flag
	if (protection_flag) {
		return false;
	}
	// 2. check fence signaled state;
	// ignore fence if this task has no command buffer
	// (which implies the fence can't reach the signaled state anyways);
	else if (command_buffer) {
		if (!fence || fence->signaled(calling_function)) {
			// 3. verify timeline semaphore signals
			for (uint32_t i = 0; i < timeline_signal_semaphores_ptrs.size(); i++) {
				if (timeline_signal_semaphores_ptrs[i]->get_counter() < timeline_semaphore_signal_values[i]) {
					return false;
				}
			}
			return true;
		}
		else {
			return false;
		}
	}
	// 4. case: protection_flag==false, no command buffer used
	else {
		return true;
	}
}

DescriptorSet& Task::get_set() const { return *set; }

CommandBuffer& Task::get_command_buffer() {
	if (!command_buffer) {
		command_buffer = std::make_unique<CommandBuffer>(*device, *command_pool);
	}
	if (!fence) {
		fence = std::make_unique<Fence>(*device, false);
	}
	return *command_buffer;
}

Fence& Task::get_fence() {
	if (!fence) {
		Log::error("invalid call of Task::get_fence() for task ", this, ": this compute task has no command buffer and therefore also no fence!");
	}
	return *fence;
}

PushConstants& Task::get_constants() {
	if (!constants) {
		constants = std::make_unique<PushConstants>();
	}
	return *constants;
}

// initialization of Task static members from outside the class
uint64_t Task::num_created = 0;
uint64_t Task::num_destroyed = 0;

// +=================================+   
// | VulkanManager                   |
// +=================================+

// shared manager for instance, device and command pools singleton class

// create a singleton with default device features
VulkanManager& VulkanManager::get_singleton() {
	if (singleton == nullptr) {
		Log::debug("creating new VulkanManager singleton");
		// First, initialize the static shared members
		// (Note: This is the ONLY place this happens)
		instance = std::make_unique<Instance>();

		instance->set_api_version(DEFAULT_API_MAJOR_VERSION, DEFAULT_API_MINOR_VERSION, DEFAULT_API_PATCH_VERSION);
		instance->set_application("Shared Vulkan Manager", 1, 0, 0);

#ifdef _WIN32
		// get the required instance extensions from GLFW and add them to the default list
		if (glfwInit() != GLFW_TRUE) {
			Log::warning("In VulkanManager::get_singleton(): glfwInit() has failed for _WIN32.");
		}
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		std::vector<const char*> all_instance_extensions(DEFAULT_INSTANCE_EXTENSIONS);
		for (uint32_t i = 0; i < glfw_extension_count; ++i) {
			all_instance_extensions.push_back(glfw_extensions[i]);
		}
		instance->enable_extensions(all_instance_extensions);
#else
		instance->enable_extensions(DEFAULT_INSTANCE_EXTENSIONS);
#endif
		instance->enable_layers(DEFAULT_INSTANCE_LAYERS);
		instance->create();

		device = std::make_unique<Device>(*instance, DEFAULT_DEVICE_FEATURES, DEFAULT_DEVICE_EXTENSIONS, DEFAULT_DEVICE);

		// setup command pools
		Log::debug("VulkanManager: creating new graphics command pool");
		shared_command_pool_graphics = std::make_unique<CommandPool>(*device, QueueFamily::GRAPHICS_QUEUE);
		Log::debug("VulkanManager: creating new compute command pool");
		shared_command_pool_compute = std::make_unique<CommandPool>(*device, QueueFamily::COMPUTE_QUEUE);
		Log::debug("VulkanManager: creating new transfer command pool");
		shared_command_pool_transfer = std::make_unique<CommandPool>(*device, QueueFamily::TRANSFER_QUEUE);

		shared_descriptor_pool = std::make_unique<DescriptorPool>(*device, MAX_DESCRIPTOR_SET_COUNT, DEFAULT_POOL_SIZE);

		// Now, create the single VulkanManager object
		singleton.reset(new VulkanManager());
	}
	return *singleton;
}

// destructor
VulkanManager::~VulkanManager() {
	Log::debug("singleton manager destructor invoked");
	tasks.clear();
	if (glfw_window) {
		glfwDestroyWindow(glfw_window);
		glfwTerminate();
	}
}

// get an available (=idle) compute task which can be used for new resources
Task& VulkanManager::get_compute_task(std::string calling_function) {
	Log::debug("VulkanManager::get_compute_task(): calling function ", calling_function, " has requested a new or available task for compute usage");
	uint32_t task_count = static_cast<uint32_t>(tasks.size());
	for (uint32_t i = 0; i < task_count; i++) {
		if (tasks[i]->available()) {
			Log::debug("Task ", tasks[i].get(), " is available to be reused (= currently doing no work, no un-signaled fences, protection_flag = false)");
			tasks[i]->reset();
			tasks[i]->calling_function = calling_function;
			tasks[i]->command_pool = shared_command_pool_compute.get();
			return *tasks[i];
		}
	}
	// create a new task if no free task has been found
	Log::debug("no idle/available/unprotected task found -> creating a new one");
	tasks.push_back(std::make_unique<Task>(*device, *shared_command_pool_compute, *shared_descriptor_pool, calling_function));
#ifdef _DEBUG
	constexpr uint32_t MAX_TASKS = 50;
	if (task_count >= MAX_TASKS) {
		Log::warning("VulkanManager is currently handling ", task_count, " compute tasks. Resource leak !?");
		log_tasks();
	}
	Log::debug("Task ", tasks[task_count].get(), " is now available to be used for function ", calling_function);
#endif
	return *tasks[task_count];
}

// get an available (=idle) graphics task which can be used for new resources
Task& VulkanManager::get_graphics_task(std::string calling_function) {
	Log::debug("VulkanManager::get_graphics_task(): calling function ", calling_function, " has requested a new or available task for graphics usage");
	uint32_t task_count = static_cast<uint32_t>(tasks.size());
	for (uint32_t i = 0; i < task_count; i++) {
		if (tasks[i]->available()) {
			Log::debug("Task ", tasks[i].get(), " is available to be reused (= currently doing no work, no un-signaled fences, protection_flag = false)");
			tasks[i]->reset();
			tasks[i]->calling_function = calling_function;
			tasks[i]->command_pool = shared_command_pool_graphics.get();
			return *tasks[i];
		}
	}
	// create a new task if no free task has been found
	Log::debug("no idle/available/unprotected task found -> creating a new one");
	tasks.push_back(std::make_unique<Task>(*device, *shared_command_pool_graphics, *shared_descriptor_pool, calling_function));
#ifdef _DEBUG
	constexpr uint32_t MAX_TASKS = 50;
	if (task_count >= MAX_TASKS) {
		Log::warning("VulkanManager is currently handling ", task_count, " tasks. Resource leak !?");
		log_tasks();
	}
	Log::debug("Task ", tasks[task_count].get(), " is now available to be used for function ", calling_function);
#endif
	return *tasks[task_count];
}

// get an available (=idle) transfer task which can be used for new resources
Task& VulkanManager::get_transfer_task(std::string calling_function) {
	Log::debug("VulkanManager::get_transfer_task(): calling function ", calling_function, " has requested a new or available task for transfer usage");
	uint32_t task_count = static_cast<uint32_t>(tasks.size());
	for (uint32_t i = 0; i < task_count; i++) {
		if (tasks[i]->available()) {
			Log::debug("Task ", tasks[i].get(), " is available to be reused (= currently doing no work, no un-signaled fences, protection_flag = false)");
			tasks[i]->reset();
			tasks[i]->calling_function = calling_function;
			tasks[i]->command_pool = shared_command_pool_transfer.get();
			return *tasks[i];
		}
	}
	// create a new task if no free task has been found
	Log::debug("no idle/available/unprotected task found -> creating a new one");
	tasks.push_back(std::make_unique<Task>(*device, *shared_command_pool_transfer, *shared_descriptor_pool, calling_function));
#ifdef _DEBUG
	constexpr uint32_t MAX_TASKS = 50;
	if (task_count >= MAX_TASKS) {
		Log::warning("VulkanManager is currently handling ", task_count, " tasks. Resource leak !?");
		log_tasks();
	}
	Log::debug("Task ", tasks[task_count].get(), " is now available to be used for function ", calling_function);
#endif
	return *tasks[task_count];
}

void VulkanManager::log_tasks() {
	uint32_t task_count = static_cast<uint32_t>(tasks.size());
	uint32_t num_available = 0;
	uint32_t num_busy = 0;
	std::unordered_map<std::string, uint32_t> tasks_per_caller;
	for (uint32_t i = 0; i < task_count; i++) {
		bool is_available = tasks[i]->available();
		num_available += static_cast<uint32_t>(is_available);
		num_busy += static_cast<uint32_t>(!is_available);
		tasks_per_caller[tasks[i]->get_calling_function()]++;
	}
	Log::debug("VulkanManager tasks: ", task_count, " active tasks (", Task::num_created, " created, ", Task::num_destroyed, " destroyed), ACTIVE TASKS: busy=", num_busy, ", available = ", num_available);
	if (!tasks_per_caller.empty()) {
		for (const auto& pair : tasks_per_caller) {
			Log::debug("--- Function \"", pair.first, "\": ", pair.second, " active tasks");
		}
	}
}

Surface& VulkanManager::get_surface(uint32_t initial_width, uint32_t initial_height, const char* window_title, GLFWmonitor* monitor, GLFWwindow* share) {
	if (!surface) {
#ifdef _WIN32
		// using GLFW for Windows
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// Tell GLFW not to create an OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);		// Window Resizing enabled
		glfw_window = glfwCreateWindow(initial_width, initial_height, window_title, monitor, share);

		if (!glfw_window) {
			Log::error("Failed to create GLFW window.");
			glfwTerminate();
		}
		surface = std::make_unique<Surface>(*instance, glfw_window);
#elif defined(__linux__)
		// For Linux, we get the Display and xcb_connection_t directly.
		Display* display = nullptr;
		xcb_connection_t* connection = nullptr;
		surface = std::make_unique<Surface>(*instance, connection, display);
#elif defined(__ANDROID__)
		// For Android, we get the ANativeWindow directly.
		ANativeWindow* window = nullptr;
		surface = std::make_unique<Surface>(*instance, window);
#else
		Log::error("Failed to acquire Surface object for this operating system.");
#endif
	}
	return *surface;
}

// provides the raw pointer to a timeline semaphore owned by the VulkanManager singleton;
// this can be helpful to make sure the semaphore's lifetime exceeds local function scopes;
Semaphore* VulkanManager::get_timeline_semaphore(uint64_t initial_value, VkPipelineStageFlagBits2 wait_dst_stage_mask, VkPipelineStageFlagBits2 signal_dst_stage_mask) {
	uint32_t index = static_cast<uint32_t>(timeline_semaphores.size());
	timeline_semaphores.emplace_back(std::make_unique<Semaphore>(*device, initial_value, wait_dst_stage_mask, signal_dst_stage_mask));
	return timeline_semaphores[index].get();
}

// getters
Device& VulkanManager::get_device() { return *device; }
const Device* VulkanManager::get_device_ptr() { return device.get(); }
const Instance& VulkanManager::get_instance() { return *instance; }
CommandPool& VulkanManager::get_command_pool_graphics() { return *shared_command_pool_graphics; }
CommandPool& VulkanManager::get_command_pool_compute() { return *shared_command_pool_compute; }
CommandPool& VulkanManager::get_command_pool_transfer() { return *shared_command_pool_transfer; }
DescriptorPool& VulkanManager::get_descriptor_pool() { return *shared_descriptor_pool; }
const VkPhysicalDeviceFeatures& VulkanManager::get_enabled_device_features() { return shared_enabled_device_features; }

// private constructor: empty, because all the work is already done in get_singleton()
VulkanManager::VulkanManager() {}

// initialization of VulkanManager static members from outside the class
std::unique_ptr<Instance> VulkanManager::instance = nullptr;
std::unique_ptr<Device> VulkanManager::device = nullptr;
std::unique_ptr<VulkanManager> VulkanManager::singleton = nullptr;
std::vector<const char*> VulkanManager::shared_instance_layer_names = {};
std::vector<const char*> VulkanManager::shared_instance_extension_names = {};
std::vector<const char*> VulkanManager::shared_device_extension_names = {};
VkPhysicalDeviceFeatures VulkanManager::shared_enabled_device_features = {};
uint32_t VulkanManager::shared_default_device_id = {};
uint32_t VulkanManager::shared_api_major_version = {};
uint32_t VulkanManager::shared_api_minor_version = {};
uint32_t VulkanManager::shared_api_patch_version = {};
std::unique_ptr<CommandPool> VulkanManager::shared_command_pool_compute = nullptr;
std::unique_ptr<CommandPool> VulkanManager::shared_command_pool_graphics = nullptr;
std::unique_ptr<CommandPool> VulkanManager::shared_command_pool_transfer = nullptr;
std::unique_ptr<DescriptorPool> VulkanManager::shared_descriptor_pool = nullptr;
std::vector<std::unique_ptr<Task>> VulkanManager::tasks = {};
std::vector<std::unique_ptr<Semaphore>> VulkanManager::timeline_semaphores = {};
std::unique_ptr<Surface> VulkanManager::surface = nullptr;
GLFWwindow* VulkanManager::glfw_window = nullptr;

// helper function to convert VkResult values to human-readable strings
std::string vkresult_to_string(VkResult result) {
	switch (result) {
	case VK_SUCCESS:
		return "VK_SUCCESS: Command successfully completed.";
	case VK_NOT_READY:
		return "VK_NOT_READY: A fence or query has not yet completed.";
	case VK_TIMEOUT:
		return "VK_TIMEOUT: A wait operation has not completed in the specified time.";
	case VK_EVENT_SET:
		return "VK_EVENT_SET: An event has been signaled.";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET: An event has been unsignaled.";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE: A command buffer or queue submission was incomplete.";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY: A host memory allocation failed.";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY: A device memory allocation failed.";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED: Initialization of a Vulkan object failed.";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST: The logical or physical device has been lost.";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED: Mapping of a device memory allocation failed.";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT: A requested layer is not present or could not be loaded.";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT: A requested extension is not supported.";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT: A requested feature is not supported.";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER: The driver is incompatible with the requested Vulkan version.";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS: Too many objects of a certain type have been created.";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED: A requested format is not supported on this device.";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL: A pool allocation has failed due to fragmentation.";
	case VK_ERROR_UNKNOWN:
		return "VK_ERROR_UNKNOWN: An unknown error has occurred.";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED: An error occurred during validation.";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY: An allocation from a Vulkan memory pool has failed.";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE: An external handle is not a valid handle of the specified type.";
	case VK_ERROR_FRAGMENTATION:
		return "VK_ERROR_FRAGMENTATION: A descriptor pool or buffer has become fragmented.";
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: A provided opaque capture address is invalid.";
	case VK_PIPELINE_COMPILE_REQUIRED:
		return "VK_PIPELINE_COMPILE_REQUIRED: The pipeline cache is not pre-populated and requires compilation.";
	case VK_ERROR_NOT_PERMITTED:
		return "VK_ERROR_NOT_PERMITTED: An operation is not permitted.";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR: The surface has been lost.";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: The requested window is already in use by Vulkan or another API.";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR: The Swapchain is not optimal for the surface, but can still be used.";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR: The Swapchain has become out of date and must be recreated.";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: The display is incompatible with the requested mode.";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV: A shader has failed to compile or link.";
	case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
		return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: The image usage flags are not supported for the video profile.";
	case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: The video picture layout is not supported for the video profile.";
	case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: The video profile operation is not supported.";
	case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: The video profile format is not supported.";
	case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: The video profile codec is not supported.";
	case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: The video standard version is not supported.";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: The plane layout for the DRM format modifier is invalid.";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: The full-screen exclusive mode has been lost.";
	case VK_THREAD_IDLE_KHR:
		return "VK_THREAD_IDLE_KHR: The deferred operation is idle.";
	case VK_THREAD_DONE_KHR:
		return "VK_THREAD_DONE_KHR: The deferred operation has completed.";
	case VK_OPERATION_DEFERRED_KHR:
		return "VK_OPERATION_DEFERRED_KHR: The deferred operation has been deferred.";
	case VK_OPERATION_NOT_DEFERRED_KHR:
		return "VK_OPERATION_NOT_DEFERRED_KHR: The deferred operation was not deferred.";
	case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
		return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: The video standard parameters are invalid.";
	case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
		return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT: The compression control has been exhausted.";
	case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
		return "VK_INCOMPATIBLE_SHADER_BINARY_EXT: The shader binary is incompatible with the device.";
	case VK_PIPELINE_BINARY_MISSING_KHR:
		return "VK_PIPELINE_BINARY_MISSING_KHR: The requested pipeline binary is missing from the cache.";
	case VK_ERROR_NOT_ENOUGH_SPACE_KHR:
		return "VK_ERROR_NOT_ENOUGH_SPACE_KHR: There is not enough space to store the pipeline binary.";
	default:
		return "Unknown VkResult value.";
	}
}

#endif // include guard close