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

#ifndef ANTKEEPER_INPUT_DEVICE_HPP
#define ANTKEEPER_INPUT_DEVICE_HPP

#include "input-device.hpp"

class event_dispatcher;

/**
 * Base class for virtual devices which generate input events.
 */
class input_device
{
public:
	input_device();
	virtual ~input_device() = default;

	void set_event_dispatcher(event_dispatcher* event_dispatcher);
	const event_dispatcher* get_event_dispatcher() const;
	event_dispatcher* get_event_dispatcher();

protected:
	event_dispatcher* event_dispatcher;
};

inline const event_dispatcher* input_device::get_event_dispatcher() const
{
	return event_dispatcher;
}

inline event_dispatcher* input_device::get_event_dispatcher()
{
	return event_dispatcher;
}

#endif // ANTKEEPER_INPUT_DEVICE_HPP
