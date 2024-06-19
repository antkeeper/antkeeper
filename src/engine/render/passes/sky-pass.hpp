// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_RENDER_SKY_PASS_HPP
#define ANTKEEPER_RENDER_SKY_PASS_HPP

#include <engine/render/pass.hpp>
#include <engine/gl/shader-template.hpp>
#include <engine/math/vector.hpp>
#include <engine/math/quaternion.hpp>
#include <engine/gl/shader-program.hpp>
#include <engine/gl/shader-variable.hpp>
#include <engine/gl/vertex-buffer.hpp>
#include <engine/gl/vertex-array.hpp>
#include <engine/gl/texture.hpp>
#include <engine/math/se3.hpp>
#include <engine/scene/object.hpp>
#include <engine/scene/light-probe.hpp>
#include <functional>

class resource_manager;

namespace render {

class material;
class model;

/**
 *
 *
 * @see Hillaire, Sébastien. "A scalable and production ready sky and atmosphere rendering technique." Computer Graphics Forum. Vol. 39. No. 4. 2020.
 */
class sky_pass: public pass
{
public:
	sky_pass(gl::pipeline* pipeline, const gl::framebuffer* framebuffer, resource_manager* resource_manager);
	
	/** Destructs a sky pass. */
	~sky_pass() override = default;
	
	void render(render::context& ctx) override;
	
	/// @name Transmittance LUT
	/// @{
	
	/**
	 * Sets the number of transmittance integration samples.
	 *
	 * @param count Integration sample count.
	 *
	 * @note Triggers rebuilding of transmittance LUT shader.
	 * @note Triggers rendering of transmittance LUT.
	 */
	void set_transmittance_lut_sample_count(std::uint16_t count);
	
	/**
	 * Sets the resolution of the transmittance LUT.
	 *
	 * @param resolution Resolution of the transmittance LUT texture, in pixels.
	 *
	 * @note Triggers rendering of transmittance LUT.
	 */
	void set_transmittance_lut_resolution(const math::vec2<std::uint16_t>& resolution);
	
	/// Returns the number of transmittance integration samples.
	[[nodiscard]] inline std::uint16_t get_transmittance_lut_sample_count() const noexcept
	{
		return m_transmittance_lut_sample_count;
	}
	
	/// Returns the resolution of the transmittance LUT texture, in pixels.
	[[nodiscard]] inline const math::vec2<std::uint16_t>& get_transmittance_lut_resolution() const noexcept
	{
		return m_transmittance_lut_resolution;
	}
	
	/// @}
	/// @name Multiscattering LUT
	/// @{
	
	/**
	 * Sets the number of multiscattering directions to sample.
	 *
	 * @param count Multiscattering direction sample count.
	 *
	 * @note Triggers rebuilding of multiscattering LUT shader.
	 * @note Triggers rendering of multiscattering LUT.
	 */
	void set_multiscattering_lut_direction_sample_count(std::uint16_t count);
	
	/**
	 * Sets the number of multiscattering scatter events to sample per sample direction.
	 *
	 * @param count Multiscattering scatter sample count.
	 *
	 * @note Triggers rebuilding of multiscattering LUT shader.
	 * @note Triggers rendering of multiscattering LUT.
	 */
	void set_multiscattering_lut_scatter_sample_count(std::uint16_t count);
	
	/**
	 * Sets the resolution of the multiscattering LUT.
	 *
	 * @param resolution Resolution of the multiscattering LUT texture, in pixels.
	 *
	 * @note Triggers rendering of multiscattering LUT.
	 */
	void set_multiscattering_lut_resolution(const math::vec2<std::uint16_t>& resolution);
	
	/// Returns the number of multiscattering direction samples.
	[[nodiscard]] inline std::uint16_t get_multiscattering_lut_direction_sample_count() const noexcept
	{
		return m_multiscattering_lut_direction_sample_count;
	}
	
	/// Returns the number of multiscattering scatter samples per direction.
	[[nodiscard]] inline std::uint16_t get_multiscattering_lut_scatter_sample_count() const noexcept
	{
		return m_multiscattering_lut_scatter_sample_count;
	}
	
	/// Returns the resolution of the multiscattering LUT texture, in pixels.
	[[nodiscard]] inline const math::vec2<std::uint16_t>& get_multiscattering_lut_resolution() const noexcept
	{
		return m_multiscattering_lut_resolution;
	}
	
	/// @}
	/// @name Luminance LUT
	/// @{
	
	/**
	 * Sets the number of luminance integration samples.
	 *
	 * @param count Integration sample count.
	 *
	 * @note Triggers rebuilding of luminance LUT shader.
	 * @note Triggers rendering of luminance LUT.
	 */
	void set_luminance_lut_sample_count(std::uint16_t count);
	
	/**
	 * Sets the resolution of the luminance LUT.
	 *
	 * @param resolution Resolution of the luminance LUT texture, in pixels.
	 *
	 * @note Triggers rendering of luminance LUT.
	 */
	void set_luminance_lut_resolution(const math::vec2<std::uint16_t>& resolution);
	
	/// Returns the number of luminance integration samples.
	[[nodiscard]] inline std::uint16_t get_luminance_lut_sample_count() const noexcept
	{
		return m_luminance_lut_sample_count;
	}
	
	/// Returns the resolution of the luminance LUT texture, in pixels.
	[[nodiscard]] inline const math::vec2<std::uint16_t>& get_luminance_lut_resolution() const noexcept
	{
		return m_luminance_lut_resolution;
	}
	
	/// @}
	
	void set_magnification(float scale);
	
	void set_sky_model(std::shared_ptr<render::model> model);
	void set_moon_model(std::shared_ptr<render::model> model);
	void set_stars_model(std::shared_ptr<render::model> model);
	
	void set_icrf_to_eus(const math::se3<float>& transformation);
	
	void set_sun_position(const math::fvec3& position);
	void set_sun_luminance(const math::fvec3& luminance);
	void set_sun_illuminance(const math::fvec3& illuminance, const math::fvec3& transmitted_illuminance);
	void set_sun_angular_radius(float radius);
	void set_planet_radius(float radius);
	void set_atmosphere_upper_limit(float limit);
	void set_observer_elevation(float elevation);
	void set_rayleigh_parameters(float scale_height, const math::fvec3& scattering);
	void set_mie_parameters(float scale_height, float scattering, float extinction, float anisotropy);
	void set_ozone_parameters(float lower_limit, float upper_limit, float mode, const math::fvec3& absorption);
	void set_airglow_luminance(const math::fvec3& luminance);
	void set_ground_albedo(const math::fvec3& albedo);
	
	void set_moon_position(const math::fvec3& position);
	void set_moon_rotation(const math::fquat& rotation);
	void set_moon_angular_radius(float angular_radius);
	void set_moon_sunlight_direction(const math::fvec3& direction);
	void set_moon_sunlight_illuminance(const math::fvec3& illuminance);
	void set_moon_planetlight_direction(const math::fvec3& direction);
	void set_moon_planetlight_illuminance(const math::fvec3& illuminance);
	void set_moon_illuminance(const math::fvec3& illuminance, const math::fvec3& transmitted_illuminance);
	

	
	void set_sky_probe(std::shared_ptr<scene::light_probe> probe);
	
	/**
	 * Sets the layer mask of the sky.
	 *
	 * @param mask 32-bit layer mask in which each set bit represents a layer in which the sky is visible.
	 */
	inline constexpr void set_layer_mask(std::uint32_t mask) noexcept
	{
		m_layer_mask = mask;
	}

private:
	void rebuild_transmittance_lut_framebuffer();
	void rebuild_transmittance_lut_shader_program();
	void rebuild_transmittance_lut_command_buffer();
	void rebuild_multiscattering_lut_framebuffer();
	void rebuild_multiscattering_lut_shader_program();
	void rebuild_multiscattering_lut_command_buffer();
	void rebuild_luminance_lut_framebuffer();
	void rebuild_luminance_lut_shader_program();
	void rebuild_luminance_lut_command_buffer();
	
	void rebuild_sky_lut_command_buffer();
	void rebuild_sky_probe_command_buffer();
	
	std::shared_ptr<gl::sampler> m_lut_sampler;
	std::unique_ptr<gl::vertex_array> m_vertex_array;
	
	// Transmittance
	std::uint16_t m_transmittance_lut_sample_count{40};
	math::vec2<std::uint16_t> m_transmittance_lut_resolution{256, 64};
	std::shared_ptr<gl::texture_2d> m_transmittance_lut_texture;
	std::shared_ptr<gl::framebuffer> m_transmittance_lut_framebuffer;
	std::shared_ptr<gl::shader_template> m_transmittance_lut_shader_template;
	std::unique_ptr<gl::shader_program> m_transmittance_lut_shader_program;
	std::vector<std::function<void()>> m_transmittance_lut_command_buffer;
	bool m_render_transmittance_lut{false};
	
	// Multiscattering
	std::uint16_t m_multiscattering_lut_direction_sample_count{64};
	std::uint16_t m_multiscattering_lut_scatter_sample_count{20};
	math::vec2<std::uint16_t> m_multiscattering_lut_resolution{32, 32};
	std::shared_ptr<gl::texture_2d> m_multiscattering_lut_texture;
	std::shared_ptr<gl::framebuffer> m_multiscattering_lut_framebuffer;
	std::shared_ptr<gl::shader_template> m_multiscattering_lut_shader_template;
	std::unique_ptr<gl::shader_program> m_multiscattering_lut_shader_program;
	std::vector<std::function<void()>> m_multiscattering_lut_command_buffer;
	bool m_render_multiscattering_lut{false};
	
	// Luminance
	std::uint16_t m_luminance_lut_sample_count{30};
	math::vec2<std::uint16_t> m_luminance_lut_resolution{200, 100};
	std::shared_ptr<gl::texture_2d> m_luminance_lut_texture;
	std::shared_ptr<gl::framebuffer> m_luminance_lut_framebuffer;
	std::shared_ptr<gl::shader_template> m_luminance_lut_shader_template;
	std::unique_ptr<gl::shader_program> m_luminance_lut_shader_program;
	std::vector<std::function<void()>> m_luminance_lut_command_buffer;
	bool m_render_luminance_lut{false};
	
	// Sky probe
	std::shared_ptr<scene::light_probe> m_sky_probe;
	std::vector<std::unique_ptr<gl::framebuffer>> m_sky_probe_framebuffers;
	std::shared_ptr<gl::shader_template> m_sky_probe_shader_template;
	std::unique_ptr<gl::shader_program> m_sky_probe_shader_program;
	std::vector<std::function<void()>> m_sky_probe_command_buffer;
	
	math::fvec3 dominant_light_direction;
	math::fvec3 dominant_light_illuminance;
	float camera_exposure;

	std::shared_ptr<gl::shader_program> sky_shader_program;
	const gl::shader_variable* sky_model_view_projection_var;
	const gl::shader_variable* sky_view_var;
	const gl::shader_variable* sky_view_projection_var;
	const gl::shader_variable* sky_inv_view_projection_var;
	const gl::shader_variable* sky_camera_position_var;
	const gl::shader_variable* sky_mouse_var;
	const gl::shader_variable* sky_resolution_var;
	const gl::shader_variable* sky_light_direction_var;
	const gl::shader_variable* sky_sun_luminance_var;
	const gl::shader_variable* sky_sun_angular_radius_var;
	const gl::shader_variable* sky_atmosphere_radii_var;
	const gl::shader_variable* sky_observer_position_var;
	const gl::shader_variable* sky_transmittance_lut_var;
	const gl::shader_variable* sky_transmittance_lut_resolution_var;
	const gl::shader_variable* sky_luminance_lut_var;
	const gl::shader_variable* sky_luminance_lut_resolution_var;
	
	std::shared_ptr<gl::shader_program> moon_shader_program;
	const gl::shader_variable* moon_model_var;
	const gl::shader_variable* moon_view_projection_var;
	const gl::shader_variable* moon_normal_model_var;
	const gl::shader_variable* moon_camera_position_var;
	const gl::shader_variable* moon_sunlight_direction_var;
	const gl::shader_variable* moon_sunlight_illuminance_var;
	const gl::shader_variable* moon_planetlight_direction_var;
	const gl::shader_variable* moon_planetlight_illuminance_var;
	const gl::shader_variable* moon_albedo_map_var;
	const gl::shader_variable* moon_normal_map_var;
	const gl::shader_variable* moon_observer_position_var;
	const gl::shader_variable* moon_sky_transmittance_lut_var;
	const gl::shader_variable* moon_atmosphere_radii_var;
	
	std::shared_ptr<render::model> sky_model;
	const material* sky_material;
	const gl::vertex_array* sky_model_vao;
	const gl::vertex_buffer* sky_model_vbo;
	gl::primitive_topology sky_model_primitive_topology;
	std::size_t sky_model_vertex_offset{};
	std::size_t sky_model_vertex_stride{};
	std::uint32_t sky_model_first_vertex{};
	std::uint32_t sky_model_vertex_count{};
	
	std::shared_ptr<render::model> moon_model;
	const material* moon_material;
	const gl::vertex_array* moon_model_vao;
	const gl::vertex_buffer* moon_model_vbo;
	gl::primitive_topology moon_model_primitive_topology;
	std::size_t moon_model_vertex_offset{};
	std::size_t moon_model_vertex_stride{};
	std::uint32_t moon_model_first_vertex{};
	std::uint32_t moon_model_vertex_count{};
	std::shared_ptr<gl::texture_2d> m_moon_albedo_map;
	std::shared_ptr<gl::texture_2d> m_moon_normal_map;
	
	std::shared_ptr<render::model> stars_model;
	const material* star_material;
	const gl::vertex_array* stars_model_vao;
	const gl::vertex_buffer* stars_model_vbo;
	gl::primitive_topology stars_model_primitive_topology;
	std::size_t stars_model_vertex_offset{};
	std::size_t stars_model_vertex_stride{};
	std::uint32_t stars_model_first_vertex{};
	std::uint32_t stars_model_vertex_count{};
	std::unique_ptr<gl::shader_program> star_shader_program;
	const gl::shader_variable* star_model_view_projection_var;
	const gl::shader_variable* star_exposure_var;
	const gl::shader_variable* star_inv_resolution_var;

	math::fvec2 mouse_position;
	
	math::fvec3 m_sun_position;
	math::fvec3 m_sun_luminance;
	math::fvec3 m_sun_illuminance;
	math::fvec3 sun_transmitted_illuminance;
	math::se3<float> m_icrf_to_eus;
	
	math::fvec3 m_moon_position;
	math::fquat m_moon_rotation;
	float m_moon_angular_radius;
	math::fvec3 m_moon_sunlight_direction;
	math::fvec3 m_moon_sunlight_illuminance;
	math::fvec3 m_moon_planetlight_direction;
	math::fvec3 m_moon_planetlight_illuminance;
	math::fvec3 m_moon_illuminance;
	math::fvec3 moon_transmitted_illuminance;
	
	float sun_angular_radius;
	float atmosphere_upper_limit;
	math::fvec4 atmosphere_radii;
	float observer_elevation;
	math::fvec3 m_observer_position;
	math::fvec4 rayleigh_parameters;
	math::fvec4 mie_parameters;
	math::fvec3 ozone_distribution;
	math::fvec3 ozone_absorption;
	math::fvec3 airglow_luminance;
	math::fvec3 m_ground_albedo{};
	
	float magnification;
	
	std::uint32_t m_layer_mask{1};
};

} // namespace render

#endif // ANTKEEPER_RENDER_SKY_PASS_HPP
