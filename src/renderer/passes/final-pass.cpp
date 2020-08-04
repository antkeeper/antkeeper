/*
 * Copyright (C) 2020  Christopher J. Howard
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

#include "renderer/passes/final-pass.hpp"
#include "resources/resource-manager.hpp"
#include "rasterizer/rasterizer.hpp"
#include "rasterizer/framebuffer.hpp"
#include "rasterizer/shader-program.hpp"
#include "rasterizer/shader-input.hpp"
#include "rasterizer/vertex-buffer.hpp"
#include "rasterizer/vertex-array.hpp"
#include "rasterizer/vertex-attribute-type.hpp"
#include "rasterizer/drawing-mode.hpp"
#include "rasterizer/texture-2d.hpp"
#include "rasterizer/texture-wrapping.hpp"
#include "rasterizer/texture-filter.hpp"
#include "renderer/vertex-attributes.hpp"
#include "renderer/render-context.hpp"
#include <vmq/vmq.hpp>
#include <cmath>
#include <glad/glad.h>

using namespace vmq;

final_pass::final_pass(::rasterizer* rasterizer, const ::framebuffer* framebuffer, resource_manager* resource_manager):
	render_pass(rasterizer, framebuffer),
	color_texture(nullptr),
	bloom_texture(nullptr)
{
	shader_program = resource_manager->load<::shader_program>("final.glsl");
	color_texture_input = shader_program->get_input("color_texture");
	bloom_texture_input = shader_program->get_input("bloom_texture");
	resolution_input = shader_program->get_input("resolution");

	const float vertex_data[] =
	{
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f
	};

	std::size_t vertex_size = 3;
	std::size_t vertex_stride = sizeof(float) * vertex_size;
	std::size_t vertex_count = 6;

	quad_vbo = new vertex_buffer(sizeof(float) * vertex_size * vertex_count, vertex_data);
	quad_vao = new vertex_array();
	quad_vao->bind_attribute(VERTEX_POSITION_LOCATION, *quad_vbo, 3, vertex_attribute_type::float_32, vertex_stride, 0);
}

final_pass::~final_pass()
{
	delete quad_vao;
	delete quad_vbo;
}

void final_pass::render(render_context* context) const
{
	rasterizer->use_framebuffer(*framebuffer);
	
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	auto viewport = framebuffer->get_dimensions();
	rasterizer->set_viewport(0, 0, std::get<0>(viewport), std::get<1>(viewport));
	
	float2 resolution = {std::get<0>(viewport), std::get<1>(viewport)};

	// Change shader program
	rasterizer->use_program(*shader_program);

	// Upload shader parameters
	color_texture_input->upload(color_texture);
	bloom_texture_input->upload(bloom_texture);
	resolution_input->upload(resolution);

	// Draw quad
	rasterizer->draw_arrays(*quad_vao, drawing_mode::triangles, 0, 6);
}

void final_pass::set_color_texture(const texture_2d* texture)
{
	this->color_texture = texture;
}

void final_pass::set_bloom_texture(const texture_2d* texture)
{
	this->bloom_texture = texture;
}
