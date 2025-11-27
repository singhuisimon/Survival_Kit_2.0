/*****************************************************************************/
/*!
\file       EventSystem.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/11/27
\brief      Implementation of the central event bus used by the engine.
*/
/*****************************************************************************/

#include "EventSystem.h"
#include "../Utility/Logger.h"

namespace Engine
{
	EventSystem &EventSystem::Instance()
	{
		static EventSystem s_instance;
		return s_instance;
	}

	void EventSystem::DispatchQueued()
	{
		// Move the queue to a local copy so handlers are allowed to queue
		// additional events while processing without causing re-entrancy
		// issues or iterator invalidation.
		std::vector<QueuedEvent> toProcess;
		toProcess.swap(m_queue);

		if (!toProcess.empty())
		{
			LOG_INFO("[EventSystem] Dispatching ", toProcess.size(), " queued event(s)");
		}

		for (QueuedEvent &qe : toProcess)
		{
			if (qe.dispatch)
			{
				qe.dispatch();
			}
		}
	}

	void EventSystem::Clear()
	{
		// Log queued events being dropped
		if (!m_queue.empty())
		{
			LOG_WARNING("[EventSystem] Clear - dropping ", m_queue.size(), " queued event(s)");

			for (std::size_t i = 0; i < m_queue.size(); ++i)
			{
				const QueuedEvent &qe = m_queue[i];
				const char *typeName = qe.debugTypeName ? qe.debugTypeName : "<unknown>";
				LOG_WARNING("[EventSystem]   dropping queued event #", i,
					" type: ", typeName);
			}
		}

		// Log channels / listeners being cleared
		if (!m_channels.empty())
		{
			LOG_WARNING("[EventSystem] Clear - clearing ", m_channels.size(),
				" channel(s)");

			for (auto const &entry : m_channels)
			{
				const std::type_index &typeIdx = entry.first;
				const Channel &chan = entry.second;

				LOG_WARNING("[EventSystem]   clearing channel type: ", typeIdx.name(),
					" (listeners: ", chan.listeners.size(), ")");
			}
		}

		m_channels.clear();
		m_queue.clear();
		m_nextListenerId = 1;
	}

} // namespace Engine
