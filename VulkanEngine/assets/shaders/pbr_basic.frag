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
    int emissiveTextureIdx;
    int metallicTextureIdx;
    int roughnessTextureIdx;
    int normalTextureIdx;
    int occlusionTextureIdx;
    
    int alphaMode;
} u_Material;

layout(set = 1, binding = 1) uniform sampler2D baseColorMap;
layout(set = 1, binding = 2) uniform sampler2D emissiveMap;
layout(set = 1, binding = 3) uniform sampler2D metallicMap;
layout(set = 1, binding = 4) uniform sampler2D roughnessMap;
layout(set = 1, binding = 5) uniform sampler2D normalMap;
layout(set = 1, binding = 6) uniform sampler2D occlusionMap;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 baseColor = u_Material.baseColorFactor;
    if (u_Material.baseColorTextureIdx != -1) {
        baseColor *= texture(baseColorMap, inTexCoord);
    }

    float metallic = u_Material.metallicFactor;
    if (u_Material.metallicTextureIdx != -1) {
        metallic *= texture(metallicMap, inTexCoord).r;
    }

    float roughness = u_Material.roughnessFactor;
    if (u_Material.roughnessTextureIdx != -1) {
        roughness *= texture(roughnessMap, inTexCoord).g;
    }
    
    outColor = baseColor;
}
