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

#include "game/state/language-menu.hpp"
#include "game/state/options-menu.hpp"
#include "application.hpp"
#include "scene/text.hpp"
#include "debug/logger.hpp"
#include "game/fonts.hpp"
#include "game/menu.hpp"

namespace game {
namespace state {

language_menu::language_menu(game::context& ctx):
	game::state::base(ctx)
{
	ctx.logger->push_task("Entering language menu state");
	
	// Construct menu item texts
	scene::text* language_name_text = new scene::text();
	scene::text* language_value_text = new scene::text();
	scene::text* back_text = new scene::text();
	
	// Build list of menu item texts
	ctx.menu_item_texts.push_back({language_name_text, language_value_text});
	ctx.menu_item_texts.push_back({back_text, nullptr});
	
	// Set content of menu item texts
	update_text_content();
	
	// Init menu item index
	game::menu::init_menu_item_index(ctx, "language");
	
	game::menu::update_text_color(ctx);
	game::menu::update_text_font(ctx);
	game::menu::align_text(ctx);
	game::menu::update_text_tweens(ctx);
	game::menu::add_text_to_ui(ctx);
	game::menu::setup_animations(ctx);
	
	// Construct menu item callbacks
	auto next_language_callback = [this, &ctx]()
	{
		// Increment language index
		++ctx.language_index;
		if (ctx.language_index >= ctx.language_count)
			ctx.language_index = 0;
		
		// Find corresponding language code and strings
		ctx.language_code = (*ctx.string_table)[0][ctx.language_index + 2];
		ctx.strings = &ctx.string_table_map[ctx.language_code];
		
		// Update language in config
		(*ctx.config)["language"] = ctx.language_code;
		
		ctx.logger->log("Language changed to \"" + ctx.language_code + "\"");
		
		// Reload fonts
		ctx.logger->push_task("Reloading fonts");
		try
		{
			game::load_fonts(ctx);
		}
		catch (...)
		{
			ctx.logger->pop_task(EXIT_FAILURE);
		}
		ctx.logger->pop_task(EXIT_SUCCESS);
		
		game::menu::update_text_font(ctx);
		this->update_text_content();
		game::menu::refresh_text(ctx);
		game::menu::align_text(ctx);
		game::menu::update_text_tweens(ctx);
	};
	auto previous_language_callback = [this, &ctx]()
	{
		// Increment language index
		--ctx.language_index;
		if (ctx.language_index < 0)
			ctx.language_index = ctx.language_count - 1;
		
		// Find corresponding language code and strings
		ctx.language_code = (*ctx.string_table)[0][ctx.language_index + 2];
		ctx.strings = &ctx.string_table_map[ctx.language_code];
		
		// Update language in config
		(*ctx.config)["language"] = ctx.language_code;
		
		ctx.logger->log("Language changed to \"" + ctx.language_code + "\"");
		
		// Reload fonts
		ctx.logger->push_task("Reloading fonts");
		try
		{
			game::load_fonts(ctx);
		}
		catch (...)
		{
			ctx.logger->pop_task(EXIT_FAILURE);
		}
		ctx.logger->pop_task(EXIT_SUCCESS);
		
		game::menu::update_text_font(ctx);
		this->update_text_content();
		game::menu::refresh_text(ctx);
		game::menu::align_text(ctx);
		game::menu::update_text_tweens(ctx);
	};
	auto select_back_callback = [&ctx]()
	{
		// Disable controls
		game::menu::clear_controls(ctx);
		
		game::menu::fade_out
		(
			ctx,
			[&ctx]()
			{
				// Queue change to options menu state
				ctx.function_queue.push
				(
					[&ctx]()
					{
						ctx.state_machine.pop();
						ctx.state_machine.emplace(new game::state::options_menu(ctx));
					}
				);
			}
		);
	};
	
	// Build list of menu select callbacks
	ctx.menu_select_callbacks.push_back(next_language_callback);
	ctx.menu_select_callbacks.push_back(select_back_callback);
	
	// Build list of menu left callbacks
	ctx.menu_left_callbacks.push_back(previous_language_callback);
	ctx.menu_left_callbacks.push_back(nullptr);
	
	// Build list of menu right callbacks
	ctx.menu_right_callbacks.push_back(next_language_callback);
	ctx.menu_right_callbacks.push_back(nullptr);
	
	// Set menu back callback
	ctx.menu_back_callback = select_back_callback;
	
	// Queue menu control setup
	ctx.function_queue.push(std::bind(game::menu::setup_controls, std::ref(ctx)));
	
	// Fade in menu
	game::menu::fade_in(ctx, nullptr);
	
	ctx.logger->pop_task(EXIT_SUCCESS);
}

language_menu::~language_menu()
{
	ctx.logger->push_task("Exiting language menu state");
	
	// Destruct menu
	game::menu::clear_controls(ctx);
	game::menu::clear_callbacks(ctx);
	game::menu::delete_animations(ctx);
	game::menu::remove_text_from_ui(ctx);
	game::menu::delete_text(ctx);
	
	ctx.logger->pop_task(EXIT_SUCCESS);
}

void language_menu::update_text_content()
{
	auto [language_name, language_value] = ctx.menu_item_texts[0];
	auto [back_name, back_value] = ctx.menu_item_texts[1];
	
	language_name->set_content((*ctx.strings)["language_menu_language"]);
	language_value->set_content((*ctx.strings)["language_name"]);
	back_name->set_content((*ctx.strings)["back"]);
}

} // namespace state
} // namespace game