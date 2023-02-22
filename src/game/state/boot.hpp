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

#ifndef ANTKEEPER_GAME_STATE_BOOT_HPP
#define ANTKEEPER_GAME_STATE_BOOT_HPP

#include "game/state/base.hpp"
#include "game/context.hpp"
#include <optional>

namespace state {

/**
 * Boots the game up on construction, and down on destruction.
 */
class boot: public ::state::base
{
public:
	/**
	 * Boots up the game.
	 *
	 * @param ctx Game context.
	 * @param argc Command line argument count.
	 * @param argv Command line argument vector.
	 */
	boot(::context& ctx, int argc, const char* const* argv);
	
	/**
	 * Boots down the game.
	 */
	virtual ~boot();
	
private:
	void parse_options(int argc, const char* const* argv);
	void setup_resources();
	void load_settings();
	void setup_window();
	void setup_input();
	void load_strings();
	void setup_rendering();
	void setup_audio();
	void setup_scenes();
	void setup_animation();
	void setup_entities();
	void setup_systems();
	void setup_controls();
	void setup_ui();
	void setup_debugging();
	void setup_loop();
	
	void loop();
	
	void shutdown_audio();
};

} // namespace state

#endif // ANTKEEPER_GAME_STATE_BOOT_HPP
