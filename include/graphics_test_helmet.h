#ifndef GRAPHICS_TEST_HELMET_SIMPLIFIED_H
#define GRAPHICS_TEST_HELMET_SIMPLIFIED_H

#include <vkcontext.h>

void graphics_test_helmet() {

	// setup environment
	VkExtent2D extent = { 3840, 2160 };
	VulkanManager& manager = VulkanManager::get_singleton();
	Device& device = manager.get_device();
	Semaphore& tl_semaphore = manager.get_timeline_semaphore(0, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
	Scene scene;
	Renderer rd(scene, extent);
	Surface& surface = manager.get_surface(extent.width, extent.height, "Vulkan Graphics Test - Damaged Helmet (Simplified, using Renderer class)");

	// load glTF model and add as scene entity
	Mesh model(device, "resources/models/gltf/DamagedHelmet2.glb", tl_semaphore, true, true); // source: Khronos Group
	uint32_t entity_id = scene.add_entity(model, { 0.0f, 0.0f, 0.0f });
	
	// set scene details
	scene.get_active_camera().set_world_up(UpAxis::Y_UP);
	scene.get_active_camera().set_position({ 0.0f, 0.0f, 3.0f });
	scene.get_active_camera().look_at(scene.get_entity(entity_id));
	scene.get_active_camera().set_aspect_ratio(extent);
	scene.get_active_camera().set_near_plane(0.01f);
	scene.get_active_camera().set_far_plane(100.0f);
	scene.set_ambient({ 0.0f, 0.0f, 0.0f });
	scene.set_exposure(0.5f);
	scene.set_contrast(1.2f);
	scene.set_ibl_intensity(0.2f);

	// add a backfill light
	uint32_t light_id = scene.add_scene_light( LightType::DIRECTIONAL_LIGHT, { 0.0f, 0.0f, -20.0f });
	scene.get_scene_light(light_id).point_to(scene.get_entity(entity_id));
	scene.get_scene_light(light_id).set_intensity(5.0f);

	// add a front spot light
	uint32_t light_id2 = scene.add_scene_light(LightType::DIRECTIONAL_LIGHT, { 50.0f, 0.0f, 100.0f });
	scene.get_scene_light(light_id2).point_to(scene.get_entity(entity_id));
	scene.get_scene_light(light_id2).set_intensity(10.0f);

	rd.set_surface(surface);

	// Add Event Listeners for Scroll / Translate / Rotate / WindowClose / WindowResize
	surface.add_event_listener(
		EventType::WINDOW_RESIZE,
		[&](const SurfaceEvent& e) {extent = { e.resize.width, e.resize.height }; rd.get_swapchain().update_extent(extent);	return true;}
	);
	surface.add_event_listener(
		EventType::WINDOW_CLOSE,
		[&](const SurfaceEvent& e) { rd.get_swapchain().destroy(); rd.get_surface().close(); return true; }
	);
	surface.add_event_listener(
		EventType::MOUSE_SCROLL,
		[&](const SurfaceEvent& e) { scene.get_entity(entity_id).translate({0.0f, 0.0f, 0.1f * e.scroll.yoffset}); return true; }
	);
	surface.add_event_listener(
		EventType::MOUSE_MOVE,
		[&](const SurfaceEvent& e) {
			auto& em = surface.get_event_manager();
			if (em.mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
				double dx, dy;
				em.mouse_delta(dx, dy);
				if (em.check_modifiers(GLFW_MOD_CONTROL)) {
					scene.get_entity(entity_id).translate({ static_cast<float_t>(dx * 0.001f), static_cast<float_t>(-dy * 0.001f), 0.0f });
				}
				else {
					scene.get_entity(entity_id).rotate({ static_cast<float>(dy * 0.1f), static_cast<float>(dx * 0.1f), 0.0f });
				}
				return true;
			}
			return false; // Let the event pass through if the button isn't held
		}
	);

	// main render loop
	while (!rd.get_surface().window_should_close()) {
		rd.get_surface().poll_events();
		if (rd.render_next_frame(tl_semaphore)) {
			rd.log_fps(100);
		}
	}
}
#endif
