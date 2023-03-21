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

#include "game/systems/subterrain-system.hpp"
#include "game/components/cavity-component.hpp"
#include <engine/entity/id.hpp>
#include <engine/render/model.hpp>
#include <engine/render/material.hpp>
#include <engine/geom/mesh-functions.hpp>
#include <engine/render/vertex-attribute.hpp>
#include <engine/gl/vertex-attribute.hpp>
#include <engine/gl/drawing-mode.hpp>
#include <engine/gl/vertex-buffer.hpp>
#include <engine/resources/resource-manager.hpp>
#include <engine/geom/marching-cubes.hpp>
#include <engine/geom/intersection.hpp>
#include <engine/utility/fundamental-types.hpp>
#include <array>
#include <limits>

subterrain_system::subterrain_system(entity::registry& registry, ::resource_manager* resource_manager):
	updatable_system(registry),
	resource_manager(resource_manager)
{

/*
	// Load subterrain materials
	subterrain_inside_material = nullptr;//resource_manager->load<::render::material>("subterrain-inside.mtl");
	subterrain_outside_material = nullptr;//resource_manager->load<::render::material>("subterrain-outside.mtl");

	// Allocate subterrain model
	subterrain_model = new ::render::model();

	// Create inside model group
	subterrain_inside_group = subterrain_model->add_group("inside");
	subterrain_inside_group->set_material(subterrain_inside_material);
	subterrain_inside_group->set_drawing_mode(gl::drawing_mode::triangles);
	subterrain_inside_group->set_start_index(0);
	subterrain_inside_group->set_index_count(0);

	// Create outside model group
	subterrain_outside_group = subterrain_model->add_group("outside");
	subterrain_outside_group->set_material(subterrain_outside_material);
	subterrain_outside_group->set_drawing_mode(gl::drawing_mode::triangles);
	subterrain_outside_group->set_start_index(0);
	subterrain_outside_group->set_index_count(0);

	// Determine vertex size (position, normal, barycentric)
	subterrain_model_vertex_size = 3 + 3 + 3;
	subterrain_model_vertex_stride = subterrain_model_vertex_size * sizeof(float);
	
	// Get model VBO and VAO
	gl::vertex_buffer* vbo = subterrain_model->get_vertex_buffer();
	gl::vertex_array* vao = subterrain_model->get_vertex_array();
	
	std::size_t attribute_offset = 0;
	
	// Define position vertex attribute
	gl::vertex_attribute position_attribute;
	position_attribute.buffer = vbo;
	position_attribute.offset = attribute_offset;
	position_attribute.stride = subterrain_model_vertex_stride;
	position_attribute.type = gl::vertex_attribute_type::float_32;
	position_attribute.components = 3;
	attribute_offset += position_attribute.components * sizeof(float);
	
	// Define normal vertex attribute
	gl::vertex_attribute normal_attribute;
	normal_attribute.buffer = vbo;
	normal_attribute.offset = attribute_offset;
	normal_attribute.stride = subterrain_model_vertex_stride;
	normal_attribute.type = gl::vertex_attribute_type::float_32;
	normal_attribute.components = 3;
	attribute_offset += normal_attribute.components * sizeof(float);
	
	// Define barycentric vertex attribute
	gl::vertex_attribute barycentric_attribute;
	barycentric_attribute.buffer = vbo;
	barycentric_attribute.offset = attribute_offset;
	barycentric_attribute.stride = subterrain_model_vertex_stride;
	barycentric_attribute.type = gl::vertex_attribute_type::float_32;
	barycentric_attribute.components = 3;
	attribute_offset += barycentric_attribute.components * sizeof(float);
	
	// Bind vertex attributes to VAO
	vao->bind(::render::vertex_attribute::position, position_attribute);
	vao->bind(::render::vertex_attribute::normal, normal_attribute);
	vao->bind(::render::vertex_attribute::barycentric, barycentric_attribute);

	// Calculate adjusted bounds to fit isosurface resolution
	//isosurface_resolution = 0.325f;
	isosurface_resolution = 0.5f;
	float ideal_volume_size = 200.0f;
	int octree_depth = std::floor(std::log(ideal_volume_size / isosurface_resolution) / std::log(2)) + 1;
	float adjusted_volume_size = std::pow(2.0f, octree_depth) * isosurface_resolution;

	// Set subterrain bounds
	subterrain_bounds.min = float3{-0.5f, -1.0f, -0.5f} * adjusted_volume_size;
	subterrain_bounds.max = float3{ 0.5f,  0.0f,  0.5f} * adjusted_volume_size;
	
	// Set subterrain model bounds
	subterrain_model->set_bounds(subterrain_bounds);

	// Allocate cube tree
	cube_tree = new ::cube_tree(subterrain_bounds, octree_depth);

	// Allocate mesh
	subterrain_mesh = new geom::mesh();

	first_run = true;
	*/
}

subterrain_system::~subterrain_system()
{
	/*
	delete subterrain_model;
	delete subterrain_mesh;
	*/
}

void subterrain_system::update(float t, float dt)
{
	/*
	if (first_run)
	{
		first_run = false;
		//auto subterrain_entity = registry.create();
		//registry.assign<model_component>(subterrain_entity, subterrain_model);
		
		subterrain_static_mesh = new scene::static_mesh(subterrain_model);
		collection->add_object(subterrain_static_mesh);
	}

	bool digging = false;

	registry.view<cavity_component>().each(
		[this, &digging](entity::id entity_id, auto& cavity)
		{
			this->dig(cavity.position, cavity.radius);
			this->registry.destroy(entity_id);

			digging = true;
		});
	
	if (digging)
	{
		
		//std::cout << "regenerating subterrain mesh...\n";
		regenerate_subterrain_mesh();
		//std::cout << "regenerating subterrain mesh... done\n";

		//std::cout << "regenerating subterrain model...\n";
		regenerate_subterrain_model();
		//std::cout << "regenerating subterrain model... done\n";
	}
	*/
}

void subterrain_system::set_scene(scene::collection* collection)
{
	//this->collection = collection;
}

void subterrain_system::regenerate_subterrain_mesh()
{
	/*
	delete subterrain_mesh;
	subterrain_mesh = new geom::mesh();
	subterrain_vertices.clear();
	subterrain_triangles.clear();
	subterrain_vertex_map.clear();

	//std::cout << "marching...\n";
	merged = 0;
	march(cube_tree);
	//std::cout << "merged " << merged << " vertices\n";
	//std::cout << "marching...done\n";

	//std::cout << "vertex count: " << subterrain_vertices.size() << std::endl;
	//std::cout << "triangle count: " << subterrain_triangles.size() << std::endl;

	//std::cout << "creating mesh...\n";
	create_triangle_mesh(*subterrain_mesh, subterrain_vertices, subterrain_triangles);
	//std::cout << "creating mesh... done\n";
	*/
}

void subterrain_system::march(::cube_tree* node)
{
	/*
	if (!node->is_leaf())
	{
		for (::cube_tree* child: node->children)
			march(child);
		return;
	}
	else if (node->depth != node->max_depth)
	{
		return;
	}

	// Get node bounds
	const geom::box<float>& bounds = node->get_bounds();

	// Polygonize cube
	float vertex_buffer[12 * 3];
	std::uint_fast8_t vertex_count;
	std::int_fast8_t triangle_buffer[5 * 3];
	std::uint_fast8_t triangle_count;
	const float* corners = &node->corners[0][0];
	const float* distances = &node->distances[0];
	geom::mc::polygonize(vertex_buffer, &vertex_count, triangle_buffer, &triangle_count, corners, distances);

	// Remap local vertex buffer indices (0-11) to mesh vertex indices
	std::uint_fast32_t vertex_remap[12];
	for (int i = 0; i < vertex_count; ++i)
	{
		const float3& vertex = reinterpret_cast<const float3&>(vertex_buffer[i * 3]);

		if (auto it = subterrain_vertex_map.find(vertex); it != subterrain_vertex_map.end())
		{
			vertex_remap[i] = it->second;
			++merged;
		}
		else
		{
			vertex_remap[i] = subterrain_vertices.size();
			subterrain_vertex_map[vertex] = subterrain_vertices.size();
			subterrain_vertices.push_back(vertex);
		}
	}

	// Add triangles
	for (std::uint_fast32_t i = 0; i < triangle_count; ++i)
	{
		subterrain_triangles.push_back(
			{
				vertex_remap[triangle_buffer[i * 3]],
				vertex_remap[triangle_buffer[i * 3 + 1]],
				vertex_remap[triangle_buffer[i * 3 + 2]]
			});
	}
	*/
}

void subterrain_system::regenerate_subterrain_model()
{
	/*
	float3* face_normals = new float3[subterrain_mesh->get_faces().size()];
	calculate_face_normals(face_normals, *subterrain_mesh);

	static const float3 barycentric_coords[3] =
	{
		float3{1, 0, 0},
		float3{0, 1, 0},
		float3{0, 0, 1}
	};

	float* vertex_data = new float[subterrain_model_vertex_size * subterrain_mesh->get_faces().size() * 3];
	float* v = vertex_data;
	for (std::size_t i = 0; i < subterrain_mesh->get_faces().size(); ++i)
	{
		geom::mesh::face* face = subterrain_mesh->get_faces()[i];
		geom::mesh::edge* ab = face->edge;
		geom::mesh::edge* bc = face->edge->next;
		geom::mesh::edge* ca = face->edge->previous;
		geom::mesh::vertex* a = ab->vertex;
		geom::mesh::vertex* b = bc->vertex;
		geom::mesh::vertex* c = ca->vertex;
		geom::mesh::vertex* vertices[3] = {a, b, c};

		for (std::size_t j = 0; j < 3; ++j)
		{
			geom::mesh::vertex* vertex = vertices[j];

			float3 n = {0, 0, 0};
			geom::mesh::edge* start = vertex->edge;
			geom::mesh::edge* edge = start;
			do
			{
				if (edge->face)
				{
					n += face_normals[edge->face->index];
				}

				edge = edge->previous->symmetric;
			}
			while (edge != start);
			n = math::normalize(n);

			//float3 n = reinterpret_cast<const float3&>(face_normals[i * 3]);

			*(v++) = vertex->position[0];
			*(v++) = vertex->position[1];
			*(v++) = vertex->position[2];

			*(v++) = n[0];
			*(v++) = n[1];
			*(v++) = n[2];

			*(v++) = barycentric_coords[j][0];
			*(v++) = barycentric_coords[j][1];
			*(v++) = barycentric_coords[j][2];
		}
	}

	// Resized VBO and upload vertex data
	gl::vertex_buffer* vbo = subterrain_model->get_vertex_buffer();
	vbo->resize(subterrain_mesh->get_faces().size() * 3 * subterrain_model_vertex_stride, vertex_data);

	// Deallocate vertex data
	delete[] face_normals;
	delete[] vertex_data;

	// Update model groups
	subterrain_inside_group->set_index_count(subterrain_mesh->get_faces().size() * 3);
	subterrain_outside_group->set_index_count(subterrain_mesh->get_faces().size() * 3);
	*/
}

void subterrain_system::dig(const float3& position, float radius)
{
	/*
	// Construct region containing the cavity sphere
	geom::box<float> region = {position, position};
	for (int i = 0; i < 3; ++i)
	{
		region.min[i] -= radius + isosurface_resolution;
		region.max[i] += radius + isosurface_resolution;
	}

	// Subdivide the octree to the maximum depth within the region
	cube_tree->subdivide_max(region);

	// Query all octree leaf nodes within the region
	std::list<::cube_tree*> nodes;
	cube_tree->visit_leaves(region,
		[&position, radius](::cube_tree& node)
		{
			for (int i = 0; i < 8; ++i)
			{
				// For outside normals (also set node initial distance to +infinity)
				//float distance = math::length(node->corners[i] - position) - radius;
				// if (distance < node->distances[i])

				float distance = radius - math::length(node.corners[i] - position);
				if (distance > node.distances[i])
					node.distances[i] = distance;
			}
		});
	*/
}

