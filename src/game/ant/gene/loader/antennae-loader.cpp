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

#include "game/ant/gene/loader/gene-loader.hpp"
#include <engine/resources/resource-loader.hpp>
#include <engine/resources/resource-manager.hpp>
#include <engine/resources/json.hpp>
#include "game/ant/gene/antennae.hpp"
#include <engine/render/model.hpp>
#include <stdexcept>

using namespace ::ant;

static void deserialize_antennae_phene(phene::antennae& phene, const json& phene_element, resource_manager* resource_manager)
{
	phene.model = nullptr;
	phene.total_antennomere_count = 0;
	phene.club_antennomere_count = 0;
	
	// Load antennae model
	if (auto element = phene_element.find("model"); element != phene_element.end())
		phene.model = resource_manager->load<render::model>(element->get<std::string>());
	
	// Parse total antennomere count
	if (auto element = phene_element.find("total_antennomere_count"); element != phene_element.end())
		phene.total_antennomere_count = element->get<int>();
	
	// Parse club antennomere count
	if (auto element = phene_element.find("club_antennomere_count"); element != phene_element.end())
		phene.club_antennomere_count = element->get<int>();
}

template <>
gene::antennae* resource_loader<gene::antennae>::load(resource_manager* resource_manager, PHYSFS_File* file, const std::filesystem::path& path)
{
	// Load JSON data
	json* data = resource_loader<json>::load(resource_manager, file, path);
	
	// Validate gene file
	auto antennae_element = data->find("antennae");
	if (antennae_element == data->end())
		throw std::runtime_error("Invalid antennae gene.");
	
	// Allocate gene
	gene::antennae* antennae = new gene::antennae();
	
	// Deserialize gene
	gene::deserialize_gene(*antennae, &deserialize_antennae_phene, *antennae_element, resource_manager);
	
	// Free JSON data
	delete data;
	
	return antennae;
}
