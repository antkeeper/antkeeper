// SPDX-FileCopyrightText: 2023 C. J. Howard
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANTKEEPER_EVENT_CHANNEL_HPP
#define ANTKEEPER_EVENT_CHANNEL_HPP

#include <functional>
#include <list>
#include <memory>
#include <utility>
#include <engine/event/subscriber.hpp>
#include <engine/event/subscription.hpp>
#include <engine/event/dispatcher.hpp>
#include <engine/event/queue.hpp>

namespace event {

template <class T>
class publisher;

/**
 * Channel through which messages are published.
 *
 * @tparam T Message type.
 */
template <class T>
class channel
{
public:
	/// Message type.
	typedef T message_type;
	
	/// Subscriber function object type.
	typedef subscriber<message_type> subscriber_type;
	
	/**
	 * Subscribes a function object to messages published through this channel.
	 *
	 * @param subscriber Subscriber function object which will received published messages.
	 *
	 * @return Shared subscription object which will unsubscribe the subscriber on destruction.
	 */
	[[nodiscard]] std::shared_ptr<subscription> subscribe(subscriber_type&& subscriber)
	{
		// Construct shared pointer to subscriber function
		std::shared_ptr<subscriber_type> shared_subscriber = std::make_shared<subscriber_type>(std::move(subscriber));
		
		// Append subscriber to subscriber list and store iterator
		auto iterator = subscribers.insert(subscribers.end(), shared_subscriber);
		
		// Construct and return a shared subscription object which removes the subscriber from the subscriber list when unsubscribed or destructed
		return std::make_shared<subscription>
		(
			std::static_pointer_cast<void>(shared_subscriber),
			[this, iterator = std::move(iterator)]
			{
				this->subscribers.erase(iterator);
			}
		);
	}
	
	/**
	 * Subscribes a message dispatcher to messages published through this channel.
	 *
	 * @param dispatcher Message dispatcher which will received published messages.
	 *
	 * @return Shared subscription object which will unsubscribe the queue on destruction.
	 */
	[[nodiscard]] std::shared_ptr<subscription> subscribe(event::dispatcher& dispatcher)
	{
		return subscribe
		(
			[&dispatcher](const message_type& message)
			{
				dispatcher.dispatch<message_type>(message);
			}
		);
	}
	
	/**
	 * Subscribes a message queue to messages published through this channel.
	 *
	 * @param queue Message queue which will received published messages.
	 *
	 * @return Shared subscription object which will unsubscribe the queue on destruction.
	 */
	[[nodiscard]] std::shared_ptr<subscription> subscribe(event::queue& queue)
	{
		return subscribe
		(
			[&queue](const message_type& message)
			{
				queue.enqueue<message_type>(message);
			}
		);
	}
	
private:
	friend class publisher<T>;
	
	/// List of subscribers.
	std::list<std::shared_ptr<subscriber_type>> subscribers;
};

} // namespace event

#endif // ANTKEEPER_EVENT_CHANNEL_HPP
