// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_NEST_VIEW_STATE_HPP
#define ANTKEEPER_NEST_VIEW_STATE_HPP

#include "game/states/game-state.hpp"
#include <engine/entity/id.hpp>
#include <engine/math/vector.hpp>
#include <engine/render/model.hpp>
#include <engine/event/subscription.hpp>
#include <engine/input/mouse-events.hpp>
#include <engine/geom/primitives/ray.hpp>
#include <engine/geom/primitives/plane.hpp>
#include <engine/math/functions.hpp>
#include <engine/scene/light-probe.hpp>
#include <engine/geom/bvh/bvh.hpp>
#include <engine/geom/brep/brep-mesh.hpp>
#include "game/ant/ant-phenome.hpp"

class nest_view_state: public game_state
{
public:
	explicit nest_view_state(::game& ctx);
	virtual ~nest_view_state();
	
private:
	void create_third_person_camera_rig();
	void destroy_third_person_camera_rig();
	
	void set_third_person_camera_zoom(double zoom);
	void set_third_person_camera_rotation(double yaw, double pitch);
	
	void zoom_third_person_camera(double zoom);
	void translate_third_person_camera(const math::dvec3& direction, double magnitude);
	void rotate_third_person_camera(const input::mouse_moved_event& event);
	
	void handle_mouse_motion(const input::mouse_moved_event& event);
	
	void update_third_person_camera();
	
	[[nodiscard]] geom::ray<float, 3> get_mouse_ray(const math::vec2<std::int32_t>& mouse_position) const;
	
	void setup_controls();
	
	std::vector<std::shared_ptr<::event::subscription>> action_subscriptions;
	std::shared_ptr<::event::subscription> mouse_motion_subscription;
	
	bool mouse_look{false};
	bool mouse_grip{false};
	bool mouse_zoom{false};
	geom::plane<float> mouse_grip_plane{{0.0, 1.0, 0.0}, 0.0};
	math::fvec3 mouse_grip_point{};
	
	bool moving{false};
	
	entity::id third_person_camera_rig_eid{entt::null};
	double third_person_camera_yaw{0.0};
	double third_person_camera_pitch{math::radians(45.0)};
	math::dvec3 third_person_camera_focal_point{0.0, 0.0, 0.0};
	double third_person_camera_zoom{0.25};
	std::uint32_t third_person_camera_zoom_step_count{6};
	
	double third_person_camera_near_focal_plane_height{2.0f};
	double third_person_camera_far_focal_plane_height{50.0f};
	
	double third_person_camera_near_hfov{math::radians(90.0)};
	double third_person_camera_far_hfov{math::radians(45.0)};
	
	/// In focal plane heights per second.
	double third_person_camera_speed{1.0f};
	
	double third_person_camera_hfov{};
	double third_person_camera_vfov{};
	double third_person_camera_focal_plane_width{};
	double third_person_camera_focal_plane_height{};
	double third_person_camera_focal_distance{};
	math::dquat third_person_camera_yaw_rotation{math::identity<math::dquat>};
	math::dquat third_person_camera_pitch_rotation{math::identity<math::dquat>};
	math::dquat third_person_camera_orientation{math::identity<math::dquat>};
	
	std::shared_ptr<render::matvar_fvec3> light_rectangle_emissive;
	std::shared_ptr<scene::light_probe> light_probe;
	
	std::shared_ptr<geom::brep_mesh> navmesh;
	std::unique_ptr<geom::bvh> navmesh_bvh;
	entity::id larva_eid;
	entity::id worker_eid;
	
	ant_phenome worker_phenome;
	geom::brep_face* navmesh_agent_face{};
	math::fvec3 navmesh_agent_position{};
	math::fvec3 navmesh_agent_normal{};
};

#endif // ANTKEEPER_NEST_VIEW_STATE_HPP
