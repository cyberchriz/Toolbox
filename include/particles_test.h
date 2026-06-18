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

	// Floor
	Mesh floor(device, "resources/models/obj/DefaultObjects/default_cube.obj", tl_semaphore, true, true, 0, UpAxis::Y_UP, 1.0f);
	floor.set_bounce_restitution(0.1f);
	floor.make_solid(true);
	floor.set_surface_friction(1.0f);
	Material& fmat = floor.get_materials()[0];
	fmat.base_color = { 0.0f, 1.0f, 1.0f, 1.0f };
	uint32_t floor_entID = scene.add_entity(floor, { 0.0f, -25.0f, 0.0f }, true);
	scene.get_entity(floor_entID).set_scale({100,1,100});
	scene.get_entity(floor_entID).enable_physics(false); // make stationary

	// particle system
	Mesh particle(device, "resources/models/obj/DefaultObjects/Metalball.obj", tl_semaphore, true, true, 0, UpAxis::Y_UP, 1.0f);
	particle.set_mass_kg(50.0f);
	particle.set_surface_friction(1.0f);
	particle.set_bounce_restitution(0.8f);
	particle.make_solid(true);
	Material& pmat = particle.get_materials()[0];
	pmat.base_color = { 1.0f, 1.0f, 1.0f, 1.0f};
	pmat.metallic = 1.0f;
	pmat.roughness = 0.1f;
	pmat.specular = { 0.8f, 0.8f, 0.8f, 1.0f };
	pmat.glossiness_factor = 1.0f;
	pmat.shininess = 1.0f;
	pmat.illum = 4;
	glm::vec3 spawn_pos = { 0.0f, 0.0f, 0.0f };
	ParticleSystem& particle_sys = scene.add_particle_system(spawn_pos);

	ParticleConfig config;
	config.gravity_enabled = true;
	config.initial_velocity_minmax = { 5, 5 };
	config.lifetime_minmax = { 20,20 };
	config.scale_minmax = { 1.5f, 3.5f };
	config.src_offset_sigma = 0.0f;
	config.tumble_strength = 0.0f;
	config.cone_angle_degrees = 360.0f;
	config.mass_changes_with_scale = true;
	particle_sys.add_particles(particle, 2000, config);
	particle_sys.emit({ 0, 1, 0 }, 10.0f);

	// set scene details
	Camera& cam = scene.get_active_camera();
	cam.set_world_up(UpAxis::Y_UP);
	cam.set_position({ 0.0f, 0.0f, 100.0f });
	cam.look_at(spawn_pos);
	cam.set_aspect_ratio(extent);
	cam.set_near_plane(0.01f);
	cam.set_far_plane(500.0f);
	scene.set_ambient({ 0.0f, 0.0f, 0.0f });
	scene.set_exposure(0.5f);
	scene.set_contrast(1.0f);
	scene.set_ibl_intensity(0.5f);

	// add a front spot light
	uint32_t light_id = scene.add_scene_light(LightType::DIRECTIONAL_LIGHT, { 50.0f, 0.0f, 100.0f });
	scene.get_scene_light(light_id).point_to(spawn_pos);
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
		[&](const SurfaceEvent& e) { cam.set_fov(cam.get_fov() + 0.1f * e.scroll.yoffset); return true; }
	);
	surface.add_event_listener(
		EventType::MOUSE_MOVE,
		[&](const SurfaceEvent& e) {
			auto& em = surface.get_event_manager();
			if (em.mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
				double dx, dy;
				em.mouse_delta(dx, dy);
				if (em.check_modifiers(GLFW_MOD_CONTROL)) {
					cam.translate({ static_cast<float_t>(dx * 0.1f), static_cast<float_t>(-dy * 0.1f), 0.0f });
				}
				else {
					cam.rotate(static_cast<float>(dy * 0.1f), static_cast<float>(dx * 0.1f), 0.0f);
				}
				return true;
			}
			return false; // Let the event pass through if the button isn't held
		}
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
