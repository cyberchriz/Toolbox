#ifndef PARTICLES_TEST_H
#define PARTICLES_TEST_H

#include <vkcontext.h>

void particles_test() {

	// setup environment
	VkExtent2D extent = { 3840, 2160 };
	VulkanManager& manager = VulkanManager::get_singleton();
	Device& device = manager.get_device();
	Semaphore& tl_semaphore = manager.get_timeline_semaphore(0, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
	Scene scene;
	Renderer rd(scene, extent);
	Surface& surface = manager.get_surface(extent.width, extent.height, "Vulkan Graphics ParticleSystem Test");
	rd.set_surface(surface);
	rd.set_fps(200);

	// particle system
	Mesh particle(device, "resources/models/obj/DefaultObjects/Metalball.obj", tl_semaphore, true, true);
	particle.set_mass_kg(50.0f);
	Material& mat = particle.get_materials()[0];
	mat.base_color = { 1.0f, 1.0f, 1.0f, 1.0f};
	mat.metallic = 1.0f;
	mat.roughness = 0.2f;
	mat.specular = { 0.8f, 0.8f, 0.8f, 1.0f };
	mat.glossiness_factor = 1.0f;
	mat.shininess = 1.0f;
	mat.illum = 4;
	uint32_t spawner_entID = scene.add_entity(particle, { 0,0,0 }, false);
	Entity& spawner_ent = scene.get_entity(spawner_entID);
	spawner_ent.enable_physics(false);
	ParticleSystem& particle_sys = scene.add_particle_system(spawner_ent);
	ParticleConfig config;
	config.gravity_enabled = true;
	config.initial_velocity_minmax = { 10, 10 };
	config.lifetime_minmax = { 20,20 };
	config.scale_minmax = { 1.0f, 1.0f };
	config.src_offset_sigma = 0.0f;
	config.tumble_strength = 1.0f;
	config.cone_angle_degrees = 360.0f;
	particle_sys.add_particles(particle, 1000, config);
	particle_sys.emit({ 1000, 0, 0 }, 30.0f);

	// set scene details
	scene.get_active_camera().set_world_up(UpAxis::Y_UP);
	scene.get_active_camera().set_position({ 0.0f, 0.0f, 20.0f });
	scene.get_active_camera().look_at(spawner_ent);
	scene.get_active_camera().set_aspect_ratio(extent);
	scene.get_active_camera().set_near_plane(0.01f);
	scene.get_active_camera().set_far_plane(100.0f);
	scene.set_ambient({ 0.0f, 0.0f, 0.0f });
	scene.set_exposure(0.5f);
	scene.set_contrast(1.0f);
	scene.set_ibl_intensity(0.5f);

	// add a front spot light
	uint32_t light_id = scene.add_scene_light(LightType::DIRECTIONAL_LIGHT, { 50.0f, 0.0f, 100.0f });
	scene.get_scene_light(light_id).point_to(spawner_ent);
	scene.get_scene_light(light_id).set_intensity(20.0f);

	// Add Event Listeners for Scroll / Translate / Rotate / WindowClose / WindowResize
	surface.add_event_listener(
		EventType::WINDOW_RESIZE,
		[&](const SurfaceEvent& e) {extent = { e.resize.width, e.resize.height }; rd.get_swapchain().update_extent(extent);	return true; }
	);
	surface.add_event_listener(
		EventType::WINDOW_CLOSE,
		[&](const SurfaceEvent& e) { rd.get_swapchain().destroy(); rd.get_surface().close(); return true; }
	);
	surface.add_event_listener(
		EventType::MOUSE_SCROLL,
		[&](const SurfaceEvent& e) { spawner_ent.translate({ 0.0f, 0.0f, 0.1f * e.scroll.yoffset }); return true; }
	);

	// main render loop
	while (!surface.window_should_close()) {
		surface.poll_events();
		if (rd.render_next_frame(tl_semaphore)) {
			rd.log_fps(100);
		}
	}
}
#endif
