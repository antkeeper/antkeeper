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

#include <engine/type/bitmap-font.hpp>
#include <engine/geom/rect-pack.hpp>
#include <stdexcept>

namespace type {

bitmap_font::bitmap_font(const font_metrics& metrics):
	font(metrics)
{}

bitmap_font::bitmap_font()
{}

bool bitmap_font::contains(char32_t code) const
{
	return glyphs.count(code) != 0;
}

void bitmap_font::insert(char32_t code, const bitmap_glyph& glyph)
{
	glyphs[code] = glyph;
}

void bitmap_font::remove(char32_t code)
{
	if (auto it = glyphs.find(code); it != glyphs.end())
		glyphs.erase(it);
}

void bitmap_font::clear()
{
	glyphs.clear();
}

bool bitmap_font::pack(bool resize)
{
	// Returns the smallest power of two that is not smaller than @p x.
	auto ceil2 = [](std::uint32_t x) -> std::uint32_t
	{
		if (x <= 1)
			return 1;
		std::uint32_t y = 2;
		--x;
		while (x >>= 1)
			y <<= 1;
		return y;
	};
	
	// Calculate initial size of the font bitmap
	std::uint32_t bitmap_w;
	std::uint32_t bitmap_h;
	if (resize)
	{
		// Find the maximum glyph dimensions
		std::uint32_t max_glyph_w = 0;
		std::uint32_t max_glyph_h = 0;
		for (auto it = glyphs.begin(); it != glyphs.end(); ++it)
		{
			max_glyph_w = std::max(max_glyph_w, static_cast<std::uint32_t>(it->second.bitmap.size().x()));
			max_glyph_h = std::max(max_glyph_h, static_cast<std::uint32_t>(it->second.bitmap.size().y()));
		}
		
		// Find minimum power of two dimensions that can accommodate maximum glyph dimensions
		bitmap_w = ceil2(max_glyph_w);
		bitmap_h = ceil2(max_glyph_h);
	}
	else
	{
		bitmap_w = static_cast<std::uint32_t>(bitmap.size().x());
		bitmap_h = static_cast<std::uint32_t>(bitmap.size().y());
	}
	
	bool packed = false;
	geom::rect_pack<std::uint32_t> glyph_pack(bitmap_w, bitmap_h);
	std::unordered_map<char32_t, const typename geom::rect_pack<std::uint32_t>::node_type*> glyph_map;
	
	while (!packed)
	{
		// For each glyph
		for (auto it = glyphs.begin(); it != glyphs.end(); ++it)
		{
			// Attempt to pack glyph bitmap
			const auto* node = glyph_pack.pack(static_cast<std::uint32_t>(it->second.bitmap.size().x()), static_cast<std::uint32_t>(it->second.bitmap.size().y()));
			
			// Abort if packing failed
			if (!node)
				break;
			
			// Map pack node to glyph character code
			glyph_map[it->first] = node;
		}
		
		// Check if not all glyphs were packed
		if (glyph_map.size() != glyphs.size())
		{
			if (!resize)
			{
				// No resize, packing failed
				packed = false;
				break;
			}
			
			// Clear glyph map
			glyph_map.clear();
			
			// Clear glyph pack
			glyph_pack.clear();
			
			// Resize glyph pack
			if (bitmap_w > bitmap_h)
				bitmap_h = ceil2(++bitmap_h);
			else
				bitmap_w = ceil2(++bitmap_w);
			glyph_pack.resize(bitmap_w, bitmap_h);
		}
		else
		{
			packed = true;
		}
	}
	
	// Copy glyph bitmaps into font bitmap
	if (packed)
	{
		// Resize font bitmap
		bitmap.resize({bitmap_w, bitmap_h, 1});
		
		// For each glyph
		for (auto it = glyphs.begin(); it != glyphs.end(); ++it)
		{
			// Find rect pack node corresponding to the glyph
			const auto* node = glyph_map[it->first];
			
			// Copy glyph bitmap data into font bitmap
			image& glyph_bitmap = it->second.bitmap;
			bitmap.copy(glyph_bitmap, {glyph_bitmap.size().x(), glyph_bitmap.size().y()}, {0, 0}, math::uvec2{node->bounds.min.x(), node->bounds.min.y()});
			
			// Record coordinates of glyph bitmap within font bitmap
			it->second.position = {node->bounds.min.x(), node->bounds.min.y()};
			
			// Clear glyph bitmap data
			glyph_bitmap.resize({0u, 0u, 0u});
		}
	}
	
	return packed;
}

void bitmap_font::unpack(bool resize)
{
	for (auto it = glyphs.begin(); it != glyphs.end(); ++it)
	{
		bitmap_glyph& glyph = it->second;
		
		// Get glyph dimensions
		std::uint32_t glyph_width = static_cast<std::uint32_t>(glyph.metrics.width + 0.5f);
		std::uint32_t glyph_height = static_cast<std::uint32_t>(glyph.metrics.height + 0.5f);
		
		// Reformat glyph bitmap if necessary
		if (!glyph.bitmap.compatible(bitmap))
		{
			glyph.bitmap.format(bitmap.channels(), bitmap.bit_depth());
		}
		
		// Resize glyph bitmap if necessary
		if (static_cast<std::uint32_t>(glyph.bitmap.size().x()) != glyph_width || static_cast<std::uint32_t>(glyph.bitmap.size().y()) != glyph_height)
		{
			glyph.bitmap.resize(math::uvec2{glyph_width, glyph_height});
		}
		
		// Copy pixel data from font bitmap to glyph bitmap
		glyph.bitmap.copy(bitmap, math::uvec2{glyph_width, glyph_height}, math::uvec2{glyph.position.x(), glyph.position.y()});
	}
	
	// Free font bitmap pixel data
	if (resize)
	{
		bitmap.resize({0, 0, 0});
	}
}

const glyph_metrics& bitmap_font::get_glyph_metrics(char32_t code) const
{
	if (auto it = glyphs.find(code); it != glyphs.end())
		return it->second.metrics;
	throw std::invalid_argument("Cannot fetch metrics of unknown bitmap glyph");
}

const bitmap_glyph& bitmap_font::get_glyph(char32_t code) const
{
	if (auto it = glyphs.find(code); it != glyphs.end())
		return it->second;
	throw std::invalid_argument("Cannot get unknown bitmap glyph");
}

bitmap_glyph& bitmap_font::get_glyph(char32_t code)
{
	if (auto it = glyphs.find(code); it != glyphs.end())
		return it->second;
	throw std::invalid_argument("Cannot get unknown bitmap glyph");
}

} // namespace type
