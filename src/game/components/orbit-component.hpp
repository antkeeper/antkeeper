// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_GAME_ORBIT_COMPONENT_HPP
#define ANTKEEPER_GAME_ORBIT_COMPONENT_HPP

#include <engine/entity/id.hpp>
#include <engine/math/vector.hpp>


struct orbit_component
{
	/// Entity ID of the parent orbit.
	entity::id parent;
	
	/// Index of the orbit in the ephemeris.
	int ephemeris_index;
	
	/// Orbit scale, for two-body orbits with one ephemeris item.
	double scale;
	
	/// Cartesian position of the orbit, w.r.t. the ICRF frame.
	math::dvec3 position;
};


#endif // ANTKEEPER_GAME_ORBIT_COMPONENT_HPP
