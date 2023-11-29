// author and copyright: Christian Suer (cyberchriz)
// description: library for parallel data structure computations on the GPU (using Vulkan)

#ifndef VKDATA_H
#define VKDATA_H

#include "vkcontext.h"
#include <cstdint>
#include <vector>
#include <vulkan_core.h>

constexpr uint32_t DEFAULT_DEVICE_ID = 0;
constexpr char* application_name = "VkData";
constexpr uint32_t application_major_version = 1;
constexpr uint32_t application_minor_version = 0;
constexpr uint32_t application_patch_version = 0;

// declare shared instance + device
Instance vkdata_shared_instance;
Device vkdata_shared_device;

template<typename T>
class VkData {
public:
	// default constructor
	VkData(){};

	// parametric constructor
	VkData(std::vector<uint32_t> dim_size) {
		this->dim_size = dim_size;
		num_dimensions = dim_size.size();
		instance = vkdata_shared_instance;
		device = vkdata_shared_device;
		data_elements = dim_size[0];
		for (uint32_t i = 1; i < num_dimensions; i++) {
			data_elements *= dim_size[i];
		}

		// one-time initialization (shared across all VkData objects)
		if (instance.get() == nullptr) {

			// enable instance layers
			std::vector<const char*> vkdata_enabled_layer_names;
			#ifdef _DEBUG
			vkdata_enabled_layer_names.push_back("VK_LAYER_KHRONOS_VALIDATION");
			#endif		
			vkdata_enabled_layer_names.push_back("VK_LAYER_LUNAR_OBJECT_TRACKER");

			// enable instance extensions
			std::vector<const char*> vkdata_enabled_extension_names;
			vkdata_enabled_extension_names.push_back("VK_EXT_compute_shader");
			vkdata_enabled_extension_names.push_back("VK_EXT_descriptor_indexing");
			vkdata_enabled_extension_names.push_back("VK_KHR_storage_buffer_storage_class");
			vkdata_enabled_extension_names.push_back("VK_KHR_dynamic_buffer_storage");

			// set create flags
			VkInstanceCreateFlags shared_instance_create_flags = 0;

			// finalize instance creation
			instance.init_api_version(VK_API_VERSION_1_2);
			instance.init_application(application_name, application_major_version, application_minor_version, application_patch_version);
			instance.init_extensions(vkdata_enabled_extension_names);
			instance.init_layers(vkdata_enabled_layer_names);
			instance.create(shared_instance_create_flags);

			// enable device features
			VkPhysicalDeviceFeatures enabled_device_features = {};
			enabled_device_features.imageCubeArray = VK_TRUE;

			// enable device extensions
			std::vector<const char*> enabled_device_extensions = {};
			enabled_device_extensions.push_back("VK_EXT_shader_atomic_float");
			enabled_device_extensions.push_back("VK_EXT_shader_image_atomic_int64");
			enabled_device_extensions.push_back("VK_KHR_shader_non_semantic_info");
			enabled_device_extensions.push_back("VK_KHR_shader_draw_parameters");

			// finalize device creation
			device = Device(instance, DEFAULT_DEVICE_ID, enabled_device_features, enabled_device_extensions);
		}

		command_pool = CommandPool(device, QueueFamily::COMPUTE);
		command_buffer = CommandBuffer(device, QueueFamily::COMPUTE, command_pool);
		VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		data_buffer = Buffer<T>(device, BufferUsage::STORAGE, data_elements, memory_properties);
	}

	// destructor
	~VkData(){}
private:
	std::vector<uint32_t> dim_size;
	uint32_t num_dimensions;
	uint64_t data_elements;
	Instance instance;
	Device device;
	CommandPool command_pool;
	CommandBuffer command_buffer;
	Buffer<T> data_buffer;
};

#endif