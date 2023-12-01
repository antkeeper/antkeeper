// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#include <engine/ai/steering/behavior/flee.hpp>

namespace ai {
namespace steering {
namespace behavior {

math::fvec3 flee(const agent& agent, const math::fvec3& target)
{
	math::fvec3 force = {0, 0, 0};
	const math::fvec3 difference = target - agent.position;
	const float sqr_distance = math::dot(difference, difference);
	
	if (sqr_distance)
	{
		const float inverse_distance = 1.0f / std::sqrt(sqr_distance);
		force = difference * inverse_distance * agent.max_force;
		force = agent.velocity - force;
	}
	
	return force;
}

} // namespace behavior
} // namespace steering
} // namespace ai
