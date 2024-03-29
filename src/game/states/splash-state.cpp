// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#include "game/states/splash-state.hpp"
#include "game/game.hpp"
#include "game/states/main-menu-state.hpp"
#include <engine/animation/animation.hpp>
#include <engine/animation/animator.hpp>
#include <engine/animation/ease.hpp>
#include <engine/animation/screen-transition.hpp>
#include <engine/debug/log.hpp>
#include <engine/math/vector.hpp>
#include <engine/render/material-flags.hpp>
#include <engine/resources/resource-manager.hpp>
#include <engine/gl/pipeline.hpp>

splash_state::splash_state(::game& ctx):
	game_state(ctx)
{
	debug::log_trace("Entering splash state...");
	
	const math::fvec2 viewport_size = math::fvec2(ctx.window->get_viewport_size());
	const math::fvec2 viewport_center = viewport_size * 0.5f;
	
	// Load splash texture
	auto splash_texture = ctx.resource_manager->load<gl::texture_2d>("splash.tex");
	
	// Get splash texture dimensions
	const auto& splash_dimensions = splash_texture->get_image_view()->get_image()->get_dimensions();
	
	// Construct splash billboard material
	splash_billboard_material = std::make_shared<render::material>();
	splash_billboard_material->set_blend_mode(render::material_blend_mode::translucent);
	splash_billboard_material->set_shader_template(ctx.resource_manager->load<gl::shader_template>("ui-element-textured.glsl"));
	splash_billboard_material->set_variable("background", std::make_shared<render::matvar_texture_2d>(1, splash_texture));
	
	auto splash_tint = std::make_shared<render::matvar_fvec4>(1, math::fvec4{1, 1, 1, 0});
	splash_billboard_material->set_variable("tint", splash_tint);
	
	// Construct splash billboard
	splash_billboard.set_material(splash_billboard_material);
	splash_billboard.set_scale({static_cast<float>(splash_dimensions[0]) * 0.5f, static_cast<float>(splash_dimensions[1]) * 0.5f, 1.0f});
	splash_billboard.set_translation({std::round(viewport_center.x()), std::round(viewport_center.y()), 0.0f});
	
	// Add splash billboard to UI scene
	ctx.ui_canvas->get_scene().add_object(splash_billboard);
	
	// Load animation timing configuration
	const float splash_fade_in_duration = 0.5;
	const float splash_duration = 2.0;
	const float splash_fade_out_duration = 0.5;
	
	// Construct splash fade in animation
	splash_fade_in_animation.set_interpolator(ease<float>::out_cubic);
	animation_channel<float>* splash_fade_in_opacity_channel = splash_fade_in_animation.add_channel(0);
	splash_fade_in_opacity_channel->insert_keyframe({0.0f, 0.0f});
	splash_fade_in_opacity_channel->insert_keyframe({splash_fade_in_duration, 1.0f});
	splash_fade_in_opacity_channel->insert_keyframe({splash_fade_in_duration + splash_duration, 1.0f});
	
	// Build splash fade out animation
	splash_fade_out_animation.set_interpolator(ease<float>::out_cubic);
	animation_channel<float>* splash_fade_out_opacity_channel = splash_fade_out_animation.add_channel(0);
	splash_fade_out_opacity_channel->insert_keyframe({0.0f, 1.0f});
	splash_fade_out_opacity_channel->insert_keyframe({splash_fade_out_duration, 0.0f});
	
	// Setup animation frame callbacks
	auto set_splash_opacity = [splash_tint]([[maybe_unused]] int channel, const float& opacity)
	{
		splash_tint->set(math::fvec4{1, 1, 1, opacity});
	};
	splash_fade_in_animation.set_frame_callback(set_splash_opacity);
	splash_fade_out_animation.set_frame_callback(set_splash_opacity);
	
	// Trigger splash fade out animation when splash fade in animation ends
	splash_fade_in_animation.set_end_callback
	(
		[this]()
		{
			this->splash_fade_out_animation.play();
		}
	);
	
	// Trigger a state change when the splash fade out animation ends
	splash_fade_out_animation.set_end_callback
	(
		[&ctx]()
		{
			// Queue change to main menu state
			ctx.function_queue.push
			(
				[&ctx]()
				{
					ctx.state_machine.pop();
					ctx.state_machine.emplace(std::make_unique<main_menu_state>(ctx, true));
				}
			);
		}
	);
	
	// Add splash fade animations to animator
	ctx.animator->add_animation(&splash_fade_in_animation);
	ctx.animator->add_animation(&splash_fade_out_animation);
	
	// Start splash fade in animation
	splash_fade_in_animation.play();
	
	// Setup window resized callback
	window_resized_subscription = ctx.window->get_resized_channel().subscribe
	(
		[&](const auto& event)
		{
			const math::fvec2 viewport_size = math::fvec2(event.window->get_viewport_size());
			const math::fvec2 viewport_center = viewport_size * 0.5f;
			splash_billboard.set_translation({std::round(viewport_center.x()), std::round(viewport_center.y()), 0.0f});
		}
	);
	
	// Construct splash skip function
	auto skip = [&]([[maybe_unused]] const auto& event)
	{
		ctx.function_queue.emplace
		(
			[&]()
			{
				// Black out screen
				ctx.window->get_graphics_pipeline().clear_attachments(gl::color_clear_bit, {{0.0f, 0.0f, 0.0f, 0.0f}});
				ctx.window->swap_buffers();
				
				// Change to main menu state
				ctx.state_machine.pop();
				ctx.state_machine.emplace(std::make_unique<main_menu_state>(ctx, true));
			}
		);
	};
	
	// Set up splash skippers
	input_mapped_subscriptions.emplace_back
	(
		ctx.input_mapper.get_gamepad_button_mapped_channel().subscribe
		(
			skip
		)
	);
	input_mapped_subscriptions.emplace_back
	(
		ctx.input_mapper.get_key_mapped_channel().subscribe
		(
			skip
		)
	);
	input_mapped_subscriptions.emplace_back
	(
		ctx.input_mapper.get_mouse_button_mapped_channel().subscribe
		(
			skip
		)
	);
	
	// Enable splash skippers next frame
	ctx.function_queue.push
	(
		[&]()
		{
			ctx.input_mapper.connect(ctx.input_manager->get_event_dispatcher());
		}
	);
	
	debug::log_trace("Entered splash state");
}

splash_state::~splash_state()
{
	debug::log_trace("Exiting splash state...");
	
	// Disable splash skippers
	ctx.input_mapper.disconnect();
	input_mapped_subscriptions.clear();
	
	// Remove splash fade animations from animator
	ctx.animator->remove_animation(&splash_fade_in_animation);
	ctx.animator->remove_animation(&splash_fade_out_animation);
	
	// Remove splash billboard from UI scene
	ctx.ui_canvas->get_scene().remove_object(splash_billboard);
	
	debug::log_trace("Exited splash state");
}
