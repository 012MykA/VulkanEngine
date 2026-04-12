#version 450

layout(location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
} u_Global;

layout(location = 0) out vec3 outUVW;

void main() {
    outUVW = inPos;

    mat4 viewNoModel = mat4(mat3(u_Global.view));
    vec4 pos = u_Global.proj * viewNoModel * vec4(inPos, 1.0);
    gl_Position = pos.xyww;
}
