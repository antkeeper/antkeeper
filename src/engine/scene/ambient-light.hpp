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

#ifndef ANTKEEPER_SCENE_AMBIENT_LIGHT_HPP
#define ANTKEEPER_SCENE_AMBIENT_LIGHT_HPP

#include <engine/scene/light.hpp>
#include <engine/math/vector.hpp>

namespace scene {

/**
 * Omnidirectional source of illuminance.
 */
class ambient_light: public light
{
public:
	[[nodiscard]] inline light_type get_light_type() const noexcept override
	{
		return light_type::ambient;
	}
	
	/**
	 * Sets the color of the light.
	 *
	 * @param color Light color.
	 */
	inline void set_color(const math::vector3<float>& color) noexcept
	{
		m_color = color;
		color_updated();
	}
	
	/**
	 * Sets the illuminance of the light on a surface perpendicular to the light direction.
	 *
	 * @param illuminance Illuminance on a surface perpendicular to the light direction.
	 */
	inline void set_illuminance(float illuminance) noexcept
	{
		m_illuminance = illuminance;
		illuminance_updated();
	}
	
	/// Returns the color of the light.
	[[nodiscard]] inline const math::vector3<float>& get_color() const noexcept
	{
		return m_color;
	}
	
	/// Returns the illuminance of the light on a surface perpendicular to the light direction.
	[[nodiscard]] inline float get_illuminance() const noexcept
	{
		return m_illuminance;
	}
	
	/// Returns the color-modulated illuminance of the light on a surface perpendicular to the light direction.
	[[nodiscard]] inline const math::vector3<float>& get_colored_illuminance() const noexcept
	{
		return m_colored_illuminance;
	}
	
private:
	void color_updated();
	void illuminance_updated();
	
	math::vector3<float> m_color{1.0f, 1.0f, 1.0f};
	float m_illuminance{};
	math::vector3<float> m_colored_illuminance{};
};

} // namespace scene

#endif // ANTKEEPER_SCENE_AMBIENT_LIGHT_HPP
