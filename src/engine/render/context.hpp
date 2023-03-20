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

#ifndef ANTKEEPER_RENDER_CONTEXT_HPP
#define ANTKEEPER_RENDER_CONTEXT_HPP

#include <engine/geom/plane.hpp>
#include <engine/geom/bounding-volume.hpp>
#include <engine/utility/fundamental-types.hpp>
#include <engine/math/transform-operators.hpp>
#include <engine/render/operation.hpp>
#include <vector>

namespace scene
{
	class camera;
	class collection;
	class object_base;
}

namespace render {

/**
 * Context of a renderer.
 */
struct context
{
	/// Pointer to the camera.
	const scene::camera* camera;
	
	/// Camera culling volume.
	const geom::bounding_volume<float>* camera_culling_volume;
	
	/// Collection of scene objects being rendered.
	const scene::collection* collection;
	
	/// Current time, in seconds.
	float t;
	
	/// Timestep, in seconds.
	float dt;
	
	/// Subframe interpolation factor.
	float alpha;
	
	/// Objects visible to the active camera.
	std::vector<scene::object_base*> objects;
	
	/// Render operations generated by visible objects.
	std::vector<const operation*> operations;
};

} // namespace render

#endif // ANTKEEPER_RENDER_CONTEXT_HPP
