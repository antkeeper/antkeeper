// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#include "game/systems/metabolic-system.hpp"
#include "game/components/isometric-growth-component.hpp"
#include "game/components/rigid-body-component.hpp"
#include <execution>

metabolic_system::metabolic_system(entity::registry& registry):
	updatable_system(registry)
{}

void metabolic_system::update(float t, float dt)
{
	// Scale timestep
	const auto scaled_timestep = dt * m_time_scale;
	
	// Handle isometric growth
	auto isometric_growth_group = registry.group<isometric_growth_component>(entt::get<rigid_body_component>);
	std::for_each
	(
		std::execution::seq,
		isometric_growth_group.begin(),
		isometric_growth_group.end(),
		[&](auto entity_id)
		{
			auto& growth = isometric_growth_group.get<isometric_growth_component>(entity_id);
			auto& rigid_body = *isometric_growth_group.get<rigid_body_component>(entity_id).body;
			
			rigid_body.set_scale(rigid_body.get_scale() + growth.rate * scaled_timestep);
		}
	);
}
