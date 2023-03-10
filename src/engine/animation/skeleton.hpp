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

#ifndef ANTKEEPER_ANIMATION_SKELETON_HPP
#define ANTKEEPER_ANIMATION_SKELETON_HPP

#include <engine/animation/bone.hpp>
#include <engine/animation/pose.hpp>
#include <engine/utility/hash/fnv1a.hpp>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Skeletal animation skeleton.
 */
struct skeleton
{
	/// Bone-space bind pose of the skeleton.
	pose bind_pose;
	
	/// Inverse skeleton-space bind pose of the skeleton.
	pose inverse_bind_pose;
	
	std::vector<bone> bones;
	
	/// Maps bone names to bone identifiers.
	std::unordered_map<hash::fnv1a32_t, bone> bone_map;
};

#endif // ANTKEEPER_ANIMATION_SKELETON_HPP
