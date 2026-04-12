#version 450

layout(location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
} u_Global;

layout(location = 0) out vec3 localPos;

void main() {
    localPos = inPos;

    vec3 flippedPos = vec3(inPos.x, -inPos.y, inPos.z);

    mat4 rotView = mat4(mat3(u_Global.view));
    vec4 clipPos = u_Global.proj * rotView * vec4(flippedPos, 1.0);

    gl_Position = clipPos.xyww;
}
