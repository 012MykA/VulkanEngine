#version 450

layout(location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 viewProj;
    mat4 view;
    mat4 proj;
} u_Global;

layout(location = 0) out vec3 localPos;

void main() {
    localPos = inPos;

    mat4 rotView = mat4(mat3(u_Global.view));
    vec4 clipPos = u_Global.proj * rotView * vec4(inPos, 1.0);

    gl_Position = clipPos.xyww;
}
