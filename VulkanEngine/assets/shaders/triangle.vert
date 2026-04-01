#version 450

// Vertex input
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

// Camera UBO
layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} u_Camera;

// Push constants
layout(push_constant) uniform PushConstants {
    mat4 model;
} u_Push;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = u_Camera.proj * u_Camera.view * u_Push.model * vec4(inPosition, 1.0);
    fragColor = inPosition;
}