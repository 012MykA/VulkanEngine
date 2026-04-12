#version 450

layout(location = 0) in vec3 localPos;

layout(set = 1, binding = 0) uniform samplerCube environmentMap;

layout(location = 0) out vec4 outColor;

void main() {
    // Constants
    const float GAMMA = 2.2;

    // Shading
    vec3 envColor = texture(environmentMap, localPos).rgb;

    // envColor = envColor / (envColor + vec3(1.0));
    // envColor = pow(envColor, vec3(1.0 / GAMMA));

    outColor = vec4(envColor, 1.0);
}
