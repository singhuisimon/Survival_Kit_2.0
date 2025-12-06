#pragma once
#include "../ECS/System.h"
#include "../ECS/Components.h"

namespace Engine {

    class ScriptSystem : public System {
    public:
        ScriptSystem() = default;  // Remove the constructor call with parameters

        void OnInit(Scene* scene) override;
        void OnUpdate(Scene* scene, Timestep ts) override;  
        void OnShutdown(Scene* scene) override;             // Changed: added Scene* parameter

        // Override these to customize behavior
        int GetPriority() const override { return 50; }     // Priority 50
        const char* GetName() const override { return "ScriptSystem"; }

    private:
        Scene* m_Scene = nullptr;
        bool m_IsShuttingDown = false;
    };

} // namespace Engine
