#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    PointLight light;
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

const float PI = 3.14159265359;

void main() {
    const float gamma = 2.2;

    // Albedo
    vec4 albedoRGBA = u_Material.baseColorFactor;
    if(u_Material.baseColorTextureIdx != -1) {
        vec4 sampledColor = texture(baseColorMap, inTexCoord);
        albedoRGBA *= vec4(pow(sampledColor.rgb, vec3(gamma)), sampledColor.a);
    }
    vec3 albedo = albedoRGBA.rgb;

    // Emissive
    vec3 emissive = u_Material.emissiveFactor;
    if(u_Material.emissiveTextureIdx != -1) {
        emissive *= pow(texture(emissiveMap, inTexCoord).rgb, vec3(gamma));
    }

    // Metallic
    float metallic = u_Material.metallicFactor;
    if(u_Material.metallicTextureIdx != -1) {
        metallic *= texture(metallicMap, inTexCoord).r;
    }

    // Roughness
    float roughness = u_Material.roughnessFactor;
    if(u_Material.roughnessTextureIdx != -1) {
        roughness *= texture(roughnessMap, inTexCoord).g;
    }

    // Occlusion
    float occlusion = 1.0;
    if(u_Material.occlusionTextureIdx != -1) {
        float aoSample = texture(occlusionMap, inTexCoord).r;
        occlusion = 1.0 + u_Material.occlusionStrength * (aoSample - 1.0);
    }

    outColor = albedoRGBA;
}
