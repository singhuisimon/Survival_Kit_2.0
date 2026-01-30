#pragma once
#ifndef EDITOR_PERFORMANCEPANEL_H
#define EDITOR_PERFORMANCEPANEL_H

// include necessary file

#include "../Utility/Logger.h"
#include "../Utility/Timestep.h"
//#include "../Profiler/Profiler.h"

namespace Engine
{
	class Editor;
	class Timestep;
	class EditorPerformancePanel
	{
	private:
		Editor* m_Editor = nullptr;
	public:
		EditorPerformancePanel(Editor* editor) : m_Editor(editor) {};
		~EditorPerformancePanel() = default;

		void PerformanceProfilePanel(Timestep ts);
	};
}

#endif // EDITOR_PERFORMANCEPANEL_H