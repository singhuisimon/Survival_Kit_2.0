#include "Input.h"
#include "Utility/Logger.h"
#include <GLFW/glfw3.h>

namespace Engine {

    // Static instance pointer for callbacks
    Input* Input::s_Instance = nullptr;

    void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        (void)window;
        if (s_Instance) {
            s_Instance->m_ScrollDelta.x += static_cast<float>(xoffset);
            s_Instance->m_ScrollDelta.y += static_cast<float>(yoffset);
            LOG_TRACE("Scroll callback: ", yoffset, " (accumulated: ", s_Instance->m_ScrollDelta.y, ")");
        }
    }

    void Input::Init(GLFWwindow* window) {
        m_Window = window;
        s_Instance = this;

        // Set scroll callback
        glfwSetScrollCallback(m_Window, ScrollCallback);

        // Get initial mouse position
        double mouseX, mouseY;
        glfwGetCursorPos(m_Window, &mouseX, &mouseY);
        m_MousePosition = glm::vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
        m_LastMousePosition = m_MousePosition;

        LOG_DEBUG("Input system initialized");
    }

    void Input::Update() {
        if (!m_Window) return;

        // Reset scroll delta at the START of update (before new events)
        m_ScrollDelta = glm::vec2(0.0f);

        // Make sure glfwPollEvents() is called BEFORE processing state transitions
        // This ensures all callbacks (including scroll) are processed before we transition states
        glfwPollEvents();

        // Update keyboard states
        for (auto& [key, state] : m_KeyStates) {
            state.previous = state.current;
            state.current = (glfwGetKey(m_Window, key) == GLFW_PRESS);
        }

        // Update mouse button states
        for (auto& [button, state] : m_MouseButtonStates) {
            state.previous = state.current;
            state.current = (glfwGetMouseButton(m_Window, button) == GLFW_PRESS);
        }

        // Update mouse position
        double mouseX, mouseY;
        glfwGetCursorPos(m_Window, &mouseX, &mouseY);
        m_MousePosition = glm::vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));

        // Calculate mouse delta
        if (m_FirstMouseMove) {
            m_LastMousePosition = m_MousePosition;
            m_FirstMouseMove = false;
        }
        m_MouseDelta = m_MousePosition - m_LastMousePosition;
        m_LastMousePosition = m_MousePosition;

        // Note: Scroll delta is reset at the start and accumulated via callback
        // Don't reset it here at the end!
    }

    bool Input::IsKeyPressed(int key) const {
        auto it = m_KeyStates.find(key);
        if (it == m_KeyStates.end()) {
            // First time checking - add to tracking
            const_cast<Input*>(this)->m_KeyStates[key] = ButtonState{};
            return glfwGetKey(m_Window, key) == GLFW_PRESS;
        }
        return it->second.current;
    }

    bool Input::IsKeyJustPressed(int key) const {
        auto it = m_KeyStates.find(key);
        if (it == m_KeyStates.end()) {
            const_cast<Input*>(this)->m_KeyStates[key] = ButtonState{};
            return false;
        }
        return it->second.current && !it->second.previous;
    }

    bool Input::IsKeyJustReleased(int key) const {
        auto it = m_KeyStates.find(key);
        if (it == m_KeyStates.end()) {
            const_cast<Input*>(this)->m_KeyStates[key] = ButtonState{};
            return false;
        }
        return !it->second.current && it->second.previous;
    }

    bool Input::IsMouseButtonPressed(int button) const {
        auto it = m_MouseButtonStates.find(button);
        if (it == m_MouseButtonStates.end()) {
            const_cast<Input*>(this)->m_MouseButtonStates[button] = ButtonState{};
            return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
        }
        return it->second.current;
    }

    bool Input::IsMouseButtonJustPressed(int button) const {
        auto it = m_MouseButtonStates.find(button);
        if (it == m_MouseButtonStates.end()) {
            const_cast<Input*>(this)->m_MouseButtonStates[button] = ButtonState{};
            return false;
        }
        return it->second.current && !it->second.previous;
    }

    bool Input::IsMouseButtonJustReleased(int button) const {
        auto it = m_MouseButtonStates.find(button);
        if (it == m_MouseButtonStates.end()) {
            const_cast<Input*>(this)->m_MouseButtonStates[button] = ButtonState{};
            return false;
        }
        return !it->second.current && it->second.previous;
    }

    void Input::SetCursorVisible(bool visible) {
        m_CursorVisible = visible;
        glfwSetInputMode(m_Window, GLFW_CURSOR,
            visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

        // Reset first mouse move when changing cursor mode
        m_FirstMouseMove = true;

        LOG_DEBUG("Cursor visibility set to: ", visible ? "visible" : "hidden");
    }

    void Input::SetCursorPosition(const glm::vec2& position) {
        glfwSetCursorPos(m_Window, position.x, position.y);
        m_MousePosition = position;
        m_LastMousePosition = position;
        m_FirstMouseMove = true;
    }

} // namespace Engine