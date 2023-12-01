// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#include <engine/type/freetype/ft-typeface.hpp>
#include <stdexcept>
#include <string>

namespace type {

ft_typeface::ft_typeface(FT_Library library, FT_Face face, std::unique_ptr<std::vector<FT_Byte>> file_buffer):
	library(library),
	face(face),
	file_buffer{std::move(file_buffer)},
	height(-1.0f)
{
	/// Build charset
	FT_UInt index;
	FT_ULong c = FT_Get_First_Char(face, &index);
	while (index)
	{
		this->charset.insert(static_cast<char32_t>(c));
		c = FT_Get_Next_Char(face, c, &index);
	}
}

ft_typeface::~ft_typeface()
{
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

bool ft_typeface::has_kerning() const
{
	return FT_HAS_KERNING(face);
}

bool ft_typeface::get_metrics(float height, font_metrics& metrics) const
{
	// Set font size
	set_face_pixel_size(height);
	
	// Get font metrics
	metrics.size = height;
	metrics.ascent = face->size->metrics.ascender / 64.0f;
	metrics.descent = face->size->metrics.descender / 64.0f;
	metrics.linespace = face->size->metrics.height / 64.0f;
	metrics.linegap = metrics.linespace - (metrics.ascent - metrics.descent);
	metrics.underline_position = FT_MulFix(face->underline_position, face->size->metrics.y_scale) / 64.0f;
	metrics.underline_thickness = FT_MulFix(face->underline_thickness, face->size->metrics.y_scale) / 64.0f;
	metrics.max_horizontal_advance = face->size->metrics.max_advance / 64.0f;
	metrics.max_vertical_advance = FT_MulFix(face->max_advance_height, face->size->metrics.y_scale) / 64.0f;
	
	return true;
}

bool ft_typeface::get_metrics(float height, char32_t code, glyph_metrics& metrics) const
{
	// Set font size
	set_face_pixel_size(height);
	
	// Get index of glyph
	FT_UInt glyph_index = FT_Get_Char_Index(face, static_cast<FT_ULong>(code));
	
	// Load glyph and render bitmap
	if (FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT))
	{
		throw std::runtime_error("FreeType failed to load glyph (error code " + std::to_string(error) + ")");
	}
	
	// Get glyph metrics
	metrics.width = face->glyph->metrics.width / 64.0f;
	metrics.height = face->glyph->metrics.height / 64.0f;
	metrics.horizontal_bearing.x() = face->glyph->metrics.horiBearingX / 64.0f;
	metrics.horizontal_bearing.y() = face->glyph->metrics.horiBearingY / 64.0f;
	metrics.vertical_bearing.x() = face->glyph->metrics.vertBearingX / 64.0f;
	metrics.vertical_bearing.y() = face->glyph->metrics.vertBearingY / 64.0f;
	metrics.horizontal_advance = face->glyph->metrics.horiAdvance / 64.0f;
	metrics.vertical_advance = face->glyph->metrics.vertAdvance / 64.0f;
	
	return true;
}

bool ft_typeface::get_bitmap(float height, char32_t code, std::vector<std::byte>& bitmap, std::uint32_t& bitmap_width, std::uint32_t& bitmap_height) const
{
	// Set font size
	set_face_pixel_size(height);
	
	// Get index of glyph
	FT_UInt glyph_index = FT_Get_Char_Index(face, static_cast<FT_ULong>(code));
	
	// Load glyph and render bitmap
	if (FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_NORMAL)))
	{
		throw std::runtime_error("FreeType failed to load glyph (error code " + std::to_string(error) + ")");
	}
	
	// Copy glyph bitmap data into bitmap
	bitmap.resize(face->glyph->bitmap.width * face->glyph->bitmap.rows);
	std::memcpy(bitmap.data(), face->glyph->bitmap.buffer, bitmap.size());
	
	bitmap_width = static_cast<std::uint32_t>(face->glyph->bitmap.width);
	bitmap_height = static_cast<std::uint32_t>(face->glyph->bitmap.rows);
	
	return true;
}

bool ft_typeface::get_kerning(float height, char32_t first, char32_t second, math::fvec2& offset) const
{
	// Check if typeface has kerning information
	if (!has_kerning())
	{
		return false;
	}
	
	// Set font size
	set_face_pixel_size(height);
	
	// Get indices of the two glyphs
	const FT_UInt first_index = FT_Get_Char_Index(face, static_cast<FT_ULong>(first));
	const FT_UInt second_index = FT_Get_Char_Index(face, static_cast<FT_ULong>(second));
	
	// Get kerning vector
	FT_Vector kerning;
	if (FT_Error error = FT_Get_Kerning(face, first_index, second_index, FT_KERNING_DEFAULT, &kerning))
	{
		throw std::runtime_error("FreeType failed to get kerning vector (error code " + std::to_string(error) + ")");
	}
	
	offset = math::fvec2{static_cast<float>(kerning.x), static_cast<float>(kerning.y)} / 64.0f;
	
	return true;
}

void ft_typeface::set_face_pixel_size(float height) const
{
	if (this->height == height)
	{
		return;
	}
	
	if (FT_Error error = FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(height)))
	{
		throw std::runtime_error("FreeType failed to set face size (error code " + std::to_string(error) + ")");
	}
	
	this->height = height;
}

} // namespace type
