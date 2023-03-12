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

#ifndef ANTKEEPER_SCENE_DIRECTIONAL_LIGHT_HPP
#define ANTKEEPER_SCENE_DIRECTIONAL_LIGHT_HPP

#include <engine/scene/light.hpp>
#include <engine/gl/texture-2d.hpp>
#include <engine/utility/fundamental-types.hpp>
#include <vector>

namespace scene {

/**
 * Light source with parallel rays and constant intensity.
 */
class directional_light: public light
{
public:
	/// Creates a directional light.
	directional_light();
	
	/// Returns light_type::directional.
	[[nodiscard]] inline light_type get_light_type() const noexcept override
	{
		return light_type::directional;
	}

	/// Returns the normalized direction vector of the light.
	[[nodiscard]] inline const float3& get_direction() const noexcept
	{
		return direction[1];
	}
	
	inline const tween<float3>& get_direction_tween() const noexcept
	{
		return direction;
	}
	
	/// @copydoc object_base::update_tweens();
	void update_tweens() override;
	
	/// @name Shadow
	/// @{
	
	/**
	 * Enables or disables shadow casting.
	 *
	 * @param caster `true` if the light should cast shadows, `false` otherwise.
	 */
	void set_shadow_caster(bool caster) noexcept;
	
	/**
	 * Sets the shadow map framebuffer.
	 *
	 * @param framebuffer Pointer to a shadow map framebuffer.
	 */
	void set_shadow_framebuffer(const gl::framebuffer* framebuffer) noexcept;
	
	/**
	 * Sets the shadow bias factor for reducing self-shadowing.
	 *
	 * @param bias Shadow bias factor.
	 */
	void set_shadow_bias(float bias) noexcept;
	
	/**
	 * Sets the number of shadow cascades.
	 *
	 * @param count Number of shadow cascades.
	 */
	void set_shadow_cascade_count(unsigned int count) noexcept;
	
	/**
	 * Sets the shadow cascade coverage factor.
	 *
	 * @param factor Percentage of the view frustum clipping range covered by shadow cascades. A value of `1.0` results in full coverage of the view frustum clipping range, `0.5` results in coverage of half of the clipping range, etc.
	 */
	void set_shadow_cascade_coverage(float factor) noexcept;
	
	/**
	 * Sets the shadow cascade distribution.
	 *
	 * @param weight Linear interpolation weight between uniform and logarithmic cascade distributions. A weight of `0.0` results in a uniform cascade distribution, while `1.0` results in a logarithmic distribution.
	 */
	void set_shadow_cascade_distribution(float weight) noexcept;
	
	/// Returns `true` if the light casts shadows, `false` otherwise.
	[[nodiscard]] inline bool is_shadow_caster() const noexcept
	{
		return shadow_caster;
	}
	
	/// Returns the shadow map framebuffer, of `nullptr` if no shadow map framebuffer is set.
	[[nodiscard]] inline const gl::framebuffer* get_shadow_framebuffer() const noexcept
	{
		return shadow_framebuffer;
	}
	
	/// Returns the shadow bias factor.
	[[nodiscard]] inline float get_shadow_bias() const noexcept
	{
		return shadow_bias;
	}
	
	/// Returns the number of shadow cascades.
	[[nodiscard]] inline unsigned int get_shadow_cascade_count() const noexcept
	{
		return shadow_cascade_count;
	}
	
	/// Returns the shadow cascade coverage factor.
	[[nodiscard]] inline float get_shadow_cascade_coverage() const noexcept
	{
		return shadow_cascade_coverage;
	}
	
	/// Returns the shadow cascade distribution weight.
	[[nodiscard]] inline float get_shadow_cascade_distribution() const noexcept
	{
		return shadow_cascade_distribution;
	}
	
	/// Returns the array of shadow cascade far clipping plane distances.
	/// @{
	[[nodiscard]] inline const std::vector<float>& get_shadow_cascade_distances() const noexcept
	{
		return shadow_cascade_distances;
	}
	[[nodiscard]] inline std::vector<float>& get_shadow_cascade_distances() noexcept
	{
		return shadow_cascade_distances;
	}
	/// @}
	
	/// Returns the array of world-space to cascade texture-space transformation matrices.
	/// @{
	[[nodiscard]] inline const std::vector<float4x4>& get_shadow_cascade_matrices() const noexcept
	{
		return shadow_cascade_matrices;
	}
	[[nodiscard]] inline std::vector<float4x4>& get_shadow_cascade_matrices() noexcept
	{
		return shadow_cascade_matrices;
	}
	/// @}
	
	/// @}

private:
	void transformed() override;

	tween<float3> direction;
	
	bool shadow_caster{false};
	const gl::framebuffer* shadow_framebuffer{nullptr};
	float shadow_bias{0.005f};
	unsigned int shadow_cascade_count{4};
	float shadow_cascade_coverage{1.0f};
	float shadow_cascade_distribution{0.8f};
	mutable std::vector<float> shadow_cascade_distances;
	mutable std::vector<float4x4> shadow_cascade_matrices;
};

} // namespace scene

#endif // ANTKEEPER_SCENE_DIRECTIONAL_LIGHT_HPP
