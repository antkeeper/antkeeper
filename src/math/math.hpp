/*
 * Copyright (C) 2020  Christopher J. Howard
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

#ifndef ANTKEEPER_MATH_HPP
#define ANTKEEPER_MATH_HPP

/// Contains mathematical functions and data types.
namespace math {}

/**
 * @defgroup vector Vector
 *
 * Vector type, functions, and operators.
 */
#include "math/vector-type.hpp"
#include "math/vector-functions.hpp"
#include "math/vector-operators.hpp"

/**
 * @defgroup matrix Matrix
 *
 * Matrix type, functions, and operators.
 */
#include "math/matrix-type.hpp"
#include "math/matrix-functions.hpp"
#include "math/matrix-operators.hpp"

/**
 * @defgroup quaternion Quaternion
 *
 * Quaternion type, functions, and operators.
 */
#include "math/quaternion-type.hpp"
#include "math/quaternion-functions.hpp"
#include "math/quaternion-operators.hpp"

/**
 * @defgroup transform Transform
 *
 * TRS transform type, functions, and operators.
 */
#include "math/transform-type.hpp"
#include "math/transform-functions.hpp"
#include "math/transform-operators.hpp"

/**
 * @defgroup io I/O
 *
 * Functions and operators that read/write vectors, matrices, or quaternions from/to streams.
 */
#include "math/stream-operators.hpp"

/**
 * @defgroup utility Utility constants and functions
 *
 * Commonly used utilities.
 */
#include "math/angles.hpp"
#include "math/constants.hpp"
#include "math/interpolation.hpp"
#include "math/random.hpp"

#endif // ANTKEEPER_MATH_HPP
