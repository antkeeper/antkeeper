// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#include <engine/animation/rest-pose.hpp>
#include <engine/animation/skeleton.hpp>
#include <algorithm>
#include <execution>

rest_pose::rest_pose(const skeleton& skeleton):
	pose(skeleton),
	m_inverse_absolute_transforms(skeleton.get_bone_count(), math::identity<bone_transform_type>)
{}

void rest_pose::update(bone_index_type first_index, std::size_t bone_count)
{
	// Update absolute transforms
	pose::update(first_index, bone_count);
	
	// Update inverse absolute transforms
	std::for_each
	(
		// std::execution::par_unseq,
		std::execution::seq,
		m_inverse_absolute_transforms.begin() + first_index,
		m_inverse_absolute_transforms.begin() + (first_index + bone_count),
		[&](auto& inverse_absolute_transform)
		{
			bone_index_type bone_index = static_cast<bone_index_type>(&inverse_absolute_transform - m_inverse_absolute_transforms.data());
			inverse_absolute_transform = math::inverse(m_absolute_transforms[bone_index]);
		}
	);
}

void rest_pose::set_skeleton(const skeleton& skeleton)
{
	pose::set_skeleton(skeleton);
	m_inverse_absolute_transforms.resize(skeleton.get_bone_count(), math::identity<bone_transform_type>);
}

void rest_pose::resize()
{
	if (m_skeleton)
	{
		const std::size_t bone_count = m_skeleton->get_bone_count();
		
		if (bone_count != m_inverse_absolute_transforms.size())
		{
			m_relative_transforms.resize(bone_count);
			m_absolute_transforms.resize(bone_count);
			m_inverse_absolute_transforms.resize(bone_count);
		}
	}
}
