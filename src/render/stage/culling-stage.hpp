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

#ifndef ANTKEEPER_RENDER_CULLING_STAGE_HPP
#define ANTKEEPER_RENDER_CULLING_STAGE_HPP

#include "render/stage.hpp"

namespace render {

/**
 * Builds a set of scene objects visible to the current camera and stores it in the render context.
 */
class culling_stage: public stage
{
public:
	/// Constructs a culling stage.
	culling_stage() = default;
	
	/// Destructs a culling stage.
	virtual ~culling_stage() = default;
	
	/// @copydoc render::stage::execute(render::context&)
	virtual void execute(render::context& ctx) final;
};

} // namespace render

#endif // ANTKEEPER_RENDER_CULLING_STAGE_HPP
