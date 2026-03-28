#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal; 
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(push_constant) uniform PushConstant
{
    mat4 model;
    mat4 viewProj;
} u_Push;

layout(location = 0) out vec2 outTexCoord;

void main()
{
    gl_Position = u_Push.viewProj * u_Push.model * vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
}
