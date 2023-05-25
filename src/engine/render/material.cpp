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

#include <engine/render/material.hpp>
#include <engine/render/material-variable.hpp>
#include <engine/resources/resource-loader.hpp>
#include <engine/resources/resource-manager.hpp>
#include <engine/render/material-flags.hpp>
#include <engine/utility/json.hpp>
#include <engine/utility/hash/combine.hpp>
#include <utility>
#include <type_traits>
#include <string>

namespace render {

material::material(const material& other)
{
	*this = other;
}

material& material::operator=(const material& other)
{
	two_sided = other.two_sided;
	blend_mode = other.blend_mode;
	shadow_mode = other.shadow_mode;
	flags = other.flags;
	shader_template = other.shader_template;
	
	variable_map.clear();
	for (const auto& [key, value]: other.variable_map)
	{
		if (value)
		{
			variable_map.emplace(key, value->clone());
		}
	}
	
	m_hash = other.m_hash;
	
	return *this;
}

void material::set_two_sided(bool two_sided) noexcept
{
	this->two_sided = two_sided;
	
	rehash();
}

void material::set_blend_mode(material_blend_mode mode) noexcept
{
	blend_mode = mode;
	
	rehash();
}

void material::set_shadow_mode(material_shadow_mode mode) noexcept
{
	shadow_mode = mode;
	
	rehash();
}

void material::set_flags(std::uint32_t flags) noexcept
{
	this->flags = flags;
	
	rehash();
}

void material::set_shader_template(std::shared_ptr<gl::shader_template> shader_template)
{
	this->shader_template = shader_template;
	
	rehash();
}

void material::set_variable(hash::fnv1a32_t key, std::shared_ptr<material_variable_base> value)
{
	variable_map[key] = std::move(value);
}

std::shared_ptr<material_variable_base> material::get_variable(hash::fnv1a32_t key) const
{
	if (auto i = variable_map.find(key); i != variable_map.end())
	{
		return i->second;
	}
	
	return nullptr;
}

void material::rehash() noexcept
{
	m_hash = 0;
	if (shader_template)
	{
		m_hash = shader_template->hash();
	}
	
	m_hash = hash::combine(m_hash, std::hash<bool>{}(two_sided));
	m_hash = hash::combine(m_hash, std::hash<material_blend_mode>{}(blend_mode));
	m_hash = hash::combine(m_hash, std::hash<material_shadow_mode>{}(shadow_mode));
	m_hash = hash::combine(m_hash, std::hash<std::uint32_t>{}(flags));
}

} // namespace render

template <typename T>
static bool read_value(T* value, const nlohmann::json& json, const std::string& name)
{
	if (auto element = json.find(name); element != json.end())
	{
		*value = element.value().get<T>();
		return true;
	}
	
	return false;
}

static bool load_texture_1d_property(resource_manager& resource_manager, render::material& material, hash::fnv1a32_t key, const nlohmann::json& json)
{
	// If JSON element is an array
	if (json.is_array())
	{
		// Create variable
		auto variable = std::make_shared<render::material_texture_1d>(json.size());
		
		// Load textures
		std::size_t i = 0;
		for (const auto& element: json)
		{
			variable->set(i, resource_manager.load<gl::texture_1d>(element.get<std::string>()));
			++i;
		}
		
		material.set_variable(key, variable);
	}
	else
	{
		// Create variable
		auto variable = std::make_shared<render::material_texture_1d>(json.size());
		
		// Load texture
		variable->set(resource_manager.load<gl::texture_1d>(json.get<std::string>()));
		
		material.set_variable(key, variable);
	}
	
	return true;
}

static bool load_texture_2d_property(resource_manager& resource_manager, render::material& material, hash::fnv1a32_t key, const nlohmann::json& json)
{
	// If JSON element is an array
	if (json.is_array())
	{
		// Create variable
		auto variable = std::make_shared<render::material_texture_2d>(json.size());
		
		// Load textures
		std::size_t i = 0;
		for (const auto& element: json)
		{
			variable->set(i, resource_manager.load<gl::texture_2d>(element.get<std::string>()));
			++i;
		}
		
		material.set_variable(key, variable);
	}
	else
	{
		// Create variable
		auto variable = std::make_shared<render::material_texture_2d>(json.size());
		
		// Load texture
		variable->set(resource_manager.load<gl::texture_2d>(json.get<std::string>()));
		
		material.set_variable(key, variable);
	}
	
	return true;
}

static bool load_texture_3d_property(resource_manager& resource_manager, render::material& material, hash::fnv1a32_t key, const nlohmann::json& json)
{
	// If JSON element is an array
	if (json.is_array())
	{
		// Create variable
		auto variable = std::make_shared<render::material_texture_3d>(json.size());
		
		// Load textures
		std::size_t i = 0;
		for (const auto& element: json)
		{
			variable->set(i, resource_manager.load<gl::texture_3d>(element.get<std::string>()));
			++i;
		}
		
		material.set_variable(key, variable);
	}
	else
	{
		// Create variable
		auto variable = std::make_shared<render::material_texture_3d>(json.size());
		
		// Load texture
		variable->set(resource_manager.load<gl::texture_3d>(json.get<std::string>()));
		
		material.set_variable(key, variable);
	}
	
	return true;
}

static bool load_texture_cube_property(resource_manager& resource_manager, render::material& material, hash::fnv1a32_t key, const nlohmann::json& json)
{
	// If JSON element is an array
	if (json.is_array())
	{
		// Create variable
		auto variable = std::make_shared<render::material_texture_cube>(json.size());
		
		// Load textures
		std::size_t i = 0;
		for (const auto& element: json)
		{
			variable->set(i, resource_manager.load<gl::texture_cube>(element.get<std::string>()));
			++i;
		}
		
		material.set_variable(key, variable);
	}
	else
	{
		// Create variable
		auto variable = std::make_shared<render::material_texture_cube>(json.size());
		
		// Load texture
		variable->set(resource_manager.load<gl::texture_cube>(json.get<std::string>()));
		
		material.set_variable(key, variable);
	}
	
	return true;
}

template <typename T>
static bool load_scalar_property(render::material& material, hash::fnv1a32_t key, const nlohmann::json& json)
{
	// If JSON element is an array
	if (json.is_array())
	{
		// Create variable
		auto variable = std::make_shared<render::material_variable<T>>(json.size());
		
		// Set variable values
		std::size_t i = 0;
		for (const auto& element: json)
		{
			variable->set(i, element.get<T>());
		}
		
		material.set_variable(key, variable);
	}
	else
	{
		material.set_variable(key, std::make_shared<render::material_variable<T>>(1, json.get<T>()));
	}
	
	return true;
}

template <typename T>
static bool load_vector_property(render::material& material, hash::fnv1a32_t key, std::size_t vector_size, const nlohmann::json& json)
{
	// If JSON element is an array of arrays
	if (json.is_array() && json.begin().value().is_array())
	{
		// Create variable
		auto variable = std::make_shared<render::material_variable<T>>(json.size());
		
		// For each vector in the array
		std::size_t i = 0;
		for (const auto& vector_element: json)
		{
			// Read vector elements
			T value;
			std::size_t j = 0;
			for (const auto& value_element: vector_element)
				value[j++] = value_element.get<typename T::element_type>();
			
			variable->set(i, value);
			
			++i;
		}
		
		material.set_variable(key, variable);
	}
	else
	{
		// Read vector elements
		T value;
		std::size_t i = 0;
		for (const auto& value_element: json)
			value[i++] = value_element.get<typename T::element_type>();
		
		material.set_variable(key, std::make_shared<render::material_variable<T>>(1, value));
	}
	
	return true;
}

template <typename T>
static bool load_matrix_property(render::material& material, hash::fnv1a32_t key, std::size_t column_count, std::size_t row_count, const nlohmann::json& json)
{
	// If JSON element is an array of arrays of arrays
	if (json.is_array() && json.begin().value().is_array())
	{
		if (json.begin().value().begin().value().is_array())
		{
			// Create variable
			auto variable = std::make_shared<render::material_variable<T>>(json.size());
			
			// For each matrix in the array
			std::size_t i = 0;
			for (const auto& matrix_element: json)
			{
				// Read vector elements
				T value;
				std::size_t j = 0;
				for (const auto& column_element: matrix_element)
				{
					std::size_t k = 0;
					for (const auto& row_element: column_element)
					{
						value[j][k] = row_element.get<typename T::element_type>();
						++k;
					}
					
					++j;
				}
				
				// Set matrix value
				variable->set(i, value);
				
				++i;
			}
			
			material.set_variable(key, variable);
			
			return true;
		}
		else
		{
			// Read matrix elements
			T value;
			std::size_t i = 0;
			for (const auto& column_element: json)
			{
				std::size_t j = 0;
				for (const auto& row_element: column_element)
				{
					value[i][j] = row_element.get<typename T::element_type>();
					++j;
				}
				
				++i;
			}
			
			material.set_variable(key, std::make_shared<render::material_variable<T>>(1, value));
			
			return true;
		}
	}
	
	return false;
}

template <>
std::unique_ptr<render::material> resource_loader<render::material>::load(::resource_manager& resource_manager, deserialize_context& ctx)
{
	auto material = std::make_unique<render::material>();
	
	// Load JSON data
	auto json = resource_loader<nlohmann::json>::load(resource_manager, ctx);
	
	// Read two sided
	bool two_sided = false;
	read_value(&two_sided, *json, "two_sided");
	material->set_two_sided(two_sided);
	
	// Read blend mode
	std::string blend_mode;
	read_value(&blend_mode, *json, "blend_mode");
	if (blend_mode == "opaque")
	{
		material->set_blend_mode(render::material_blend_mode::opaque);
	}
	else if (blend_mode == "masked")
	{
		material->set_blend_mode(render::material_blend_mode::masked);
	}
	else if (blend_mode == "translucent")
	{
		material->set_blend_mode(render::material_blend_mode::translucent);
	}
	
	// Read shadow mode
	std::string shadow_mode;
	read_value(&shadow_mode, *json, "shadow_mode");
	if (shadow_mode == "opaque")
	{
		material->set_shadow_mode(render::material_shadow_mode::opaque);
	}
	else if (shadow_mode == "none")
	{
		material->set_shadow_mode(render::material_shadow_mode::none);
	}
	
	// Init material flags
	std::uint32_t flags = 0;
	
	// Read depth mode
	std::string depth_mode;
	read_value(&depth_mode, *json, "depth_mode");
	if (depth_mode == "in_front")
		flags |= MATERIAL_FLAG_X_RAY;
	
	// Read decal mode
	std::string decal_mode;
	read_value(&decal_mode, *json, "decal_mode");
	if (decal_mode == "decal")
		flags |= MATERIAL_FLAG_DECAL;
	else if (decal_mode == "surface")
		flags |= MATERIAL_FLAG_DECAL_SURFACE;
	
	// Set material flags
	material->set_flags(flags);
	
	// Read shader template filename
	std::string shader_template_filename;
	if (read_value(&shader_template_filename, *json, "shader_template"))
	{
		// Loader shader template
		material->set_shader_template(resource_manager.load<gl::shader_template>(shader_template_filename));
	}
	
	// Read material variables
	if (auto variables_element = json->find("variables"); variables_element != json->end())
	{
		for (const auto& variable_element: variables_element.value())
		{
			// Read variable name
			std::string name;
			if (!read_value(&name, variable_element, "name"))
			{
				// Ignore nameless properties
				continue;
			}
			
			// Read variable type
			std::string type;
			if (!read_value(&type, variable_element, "type"))
			{
				// Ignore typeless properties
				continue;
			}
			
			// Find value element
			auto value_element = variable_element.find("value");
			if (value_element == variable_element.end())
			{
				// Ignore valueless properties
				continue;
			}
			
			// Hash variable name
			const hash::fnv1a32_t key = hash::fnv1a32<char>(name);
			
			if (type == "texture_1d")
			{
				load_texture_1d_property(resource_manager, *material, key, value_element.value());
			}
			else if (type == "texture_2d")
			{
				load_texture_2d_property(resource_manager, *material, key, value_element.value());
			}
			else if (type == "texture_3d")
			{
				load_texture_3d_property(resource_manager, *material, key, value_element.value());
			}
			else if (type == "texture_cube")
			{
				load_texture_cube_property(resource_manager, *material, key, value_element.value());
			}
			// If variable type is a matrix
			else if (type[type.size() - 2] == 'x' &&
				std::isdigit(type[type.size() - 3]) &&
				std::isdigit(type.back()))
			{
				std::size_t columns = std::stoul(type.substr(type.size() - 3, 1));
				std::size_t rows = std::stoul(type.substr(type.size() - 1, 1));
				
				if (type.find("float") != std::string::npos)
				{
					if (columns == 2 && rows == 2)
						load_matrix_property<float2x2>(*material, key, columns, rows, value_element.value());
					else if (columns == 3 && rows == 3)
						load_matrix_property<float3x3>(*material, key, columns, rows, value_element.value());
					else if (columns == 4 && rows == 4)
						load_matrix_property<float4x4>(*material, key, columns, rows, value_element.value());
				}
			}
			// If variable type is a vector
			else if (std::isdigit(type.back()))
			{
				std::size_t size = std::stoul(type.substr(type.size() - 1, 1));
				
				if (type.find("float") != std::string::npos)
				{
					if (size == 2)
						load_vector_property<float2>(*material, key, size, value_element.value());
					else if (size == 3)
						load_vector_property<float3>(*material, key, size, value_element.value());
					else if (size == 4)
						load_vector_property<float4>(*material, key, size, value_element.value());
				}
				else if (type.find("uint") != std::string::npos)
				{
					if (size == 2)
						load_vector_property<uint2>(*material, key, size, value_element.value());
					else if (size == 3)
						load_vector_property<uint3>(*material, key, size, value_element.value());
					else if (size == 4)
						load_vector_property<uint4>(*material, key, size, value_element.value());
				}
				else if (type.find("int") != std::string::npos)
				{
					if (size == 2)
						load_vector_property<int2>(*material, key, size, value_element.value());
					else if (size == 3)
						load_vector_property<int3>(*material, key, size, value_element.value());
					else if (size == 4)
						load_vector_property<int4>(*material, key, size, value_element.value());
				}
				else if (type.find("bool") != std::string::npos)
				{
					if (size == 2)
						load_vector_property<bool2>(*material, key, size, value_element.value());
					else if (size == 3)
						load_vector_property<bool3>(*material, key, size, value_element.value());
					else if (size == 4)
						load_vector_property<bool4>(*material, key, size, value_element.value());
				}
			}
			// If variable type is a scalar
			else
			{
				if (type.find("float") != std::string::npos)
					load_scalar_property<float>(*material, key, value_element.value());
				else if (type.find("uint") != std::string::npos)
					load_scalar_property<unsigned int>(*material, key, value_element.value());
				else if (type.find("int") != std::string::npos)
					load_scalar_property<int>(*material, key, value_element.value());
				else if (type.find("bool") != std::string::npos)
					load_scalar_property<bool>(*material, key, value_element.value());
			}
		}
	}
	
	return material;
}
