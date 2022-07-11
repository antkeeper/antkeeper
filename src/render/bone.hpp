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

#ifndef ANTKEEPER_RENDER_BONE_HPP
#define ANTKEEPER_RENDER_BONE_HPP

#include "math/transform-type.hpp"

namespace render {

/**
 * Skeletal animation bone.
 */
struct bone
{
	/// Pointer to the parent bone.
	bone* parent;
	
	/// Local transform, relative to the parent bone.
	math::transform<float> transform;
	
	/// Length of the bone.
	float length;
};

} // namespace render

#endif // ANTKEEPER_RENDER_BONE_HPP
