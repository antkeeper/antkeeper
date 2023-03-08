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

#ifndef ANTKEEPER_GL_SHADER_INPUT_HPP
#define ANTKEEPER_GL_SHADER_INPUT_HPP

#include <engine/utility/fundamental-types.hpp>
#include <engine/gl/shader-variable-type.hpp>
#include <cstdint>
#include <span>
#include <memory>

namespace gl {

class texture_1d;
class texture_2d;
class texture_3d;
class texture_cube;

/**
 * Shader program variable.
 */
class shader_variable
{
public:
	/**
	 * Returns the shader variable data type.
	 */
	[[nodiscard]] virtual constexpr shader_variable_type type() const noexcept = 0;
	
	/**
	 * Returns the number of elements in an array variable, or `1` if the variable is not an array.
	 */
	[[nodiscard]] inline std::size_t size() const noexcept
	{
		return m_size;
	}
	
	/**
	 * Updates the value of the variable. If the variable is an array, the value of the first element will be updated.
	 *
	 * @param value Update value.
	 *
	 * @throw std::invalid_argument Shader variable type mismatch.
	 */
	///@{
	virtual void update(bool value) const;
	virtual void update(const bool2& value) const;
	virtual void update(const bool3& value) const;
	virtual void update(const bool4& value) const;
	
	virtual void update(int value) const;
	virtual void update(const int2& value) const;
	virtual void update(const int3& value) const;
	virtual void update(const int4& value) const;
	
	virtual void update(unsigned int value) const;
	virtual void update(const uint2& value) const;
	virtual void update(const uint3& value) const;
	virtual void update(const uint4& value) const;
	
	virtual void update(float value) const;
	virtual void update(const float2& value) const;
	virtual void update(const float3& value) const;
	virtual void update(const float4& value) const;
	
	virtual void update(const float2x2& value) const;
	virtual void update(const float3x3& value) const;
	virtual void update(const float4x4& value) const;
	
	virtual void update(const texture_1d& value) const;
	virtual void update(const texture_2d& value) const;
	virtual void update(const texture_3d& value) const;
	virtual void update(const texture_cube& value) const;
	///@}
	
	/**
	 * Updates the value of a single element in an array variable.
	 *
	 * @param value Update value.
	 * @param index Index of the element to update.
	 *
	 * @throw std::invalid_argument Shader variable type mismatch.
	 */
	/// @{
	virtual void update(bool value, std::size_t index) const;
	virtual void update(const bool2& value, std::size_t index) const;
	virtual void update(const bool3& value, std::size_t index) const;
	virtual void update(const bool4& value, std::size_t index) const;
	
	virtual void update(int value, std::size_t index) const;
	virtual void update(const int2& value, std::size_t index) const;
	virtual void update(const int3& value, std::size_t index) const;
	virtual void update(const int4& value, std::size_t index) const;
	
	virtual void update(unsigned int value, std::size_t index) const;
	virtual void update(const uint2& value, std::size_t index) const;
	virtual void update(const uint3& value, std::size_t index) const;
	virtual void update(const uint4& value, std::size_t index) const;
	
	virtual void update(float value, std::size_t index) const;
	virtual void update(const float2& value, std::size_t index) const;
	virtual void update(const float3& value, std::size_t index) const;
	virtual void update(const float4& value, std::size_t index) const;
	
	virtual void update(const float2x2& value, std::size_t index) const;
	virtual void update(const float3x3& value, std::size_t index) const;
	virtual void update(const float4x4& value, std::size_t index) const;
	
	virtual void update(const texture_1d& value, std::size_t index) const;
	virtual void update(const texture_2d& value, std::size_t index) const;
	virtual void update(const texture_3d& value, std::size_t index) const;
	virtual void update(const texture_cube& value, std::size_t index) const;
	///@}
	
	/**
	 * Updates the values of one or more elements in an array variable.
	 *
	 * @param values Contiguous sequence of update values.
	 * @param index Index of the first element to update.
	 *
	 * @throw std::invalid_argument Shader variable type mismatch.
	 */
	///@{
	virtual void update(std::span<const bool> values, std::size_t index = 0) const;
	virtual void update(std::span<const bool2> values, std::size_t index = 0) const;
	virtual void update(std::span<const bool3> values, std::size_t index = 0) const;
	virtual void update(std::span<const bool4> values, std::size_t index = 0) const;
	
	virtual void update(std::span<const int> values, std::size_t index = 0) const;
	virtual void update(std::span<const int2> values, std::size_t index = 0) const;
	virtual void update(std::span<const int3> values, std::size_t index = 0) const;
	virtual void update(std::span<const int4> values, std::size_t index = 0) const;
	
	virtual void update(std::span<const unsigned int> values, std::size_t index = 0) const;
	virtual void update(std::span<const uint2> values, std::size_t index = 0) const;
	virtual void update(std::span<const uint3> values, std::size_t index = 0) const;
	virtual void update(std::span<const uint4> values, std::size_t index = 0) const;
	
	virtual void update(std::span<const float> values, std::size_t index = 0) const;
	virtual void update(std::span<const float2> values, std::size_t index = 0) const;
	virtual void update(std::span<const float3> values, std::size_t index = 0) const;
	virtual void update(std::span<const float4> values, std::size_t index = 0) const;
	
	virtual void update(std::span<const float2x2> values, std::size_t index = 0) const;
	virtual void update(std::span<const float3x3> values, std::size_t index = 0) const;
	virtual void update(std::span<const float4x4> values, std::size_t index = 0) const;
	
	virtual void update(std::span<const texture_1d* const> values, std::size_t index = 0) const;
	virtual void update(std::span<const texture_2d* const> values, std::size_t index = 0) const;
	virtual void update(std::span<const texture_3d* const> values, std::size_t index = 0) const;
	virtual void update(std::span<const texture_cube* const> values, std::size_t index = 0) const;
	
	virtual void update(std::span<const std::shared_ptr<texture_1d>> values, std::size_t index = 0) const;
	virtual void update(std::span<const std::shared_ptr<texture_2d>> values, std::size_t index = 0) const;
	virtual void update(std::span<const std::shared_ptr<texture_3d>> values, std::size_t index = 0) const;
	virtual void update(std::span<const std::shared_ptr<texture_cube>> values, std::size_t index = 0) const;
	///@}
	
protected:
	/**
	 * Constructs a shader variable.
	 *
	 * @param size Number of elements in the array, or `1` if the variable is not an array.
	 */
	explicit shader_variable(std::size_t size) noexcept;
	
private:
	const std::size_t m_size{0};
};

} // namespace gl

#endif // ANTKEEPER_GL_SHADER_VARIABLE_HPP