/**
 * @file TagComponent.h
 * @brief Tag component - human-readable name for entities
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "../Asset/ResourceTypes.h"
#include <string>

namespace Engine {

    /**
     * @brief Tag component - provides a human-readable name for entities
     * @details This component is typically added to every entity to give it
     *          a meaningful name for debugging, editor display, and identification.
     */
    struct TagComponent {
        /// Unique identifier for this component instance
        xresource::instance_guid ComponentGUID;

        /// Human-readable name/tag for the entity
        std::string Tag;

        /**
         * @brief Default constructor - creates tag with default "Entity" name
         */
        TagComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , Tag("Entity") {
        }

        /**
         * @brief Constructor with custom tag
         * @param tag Initial tag/name for the entity
         */
        explicit TagComponent(const std::string& tag)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , Tag(tag) {
        }

        /**
         * @brief Set the tag/name
         * @param tag New tag/name for the entity
         */
        void SetTag(const std::string& tag) {
            Tag = tag;
        }

        /**
         * @brief Get the tag/name
         * @return Current tag/name
         */
        const std::string& GetTag() const {
            return Tag;
        }

        /**
         * @brief Check if tag is empty
         * @return True if tag is empty string
         */
        bool IsEmpty() const {
            return Tag.empty();
        }
    };

} // namespace Engine