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

#ifndef ANTKEEPER_GAME_STRINGS_HPP
#define ANTKEEPER_GAME_STRINGS_HPP

#include "game/game.hpp"
#include <cstdint>
#include <string>


/**
 * Returns a localized string.
 *
 * @param[in] ctx Game context.
 * @param[in] key String key.
 * @param[out] string String value.
 *
 * @return `true` if the string was found, `false` otherwise.
 */
[[nodiscard]] std::string get_string(const ::game& ctx, std::uint32_t key);


#endif // ANTKEEPER_GAME_STRINGS_HPP
