// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_EVENT_PUBLISHER_HPP
#define ANTKEEPER_EVENT_PUBLISHER_HPP

#include <algorithm>
#include <execution>
#include <engine/event/channel.hpp>

namespace event {

/**
 * Publishes messages to subscribers.
 *
 * @tparam T Message type.
 */
template <class T>
class publisher
{
public:
	/// Message type.
	typedef T message_type;
	
	/// Channel type.
	typedef channel<message_type> channel_type;
	
	/**
	 * Publishes a message.
	 *
	 * @tparam ExecutionPolicy Execution policy type.
	 *
	 * @param policy Execution policy to use.
	 * @param message Message to publish.
	 */
	/// @{
	template <class ExecutionPolicy>
	void publish(ExecutionPolicy&& policy, const message_type& message) const
	{
		std::for_each
		(
			policy,
			std::begin(m_channel.subscribers),
			std::end(m_channel.subscribers),
			[&](const auto& subscriber)
			{
				(*subscriber)(message);
			}
		);
	}
	
	void publish(const message_type& message) const
	{
		publish(std::execution::seq, message);
	}
	/// @}
	
	/**
	 * Returns the channel through which messages are published.
	 */
	/// @{
	[[nodiscard]] inline const channel_type& channel() const noexcept
	{
		return m_channel;
	}
	
	[[nodiscard]] inline channel_type& channel() noexcept
	{
		return m_channel;
	}
	/// @}
	
private:
	channel_type m_channel;
};

} // namespace event

#endif // ANTKEEPER_EVENT_PUBLISHER_HPP
