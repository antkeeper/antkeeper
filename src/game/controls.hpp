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

#ifndef ANTKEEPER_GAME_CONTROLS_HPP
#define ANTKEEPER_GAME_CONTROLS_HPP

#include "game/game.hpp"
#include <engine/utility/json.hpp>
#include <engine/input/gamepad.hpp>
#include <filesystem>


/**
 * Resets a control profile to default settings.
 *
 * @param profile Control profile to reset.
 */
void reset_control_profile(::control_profile& profile);

/**
 * Applies a control profile to the game context.
 *
 * @param ctx Game context.
 * @param profile Control profile to apply.
 */
void apply_control_profile(::game& ctx, const ::control_profile& profile);

/**
 * Updates a control profile after actions have been remapped.
 *
 * @param ctx Game context.
 * @param profile Control profile to update.
 */
void update_control_profile(::game& ctx, ::control_profile& profile);

void setup_window_controls(::game& ctx);
void setup_menu_controls(::game& ctx);
void setup_game_controls(::game& ctx);

void enable_window_controls(::game& ctx);
void enable_menu_controls(::game& ctx);
void enable_game_controls(::game& ctx);
void enable_keeper_controls(::game& ctx);

void disable_window_controls(::game& ctx);
void disable_menu_controls(::game& ctx);
void disable_game_controls(::game& ctx);
void disable_keeper_controls(::game& ctx);


#endif // ANTKEEPER_GAME_CONTROLS_HPP
