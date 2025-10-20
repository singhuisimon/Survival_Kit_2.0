
/**
 * @file Script.cxx
 * @brief Implementation of the Script base class methods for managed scripts.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "Script.hxx"
#include "../ScriptCore/Application.h"



namespace ScriptAPI
{
    TransformComponent Script::GetTransformComponent()
    {
        return TransformComponent(entityId);
    }
    void Script::SetEntityId(int id)
    {
        entityId = id;
    }
}