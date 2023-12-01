// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_GAME_TRACK_TO_CONSTRAINT_HPP
#define ANTKEEPER_GAME_TRACK_TO_CONSTRAINT_HPP

#include <engine/entity/id.hpp>
#include <engine/math/vector.hpp>


/**
 * Rotates a transform to face a target.
 */
struct track_to_constraint
{
	/// Target entity ID.
	entity::id target;
	
	/// Up direction vector.
	math::fvec3 up;
};


#endif // ANTKEEPER_GAME_TRACK_TO_CONSTRAINT_HPP
