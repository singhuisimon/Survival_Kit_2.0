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

uniform sampler2D text;
uniform vec3 textColor;

uniform float uThickness;
const float edge  = 0.01;

const float borderWidth = 0.5;
const float borderEdge  = 0.1;

const vec3 outlineColor = vec3(1.0, 0.0, 0.0);

void main()
{   
    float distance = 1.0 - texture(text, TexCoords).r;
    float alpha    = 1.0 - smoothstep(uThickness, uThickness + edge, distance);

    color = vec4(textColor, alpha);
}
