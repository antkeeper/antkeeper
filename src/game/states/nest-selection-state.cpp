/*
 * Copyright (C) 2023  Christopher J. Howard
 *
 * This file is part of Antkeeper source code.
 *
 * Antkeeper source code is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Antkeeper source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Antkeeper source code.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "game/ant/ant-cladogenesis.hpp"
#include "game/ant/ant-genome.hpp"
#include "game/ant/ant-morphogenesis.hpp"
#include "game/ant/ant-phenome.hpp"
#include "game/commands/commands.hpp"
#include "game/components/camera-component.hpp"
#include "game/components/constraint-stack-component.hpp"
#include "game/components/scene-component.hpp"
#include "game/components/picking-component.hpp"
#include "game/components/spring-component.hpp"
#include "game/components/physics-component.hpp"
#include "game/components/steering-component.hpp"
#include "game/components/terrain-component.hpp"
#include "game/components/legged-locomotion-component.hpp"
#include "game/components/transform-component.hpp"
#include "game/constraints/child-of-constraint.hpp"
#include "game/constraints/copy-rotation-constraint.hpp"
#include "game/constraints/copy-scale-constraint.hpp"
#include "game/constraints/copy-transform-constraint.hpp"
#include "game/constraints/copy-translation-constraint.hpp"
#include "game/constraints/ease-to-constraint.hpp"
#include "game/constraints/pivot-constraint.hpp"
#include "game/constraints/spring-rotation-constraint.hpp"
#include "game/constraints/spring-to-constraint.hpp"
#include "game/constraints/spring-translation-constraint.hpp"
#include "game/constraints/three-dof-constraint.hpp"
#include "game/constraints/track-to-constraint.hpp"
#include "game/controls.hpp"
#include "game/spawn.hpp"
#include "game/states/nest-selection-state.hpp"
#include "game/states/pause-menu-state.hpp"
#include "game/systems/astronomy-system.hpp"
#include "game/systems/atmosphere-system.hpp"
#include "game/systems/camera-system.hpp"
#include "game/systems/collision-system.hpp"
#include "game/world.hpp"
#include <engine/animation/ease.hpp>
#include <engine/animation/screen-transition.hpp>
#include <engine/config.hpp>
#include <engine/entity/archetype.hpp>
#include <engine/input/mouse.hpp>
#include <engine/math/interpolation.hpp>
#include <engine/math/projection.hpp>
#include <engine/physics/light/exposure.hpp>
#include <engine/render/passes/clear-pass.hpp>
#include <engine/render/passes/ground-pass.hpp>
#include <engine/resources/resource-manager.hpp>
#include <engine/utility/state-machine.hpp>

nest_selection_state::nest_selection_state(::game& ctx):
	game_state(ctx)
{
	debug::log::trace("Entering nest selection state...");
	
	// Create world if not yet created
	if (ctx.entities.find("earth") == ctx.entities.end())
	{
		// Create cosmos
		::world::cosmogenesis(ctx);
		
		// Create observer
		::world::create_observer(ctx);
	}
	
	ctx.active_ecoregion = ctx.resource_manager->load<::ecoregion>("seedy-scrub.eco");
	::world::enter_ecoregion(ctx, *ctx.active_ecoregion);
	
	debug::log::trace("Generating genome...");
	std::unique_ptr<ant_genome> genome = ant_cladogenesis(ctx.active_ecoregion->gene_pools[0], ctx.rng);
	debug::log::trace("Generated genome");
	
	debug::log::trace("Building worker phenome...");
	ant_phenome worker_phenome = ant_phenome(*genome, ant_caste_type::worker);
	debug::log::trace("Built worker phenome...");
	
	debug::log::trace("Generating worker model...");
	std::shared_ptr<render::model> worker_model = ant_morphogenesis(worker_phenome);
	debug::log::trace("Generated worker model");
	
	// Create worker entity(s)
	worker_ant_eid = ctx.entity_registry->create();
	transform_component worker_transform_component;
	worker_transform_component.local = math::transform<float>::identity;
	worker_transform_component.local.translation = {0, 0.5f, -4};
	worker_transform_component.world = worker_transform_component.local;
	worker_transform_component.warp = true;
	ctx.entity_registry->emplace<transform_component>(worker_ant_eid, worker_transform_component);
	
	ctx.entity_registry->emplace<scene_component>(worker_ant_eid, std::make_unique<scene::static_mesh>(worker_model), std::uint8_t{1});
	
	// Disable UI color clear
	ctx.ui_clear_pass->set_cleared_buffers(false, true, false);
	
	// Set world time
	::world::set_time(ctx, 2022, 6, 21, 12, 0, 0.0);
	
	// Init time scale
	double time_scale = 60.0;
	
	// Set time scale
	::world::set_time_scale(ctx, time_scale);
	
	// Setup and enable sky and ground passes
	ctx.sky_pass->set_enabled(true);
	ctx.ground_pass->set_enabled(true);
	
	// Switch to surface camera
	ctx.underground_camera->set_active(false);
	ctx.surface_camera->set_active(true);
	
	// Set camera exposure
	const float ev100_sunny16 = physics::light::ev::from_settings(16.0f, 1.0f / 100.0f, 100.0f);
	ctx.surface_camera->set_exposure(ev100_sunny16);
	
	const auto& viewport_size = ctx.window->get_viewport_size();
	const float aspect_ratio = static_cast<float>(viewport_size[0]) / static_cast<float>(viewport_size[1]);
	
	// Init first person camera rig parameters
	first_person_camera_rig_translation_spring_angular_frequency = period_to_rads(0.125f);
	first_person_camera_rig_rotation_spring_angular_frequency = period_to_rads(0.125f);
	first_person_camera_rig_fov_spring_angular_frequency = period_to_rads(0.125f);
	first_person_camera_rig_min_elevation = 0.25f;
	first_person_camera_rig_max_elevation = 150.0f;
	first_person_camera_near_fov = math::vertical_fov(math::radians(100.0f), aspect_ratio);
	first_person_camera_far_fov = math::vertical_fov(math::radians(60.0f), aspect_ratio);
	first_person_camera_near_speed = 5.0f;
	first_person_camera_far_speed = 140.0f;
	first_person_camera_rig_pedestal_speed = 2.0f;
	first_person_camera_rig_pedestal = 0.0f;
	
	// Create first person camera rig
	create_first_person_camera_rig();
	
	// Satisfy first person camera rig constraints
	satisfy_first_person_camera_rig_constraints();
	
	// Setup controls
	setup_controls();
	
	// auto color_checker_archetype = ctx.resource_manager->load<entity::archetype>("color-checker.ent");
	// color_checker_archetype->create(*ctx.entity_registry);
	// auto ruler_archetype = ctx.resource_manager->load<entity::archetype>("ruler-10cm.ent");
	// ruler_archetype->create(*ctx.entity_registry);
	
	auto plane_archetype = ctx.resource_manager->load<entity::archetype>("desert-scrub-plane.ent");
	auto plane_eid = plane_archetype->create(*ctx.entity_registry);
	
	auto yucca_archetype = ctx.resource_manager->load<entity::archetype>("yucca-plant-l.ent");
	auto yucca_eid = yucca_archetype->create(*ctx.entity_registry);
	::command::warp_to(*ctx.entity_registry, yucca_eid, {0, 0, 30});
	
	yucca_archetype = ctx.resource_manager->load<entity::archetype>("yucca-plant-m.ent");
	yucca_eid = yucca_archetype->create(*ctx.entity_registry);
	::command::warp_to(*ctx.entity_registry, yucca_eid, {400, 0, 200});
	
	yucca_archetype = ctx.resource_manager->load<entity::archetype>("yucca-plant-s.ent");
	yucca_eid = yucca_archetype->create(*ctx.entity_registry);
	::command::warp_to(*ctx.entity_registry, yucca_eid, {-300, 0, -300});
	
	auto cactus_plant_archetype = ctx.resource_manager->load<entity::archetype>("barrel-cactus-plant-l.ent");
	auto cactus_plant_eid = cactus_plant_archetype->create(*ctx.entity_registry);
	::command::warp_to(*ctx.entity_registry, cactus_plant_eid, {-100, 0, -200});
	
	cactus_plant_archetype = ctx.resource_manager->load<entity::archetype>("barrel-cactus-plant-m.ent");
	cactus_plant_eid = cactus_plant_archetype->create(*ctx.entity_registry);
	::command::warp_to(*ctx.entity_registry, cactus_plant_eid, {100, 0, -70});
	
	cactus_plant_archetype = ctx.resource_manager->load<entity::archetype>("barrel-cactus-plant-s.ent");
	cactus_plant_eid = cactus_plant_archetype->create(*ctx.entity_registry);
	::command::warp_to(*ctx.entity_registry, cactus_plant_eid, {50, 0, 80});
	
	auto cactus_seed_archetype = ctx.resource_manager->load<entity::archetype>("barrel-cactus-seed.ent");
	auto cactus_seed_eid = cactus_seed_archetype->create(*ctx.entity_registry);
	::command::warp_to(*ctx.entity_registry, cactus_seed_eid, {10, 0, 10});
	
	// Queue enable game controls
	ctx.function_queue.push
	(
		[&ctx]()
		{
			::enable_game_controls(ctx);
			::enable_keeper_controls(ctx);
		}
	);
	
	// Queue fade in
	ctx.fade_transition_color->set({0, 0, 0});
	ctx.function_queue.push(std::bind(&screen_transition::transition, ctx.fade_transition.get(), 1.0f, true, ease<float>::out_sine, true, nullptr));
	
	debug::log::trace("Entered nest selection state");
}

nest_selection_state::~nest_selection_state()
{
	debug::log::trace("Exiting nest selection state...");
	
	// Disable game controls
	::disable_game_controls(ctx);
	::disable_keeper_controls(ctx);
	
	destroy_first_person_camera_rig();
	
	debug::log::trace("Exited nest selection state");
}

void nest_selection_state::create_first_person_camera_rig()
{
	// Construct first person camera rig track to constraint
	track_to_constraint first_person_camera_rig_track_to;
	first_person_camera_rig_track_to.target = worker_ant_eid;
	first_person_camera_rig_track_to.up = {0.0f, 1.0f, 0.0f};
	
	constraint_stack_node_component first_person_camera_rig_track_to_node;
	first_person_camera_rig_track_to_node.active = false;
	first_person_camera_rig_track_to_node.weight = 1.0f;
	first_person_camera_rig_track_to_node.next = entt::null;
	first_person_camera_rig_track_to_eid = ctx.entity_registry->create();
	ctx.entity_registry->emplace<track_to_constraint>(first_person_camera_rig_track_to_eid, first_person_camera_rig_track_to);
	ctx.entity_registry->emplace<constraint_stack_node_component>(first_person_camera_rig_track_to_eid, first_person_camera_rig_track_to_node);
	
	// Construct first person camera rig spring rotation constraint
	spring_rotation_constraint first_person_camera_rig_spring_rotation;
	first_person_camera_rig_spring_rotation.spring =
	{
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		1.0f,
		first_person_camera_rig_rotation_spring_angular_frequency
	};
	constraint_stack_node_component first_person_camera_rig_spring_rotation_node;
	first_person_camera_rig_spring_rotation_node.active = true;
	first_person_camera_rig_spring_rotation_node.weight = 1.0f;
	first_person_camera_rig_spring_rotation_node.next = first_person_camera_rig_track_to_eid;
	first_person_camera_rig_spring_rotation_eid = ctx.entity_registry->create();
	ctx.entity_registry->emplace<spring_rotation_constraint>(first_person_camera_rig_spring_rotation_eid, first_person_camera_rig_spring_rotation);
	ctx.entity_registry->emplace<constraint_stack_node_component>(first_person_camera_rig_spring_rotation_eid, first_person_camera_rig_spring_rotation_node);
	
	// Construct first person camera rig spring translation constraint
	spring_translation_constraint first_person_camera_rig_spring_translation;
	first_person_camera_rig_spring_translation.spring =
	{
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		1.0f,
		first_person_camera_rig_translation_spring_angular_frequency
	};
	constraint_stack_node_component first_person_camera_rig_spring_translation_node;
	first_person_camera_rig_spring_translation_node.active = false;
	first_person_camera_rig_spring_translation_node.weight = 1.0f;
	first_person_camera_rig_spring_translation_node.next = first_person_camera_rig_spring_rotation_eid;
	first_person_camera_rig_spring_translation_eid = ctx.entity_registry->create();
	ctx.entity_registry->emplace<spring_translation_constraint>(first_person_camera_rig_spring_translation_eid, first_person_camera_rig_spring_translation);
	ctx.entity_registry->emplace<constraint_stack_node_component>(first_person_camera_rig_spring_translation_eid, first_person_camera_rig_spring_translation_node);
	
	// Construct first person camera rig constraint stack
	constraint_stack_component first_person_camera_rig_constraint_stack;
	first_person_camera_rig_constraint_stack.priority = 2;
	first_person_camera_rig_constraint_stack.head = first_person_camera_rig_spring_translation_eid;
	
	// Construct first person camera rig transform component
	transform_component first_person_camera_rig_transform;
	first_person_camera_rig_transform.local = math::transform<float>::identity;
	first_person_camera_rig_transform.world = first_person_camera_rig_transform.local;
	first_person_camera_rig_transform.warp = true;
	
	// Construct first person camera rig legged locomotion component
	legged_locomotion_component first_person_camera_rig_locomotion;
	
	// Construct first person camera rig physics component
	physics_component first_person_camera_rig_physics;
	first_person_camera_rig_physics.mass = 90.0f;
	
	// Construct first person camera rig camera component
	camera_component first_person_camera_rig_camera;
	first_person_camera_rig_camera.object = ctx.surface_camera.get();
	
	// Construct first person camera rig entity
	first_person_camera_rig_eid = ctx.entity_registry->create();
	ctx.entity_registry->emplace<camera_component>(first_person_camera_rig_eid, first_person_camera_rig_camera);
	ctx.entity_registry->emplace<transform_component>(first_person_camera_rig_eid, first_person_camera_rig_transform);
	ctx.entity_registry->emplace<physics_component>(first_person_camera_rig_eid, first_person_camera_rig_physics);
	ctx.entity_registry->emplace<legged_locomotion_component>(first_person_camera_rig_eid, first_person_camera_rig_locomotion);
	ctx.entity_registry->emplace<constraint_stack_component>(first_person_camera_rig_eid, first_person_camera_rig_constraint_stack);
	
	// Construct first person camera rig fov spring
	spring1_component first_person_camera_rig_fov_spring;
	first_person_camera_rig_fov_spring.spring =
	{
		0.0f,
		0.0f,
		0.0f,
		1.0f,
		first_person_camera_rig_fov_spring_angular_frequency
	};
	first_person_camera_rig_fov_spring.callback = [&](float fov)
	{
		ctx.surface_camera->set_perspective(fov, ctx.surface_camera->get_aspect_ratio(), ctx.surface_camera->get_clip_near(), ctx.surface_camera->get_clip_far());
	};
	
	// Construct first person camera rig fov spring entity
	first_person_camera_rig_fov_spring_eid = ctx.entity_registry->create();
	ctx.entity_registry->emplace<spring1_component>(first_person_camera_rig_fov_spring_eid, first_person_camera_rig_fov_spring);
	
	set_first_person_camera_rig_pedestal(first_person_camera_rig_pedestal);
}

void nest_selection_state::destroy_first_person_camera_rig()
{
	ctx.entity_registry->destroy(first_person_camera_rig_eid);
	ctx.entity_registry->destroy(first_person_camera_rig_spring_translation_eid);
	ctx.entity_registry->destroy(first_person_camera_rig_spring_rotation_eid);
	ctx.entity_registry->destroy(first_person_camera_rig_fov_spring_eid);
}

void nest_selection_state::set_first_person_camera_rig_pedestal(float pedestal)
{
	first_person_camera_rig_pedestal = pedestal;
	const float elevation = math::log_lerp(first_person_camera_rig_min_elevation, first_person_camera_rig_max_elevation, first_person_camera_rig_pedestal);
	const float fov = math::log_lerp(first_person_camera_near_fov, first_person_camera_far_fov, first_person_camera_rig_pedestal);
	
	ctx.entity_registry->patch<spring_translation_constraint>
	(
		first_person_camera_rig_spring_translation_eid,
		[&](auto& component)
		{
			component.spring.x1[1] = elevation;
		}
	);
	
	ctx.entity_registry->patch<spring1_component>
	(
		first_person_camera_rig_fov_spring_eid,
		[&](auto& component)
		{
			component.spring.x1 = fov;
		}
	);
}

void nest_selection_state::move_first_person_camera_rig(const float2& direction, float factor)
{
	const float speed = math::log_lerp(first_person_camera_near_speed, first_person_camera_far_speed, first_person_camera_rig_pedestal) * factor;
	
	const spring_rotation_constraint& first_person_camera_rig_spring_rotation = ctx.entity_registry->get<spring_rotation_constraint>(first_person_camera_rig_spring_rotation_eid);
	
	const math::quaternion<float> yaw_rotation = math::angle_axis(first_person_camera_rig_spring_rotation.spring.x0[0], float3{0.0f, 1.0f, 0.0f});
	const float3 rotated_direction = math::normalize(yaw_rotation * float3{direction[0], 0.0f, direction[1]});
	const float3 velocity = rotated_direction * speed;
	
	ctx.entity_registry->patch<spring_translation_constraint>
	(
		first_person_camera_rig_spring_translation_eid,
		[&](auto& component)
		{
			component.spring.x1 += velocity * static_cast<float>(ctx.loop.get_update_period());
		}
	);
}

void nest_selection_state::satisfy_first_person_camera_rig_constraints()
{
	// Satisfy first person camera rig spring translation constraint
	ctx.entity_registry->patch<spring_translation_constraint>
	(
		first_person_camera_rig_spring_translation_eid,
		[&](auto& component)
		{
			component.spring.x0 = component.spring.x1;
			component.spring.v *= 0.0f;
		}
	);
	
	// Satisfy first person camera rig spring rotation constraint
	ctx.entity_registry->patch<spring_rotation_constraint>
	(
		first_person_camera_rig_spring_rotation_eid,
		[&](auto& component)
		{
			component.spring.x0 = component.spring.x1;
			component.spring.v *= 0.0f;
		}
	);
	
	// Satisfy first person camera rig fov spring
	ctx.entity_registry->patch<spring1_component>
	(
		first_person_camera_rig_fov_spring_eid,
		[&](auto& component)
		{
			component.spring.x0 = component.spring.x1;
			component.spring.v *= 0.0f;
		}
	);
}

void nest_selection_state::setup_controls()
{
	// Enable/toggle mouse look
	action_subscriptions.emplace_back
	(
		ctx.mouse_look_action.get_activated_channel().subscribe
		(
			[&](const auto& event)
			{
				if (ctx.toggle_mouse_look)
				{
					mouse_look = !mouse_look;
				}
				else
				{
					mouse_look = true;
				}
				
				//ctx.input_manager->set_cursor_visible(!mouse_look);
				ctx.input_manager->set_relative_mouse_mode(mouse_look);
			}
		)
	);
	
	// Disable mouse look
	action_subscriptions.emplace_back
	(
		ctx.mouse_look_action.get_deactivated_channel().subscribe
		(
			[&](const auto& event)
			{
				if (!ctx.toggle_mouse_look && mouse_look)
				{
					mouse_look = false;
					//ctx.input_manager->set_cursor_visible(true);
					ctx.input_manager->set_relative_mouse_mode(false);
				}
			}
		)
	);
	
	// Mouse look
	mouse_motion_subscription = ctx.input_manager->get_event_queue().subscribe<input::mouse_moved_event>
	(
		[&](const auto& event)
		{
			if (!mouse_look)
			{
				return;
			}
			
			const float mouse_tilt_factor = ctx.mouse_tilt_sensitivity * 0.01f * (ctx.invert_mouse_tilt ? -1.0f : 1.0f);
			const float mouse_pan_factor = ctx.mouse_pan_sensitivity * 0.01f * (ctx.invert_mouse_pan ? -1.0f : 1.0f);
			
			ctx.entity_registry->patch<spring_rotation_constraint>
			(
				first_person_camera_rig_spring_rotation_eid,
				[&](auto& component)
				{
					component.spring.x1[0] -= mouse_pan_factor * static_cast<float>(event.difference.x());
					component.spring.x1[1] += mouse_tilt_factor * static_cast<float>(event.difference.y());
					component.spring.x1[1] = std::min(math::half_pi<float>, std::max(-math::half_pi<float>, component.spring.x1[1]));
				}
			);
		}
	);
	
	constexpr float movement_speed = 10000.0f;
	
	auto move_first_person_camera_rig = [&](const float2& direction, float speed)
	{
		const spring_rotation_constraint& first_person_camera_rig_spring_rotation = ctx.entity_registry->get<spring_rotation_constraint>(first_person_camera_rig_spring_rotation_eid);
		
		const math::quaternion<float> yaw_rotation = math::angle_axis(first_person_camera_rig_spring_rotation.spring.x0[0], float3{0.0f, 1.0f, 0.0f});
		const float3 rotated_direction = yaw_rotation * float3{direction[0], 0.0f, direction[1]};
		
		const float3 force = rotated_direction * speed;
		
		ctx.entity_registry->patch<legged_locomotion_component>
		(
			first_person_camera_rig_eid,
			[&](auto& component)
			{
				component.force = force;
			}
		);
	};
	
	auto stop_first_person_camera_rig = [&]()
	{
		ctx.entity_registry->patch<legged_locomotion_component>
		(
			first_person_camera_rig_eid,
			[&](auto& component)
			{
				component.force = {0.0f, 0.0f, 0.0f};
			}
		);
	};
	
	// Move forward
	action_subscriptions.emplace_back
	(
		ctx.move_forward_action.get_active_channel().subscribe
		(
			[&, move_first_person_camera_rig](const auto& event)
			{
				move_first_person_camera_rig({0.0f, -1.0f}, movement_speed * event.input_value);
			}
		)
	);
	action_subscriptions.emplace_back
	(
		ctx.move_forward_action.get_deactivated_channel().subscribe
		(
			[&, stop_first_person_camera_rig](const auto& event)
			{
				stop_first_person_camera_rig();
			}
		)
	);
	
	// Move back
	action_subscriptions.emplace_back
	(
		ctx.move_back_action.get_active_channel().subscribe
		(
			[&, move_first_person_camera_rig](const auto& event)
			{
				move_first_person_camera_rig({0, 1}, movement_speed * event.input_value);
			}
		)
	);
	action_subscriptions.emplace_back
	(
		ctx.move_back_action.get_deactivated_channel().subscribe
		(
			[&, stop_first_person_camera_rig](const auto& event)
			{
				stop_first_person_camera_rig();
			}
		)
	);
	
	// Move left
	action_subscriptions.emplace_back
	(
		ctx.move_left_action.get_active_channel().subscribe
		(
			[&, move_first_person_camera_rig](const auto& event)
			{
				move_first_person_camera_rig({-1, 0}, movement_speed * event.input_value);
			}
		)
	);
	action_subscriptions.emplace_back
	(
		ctx.move_left_action.get_deactivated_channel().subscribe
		(
			[&, stop_first_person_camera_rig](const auto& event)
			{
				stop_first_person_camera_rig();
			}
		)
	);
	
	// Move right
	action_subscriptions.emplace_back
	(
		ctx.move_right_action.get_active_channel().subscribe
		(
			[&, move_first_person_camera_rig](const auto& event)
			{
				move_first_person_camera_rig({1, 0}, movement_speed * event.input_value);
			}
		)
	);
	action_subscriptions.emplace_back
	(
		ctx.move_right_action.get_deactivated_channel().subscribe
		(
			[&, stop_first_person_camera_rig](const auto& event)
			{
				stop_first_person_camera_rig();
			}
		)
	);
	
	// Move up
	action_subscriptions.emplace_back
	(
		ctx.move_up_action.get_active_channel().subscribe
		(
			[&](const auto& event)
			{
				ctx.entity_registry->patch<physics_component>
				(
					first_person_camera_rig_eid,
					[&](auto& component)
					{
						component.force += float3{0, movement_speed * event.input_value, 0};
					}
				);
			}
		)
	);
	
	// Move down
	action_subscriptions.emplace_back
	(
		ctx.move_down_action.get_active_channel().subscribe
		(
			[&](const auto& event)
			{
				ctx.entity_registry->patch<physics_component>
				(
					first_person_camera_rig_eid,
					[&](auto& component)
					{
						component.force -= float3{0, movement_speed * event.input_value, 0};
					}
				);
			}
		)
	);
	
	// Focus
	action_subscriptions.emplace_back
	(
		ctx.focus_action.get_activated_channel().subscribe
		(
			[&](const auto& event)
			{
				ctx.entity_registry->patch<constraint_stack_node_component>
				(
					first_person_camera_rig_track_to_eid,
					[&](auto& component)
					{
						component.active = true;
					}
				);
			}
		)
	);
	action_subscriptions.emplace_back
	(
		ctx.focus_action.get_deactivated_channel().subscribe
		(
			[&](const auto& event)
			{
				ctx.entity_registry->patch<constraint_stack_node_component>
				(
					first_person_camera_rig_track_to_eid,
					[&](auto& component)
					{
						component.active = false;
					}
				);
			}
		)
	);
}

void nest_selection_state::enable_controls()
{
	/*
	// Reset mouse look
	mouse_look = false;
	
	double time_scale = 0.0;
	double ff_time_scale = 60.0 * 200.0;
	
	// Init control settings
	float mouse_tilt_sensitivity = 1.0f;
	float mouse_pan_sensitivity = 1.0f;
	bool mouse_invert_tilt = false;
	bool mouse_invert_pan = false;
	bool mouse_look_toggle = false;
	float gamepad_tilt_sensitivity = 1.0f;
	float gamepad_pan_sensitivity = 1.0f;
	bool gamepad_invert_tilt = false;
	bool gamepad_invert_pan = false;
	
	// Read control settings
	if (ctx.config->contains("mouse_tilt_sensitivity"))
		mouse_tilt_sensitivity = math::radians((*ctx.config)["mouse_tilt_sensitivity"].get<float>());
	if (ctx.config->contains("mouse_pan_sensitivity"))
		mouse_pan_sensitivity = math::radians((*ctx.config)["mouse_pan_sensitivity"].get<float>());
	if (ctx.config->contains("mouse_invert_tilt"))
		mouse_invert_tilt = (*ctx.config)["mouse_invert_tilt"].get<bool>();
	if (ctx.config->contains("mouse_invert_pan"))
		mouse_invert_pan = (*ctx.config)["mouse_invert_pan"].get<bool>();
	if (ctx.config->contains("mouse_look_toggle"))
		mouse_look_toggle = (*ctx.config)["mouse_look_toggle"].get<bool>();
	if (ctx.config->contains("gamepad_tilt_sensitivity"))
		gamepad_tilt_sensitivity = math::radians((*ctx.config)["gamepad_tilt_sensitivity"].get<float>());
	if (ctx.config->contains("gamepad_pan_sensitivity"))
		gamepad_pan_sensitivity = math::radians((*ctx.config)["gamepad_pan_sensitivity"].get<float>());
	if (ctx.config->contains("gamepad_invert_tilt"))
		gamepad_invert_tilt = (*ctx.config)["gamepad_invert_tilt"].get<bool>();
	if (ctx.config->contains("gamepad_invert_pan"))
		gamepad_invert_pan = (*ctx.config)["gamepad_invert_pan"].get<bool>();
	
	// Determine tilt and pan factors according to sensitivity and inversion
	const float mouse_tilt_factor = mouse_tilt_sensitivity * (mouse_invert_tilt ? -1.0f : 1.0f);
	const float mouse_pan_factor = mouse_pan_sensitivity * (mouse_invert_pan ? -1.0f : 1.0f);
	const float gamepad_tilt_factor = gamepad_tilt_sensitivity * (gamepad_invert_tilt ? -1.0f : 1.0f);
	const float gamepad_pan_factor = gamepad_pan_sensitivity * (gamepad_invert_pan ? -1.0f : 1.0f);
	
	
	ctx.controls["look_right_gamepad"]->set_active_callback
	(
		[&, gamepad_pan_factor](float value)
		{
			ctx.entity_registry->patch<spring_rotation_constraint>
			(
				first_person_camera_rig_spring_rotation_eid,
				[&, gamepad_pan_factor](auto& component)
				{
					component.spring.x1[0] -= gamepad_pan_factor * value * static_cast<float>(ctx.loop.get_update_period());
				}
			);
		}
	);
	ctx.controls["look_left_gamepad"]->set_active_callback
	(
		[&, gamepad_pan_factor](float value)
		{
			ctx.entity_registry->patch<spring_rotation_constraint>
			(
				first_person_camera_rig_spring_rotation_eid,
				[&, gamepad_pan_factor](auto& component)
				{
					component.spring.x1[0] += gamepad_pan_factor * value * static_cast<float>(ctx.loop.get_update_period());
				}
			);
		}
	);
	ctx.controls["look_up_gamepad"]->set_active_callback
	(
		[&, gamepad_tilt_factor](float value)
		{
			ctx.entity_registry->patch<spring_rotation_constraint>
			(
				first_person_camera_rig_spring_rotation_eid,
				[&, gamepad_tilt_factor](auto& component)
				{
					component.spring.x1[1] -= gamepad_tilt_factor * value * static_cast<float>(ctx.loop.get_update_period());
					component.spring.x1[1] = std::max(-math::half_pi<float>, component.spring.x1[1]);
				}
			);
		}
	);
	ctx.controls["look_down_gamepad"]->set_active_callback
	(
		[&, gamepad_tilt_factor](float value)
		{
			ctx.entity_registry->patch<spring_rotation_constraint>
			(
				first_person_camera_rig_spring_rotation_eid,
				[&, gamepad_tilt_factor](auto& component)
				{
					component.spring.x1[1] += gamepad_tilt_factor * value * static_cast<float>(ctx.loop.get_update_period());
					component.spring.x1[1] = std::min(math::half_pi<float>, component.spring.x1[1]);
				}
			);
		}
	);
	
	// Pedestal up control
	ctx.controls["move_up"]->set_active_callback
	(
		[&](float value)
		{
			set_first_person_camera_rig_pedestal(std::min(1.0f, first_person_camera_rig_pedestal + first_person_camera_rig_pedestal_speed * static_cast<float>(ctx.loop.get_update_period())));
		}
	);
	
	// Pedestal down control
	ctx.controls["move_down"]->set_active_callback
	(
		[&](float value)
		{
			set_first_person_camera_rig_pedestal(std::max(0.0f, first_person_camera_rig_pedestal - first_person_camera_rig_pedestal_speed * static_cast<float>(ctx.loop.get_update_period())));
		}
	);
	
	
	// Fast-forward
	ctx.controls["fast_forward"]->set_activated_callback
	(
		[&ctx = this->ctx, ff_time_scale]()
		{
			::world::set_time_scale(ctx, ff_time_scale);
		}
	);
	ctx.controls["fast_forward"]->set_deactivated_callback
	(
		[&ctx = this->ctx, time_scale]()
		{
			::world::set_time_scale(ctx, time_scale);
		}
	);
	ctx.controls["rewind"]->set_activated_callback
	(
		[&ctx = this->ctx, ff_time_scale]()
		{
			::world::set_time_scale(ctx, -ff_time_scale);
		}
	);
	ctx.controls["rewind"]->set_deactivated_callback
	(
		[&ctx = this->ctx, time_scale]()
		{
			::world::set_time_scale(ctx, time_scale);
		}
	);
	*/
}

void nest_selection_state::disable_controls()
{
}

