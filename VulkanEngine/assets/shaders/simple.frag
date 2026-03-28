#version 450

layout(location = 0) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform sampler2D u_Albedo;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(u_Albedo, inTexCoord);
}
