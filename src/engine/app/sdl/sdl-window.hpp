// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_APP_SDL_WINDOW_HPP
#define ANTKEEPER_APP_SDL_WINDOW_HPP

#include <engine/app/window.hpp>
#include <SDL2/SDL.h>
#include <memory>

namespace app {

/**
 * 
 */
class sdl_window: public window
{
public:
	sdl_window
	(
		const std::string& title,
		const math::ivec2& windowed_position,
		const math::ivec2& windowed_size,
		bool maximized,
		bool fullscreen,
		bool v_sync
	);
	
	sdl_window(const sdl_window&) = delete;
	sdl_window(sdl_window&&) = delete;
	sdl_window& operator=(const sdl_window&) = delete;
	sdl_window& operator=(sdl_window&&) = delete;
	
	virtual ~sdl_window();
	void set_title(const std::string& title) override;
	void set_position(const math::ivec2& position) override;
	void set_size(const math::ivec2& size) override;
	void set_minimum_size(const math::ivec2& size) override;
	void set_maximum_size(const math::ivec2& size) override;
	void set_maximized(bool maximized) override;
	void set_fullscreen(bool fullscreen) override;
	void set_v_sync(bool v_sync) override;
	void make_current() override;
	void swap_buffers() override;
	
	[[nodiscard]] inline const gl::pipeline& get_graphics_pipeline() const noexcept override
	{
		return *m_graphics_pipeline;
	}
	
	[[nodiscard]] inline gl::pipeline& get_graphics_pipeline() noexcept override
	{
		return *m_graphics_pipeline;
	}
	
private:
	friend class sdl_window_manager;
	
	SDL_Window* m_internal_window;
	SDL_GLContext m_internal_context;
	std::unique_ptr<gl::pipeline> m_graphics_pipeline;
};

} // namespace app

#endif // ANTKEEPER_APP_SDL_WINDOW_HPP
