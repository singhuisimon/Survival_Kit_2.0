#include "EditorPropertyPanel.h"
#include "../Utility/Logger.h"
#include "../Engine/Editor/Editor.h"

namespace Engine
{
	void EditorPropertyPanel::PropertyPanel()
	{
		if (!m_Editor->GetPropertyWindowRef()) return;

		Scene* m_Scene = m_Editor->GetActiveScene();
		
		Entity m_SelectedEntity = m_Editor->GetSelectedEntity();

		if (ImGui::Begin("Properties"), &m_Editor->GetPropertyWindowRef())
		{
			uint32_t entityHandle = static_cast<uint32_t>(m_SelectedEntity.GetHandle());
			ImGui::Text("SelectedEntity %u", entityHandle);
			//LOG_DEBUG("SelectedEntity handle: ", entityHandle);
		}

		ImGui::End();

	}
}
