/*
 * Copyright (C) 2021  Christopher J. Howard
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

#ifndef ANTKEEPER_RENDER_SKY_PASS_HPP
#define ANTKEEPER_RENDER_SKY_PASS_HPP

#include "render/pass.hpp"
#include "utility/fundamental-types.hpp"
#include "event/event-handler.hpp"
#include "event/input-events.hpp"
#include "animation/tween.hpp"
#include "math/quaternion-type.hpp"
#include "gl/shader-program.hpp"
#include "gl/shader-input.hpp"
#include "gl/vertex-buffer.hpp"
#include "gl/vertex-array.hpp"
#include "gl/texture-2d.hpp"
#include "gl/drawing-mode.hpp"
#include "math/se3.hpp"
#include "scene/object.hpp"

class resource_manager;

namespace render {

class material;
class model;

/**
 *
 */
class sky_pass: public pass,
	public event_handler<mouse_moved_event>
{
public:
	sky_pass(gl::rasterizer* rasterizer, const gl::framebuffer* framebuffer, resource_manager* resource_manager);
	virtual ~sky_pass();
	virtual void render(const render::context& ctx, render::queue& queue) const final;
	
	void update_tweens();
	
	void set_magnification(float scale);
	
	void set_sky_model(const model* model);
	void set_moon_model(const model* model);
	void set_stars_model(const model* model);
	void set_clouds_model(const model* model);
	
	void set_icrf_to_eus(const math::transformation::se3<float>& transformation);
	
	void set_sun_position(const float3& position);
	void set_sun_luminance(const float3& luminance);
	void set_sun_illuminance(const float3& illuminance);
	void set_sun_angular_radius(float radius);
	void set_planet_radius(float radius);
	void set_atmosphere_upper_limit(float limit);
	void set_observer_elevation(float elevation);
	void set_rayleigh_parameters(float scale_height, const float3& scattering);
	void set_mie_parameters(float scale_height, float scattering, float absorption, float anisotropy);
	void set_ozone_parameters(float lower_limit, float upper_limit, float mode, const float3& absorption);
	
	void set_moon_position(const float3& position);
	void set_moon_rotation(const math::quaternion<float>& rotation);
	void set_moon_angular_radius(float angular_radius);
	void set_moon_sunlight_direction(const float3& direction);
	void set_moon_sunlight_illuminance(const float3& illuminance);
	void set_moon_planetlight_direction(const float3& direction);
	void set_moon_planetlight_illuminance(const float3& illuminance);

private:
	virtual void handle_event(const mouse_moved_event& event);
	
	gl::vertex_buffer* quad_vbo;
	gl::vertex_array* quad_vao;
	gl::texture_2d* transmittance_texture;
	gl::framebuffer* transmittance_framebuffer;
	float2 transmittance_inverse_lut_resolution;
	
	gl::shader_program* transmittance_shader_program;
	const gl::shader_input* transmittance_atmosphere_radii_input;
	const gl::shader_input* transmittance_rayleigh_parameters_input;
	const gl::shader_input* transmittance_mie_parameters_input;
	const gl::shader_input* transmittance_ozone_distribution_input;
	const gl::shader_input* transmittance_ozone_absorption_input;
	const gl::shader_input* transmittance_inverse_lut_resolution_input;

	gl::shader_program* sky_shader_program;
	const gl::shader_input* model_view_projection_input;
	const gl::shader_input* mouse_input;
	const gl::shader_input* resolution_input;
	const gl::shader_input* time_input;
	const gl::shader_input* exposure_input;
	const gl::shader_input* sun_direction_input;
	const gl::shader_input* sun_luminance_input;
	const gl::shader_input* sun_illuminance_input;
	const gl::shader_input* sun_angular_radius_input;
	const gl::shader_input* atmosphere_radii_input;
	const gl::shader_input* observer_position_input;
	const gl::shader_input* rayleigh_parameters_input;
	const gl::shader_input* mie_parameters_input;
	const gl::shader_input* ozone_distribution_input;
	const gl::shader_input* ozone_absorption_input;
	const gl::shader_input* transmittance_lut_input;
	const gl::shader_input* inverse_transmittance_lut_resolution_input;
	
	gl::shader_program* moon_shader_program;
	const gl::shader_input* moon_model_input;
	const gl::shader_input* moon_view_projection_input;
	const gl::shader_input* moon_normal_model_input;
	const gl::shader_input* moon_camera_position_input;
	const gl::shader_input* moon_sunlight_direction_input;
	const gl::shader_input* moon_sunlight_illuminance_input;
	const gl::shader_input* moon_planetlight_direction_input;
	const gl::shader_input* moon_planetlight_illuminance_input;
	
	const model* sky_model;
	const material* sky_material;
	const gl::vertex_array* sky_model_vao;
	gl::drawing_mode sky_model_drawing_mode;
	std::size_t sky_model_start_index;
	std::size_t sky_model_index_count;
	
	const model* moon_model;
	const material* moon_material;
	const gl::vertex_array* moon_model_vao;
	gl::drawing_mode moon_model_drawing_mode;
	std::size_t moon_model_start_index;
	std::size_t moon_model_index_count;
	
	const model* stars_model;
	const material* star_material;
	const gl::vertex_array* stars_model_vao;
	gl::drawing_mode stars_model_drawing_mode;
	std::size_t stars_model_start_index;
	std::size_t stars_model_index_count;
	gl::shader_program* star_shader_program;
	const gl::shader_input* star_model_view_input;
	const gl::shader_input* star_projection_input;
	const gl::shader_input* star_exposure_input;
	const gl::shader_input* star_distance_input;
	
	const model* clouds_model;
	const material* cloud_material;
	const gl::vertex_array* clouds_model_vao;
	gl::drawing_mode clouds_model_drawing_mode;
	std::size_t clouds_model_start_index;
	std::size_t clouds_model_index_count;
	
	gl::shader_program* cloud_shader_program;
	const gl::shader_input* cloud_model_view_projection_input;
	const gl::shader_input* cloud_sun_direction_input;
	const gl::shader_input* cloud_sun_illuminance_input;
	const gl::shader_input* cloud_camera_position_input;
	const gl::shader_input* cloud_camera_exposure_input;

	const gl::texture_2d* blue_noise_map;
	const gl::texture_2d* sky_gradient;
	const gl::texture_2d* sky_gradient2;
	float2 mouse_position;
	
	tween<float3> sun_position_tween;
	tween<float3> sun_luminance_tween;
	tween<float3> sun_illuminance_tween;
	tween<float3> icrf_to_eus_translation;
	tween<math::quaternion<float>> icrf_to_eus_rotation;
	
	tween<float3> moon_position_tween;
	tween<math::quaternion<float>> moon_rotation_tween;
	tween<float> moon_angular_radius_tween;
	tween<float3> moon_sunlight_direction_tween;
	tween<float3> moon_sunlight_illuminance_tween;
	tween<float3> moon_planetlight_direction_tween;
	tween<float3> moon_planetlight_illuminance_tween;
	
	float sun_angular_radius;
	float atmosphere_upper_limit;
	float3 atmosphere_radii;
	float observer_elevation;
	tween<float3> observer_position_tween;
	float4 rayleigh_parameters;
	float4 mie_parameters;
	float3 ozone_distribution;
	float3 ozone_absorption;
	
	float magnification;
};

} // namespace render

#endif // ANTKEEPER_RENDER_SKY_PASS_HPP
