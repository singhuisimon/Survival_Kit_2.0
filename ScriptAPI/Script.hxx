#pragma once
/**
 * @file Script.hxx
 * @brief Declaration of the abstract Script base class for all C# MonoBehaviour scripts.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * @details Provides the base functionality for managed scripts including entity
 *          association and transform component access. All user-created scripts
 *          inherit from this class and override the Update method.
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#include "TransformComponent.hxx"
namespace ScriptAPI
{
    public ref class Script abstract
    {
    public:
        void virtual Update() {};
        TransformComponent GetTransformComponent();

    internal:
        void SetEntityId(int id);
    private:
        int entityId;
    };
}