#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
} u_Global;

layout(set = 1, binding = 0) uniform MaterialPBRData {
    // --- Factors ---
    vec4 baseColorFactor;
    vec3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaCutoff;
    float normalScale;
    float occlusionStrength;

    // --- Texture Indices ---
    int baseColorTextureIdx;
    int metallicRoughnessTextureIdx;
    int normalTextureIdx;
    int occlusionTextureIdx;
    int emissiveTextureIdx;
    
    int alphaMode;
} u_Material;

layout(set = 1, binding = 1) uniform sampler2D baseColorMap;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessMap;
layout(set = 1, binding = 3) uniform sampler2D normalMap;
layout(set = 1, binding = 4) uniform sampler2D occlusionMap;
layout(set = 1, binding = 5) uniform sampler2D emissiveMap;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 baseColor = u_Material.baseColorFactor;
    if (u_Material.baseColorTextureIdx != -1) {
        baseColor *= texture(baseColorMap, inTexCoord);
    }
    
    outColor = baseColor;
}
