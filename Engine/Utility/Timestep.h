#pragma once

namespace Engine {

    /**
     * @brief Wrapper around delta time for type safety and convenience
     * @details Simple value type that represents time elapsed between frames
     */
    class Timestep {
    public:
        Timestep(float time = 0.0f)
            : m_Time(time) {
        }

        // Implicit conversion to float for convenience
        operator float() const { return m_Time; }

        // Explicit getters
        float GetSeconds() const { return m_Time; }
        float GetMilliseconds() const { return m_Time * 1000.0f; }

    private:
        float m_Time; // Time in seconds
    };

} // namespace Engine