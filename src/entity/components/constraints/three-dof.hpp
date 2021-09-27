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

#ifndef ANTKEEPER_ENTITY_COMPONENT_CONSTRAINT_THREE_DOF_HPP
#define ANTKEEPER_ENTITY_COMPONENT_CONSTRAINT_THREE_DOF_HPP

namespace entity {
namespace component {
namespace constraint {

/**
 * Springs to a target entity.
 */
struct three_dof
{
	/// Yaw rotation angle, in radians.
	float yaw;
	
	/// Pitch rotation angle, in radians.
	float pitch;
	
	/// Roll rotation angle, in radians.
	float roll;
};

} // namespace constraint
} // namespace component
} // namespace entity

#endif // ANTKEEPER_ENTITY_COMPONENT_CONSTRAINT_THREE_DOF_HPP
