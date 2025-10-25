#pragma once
#include <entt/entt.hpp>

namespace Engine {

    /**
     * @brief Marks an entity as an FMOD 3D Listener (usually 1 per scene)
     */
    struct ListenerComponent {
        
        bool Active; // Is this the primary active listener?

        ListenerComponent() : Active(true){}

    };

} //namespace Engine