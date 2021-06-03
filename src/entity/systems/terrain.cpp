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

#include "terrain.hpp"
#include "entity/components/model.hpp"
#include "entity/components/collision.hpp"
#include "entity/components/transform.hpp"
#include "cart/relief-map.hpp"
#include "renderer/model.hpp"
#include "geom/mesh.hpp"
#include "geom/mesh-functions.hpp"
#include "renderer/vertex-attributes.hpp"
#include "gl/vertex-attribute-type.hpp"
#include "gl/drawing-mode.hpp"
#include "gl/vertex-buffer.hpp"
#include "resources/resource-manager.hpp"
#include "resources/image.hpp"
#include "utility/fundamental-types.hpp"
#include <limits>

namespace entity {
namespace system {

terrain::terrain(entity::registry& registry, ::resource_manager* resource_manager):
	updatable(registry),
	resource_manager(resource_manager)
{
	registry.on_construct<component::terrain>().connect<&terrain::on_terrain_construct>(this);
	registry.on_destroy<component::terrain>().connect<&terrain::on_terrain_destroy>(this);

	heightmap = resource_manager->load<image>("grassland-heightmap.png");
	heightmap_size = 2000.0f;
	heightmap_scale = 150.0f;
}

terrain::~terrain()
{}

void terrain::update(double t, double dt)
{
	registry.view<component::terrain, component::transform>().each(
		[this](entity::id entity_id, auto& terrain, auto& transform)
		{
			transform.local.translation = float3{(float)terrain.x * patch_size, 0.0f, (float)terrain.z * patch_size};
			transform.warp = true;
		});
}

void terrain::set_patch_size(float size)
{
	patch_size = size;
}

geom::mesh* terrain::generate_terrain_mesh(float size, int subdivisions)
{
	auto elevation = [](float u, float v) -> float
	{
		return 0.0f;
	};
	
	return cart::map_elevation(elevation, size, subdivisions);
}

model* terrain::generate_terrain_model(geom::mesh* terrain_mesh)
{
	// Allocate model
	model* terrain_model = new model();

	// Get model's VAO and VBO
	gl::vertex_buffer* vbo = terrain_model->get_vertex_buffer();
	gl::vertex_array* vao = terrain_model->get_vertex_array();

	// Resize VBO
	int vertex_size = 3 + 2 + 3 + 4 + 3;
	int vertex_stride = vertex_size * sizeof(float);
	vbo->resize(terrain_mesh->get_faces().size() * 3 * vertex_stride, nullptr);

	// Bind vertex attributes
	std::size_t offset = 0;
	vao->bind_attribute(VERTEX_POSITION_LOCATION, *vbo, 3, gl::vertex_attribute_type::float_32, vertex_stride, 0);
	offset += 3;
	vao->bind_attribute(VERTEX_TEXCOORD_LOCATION, *vbo, 2, gl::vertex_attribute_type::float_32, vertex_stride, sizeof(float) * offset);
	offset += 2;
	vao->bind_attribute(VERTEX_NORMAL_LOCATION, *vbo, 3, gl::vertex_attribute_type::float_32, vertex_stride, sizeof(float) * offset);
	offset += 3;
	vao->bind_attribute(VERTEX_TANGENT_LOCATION, *vbo, 4, gl::vertex_attribute_type::float_32, vertex_stride, sizeof(float) * offset);
	offset += 4;
	vao->bind_attribute(VERTEX_BARYCENTRIC_LOCATION, *vbo, 3, gl::vertex_attribute_type::float_32, vertex_stride, sizeof(float) * offset);
	offset += 3;
	
	// Create model group
	model_group* model_group = terrain_model->add_group("terrain");
	model_group->set_material(resource_manager->load<material>("forest-terrain.mtl"));
	model_group->set_drawing_mode(gl::drawing_mode::triangles);
	model_group->set_start_index(0);
	model_group->set_index_count(terrain_mesh->get_faces().size() * 3);

	return terrain_model;
}

void terrain::project_terrain_mesh(geom::mesh* terrain_mesh, const component::terrain& component)
{
	
	float offset_x = (float)component.x * patch_size;
	float offset_z = (float)component.z * patch_size;

	for (geom::mesh::vertex* vertex: terrain_mesh->get_vertices())
	{
		int pixel_x = (vertex->position[0] + offset_x + heightmap_size * 0.5f) / heightmap_size * (float)(heightmap->get_width() - 1);
		int pixel_y = (vertex->position[2] + offset_z + heightmap_size * 0.5f) / heightmap_size * (float)(heightmap->get_height() - 1);

		pixel_x = std::max<int>(0, std::min<int>(heightmap->get_width() - 1, pixel_x));
		pixel_y = std::max<int>(0, std::min<int>(heightmap->get_height() - 1, pixel_y));

		int pixel_index = (pixel_y * heightmap->get_width() + pixel_x) * heightmap->get_channels();
		const unsigned char* pixel = static_cast<const unsigned char*>(heightmap->get_pixels()) + pixel_index;

		float elevation = (static_cast<float>(*pixel) / 255.0f - 0.5) * heightmap_scale;
		vertex->position[1] = elevation;
	}
	
}

void terrain::update_terrain_model(model* terrain_model, geom::mesh* terrain_mesh)
{
	const std::vector<geom::mesh::face*>& faces = terrain_mesh->get_faces();
	const std::vector<geom::mesh::vertex*>& vertices = terrain_mesh->get_vertices();

	geom::aabb<float> bounds = calculate_bounds(*terrain_mesh);
	float bounds_width = bounds.max_point.x - bounds.min_point.x;
	float bounds_height = bounds.max_point.y - bounds.min_point.y;
	float bounds_depth = bounds.max_point.z - bounds.min_point.z;
	
	static const float3 barycentric_coords[3] =
	{
		float3{1, 0, 0},
		float3{0, 1, 0},
		float3{0, 0, 1}
	};

	int triangle_count = faces.size();
	int vertex_count = triangle_count * 3;
	int vertex_size = 3 + 2 + 3 + 4 + 3;

	// Allocate vertex data
	float* vertex_data = new float[vertex_size * vertex_count];

	// Allocate and calculate face normals
	float3* face_normals = new float3[faces.size()];
	calculate_face_normals(face_normals, *terrain_mesh);
	
	// Allocate and calculate vertex normals
	float3* vertex_normals = new float3[vertices.size()];
	for (std::size_t i = 0; i < vertices.size(); ++i)
	{
		const geom::mesh::vertex* vertex = vertices[i];

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
		
		vertex_normals[i] = n;
	}
	
	// Allocate and generate vertex texture coordinates
	float2* vertex_texcoords = new float2[vertices.size()];
	for (std::size_t i = 0; i < vertices.size(); ++i)
	{
		const geom::mesh::vertex* vertex = vertices[i];
		vertex_texcoords[i].x = (vertex->position.x - bounds.min_point.x) / bounds_width;
		vertex_texcoords[i].y = (vertex->position.z - bounds.min_point.z) / bounds_depth;
	}
	
	// Allocate and calculate vertex tangents
	float4* vertex_tangents = new float4[vertices.size()];
	calculate_vertex_tangents(vertex_tangents, vertex_texcoords, vertex_normals, *terrain_mesh);

	// Generate vertex data
	float* v = vertex_data;
	for (int i = 0; i < triangle_count; ++i)
	{
		const geom::mesh::face* triangle = faces[i];
		const geom::mesh::vertex* a = triangle->edge->vertex;
		const geom::mesh::vertex* b = triangle->edge->next->vertex;
		const geom::mesh::vertex* c = triangle->edge->previous->vertex;
		const geom::mesh::vertex* abc[] = {a, b, c};

		for (int j = 0; j < 3; ++j)
		{
			const geom::mesh::vertex* vertex = abc[j];
			const float3& position = vertex->position;
			const float2& texcoord = vertex_texcoords[vertex->index];
			const float3& normal = vertex_normals[vertex->index];
			const float4& tangent = vertex_tangents[vertex->index];
			const float3& barycentric = barycentric_coords[j];

			*(v++) = position.x;
			*(v++) = position.y;
			*(v++) = position.z;
			
			*(v++) = texcoord.x;
			*(v++) = texcoord.y;
			
			*(v++) = normal.x;
			*(v++) = normal.y;
			*(v++) = normal.z;
			
			*(v++) = tangent.x;
			*(v++) = tangent.y;
			*(v++) = tangent.z;
			*(v++) = tangent.w;
			
			*(v++) = barycentric.x;
			*(v++) = barycentric.y;
			*(v++) = barycentric.z;
		}
	}
	
	// Update bounds
	terrain_model->set_bounds(bounds);

	// Update VBO
	terrain_model->get_vertex_buffer()->update(0, vertex_count * vertex_size * sizeof(float), vertex_data);

	// Free vertex data
	delete[] face_normals;
	delete[] vertex_normals;
	delete[] vertex_texcoords;
	delete[] vertex_tangents;
	delete[] vertex_data;
}

void terrain::on_terrain_construct(entity::registry& registry, entity::id entity_id, component::terrain& component)
{
	geom::mesh* terrain_mesh = generate_terrain_mesh(patch_size, component.subdivisions);
	model* terrain_model = generate_terrain_model(terrain_mesh);
	project_terrain_mesh(terrain_mesh, component);
	update_terrain_model(terrain_model, terrain_mesh);

	// Assign the entity a collision component with the terrain mesh
	component::collision collision;
	collision.mesh = terrain_mesh;
	collision.bounds = calculate_bounds(*terrain_mesh);
	collision.mesh_accelerator.build(*collision.mesh);
	registry.assign_or_replace<component::collision>(entity_id, collision);

	// Assign the entity a model component with the terrain model
	component::model model;
	model.render_model = terrain_model;
	model.instance_count = 0;
	model.layers = 1;
	registry.assign_or_replace<component::model>(entity_id, model);

	// Assign the entity a transform component
	component::transform transform;
	transform.local = math::identity_transform<float>;
	transform.local.translation = float3{(float)component.x * patch_size, 0.0f, (float)component.z * patch_size};
	transform.warp = true;
	registry.assign_or_replace<component::transform>(entity_id, transform);
}

void terrain::on_terrain_destroy(entity::registry& registry, entity::id entity_id)
{
	/*
	if (auto it = terrain_map.find(entity_id); it != terrain_map.end())
	{
		delete std::get<0>(it->second);
		delete std::get<1>(it->second);
		terrain_map.erase(it);
	}
	*/
}

} // namespace system
} // namespace entity