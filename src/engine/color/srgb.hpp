// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_COLOR_SRGB_HPP
#define ANTKEEPER_COLOR_SRGB_HPP

#include <engine/color/rgb.hpp>
#include <engine/color/illuminants.hpp>
#include <engine/math/vector.hpp>
#include <cmath>

namespace color {

/// @name sRGB color space
/// @{

/**
 * sRGB opto-electronic transfer function (OETF). Maps a linear sRGB color to a non-linear sRGB signal.
 *
 * @param x Linear sRGB color.
 *
 * @return Non-linear sRGB signal.
 *
 * @see IEC 61966-2-1:1999
 */
template <class T>
[[nodiscard]] math::vec3<T> srgb_oetf(const math::vec3<T>& x)
{
	auto f = [](T x) -> T
	{
		return x > T{0.0031308} ? std::pow(x, T{1.0 / 2.4}) * T{1.055} - T{0.055} : x * T{12.92};
	};
	
	return {f(x[0]), f(x[1]), f(x[2])};
}

/**
 * sRGB electro-optical transfer function (EOTF). Maps a non-linear sRGB signal to a linear sRGB color.
 *
 * @param x Non-linear sRGB signal.
 *
 * @return Linear sRGB color.
 *
 * @see IEC 61966-2-1:1999
 */
template <class T>
[[nodiscard]] math::vec3<T> srgb_eotf(const math::vec3<T>& x)
{
	auto f = [](T x) -> T
	{
		return x > T{0.0031308 * 12.92} ? std::pow((x + T{0.055}) / T{1.055}, T{2.4}) : x / T{12.92};
	};
	
	return {f(x[0]), f(x[1]), f(x[2])};
}

/// sRGB color space.
template <class T>
constexpr rgb_color_space<T> srgb
(
	{T{0.64}, T{0.33}},
	{T{0.30}, T{0.60}},
	{T{0.15}, T{0.06}},
	deg2_d65<T>,
	&srgb_eotf<T>,
	&srgb_oetf<T>
);

/// @}

} // namespace color

#endif // ANTKEEPER_COLOR_SRGB_HPP
