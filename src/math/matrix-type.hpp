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

#ifndef ANTKEEPER_MATH_MATRIX_TYPE_HPP
#define ANTKEEPER_MATH_MATRIX_TYPE_HPP

#include "math/vector.hpp"
#include <cstddef>
#include <utility>

namespace math {

/**
 * An NxM matrix.
 *
 * @tparam T Matrix element type.
 * @tparam N Number of columns.
 * @tparam M Number of rows.
 */
template <typename T, std::size_t N, std::size_t M>
struct matrix
{
	typedef T element_type;
	typedef vector<element_type, M> row_type;
	row_type columns[N];
	
	/// Identity matrix.
	static const matrix identity;

	inline constexpr row_type& operator[](std::size_t i) noexcept { return columns[i]; }
	inline constexpr const row_type& operator[](std::size_t i) const noexcept { return columns[i]; }
};

template <typename T, std::size_t I, std::size_t... Is>
constexpr vector<T, sizeof...(Is)> identity_matrix_row(const std::index_sequence<Is...>&)
{
	return {(Is == I ? T{1} : T{0})...};
}

template <typename T, std::size_t... Is>
constexpr matrix<T, sizeof...(Is), sizeof...(Is)> identity_matrix(const std::index_sequence<Is...>& is)
{
	return {{identity_matrix_row<T, Is>(is)...}};
}

template <typename T, std::size_t N, std::size_t M>
constexpr matrix<T, N, M> matrix<T, N, M>::identity = identity_matrix<T>(std::make_index_sequence<N>{});

/// 2x2 matrix.
template <typename T>
using matrix2 = matrix<T, 2, 2>;

/// 2x2 matrix.
template <typename T>
using matrix2x2 = matrix<T, 2, 2>;

/// 3x3 matrix.
template <typename T>
using matrix3 = matrix<T, 3, 3>;

/// 3x3 matrix.
template <typename T>
using matrix3x3 = matrix<T, 3, 3>;

/// 4x4 matrix.
template <typename T>
using matrix4 = matrix<T, 4, 4>;

/// 4x4 matrix.
template <typename T>
using matrix4x4 = matrix<T, 4, 4>;

} // namespace math

#endif // ANTKEEPER_MATH_MATRIX_TYPE_HPP
