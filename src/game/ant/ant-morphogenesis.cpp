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

#include "game/ant/ant-morphogenesis.hpp"
#include "game/ant/ant-bone-set.hpp"
#include "game/ant/ant-skeleton.hpp"
#include <engine/render/material.hpp>
#include <engine/render/vertex-attribute.hpp>
#include <engine/math/quaternion.hpp>
#include <engine/debug/log.hpp>
#include <engine/geom/primitives/box.hpp>
#include <engine/animation/bone.hpp>
#include <unordered_set>
#include <optional>

namespace {

/**
 * Reskins model vertices.
 *
 * @param vertex_data Vertex buffer data.
 * @param vertex_count Number of vertices to reskin.
 * @param position_attribute Vertex position attribute.
 * @param normal_attribute Vertex normal attribute.
 * @param tangent_attribute Vertex tangent attribute.
 * @param bone_index_attribute Vertex bone index attribute.
 * @param reskin_map Map of old bone index to a tuple containing the new bone index and a vertex transformation.
 */
void reskin_vertices
(
	std::byte* vertex_data,
	std::size_t vertex_count,
	const gl::vertex_attribute& position_attribute,
	const gl::vertex_attribute& normal_attribute,
	const gl::vertex_attribute& tangent_attribute,
	const gl::vertex_attribute& bone_index_attribute,
	const std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>>& reskin_map
)
{
	std::byte* position_data = vertex_data + position_attribute.offset;
	std::byte* normal_data = vertex_data + normal_attribute.offset;
	std::byte* tangent_data = vertex_data + tangent_attribute.offset;
	std::byte* bone_index_data = vertex_data + bone_index_attribute.offset;
	
	for (std::size_t i = 0; i < vertex_count; ++i)
	{
		// Get bone index of current vertex
		std::uint16_t& bone_index = reinterpret_cast<std::uint16_t&>(*(bone_index_data + bone_index_attribute.stride * i));
		
		// Ignore vertices with unmapped bone indices
		auto entry = reskin_map.find(static_cast<bone_index_type>(bone_index));
		if (entry == reskin_map.end())
		{
			continue;
		}
		
		const auto& [new_bone_index, transform] = entry->second;
		
		// Update bone index
		bone_index = static_cast<std::uint16_t>(new_bone_index);
		
		// Get vertex attributes
		float* px = reinterpret_cast<float*>(position_data + position_attribute.stride * i);
		float* py = px + 1;
		float* pz = py + 1;
		float* nx = reinterpret_cast<float*>(normal_data + normal_attribute.stride * i);
		float* ny = nx + 1;
		float* nz = ny + 1;
		float* tx = reinterpret_cast<float*>(tangent_data + tangent_attribute.stride * i);
		float* ty = tx + 1;
		float* tz = ty + 1;
		
		// Transform vertex attributes
		const float3 position = (*transform) * float3{*px, *py, *pz};
		const float3 normal = math::normalize(transform->rotation * float3{*nx, *ny, *nz});
		const float3 tangent = math::normalize(transform->rotation * float3{*tx, *ty, *tz});
		
		// Update vertex attributes
		*px = position.x();
		*py = position.y();
		*pz = position.z();
		*nx = normal.x();
		*ny = normal.y();
		*nz = normal.z();
		*tx = tangent.x();
		*ty = tangent.y();
		*tz = tangent.z();
	}
}

/**
 * Tags the vertices of a body part by storing a value in the fourth bone index.
 *
 * @param vertex_data Vertex buffer data.
 * @param bone_index_attribute Vertex bone index attribute.
 * @param reskin_map Map of old bone index to a tuple containing the new bone index and a vertex transformation.
 */
void tag_vertices
(
	std::span<std::byte> vertex_data,
	const gl::vertex_attribute& bone_index_attribute,
	std::uint16_t vertex_tag
)
{
	std::byte* bone_index_data = vertex_data.data() + bone_index_attribute.offset;
	
	for (std::size_t i = 0; i < vertex_data.size(); ++i)
	{
		// Get bone indices of current vertex
		std::uint16_t* bone_indices = reinterpret_cast<std::uint16_t*>(bone_index_data + bone_index_attribute.stride * i);
		
		// Tag fourth bone index
		bone_indices[3] = vertex_tag;
	}
}

/**
 * Calculates the bounds of vertex data.
 *
 * @param vertex_data Pointer to vertex data.
 * @param vertex_count Number of vertices.
 * @param position_attribute Vertex position attribute.
 *
 * @return Bounds of the vertex data.
 */
[[nodiscard]] geom::box<float> calculate_bounds
(
	const std::byte* vertex_data,
	std::size_t vertex_count,
	const gl::vertex_attribute& position_attribute
)
{
	const std::byte* position_data = vertex_data + position_attribute.offset;
	
	geom::box<float> bounds
	{
		{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()},
		{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()}
	};
	
	for (std::size_t i = 0; i < vertex_count; ++i)
	{
		const float* px = reinterpret_cast<const float*>(position_data + position_attribute.stride * i);
		const float* py = px + 1;
		const float* pz = py + 1;
		
		bounds.extend(float3{*px, *py, *pz});
	}
	
	return bounds;
}

/**
 * Generates an ant exoskeleton material.
 *
 * @param pigmentation Ant pigmentation phene.
 * @param sculpturing Ant sculpturing phene.
 *
 * @return Generated ant exoskeleton material.
 */
[[nodiscard]] std::unique_ptr<render::material> generate_ant_exoskeleton_material
(
	const ant_pigmentation_phene& pigmentation,
	const ant_sculpturing_phene& sculpturing
)
{
	// Allocate copy of pigmentation material
	std::unique_ptr<render::material> exoskeleton_material = std::make_unique<render::material>(*pigmentation.material);
	
	// Set roughness variable
	exoskeleton_material->set_variable("roughness", std::make_shared<render::material_float>(1, sculpturing.roughness));
	
	// Set normal map variable
	exoskeleton_material->set_variable("normal_map", std::make_shared<render::material_texture_2d>(1, sculpturing.normal_map));
	
	return exoskeleton_material;
}

} // namespace

std::unique_ptr<render::model> ant_morphogenesis(const ant_phenome& phenome)
{
	// Generate exoskeleton material
	std::shared_ptr<render::material> exoskeleton_material = generate_ant_exoskeleton_material(*phenome.pigmentation, *phenome.sculpturing);
	
	// Get body part models
	const render::model* mesosoma_model = phenome.mesosoma->model.get();
	const render::model* legs_model = phenome.legs->model.get();
	const render::model* head_model = phenome.head->model.get();
	const render::model* mandibles_model = phenome.mandibles->model.get();
	const render::model* antennae_model = phenome.antennae->model.get();
	const render::model* waist_model = phenome.waist->model.get();
	const render::model* gaster_model = phenome.gaster->model.get();
	const render::model* sting_model = phenome.sting->model.get();
	const render::model* eyes_model = phenome.eyes->model.get();
	const render::model* lateral_ocelli_model = phenome.ocelli->lateral_ocelli_model.get();
	const render::model* median_ocellus_model = phenome.ocelli->median_ocellus_model.get();
	const render::model* forewings_model = phenome.wings->forewings_model.get();
	const render::model* hindwings_model = phenome.wings->hindwings_model.get();
	
	// Check for presence of required part models
	if (!mesosoma_model)
	{
		throw std::runtime_error("Ant phenome missing mesosoma model");
	}
	if (!legs_model)
	{
		throw std::runtime_error("Ant phenome missing legs model");
	}
	if (!head_model)
	{
		throw std::runtime_error("Ant phenome missing head model");
	}
	if (!mandibles_model)
	{
		throw std::runtime_error("Ant phenome missing mandibles model");
	}
	if (!antennae_model)
	{
		throw std::runtime_error("Ant phenome missing antennae model");
	}
	if (!waist_model)
	{
		throw std::runtime_error("Ant phenome missing waist model");
	}
	if (!gaster_model)
	{
		throw std::runtime_error("Ant phenome missing gaster model");
	}
	if (phenome.sting->present && !sting_model)
	{
		throw std::runtime_error("Ant phenome missing sting model");
	}
	if (phenome.eyes->present && !eyes_model)
	{
		throw std::runtime_error("Ant phenome missing eyes model");
	}
	if (phenome.ocelli->lateral_ocelli_present && !lateral_ocelli_model)
	{
		throw std::runtime_error("Ant phenome missing lateral ocelli model");
	}
	if (phenome.ocelli->median_ocellus_present && !median_ocellus_model)
	{
		throw std::runtime_error("Ant phenome missing median ocellus model");
	}
	if (phenome.wings->present)
	{
		if (!forewings_model)
		{
			throw std::runtime_error("Ant phenome missing forewings model");
		}
		if (!hindwings_model)
		{
			throw std::runtime_error("Ant phenome missing hindwings model");
		}
	}
	
	// Get body part vertex buffers
	const gl::vertex_buffer* mesosoma_vbo = mesosoma_model->get_vertex_buffer().get();
	const gl::vertex_buffer* legs_vbo = legs_model->get_vertex_buffer().get();
	const gl::vertex_buffer* head_vbo = head_model->get_vertex_buffer().get();
	const gl::vertex_buffer* mandibles_vbo = mandibles_model->get_vertex_buffer().get();
	const gl::vertex_buffer* antennae_vbo = antennae_model->get_vertex_buffer().get();
	const gl::vertex_buffer* waist_vbo = waist_model->get_vertex_buffer().get();
	const gl::vertex_buffer* gaster_vbo = gaster_model->get_vertex_buffer().get();
	const gl::vertex_buffer* sting_vbo = (phenome.sting->present) ? sting_model->get_vertex_buffer().get() : nullptr;
	const gl::vertex_buffer* eyes_vbo = (phenome.eyes->present) ? eyes_model->get_vertex_buffer().get() : nullptr;
	const gl::vertex_buffer* lateral_ocelli_vbo = (phenome.ocelli->lateral_ocelli_present) ? lateral_ocelli_model->get_vertex_buffer().get() : nullptr;
	const gl::vertex_buffer* median_ocellus_vbo = (phenome.ocelli->median_ocellus_present) ? median_ocellus_model->get_vertex_buffer().get() : nullptr;
	const gl::vertex_buffer* forewings_vbo = (phenome.wings->present) ? forewings_model->get_vertex_buffer().get() : nullptr;
	const gl::vertex_buffer* hindwings_vbo = (phenome.wings->present) ? hindwings_model->get_vertex_buffer().get() : nullptr;
	
	// Determine combined size of vertex buffers and save offsets
	std::size_t vertex_buffer_size = 0;
	const std::size_t mesosoma_vbo_offset = vertex_buffer_size;
	vertex_buffer_size += mesosoma_vbo->size();
	const std::size_t legs_vbo_offset = vertex_buffer_size;
	vertex_buffer_size += legs_vbo->size();
	const std::size_t head_vbo_offset = vertex_buffer_size;
	vertex_buffer_size += head_vbo->size();
	const std::size_t mandibles_vbo_offset = vertex_buffer_size;
	vertex_buffer_size += mandibles_vbo->size();
	const std::size_t antennae_vbo_offset = vertex_buffer_size;
	vertex_buffer_size += antennae_vbo->size();
	const std::size_t waist_vbo_offset = vertex_buffer_size;
	vertex_buffer_size += waist_vbo->size();
	const std::size_t gaster_vbo_offset = vertex_buffer_size;
	vertex_buffer_size += gaster_vbo->size();
	const std::size_t sting_vbo_offset = vertex_buffer_size;
	if (phenome.sting->present)
	{
		vertex_buffer_size += sting_vbo->size();
	}
	const std::size_t eyes_vbo_offset = vertex_buffer_size;
	if (phenome.eyes->present)
	{
		vertex_buffer_size += eyes_vbo->size();
	}
	const std::size_t lateral_ocelli_vbo_offset = vertex_buffer_size;
	if (phenome.ocelli->lateral_ocelli_present)
	{
		vertex_buffer_size += lateral_ocelli_vbo->size();
	}
	const std::size_t median_ocellus_vbo_offset = vertex_buffer_size;
	if (phenome.ocelli->median_ocellus_present)
	{
		vertex_buffer_size += median_ocellus_vbo->size();
	}
	std::size_t forewings_vbo_offset;
	std::size_t hindwings_vbo_offset;
	if (phenome.wings->present)
	{
		forewings_vbo_offset = vertex_buffer_size;
		vertex_buffer_size += forewings_vbo->size();
		hindwings_vbo_offset = vertex_buffer_size;
		vertex_buffer_size += hindwings_vbo->size();
	}
	
	// Allocate combined vertex buffer data
	std::vector<std::byte> vertex_buffer_data(vertex_buffer_size);
	
	// Read body part vertex buffer data into combined vertex buffer data
	mesosoma_vbo->read({vertex_buffer_data.data() + mesosoma_vbo_offset, mesosoma_vbo->size()});
	legs_vbo->read({vertex_buffer_data.data() + legs_vbo_offset, legs_vbo->size()});
	head_vbo->read({vertex_buffer_data.data() + head_vbo_offset, head_vbo->size()});
	mandibles_vbo->read({vertex_buffer_data.data() + mandibles_vbo_offset, mandibles_vbo->size()});
	antennae_vbo->read({vertex_buffer_data.data() + antennae_vbo_offset, antennae_vbo->size()});
	waist_vbo->read({vertex_buffer_data.data() + waist_vbo_offset, waist_vbo->size()});
	gaster_vbo->read({vertex_buffer_data.data() + gaster_vbo_offset, gaster_vbo->size()});
	if (phenome.sting->present)
	{
		sting_vbo->read({vertex_buffer_data.data() + sting_vbo_offset, sting_vbo->size()});
	}
	if (phenome.eyes->present)
	{
		eyes_vbo->read({vertex_buffer_data.data() + eyes_vbo_offset, eyes_vbo->size()});
	}
	if (phenome.ocelli->lateral_ocelli_present)
	{
		lateral_ocelli_vbo->read({vertex_buffer_data.data() + lateral_ocelli_vbo_offset, lateral_ocelli_vbo->size()});
	}
	if (phenome.ocelli->median_ocellus_present)
	{
		median_ocellus_vbo->read({vertex_buffer_data.data() + median_ocellus_vbo_offset, median_ocellus_vbo->size()});
	}
	if (phenome.wings->present)
	{
		forewings_vbo->read({vertex_buffer_data.data() + forewings_vbo_offset, forewings_vbo->size()});
		hindwings_vbo->read({vertex_buffer_data.data() + hindwings_vbo_offset, hindwings_vbo->size()});
	}
	
	// Allocate model
	std::unique_ptr<render::model> model = std::make_unique<render::model>();
	
	// Setup model VAO
	gl::vertex_array& model_vao = *model->get_vertex_array();
	for (auto [location, attribute]: mesosoma_model->get_vertex_array()->attributes())
	{
		attribute.buffer = model->get_vertex_buffer().get();
		model_vao.bind(location, attribute);
	}
	
	// Get vertex attributes
	const gl::vertex_attribute* position_attribute = nullptr;
	const gl::vertex_attribute* normal_attribute = nullptr;
	const gl::vertex_attribute* tangent_attribute = nullptr;
	const gl::vertex_attribute* bone_index_attribute = nullptr;
	const auto& vertex_attribute_map = model_vao.attributes();
	if (auto it = vertex_attribute_map.find(render::vertex_attribute::position); it != vertex_attribute_map.end())
	{
		position_attribute = &it->second;
	}
	if (auto it = vertex_attribute_map.find(render::vertex_attribute::normal); it != vertex_attribute_map.end())
	{
		normal_attribute = &it->second;
	}
	if (auto it = vertex_attribute_map.find(render::vertex_attribute::tangent); it != vertex_attribute_map.end())
	{
		tangent_attribute = &it->second;
	}
	if (auto it = vertex_attribute_map.find(render::vertex_attribute::bone_index); it != vertex_attribute_map.end())
	{
		bone_index_attribute = &it->second;
	}
	
	// Generate ant skeleton
	::skeleton& skeleton = model->get_skeleton();
	ant_bone_set bones;
	generate_ant_skeleton(skeleton, bones, phenome);
	const auto& rest_pose = skeleton.get_rest_pose();
	
	// Get number of vertices for each body part
	const std::uint32_t mesosoma_vertex_count = (mesosoma_model->get_groups()).front().index_count;
	const std::uint32_t legs_vertex_count = (legs_model->get_groups()).front().index_count;
	const std::uint32_t head_vertex_count = (head_model->get_groups()).front().index_count;
	const std::uint32_t mandibles_vertex_count = (mandibles_model->get_groups()).front().index_count;
	const std::uint32_t antennae_vertex_count = (antennae_model->get_groups()).front().index_count;
	const std::uint32_t waist_vertex_count = (waist_model->get_groups()).front().index_count;
	const std::uint32_t gaster_vertex_count = (gaster_model->get_groups()).front().index_count;
	const std::uint32_t sting_vertex_count = (phenome.sting->present) ? (sting_model->get_groups()).front().index_count : 0;
	const std::uint32_t eyes_vertex_count = (phenome.eyes->present) ? (eyes_model->get_groups()).front().index_count : 0;
	const std::uint32_t lateral_ocelli_vertex_count = (phenome.ocelli->lateral_ocelli_present) ? (lateral_ocelli_model->get_groups()).front().index_count : 0;
	const std::uint32_t median_ocellus_vertex_count = (phenome.ocelli->median_ocellus_present) ? (median_ocellus_model->get_groups()).front().index_count : 0;
	const std::uint32_t forewings_vertex_count = (phenome.wings->present) ? (forewings_model->get_groups()).front().index_count : 0;
	const std::uint32_t hindwings_vertex_count = (phenome.wings->present) ? (hindwings_model->get_groups()).front().index_count : 0;
	
	// Get body part skeletons
	const ::skeleton& mesosoma_skeleton = phenome.mesosoma->model->get_skeleton();
	const ::skeleton& legs_skeleton = phenome.legs->model->get_skeleton();
	const ::skeleton& head_skeleton = phenome.head->model->get_skeleton();
	const ::skeleton& mandibles_skeleton = phenome.mandibles->model->get_skeleton();
	const ::skeleton& antennae_skeleton = phenome.antennae->model->get_skeleton();
	const ::skeleton& waist_skeleton = phenome.waist->model->get_skeleton();
	const ::skeleton& gaster_skeleton = phenome.gaster->model->get_skeleton();
	const ::skeleton* sting_skeleton = (phenome.sting->present) ? &phenome.sting->model->get_skeleton() : nullptr;
	const ::skeleton* eyes_skeleton = (phenome.eyes->present) ? &phenome.eyes->model->get_skeleton() : nullptr;
	const ::skeleton* lateral_ocelli_skeleton = (phenome.ocelli->lateral_ocelli_present) ? &phenome.ocelli->lateral_ocelli_model->get_skeleton() : nullptr;
	const ::skeleton* median_ocellus_skeleton = (phenome.ocelli->median_ocellus_present) ? &phenome.ocelli->median_ocellus_model->get_skeleton() : nullptr;
	const ::skeleton* forewings_skeleton = (phenome.wings->present) ? &phenome.wings->forewings_model->get_skeleton() : nullptr;
	const ::skeleton* hindwings_skeleton = (phenome.wings->present) ? &phenome.wings->hindwings_model->get_skeleton() : nullptr;
	
	auto get_bone_transform = [](const ::skeleton& skeleton, hash::fnv1a32_t bone_name)
	{
		return skeleton.get_rest_pose().get_relative_transform(*skeleton.get_bone_index(bone_name));
	};
	
	// Calculate transformations from part space to body space
	const math::transform<float> legs_to_body = math::transform<float>::identity();
	const math::transform<float> head_to_body = rest_pose.get_absolute_transform(*bones.mesosoma) * get_bone_transform(mesosoma_skeleton, "head");
	const math::transform<float> mandible_l_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "mandible_l");
	const math::transform<float> mandible_r_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "mandible_r");
	const math::transform<float> antenna_l_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "antenna_l");
	const math::transform<float> antenna_r_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "antenna_r");
	const math::transform<float> waist_to_body = rest_pose.get_absolute_transform(*bones.mesosoma) * get_bone_transform(mesosoma_skeleton, "petiole");
	
	math::transform<float> gaster_to_body;
	if (phenome.waist->postpetiole_present)
	{
		gaster_to_body = rest_pose.get_absolute_transform(*bones.postpetiole) * get_bone_transform(waist_skeleton, "gaster");
	}
	else if (phenome.waist->petiole_present)
	{
		gaster_to_body = rest_pose.get_absolute_transform(*bones.petiole) * get_bone_transform(waist_skeleton, "gaster");
	}
	else
	{
		gaster_to_body = rest_pose.get_absolute_transform(*bones.mesosoma) * get_bone_transform(waist_skeleton, "gaster");
	}
	
	math::transform<float> sting_to_body;
	if (phenome.sting->present)
	{
		sting_to_body = gaster_to_body * get_bone_transform(gaster_skeleton, "sting");
	}
	
	math::transform<float> eye_l_to_body;
	math::transform<float> eye_r_to_body;
	if (phenome.eyes->present)
	{
		eye_l_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "eye_l");
		eye_r_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "eye_r");
	}
	
	math::transform<float> ocellus_l_to_body;
	math::transform<float> ocellus_r_to_body;
	math::transform<float> ocellus_m_to_body;
	if (phenome.ocelli->lateral_ocelli_present)
	{
		ocellus_l_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "ocellus_l");
		ocellus_r_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "ocellus_r");
	}
	if (phenome.ocelli->median_ocellus_present)
	{
		ocellus_m_to_body = rest_pose.get_absolute_transform(*bones.head) * get_bone_transform(head_skeleton, "ocellus_m");
	}
	
	math::transform<float> forewing_l_to_body;
	math::transform<float> forewing_r_to_body;
	math::transform<float> hindwing_l_to_body;
	math::transform<float> hindwing_r_to_body;
	if (phenome.wings->present)
	{
		forewing_l_to_body = rest_pose.get_absolute_transform(*bones.mesosoma) * get_bone_transform(mesosoma_skeleton, "forewing_l");
		forewing_r_to_body = rest_pose.get_absolute_transform(*bones.mesosoma) * get_bone_transform(mesosoma_skeleton, "forewing_r");
		hindwing_l_to_body = rest_pose.get_absolute_transform(*bones.mesosoma) * get_bone_transform(mesosoma_skeleton, "hindwing_l");
		hindwing_r_to_body = rest_pose.get_absolute_transform(*bones.mesosoma) * get_bone_transform(mesosoma_skeleton, "hindwing_r");
	}
	
	// Build legs vertex reskin map
	const std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> legs_reskin_map
	{
		{*legs_skeleton.get_bone_index("procoxa_l"),     {*bones.procoxa_l,    &legs_to_body}},
		{*legs_skeleton.get_bone_index("profemur_l"),    {*bones.profemur_l,   &legs_to_body}},
		{*legs_skeleton.get_bone_index("protibia_l"),    {*bones.protibia_l,   &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus1_l"),  {*bones.protarsus_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus2_l"),  {*bones.protarsus_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus3_l"),  {*bones.protarsus_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus4_l"),  {*bones.protarsus_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus5_l"),  {*bones.protarsus_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("procoxa_r"),     {*bones.procoxa_r,    &legs_to_body}},
		{*legs_skeleton.get_bone_index("profemur_r"),    {*bones.profemur_r,   &legs_to_body}},
		{*legs_skeleton.get_bone_index("protibia_r"),    {*bones.protibia_r,   &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus1_r"),  {*bones.protarsus_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus2_r"),  {*bones.protarsus_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus3_r"),  {*bones.protarsus_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus4_r"),  {*bones.protarsus_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("protarsus5_r"),  {*bones.protarsus_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesocoxa_l"),    {*bones.mesocoxa_l,   &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesofemur_l"),   {*bones.mesofemur_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotibia_l"),   {*bones.mesotibia_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus1_l"), {*bones.mesotarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus2_l"), {*bones.mesotarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus3_l"), {*bones.mesotarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus4_l"), {*bones.mesotarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus5_l"), {*bones.mesotarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesocoxa_r"),    {*bones.mesocoxa_r,   &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesofemur_r"),   {*bones.mesofemur_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotibia_r"),   {*bones.mesotibia_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus1_r"), {*bones.mesotarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus2_r"), {*bones.mesotarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus3_r"), {*bones.mesotarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus4_r"), {*bones.mesotarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("mesotarsus5_r"), {*bones.mesotarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metacoxa_l"),    {*bones.metacoxa_l,   &legs_to_body}},
		{*legs_skeleton.get_bone_index("metafemur_l"),   {*bones.metafemur_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatibia_l"),   {*bones.metatibia_l,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus1_l"), {*bones.metatarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus2_l"), {*bones.metatarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus3_l"), {*bones.metatarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus4_l"), {*bones.metatarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus5_l"), {*bones.metatarsus_l, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metacoxa_r"),    {*bones.metacoxa_r,   &legs_to_body}},
		{*legs_skeleton.get_bone_index("metafemur_r"),   {*bones.metafemur_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatibia_r"),   {*bones.metatibia_r,  &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus1_r"), {*bones.metatarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus2_r"), {*bones.metatarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus3_r"), {*bones.metatarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus4_r"), {*bones.metatarsus_r, &legs_to_body}},
		{*legs_skeleton.get_bone_index("metatarsus5_r"), {*bones.metatarsus_r, &legs_to_body}}
	};
	
	// Build head vertex reskin map
	const std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> head_reskin_map
	{
		{*head_skeleton.get_bone_index("head"), {*bones.head, &head_to_body}}
	};
	
	// Build mandibles vertex reskin map
	const std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> mandibles_reskin_map
	{
		{*mandibles_skeleton.get_bone_index("mandible_l"), {*bones.mandible_l, &mandible_l_to_body}},
		{*mandibles_skeleton.get_bone_index("mandible_r"), {*bones.mandible_r, &mandible_r_to_body}}
	};
	
	// Build antennae vertex reskin map
	std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> antennae_reskin_map
	{
		{*antennae_skeleton.get_bone_index("antennomere1_l"), {*bones.antennomere1_l, &antenna_l_to_body}},
		{*antennae_skeleton.get_bone_index("antennomere2_l"), {*bones.antennomere2_l, &antenna_l_to_body}},
		{*antennae_skeleton.get_bone_index("antennomere1_r"), {*bones.antennomere1_r, &antenna_r_to_body}},
		{*antennae_skeleton.get_bone_index("antennomere2_r"), {*bones.antennomere2_r, &antenna_r_to_body}}
	};
	for (std::uint8_t i = 3; i <= phenome.antennae->total_antennomere_count; ++i)
	{
		const std::string antennomere_l_name = std::format("antennomere{}_l", i);
		const std::string antennomere_r_name = std::format("antennomere{}_r", i);
		
		const hash::fnv1a32_t antennomere_l_key = hash::fnv1a32<char>(antennomere_l_name);
		const hash::fnv1a32_t antennomere_r_key = hash::fnv1a32<char>(antennomere_r_name);
		
		antennae_reskin_map.emplace(*antennae_skeleton.get_bone_index(antennomere_l_key), std::tuple(*bones.antennomere2_l, &antenna_l_to_body));
		antennae_reskin_map.emplace(*antennae_skeleton.get_bone_index(antennomere_r_key), std::tuple(*bones.antennomere2_r, &antenna_r_to_body));
	}
	
	// Build waist vertex reskin map
	std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> waist_reskin_map;
	if (phenome.waist->petiole_present)
	{
		waist_reskin_map.emplace(*waist_skeleton.get_bone_index("petiole"), std::tuple(*bones.petiole, &waist_to_body));
	}
	if (phenome.waist->postpetiole_present)
	{
		waist_reskin_map.emplace(*waist_skeleton.get_bone_index("postpetiole"), std::tuple(*bones.postpetiole, &waist_to_body));
	}
	
	// Build gaster vertex reskin map
	const std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> gaster_reskin_map
	{
		{*gaster_skeleton.get_bone_index("gaster"), {*bones.gaster, &gaster_to_body}}
	};
	
	// Build sting vertex reskin map
	std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> sting_reskin_map;
	if (phenome.sting->present)
	{
		sting_reskin_map.emplace(*sting_skeleton->get_bone_index("sting"), std::tuple(*bones.sting, &sting_to_body));
	}
	
	// Build eyes vertex reskin map
	std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> eyes_reskin_map;
	if (phenome.eyes->present)
	{
		eyes_reskin_map.emplace(*eyes_skeleton->get_bone_index("eye_l"), std::tuple(*bones.head, &eye_l_to_body));
		eyes_reskin_map.emplace(*eyes_skeleton->get_bone_index("eye_r"), std::tuple(*bones.head, &eye_r_to_body));
	}
	
	// Build lateral ocelli vertex reskin map
	std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> lateral_ocelli_reskin_map;
	if (phenome.ocelli->lateral_ocelli_present)
	{
		lateral_ocelli_reskin_map.emplace(*lateral_ocelli_skeleton->get_bone_index("ocellus_l"), std::tuple(*bones.head, &ocellus_l_to_body));
		lateral_ocelli_reskin_map.emplace(*lateral_ocelli_skeleton->get_bone_index("ocellus_r"), std::tuple(*bones.head, &ocellus_r_to_body));
	}
	
	// Build median ocellus vertex reskin map
	std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> median_ocellus_reskin_map;
	if (phenome.ocelli->median_ocellus_present)
	{
		median_ocellus_reskin_map.emplace(*median_ocellus_skeleton->get_bone_index("ocellus_m"), std::tuple(*bones.head, &ocellus_m_to_body));
	}
	
	// Build wings vertex reskin maps
	std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> forewings_reskin_map;
	std::unordered_map<bone_index_type, std::tuple<bone_index_type, const math::transform<float>*>> hindwings_reskin_map;
	if (phenome.wings->present)
	{
		forewings_reskin_map.emplace(*forewings_skeleton->get_bone_index("forewing_l"), std::tuple(*bones.forewing_l, &forewing_l_to_body));
		forewings_reskin_map.emplace(*forewings_skeleton->get_bone_index("forewing_r"), std::tuple(*bones.forewing_r, &forewing_r_to_body));
		hindwings_reskin_map.emplace(*hindwings_skeleton->get_bone_index("hindwing_l"), std::tuple(*bones.hindwing_l, &hindwing_l_to_body));
		hindwings_reskin_map.emplace(*hindwings_skeleton->get_bone_index("hindwing_r"), std::tuple(*bones.hindwing_r, &hindwing_r_to_body));
	}
	
	// Reskin legs vertices
	reskin_vertices(vertex_buffer_data.data() + legs_vbo_offset, legs_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, legs_reskin_map);
	
	// Reskin head vertices
	reskin_vertices(vertex_buffer_data.data() + head_vbo_offset, head_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, head_reskin_map);
	
	// Reskin mandibles vertices
	reskin_vertices(vertex_buffer_data.data() + mandibles_vbo_offset, mandibles_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, mandibles_reskin_map);
	
	// Reskin antennae vertices
	reskin_vertices(vertex_buffer_data.data() + antennae_vbo_offset, antennae_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, antennae_reskin_map);
	
	// Reskin waist vertices
	reskin_vertices(vertex_buffer_data.data() + waist_vbo_offset, waist_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, waist_reskin_map);
	
	// Reskin gaster vertices
	reskin_vertices(vertex_buffer_data.data() + gaster_vbo_offset, gaster_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, gaster_reskin_map);
	
	// Reskin sting vertices
	if (phenome.sting->present)
	{
		reskin_vertices(vertex_buffer_data.data() + sting_vbo_offset, sting_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, sting_reskin_map);
	}
	
	// Reskin eyes vertices
	if (phenome.eyes->present)
	{
		reskin_vertices(vertex_buffer_data.data() + eyes_vbo_offset, eyes_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, eyes_reskin_map);
	}
	
	// Reskin lateral ocelli vertices
	if (phenome.ocelli->lateral_ocelli_present)
	{
		reskin_vertices(vertex_buffer_data.data() + lateral_ocelli_vbo_offset, lateral_ocelli_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, lateral_ocelli_reskin_map);
	}
	
	// Reskin median ocellus vertices
	if (phenome.ocelli->median_ocellus_present)
	{
		reskin_vertices(vertex_buffer_data.data() + median_ocellus_vbo_offset, median_ocellus_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, median_ocellus_reskin_map);
	}
	
	// Reskin wings vertices
	if (phenome.wings->present)
	{
		reskin_vertices(vertex_buffer_data.data() + forewings_vbo_offset, forewings_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, forewings_reskin_map);
		
		reskin_vertices(vertex_buffer_data.data() + hindwings_vbo_offset, hindwings_vertex_count, *position_attribute, *normal_attribute, *tangent_attribute, *bone_index_attribute, hindwings_reskin_map);
	}
	
	// Tag eye vertices
	if (phenome.eyes->present)
	{
		tag_vertices({vertex_buffer_data.data() + eyes_vbo_offset, vertex_buffer_data.data() + eyes_vbo_offset + eyes_vertex_count}, *bone_index_attribute, 1);
	}
	
	// Upload vertex buffer data to model VBO
	model->get_vertex_buffer()->repurpose(gl::buffer_usage::static_draw, vertex_buffer_size, vertex_buffer_data);
	
	// Allocate model groups
	model->get_groups().resize(1);
	
	// Construct model group
	render::model_group& model_group = model->get_groups()[0];
	model_group.id = "exoskeleton";
	model_group.material = exoskeleton_material;
	model_group.drawing_mode = gl::drawing_mode::triangles;
	model_group.start_index = 0;
	model_group.index_count = mesosoma_vertex_count +
		legs_vertex_count +
		head_vertex_count +
		mandibles_vertex_count +
		antennae_vertex_count +
		waist_vertex_count +
		gaster_vertex_count +
		sting_vertex_count +
		eyes_vertex_count +
		lateral_ocelli_vertex_count +
		median_ocellus_vertex_count;
	
	/*
	if (phenome.wings->present)
	{
		// Construct forewings model group
		render::model_group& forewings_group = model->get_groups()[++model_group_index];
		forewings_group.id = "forewings";
		forewings_group.material = forewings_model->get_groups().front().material;
		forewings_group.drawing_mode = gl::drawing_mode::triangles;
		forewings_group.start_index = index_offset;
		forewings_group.index_count = forewings_vertex_count;
		
		index_offset += forewings_group.index_count;
		
		// Construct hindwings model group
		render::model_group& hindwings_group = model->get_groups()[++model_group_index];
		hindwings_group.id = "hindwings";
		hindwings_group.material = hindwings_model->get_groups().front().material;
		hindwings_group.drawing_mode = gl::drawing_mode::triangles;
		hindwings_group.start_index = index_offset;
		hindwings_group.index_count = hindwings_vertex_count;
		
		index_offset += hindwings_group.index_count;
	}
	*/
	
	// Calculate model bounding box
	model->get_bounds() = calculate_bounds(vertex_buffer_data.data(), model_group.index_count, *position_attribute);
	
	return model;
}
