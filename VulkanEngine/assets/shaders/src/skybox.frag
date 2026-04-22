#version 450

layout(location = 0) in vec3 localPos;

layout(set = 1, binding = 0) uniform samplerCube environmentMap;

layout(location = 0) out vec4 outColor;

// ACES Tone Mapping
vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float aces(float x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 envColor = texture(environmentMap, localPos).rgb;
    envColor = aces(envColor);

    outColor = vec4(envColor, 1.0);
}
