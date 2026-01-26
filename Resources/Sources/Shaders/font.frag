#version 450 core

///////////////////////////////////////////////////////////////////////////////////////
///
/// \file     font.frag
///
/// \brief    Font fragment shader for rendering text with outline.
///
/// \authors  Tan Jun Rui [100%]
///
/// Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////////////

in vec2 TexCoords;
out vec4 color;

layout (binding = 11) uniform sampler2D text;
uniform vec3 textColor;

const float uThickness = 0.5;
const float edge  = 0.01;

const float borderWidth = 0.5;
const float borderEdge  = 0.1;

const vec3 outlineColor = vec3(1.0, 0.0, 0.0);

void main()
{   
    // float distance = 1.0 - texture(text, TexCoords).r;
    // float alpha    = 1.0 - smoothstep(uThickness, uThickness + edge, distance);

    color = vec4(1.0, 0.0, 0.0, 1.0);   //vec4(textColor, alpha);
}
