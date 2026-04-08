#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// Global UBO
layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
} u_Global;

layout(location = 0) out vec4 outColor;

void main()
{    
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
