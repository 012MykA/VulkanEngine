#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

// Global UBO
layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
} u_Global;

// Push constants
layout(push_constant) uniform PushConstants {
    mat4 model;
} u_Push;

// Output
layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;

void main() {
    vec4 worldPos = u_Push.model * vec4(inPosition, 1.0);

    outWorldPos = worldPos.rgb;
    outNormal = normalize(mat3(transpose(inverse(u_Push.model))) * inNormal);
    outTexCoord = inTexCoord;
    
    gl_Position = u_Global.proj * u_Global.view * worldPos;
}