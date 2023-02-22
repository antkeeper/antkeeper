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
#include "game/ant/gene/diet.hpp"
#include <engine/math/angles.hpp>
#include <engine/math/numbers.hpp>
#include <stdexcept>

using namespace ::ant;

static void deserialize_diet_phene(phene::diet& phene, const json& phene_element, resource_manager* resource_manager)
{
	
}

template <>
gene::diet* resource_loader<gene::diet>::load(resource_manager* resource_manager, PHYSFS_File* file, const std::filesystem::path& path)
{
	// Load JSON data
	json* data = resource_loader<json>::load(resource_manager, file, path);
	
	// Validate gene file
	auto diet_element = data->find("diet");
	if (diet_element == data->end())
		throw std::runtime_error("Invalid diet gene.");
	
	// Allocate gene
	gene::diet* diet = new gene::diet();
	
	// Deserialize gene
	gene::deserialize_gene(*diet, &deserialize_diet_phene, *diet_element, resource_manager);
	
	// Free JSON data
	delete data;
	
	return diet;
}
