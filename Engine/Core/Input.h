#pragma once
#include <glm/glm.hpp>
#include <unordered_map>

// Forward declare GLFW types
struct GLFWwindow;

namespace Engine {

    /**
     * @brief Input system - handles keyboard and mouse input
     * @details Direct input handling without manager pattern
     */
    class Input {
    public:
        Input() = default;
        ~Input() = default;

        /**
         * @brief Initialize with GLFW window
         * @param window GLFW window handle
         */
        void Init(GLFWwindow* window);

        /**
         * @brief Update input states (call once per frame)
         */
        void Update();

        // ===== KEYBOARD =====

        /**
         * @brief Check if key is currently held down
         */
        bool IsKeyPressed(int key) const;

        /**
         * @brief Check if key was just pressed this frame
         */
        bool IsKeyJustPressed(int key) const;

        /**
         * @brief Check if key was just released this frame
         */
        bool IsKeyJustReleased(int key) const;

        // ===== MOUSE =====

        /**
         * @brief Check if mouse button is currently held down
         */
        bool IsMouseButtonPressed(int button) const;

        /**
         * @brief Check if mouse button was just pressed this frame
         */
        bool IsMouseButtonJustPressed(int button) const;

        /**
         * @brief Check if mouse button was just released this frame
         */
        bool IsMouseButtonJustReleased(int button) const;

        /**
         * @brief Get current mouse position
         */
        glm::vec2 GetMousePosition() const { return m_MousePosition; }

        /**
         * @brief Get mouse delta (movement since last frame)
         */
        glm::vec2 GetMouseDelta() const { return m_MouseDelta; }

        /**
         * @brief Get mouse scroll delta
         */
        glm::vec2 GetScrollDelta() const { return m_ScrollDelta; }

        /**
         * @brief Set mouse cursor visibility
         */
        void SetCursorVisible(bool visible);

        /**
         * @brief Check if cursor is visible
         */
        bool IsCursorVisible() const { return m_CursorVisible; }

        /**
         * @brief Set cursor position
         */
        void SetCursorPosition(const glm::vec2& position);

    private:
        // State tracking
        struct ButtonState {
            bool current = false;
            bool previous = false;
        };

        GLFWwindow* m_Window = nullptr;

        // States
        std::unordered_map<int, ButtonState> m_KeyStates;
        std::unordered_map<int, ButtonState> m_MouseButtonStates;

        glm::vec2 m_MousePosition = glm::vec2(0.0f);
        glm::vec2 m_LastMousePosition = glm::vec2(0.0f);
        glm::vec2 m_MouseDelta = glm::vec2(0.0f);
        glm::vec2 m_ScrollDelta = glm::vec2(0.0f);

        bool m_CursorVisible = true;
        bool m_FirstMouseMove = true;

        // Static callback for scroll events
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        // Instance pointer for callback
        static Input* s_Instance;
    };

} // namespace Engine