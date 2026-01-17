#pragma once

#include <cstdint>
#include <string>

#include <mono/metadata/object.h>

// Your ScriptComponent (as you provided)
#include "../Component/ScriptComponent.h" // adjust path if needed

namespace Engine::ScriptHandleUtil
{
	// ----------------------------------------------------------------------------
	// Source of truth:
	//   - ScriptComponent::GCHandle == 0  => no managed instance
	//   - ScriptComponent::GCHandle != 0  => managed instance exists; use gchandle_get_target
	// ScriptComponent::ScriptInstance is treated as a cache only.
	// ----------------------------------------------------------------------------

	inline MonoObject *Resolve(ScriptComponent &sc)
	{
		if (sc.GCHandle == 0)
		{
			// Legacy fallback: if old code still sets ScriptInstance directly
			return reinterpret_cast<MonoObject *>(sc.ScriptInstance);
		}

		MonoObject *obj = mono_gchandle_get_target(sc.GCHandle);
		sc.ScriptInstance = obj; // keep cache in sync
		return obj;
	}

	inline void AdoptFromObject(ScriptComponent &sc, MonoObject *obj, bool pinned = false)
	{
		// Clear existing handle if present
		if (sc.GCHandle != 0)
		{
			mono_gchandle_free(sc.GCHandle);
			sc.GCHandle = 0;
		}

		sc.ScriptInstance = obj;

		if (obj)
			sc.GCHandle = mono_gchandle_new(obj, pinned ? 1 : 0);
	}

	inline void Destroy(ScriptComponent &sc)
	{
		if (sc.GCHandle != 0)
		{
			mono_gchandle_free(sc.GCHandle);
			sc.GCHandle = 0;
		}

		sc.ScriptInstance = nullptr;
		sc.Started = false;
	}

	inline void EnsureHandleIfLegacyPointerPresent(ScriptComponent &sc, bool pinned = false)
	{
		if (sc.GCHandle != 0)
			return;

		MonoObject *legacy = reinterpret_cast<MonoObject *>(sc.ScriptInstance);
		if (!legacy)
			return;

		sc.GCHandle = mono_gchandle_new(legacy, pinned ? 1 : 0);
	}
}
