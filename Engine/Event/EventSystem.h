/*****************************************************************************/
/*!
\file       EventSystem.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/11/27
\brief      Lightweight event bus for engine- and gameplay-level messaging.

			The event system provides a type-safe publish / subscribe API
			that can be used by native systems (ECS, physics, rendering)
			as well as the scripting layer. Events are dispatched on the
			main thread and are not thread-safe.

			Scripts can interact with the same bus via InternalCalls by
			using the ScriptEvent type (see below) and a small C# wrapper.
*/
/*****************************************************************************/
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine
{
	/*************************************************************************/
	/*!
	\brief  Generic event type used as the bridge for scripting.

			Script code can publish named events with an optional payload
			string (for example JSON or a simple delimited format). Native
			systems that care about script events can subscribe to this
			type and filter on the name.
	*/
	/*************************************************************************/
	struct ScriptEvent
	{
		std::string name;    //!< Logical event name ("OnPlayerDied", ...)
		std::string payload; //!< Optional payload, interpretation is user-defined
	};

	/*************************************************************************/
	/*!
	\brief  Central event bus used by the engine and gameplay systems.

			The bus routes events by C++ type. Each distinct event struct
			becomes its own logical "channel". Subscribers receive events
			in the order they were published when DispatchQueued is called.

			The implementation is single-threaded and not designed for use
			from background threads.
	*/
	/*************************************************************************/
	class EventSystem
	{
	public:
		using ListenerId = std::uint64_t;

		/*! Global access point. */
		static EventSystem &Instance();

		/*!
		\brief  Subscribe a handler to a given event type.

				Returns a ListenerId that can be used to unsubscribe later.
				The handler must remain valid for as long as the subscription
				is active.
		*/
		template <typename EventT, typename Callable>
		ListenerId Subscribe(Callable &&callable);

		/*!
		\brief  Unsubscribe a handler for the given event type.

				If the id is not found, this is a no-op.
		*/
		template <typename EventT>
		void Unsubscribe(ListenerId id);

		/*!
		\brief  Immediately dispatch an event to all listeners of EventT.

				In most gameplay code it is preferable to queue events so
				they are processed in a controlled point in the frame.
		*/
		template <typename EventT>
		void Publish(EventT const &event);

		/*!
		\brief  Convenience helper to construct and publish an event in-place.
		*/
		template <typename EventT, typename... Args>
		void EmplacePublish(Args &&...args);

		/*!
		\brief  Queue an event to be dispatched later.

				Queued events are copied into internal storage and delivered
				when DispatchQueued() is called.
		*/
		template <typename EventT>
		void Queue(EventT const &event);

		/*!
		\brief  Convenience helper to construct and queue an event in-place.
		*/
		template <typename EventT, typename... Args>
		void EmplaceQueue(Args &&...args);

		/*!
		\brief  Dispatch all queued events.

				This should typically be called once per frame from the main
				loop or from the ECS world / scene update.
		*/
		void DispatchQueued();

		/*!
		\brief  Remove all listeners and queued events.

				Intended primarily for tests or when switching scenes.
		*/
		void Clear();

	private:
		struct ListenerRecord
		{
			ListenerId id{};
			std::function<void(void const *)> callback;
		};

		struct Channel
		{
			std::vector<ListenerRecord> listeners;
		};

		struct QueuedEvent
		{
			std::function<void()> dispatch;
			const char *debugTypeName = nullptr;
		};


		template <typename EventT>
		Channel &GetChannel();

		template <typename EventT>
		Channel const &GetChannel() const;

		std::unordered_map<std::type_index, Channel> m_channels;
		std::vector<QueuedEvent>                     m_queue;
		ListenerId                                   m_nextListenerId{ 1 };
	};

	//-------------------------------------------------------------------------
	// Template implementation
	//-------------------------------------------------------------------------

	template <typename EventT, typename Callable>
	inline EventSystem::ListenerId EventSystem::Subscribe(Callable &&callable)
	{
		Channel &channel = GetChannel<EventT>();

		ListenerRecord rec;
		rec.id = m_nextListenerId++;
		rec.callback = [fn = std::forward<Callable>(callable)](void const *ptr)
			{
				fn(*static_cast<EventT const *>(ptr));
			};

		channel.listeners.emplace_back(std::move(rec));
		return m_nextListenerId - 1;
	}

	template <typename EventT>
	inline void EventSystem::Unsubscribe(ListenerId id)
	{
		Channel &channel = GetChannel<EventT>();

		for (auto it = channel.listeners.begin(); it != channel.listeners.end(); ++it)
		{
			if (it->id == id)
			{
				channel.listeners.erase(it);
				break;
			}
		}
	}

	template <typename EventT>
	inline void EventSystem::Publish(EventT const &event)
	{
		Channel &channel = GetChannel<EventT>();
		for (auto const &listener : channel.listeners)
		{
			if (listener.callback)
			{
				listener.callback(&event);
			}
		}
	}

	template <typename EventT, typename... Args>
	inline void EventSystem::EmplacePublish(Args &&...args)
	{
		EventT ev{ std::forward<Args>(args)... };
		Publish(ev);
	}

	template <typename EventT>
	inline void EventSystem::Queue(EventT const &event)
	{
		// Copy the event into the queued lambda so lifetime is managed.
		EventT copy = event;

		QueuedEvent qe;
		qe.dispatch = [this, copy]() { Publish(copy); };
		qe.debugTypeName = typeid(EventT).name(); // static lifetime, safe to keep pointer

		m_queue.emplace_back(std::move(qe));
	}


	template <typename EventT, typename... Args>
	inline void EventSystem::EmplaceQueue(Args &&...args)
	{
		EventT ev{ std::forward<Args>(args)... };
		Queue(ev);
	}

	template <typename EventT>
	inline EventSystem::Channel &EventSystem::GetChannel()
	{
		std::type_index key(typeid(EventT));
		return m_channels[key]; // Will default-construct channel if not present
	}

	template <typename EventT>
	inline EventSystem::Channel const &EventSystem::GetChannel() const
	{
		std::type_index      key(typeid(EventT));
		auto                 it = m_channels.find(key);
		static const Channel empty{};
		return (it != m_channels.end()) ? it->second : empty;
	}

} // namespace Engine
