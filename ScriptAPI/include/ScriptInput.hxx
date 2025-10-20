#pragma once

namespace ScriptAPI {
    class ScriptInput {
    public:
        static bool IsKeyDown(int keyCode);
        static float GetMouseX();
        static float GetMouseY();
    };
}
