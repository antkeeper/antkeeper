// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_TYPE_TYPEFACE_HPP
#define ANTKEEPER_TYPE_TYPEFACE_HPP

#include <engine/type/font-metrics.hpp>
#include <engine/type/glyph-metrics.hpp>
#include <engine/math/vector.hpp>
#include <vector>
#include <unordered_set>

namespace type {

/// Emumerates typeface styles.
enum class typeface_style
{
	/// Normal typeface style.
	normal,
	
	/// Italic typeface style.
	italic,
	
	/// Oblique typeface style.
	oblique
};

/**
 * Abstract base class for a typeface, which corresponds to a single digital font file.
 *
 * @see type::font
 */
class typeface
{
public:
	/**
	 * Creates a typeface, setting its style and weight.
	 *
	 * @param style Typeface style.
	 * @param weight Typeface weight.
	 */
	typeface(typeface_style style, int weight);
	
	/// Creates an empty typeface.
	typeface();
	
	/// Destroys a typeface.
	virtual ~typeface() = default;
	
	/**
	 * Sets the style of the typeface.
	 *
	 * @param style Typeface style.
	 */
	void set_style(typeface_style style);
	
	/**
	 * Sets the weight of the typeface.
	 *
	 * @param weight Typeface weight.
	 */
	void set_weight(int weight);
	
	/// Returns the style of the typeface.
	[[nodiscard]] inline typeface_style get_style() const noexcept
	{
		return style;
	}
	
	/// Returns the weight of the typeface.
	[[nodiscard]] inline int get_weight() const noexcept
	{
		return weight;
	}
	
	/// Returns `true` if the typeface contains kerning information, `false` otherwise.
	virtual bool has_kerning() const = 0;
	
	/**
	 * Gets metrics for a font of the specified size.
	 *
	 * @param[in] height Height of the font, in pixels.
	 * @param[out] metrics Font metrics.
	 * @return `true` if font metrics were returned, `false` otherwise.
	 */
	virtual bool get_metrics(float height, font_metrics& metrics) const = 0;
	
	/**
	 * Gets metrics for a glyph in a font of the specified size.
	 *
	 * @param[in] height Height of the font, in pixels.
	 * @param[in] code UTF-32 character code of a glyph.
	 * @param[out] metrics Glyph metrics.
	 * @return `true` if glyph metrics were returned, `false` otherwise.
	 */
	virtual bool get_metrics(float height, char32_t code, glyph_metrics& metrics) const = 0;
	
	/**
	 * Gets a bitmap of a glyph in a font of the specified size.
	 *
	 * @param[in] height Height of the font, in pixels.
	 * @param[in] code UTF-32 character code of a glyph.
	 * @param[out] bitmap Glyph bitmap data.
	 * @return `true` if glyph bitmap data was returned, `false` otherwise.
	 */
	virtual bool get_bitmap(float height, char32_t code, std::vector<std::byte>& bitmap, std::uint32_t& bitmap_width, std::uint32_t& bitmap_height) const = 0;
	
	/**
	 * Gets the kerning offset for a pair of glyphs.
	 *
	 * @param[in] height Height of the font, in pixels.
	 * @param[in] first UTF-32 character code of the first glyph.
	 * @param[in] second UTF-32 character code of the second glyph.
	 * @param[out] offset Kerning offset.
	 * @return `true` if a kerning offset was returned, `false` otherwise.
	 */
	virtual bool get_kerning(float height, char32_t first, char32_t second, math::fvec2& offset) const = 0;
	
	/// Returns the set of characters supported by the typeface.
	[[nodiscard]] inline const std::unordered_set<char32_t>& get_charset() const noexcept
	{
		return charset;
	}
	
protected:
	std::unordered_set<char32_t> charset;
	
private:
	typeface_style style;
	int weight;
};

} // namespace type

#endif // ANTKEEPER_TYPE_TYPEFACE_HPP
