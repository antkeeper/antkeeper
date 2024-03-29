// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#include <engine/geom/brep/brep-vertex.hpp>
#include <engine/geom/brep/brep-mesh.hpp>

namespace geom {

void brep_vertex_edge_list::push_back(brep_edge* edge)
{
	// Determine index of vertex in new edge vertex array
	const std::size_t i = (edge->m_vertices[1] == m_vertex);
	
	if (empty())
	{
		// List empty, initialize
		m_head = edge;
		edge->m_vertex_next[i] = edge;
		edge->m_vertex_previous[i] = edge;
	}
	else
	{
		// Determine index of vertex in head edge vertex array
		const std::size_t j = (m_head->m_vertices[1] == m_vertex);
		
		// Append edge
		edge->m_vertex_next[i] = m_head;
		edge->m_vertex_previous[i] = m_head->m_vertex_previous[j];
		edge->m_vertex_previous[i]->m_vertex_next[edge->m_vertex_previous[i]->m_vertices[1] == m_vertex] = edge;
		m_head->m_vertex_previous[j] = edge;
	}
	
	++m_size;
}

void brep_vertex_edge_list::remove(brep_edge* edge)
{
	// Determine index of vertex in edge vertex array
	const std::size_t i = (edge->m_vertices[1] == m_vertex);
	
	// Get pointers to the next and previous edges
	brep_edge* next = edge->m_vertex_next[i];
	brep_edge* previous = edge->m_vertex_previous[i];
	
	// Directly link next and previous edges
	next->m_vertex_previous[next->m_vertices[1] == m_vertex] = previous;
	previous->m_vertex_next[previous->m_vertices[1] == m_vertex] = next;
	
	// If edge was the list head, update head
	if (m_head == edge)
	{
		m_head = next;
	}
	
	--m_size;
}

brep_vertex* brep_vertex_container::emplace_back()
{
	brep_vertex* vertex = brep_element_container<brep_vertex>::emplace_back();
	vertex->m_index = size() - 1;
	vertex->m_edges.m_vertex = vertex;
	return vertex;
};

void brep_vertex_container::erase(brep_vertex* vertex)
{
	// Erase all edges bounded by this vertex
	while (!vertex->edges().empty())
	{
		m_mesh->edges().erase(vertex->edges().back());
	}
	
	// Erase vertex
	brep_element_container<brep_vertex>::erase(vertex);
}

void brep_vertex_container::clear() noexcept
{
	while (!empty())
	{
		erase(back());
	}
}

} // namespace geom
