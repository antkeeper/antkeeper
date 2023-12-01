// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_GEOM_PRIMITIVES_CIRCLE_HPP
#define ANTKEEPER_GEOM_PRIMITIVES_CIRCLE_HPP

#include <engine/geom/primitives/hypersphere.hpp>

namespace geom {
namespace primitives {

/**
 * 2-dimensional hypersphere.
 *
 * @tparam T Real type.
 */
template <class T>
using circle = hypersphere<T, 2>;

} // namespace primitives
} // namespace geom

#endif // ANTKEEPER_GEOM_PRIMITIVES_CIRCLE_HPP
