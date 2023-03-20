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

#include <engine/scene/camera.hpp>
#include <engine/math/quaternion.hpp>
#include <engine/math/projection.hpp>

namespace scene {

geom::primitive::ray<float, 3> camera::pick(const float2& ndc) const
{
	const float4x4 inverse_view_projection = math::inverse(m_view_projection);
	
	const float4 near = inverse_view_projection * float4{ndc[0], ndc[1], 1.0f, 1.0f};
	const float4 far = inverse_view_projection * float4{ndc[0], ndc[1], 0.0f, 1.0f};
	
	const float3 origin = float3{near[0], near[1], near[2]} / near[3];
	const float3 direction = math::normalize(float3{far[0], far[1], far[2]} / far[3] - origin);
	
	return {origin, direction};
}

float3 camera::project(const float3& object, const float4& viewport) const
{
	float4 result = m_view_projection * float4{object[0], object[1], object[2], 1.0f};
	result[0] = (result[0] / result[3]) * 0.5f + 0.5f;
	result[1] = (result[1] / result[3]) * 0.5f + 0.5f;
	result[2] = (result[2] / result[3]) * 0.5f + 0.5f;
	
	result[0] = result[0] * viewport[2] + viewport[0];
	result[1] = result[1] * viewport[3] + viewport[1];
	
	return math::vector<float, 3>(result);
}

float3 camera::unproject(const float3& window, const float4& viewport) const
{
	float4 result;
	result[0] = ((window[0] - viewport[0]) / viewport[2]) * 2.0f - 1.0f;
	result[1] = ((window[1] - viewport[1]) / viewport[3]) * 2.0f - 1.0f;
	//result[2] = window[2] * 2.0f - 1.0f; z: [-1, 1]
	//result[2] = window[2]; // z: [0, 1]
	result[2] = 1.0f - window[2]; // z: [1, 0]
	result[3] = 1.0f;
	
	result = math::inverse(m_view_projection) * result;

	return math::vector<float, 3>(result) * (1.0f / result[3]);
}

void camera::set_perspective(float fov, float aspect_ratio, float clip_near, float clip_far)
{
	m_orthographic = false;
	
	// Update perspective projection parameters
	m_fov = fov;
	m_aspect_ratio = aspect_ratio;
	m_clip_near = clip_near;
	m_clip_far = clip_far;
	
	// Recalculate projection matrix
	m_projection = math::perspective_half_z(m_fov, m_aspect_ratio, m_clip_far, m_clip_near);
	
	// Recalculate view-projection matrix
	m_view_projection = m_projection * m_view;
	
	// Recalculate view frustum
	/// @TODO: this is a hack to fix the half z projection matrix view frustum
	m_view_frustum.set_matrix(math::perspective(m_fov, m_aspect_ratio, m_clip_near, m_clip_far) * m_view);
}

void camera::set_orthographic(float clip_left, float clip_right, float clip_bottom, float clip_top, float clip_near, float clip_far)
{
	m_orthographic = true;
	
	// Update signed distances to clipping planes
	m_clip_left = clip_left;
	m_clip_right = clip_right;
	m_clip_bottom = clip_bottom;
	m_clip_top = clip_top;
	m_clip_near = clip_near;
	m_clip_far = clip_far;
	
	// Update projection matrix
	m_projection = math::ortho_half_z(m_clip_left, m_clip_right, m_clip_bottom, m_clip_top, m_clip_far, m_clip_near);
	
	// Recalculate view-projection matrix
	m_view_projection = m_projection * m_view;
	
	// Recalculate view frustum
	m_view_frustum.set_matrix(m_view_projection);
}

void camera::set_exposure_value(float ev100)
{
	m_exposure_value = ev100;
	m_exposure_normalization = 1.0f / (std::exp2(m_exposure_value) * 1.2f);
}

void camera::transformed()
{
	// Recalculate view and view-projection matrices
	m_forward = get_rotation() * math::vector<float, 3>{0.0f, 0.0f, -1.0f};
	m_up = get_rotation() * math::vector<float, 3>{0.0f, 1.0f, 0.0f};
	m_view = math::look_at(get_translation(), get_translation() + m_forward, m_up);
	m_view_projection = m_projection * m_view;
	
	// Recalculate view frustum
	if (m_orthographic)
	{
		m_view_frustum.set_matrix(m_view_projection);
	}
	else
	{
		/// @TODO: this is a hack to fix the half z projection matrix view frustum
		m_view_frustum.set_matrix(math::perspective(m_fov, m_aspect_ratio, m_clip_near, m_clip_far) * m_view);
	}
}

} // namespace scene
