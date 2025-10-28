/**
 * @file survival_kit_obj.frag
 * @brief Declaration of the objects fragment shader for the game engine.
 * @details Manages per-fragment data of the object 
 * @author Chua Wen Bin Kenny
 * @date 25 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#version 420 core

layout(location = 0) out uint outId;

// Bind this uniform from CPU with the entity's unique ID.
// Reserve 0 as "no hit".
uniform uint u_ObjectID;

void main()
{
    outId = u_ObjectID;
}