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

#include <engine/animation/ik/constraints/euler-ik-constraint.hpp>
#include <algorithm>
#include <cmath>

void euler_ik_constraint::solve(math::fquat& q)
{
	// Store w-component of quaternion
	float old_w = q.w();
	
	// Derive Euler angles from quaternion
	auto angles = math::euler_from_quat(m_rotation_sequence, q);
	
	// Constrain Euler angles
	angles = math::clamp(angles, m_min_angles, m_max_angles);
	
	// Rebuild quaternion from constrained Euler angles
	q = math::euler_to_quat(m_rotation_sequence, angles);
	
	// Restore quaternion sign
	q.w() = std::copysign(q.w(), old_w);
}