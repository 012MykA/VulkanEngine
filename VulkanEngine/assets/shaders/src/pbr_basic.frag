#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

#define MAX_LIGHTS 10

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;

    PointLight lights[MAX_LIGHTS];
    int lightCount;
} u_Global;

layout(set = 0, binding = 1) uniform samplerCube u_IrradianceMap;
layout(set = 0, binding = 2) uniform samplerCube u_PrefilteredMap;
layout(set = 0, binding = 3) uniform sampler2D u_BrdfLUT;

layout(set = 1, binding = 0) uniform MaterialPBRData {
    // --- Factors ---
    vec4 baseColorFactor;
    vec3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float alphaCutoff;
    float normalScale;
    float occlusionStrength;

    // --- UV Transform ---
    vec2 uvScale;
    vec2 uvOffset;

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

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

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
    vec2 UV = inTexCoord * u_Material.uvScale + u_Material.uvOffset;
    
    // Albedo
    vec4 albedoRGBA = u_Material.baseColorFactor;
    if(u_Material.baseColorTextureIdx != -1) {
        albedoRGBA *= texture(baseColorMap, UV);
    }
    vec3 albedo = albedoRGBA.rgb;

    // Emissive
    vec3 emissive = u_Material.emissiveFactor;
    if(u_Material.emissiveTextureIdx != -1) {
        emissive *= texture(emissiveMap, UV).rgb;
    }

    // Metallic
    float metallic = u_Material.metallicFactor;
    if(u_Material.metallicTextureIdx != -1) {
        metallic *= texture(metallicMap, UV).r;
    }

    // Roughness
    float roughness = u_Material.roughnessFactor;
    if(u_Material.roughnessTextureIdx != -1) {
        roughness *= texture(roughnessMap, UV).g;
    }

    // Occlusion
    float occlusion = 1.0;
    if(u_Material.occlusionTextureIdx != -1) {
        float aoSample = texture(occlusionMap, UV).r;
        occlusion = 1.0 + u_Material.occlusionStrength * (aoSample - 1.0);
    }

    // Constants
    const vec3 DIELECTRIC_F0 = vec3(0.04);

    // const float AMBIENT_INTENSITY = 0.001;

    // const float GAMMA = 2.2;

    const float EPSILON = 0.0001;

    // --- Shading ---
    vec3 N;
    if(u_Material.normalTextureIdx != -1) {
        vec3 tangent = normalize(inTangent.xyz);
        vec3 normal = normalize(inNormal);
        vec3 bitangent = normalize(cross(normal, tangent) * inTangent.w);

        mat3 TBN = mat3(tangent, bitangent, normal);

        vec3 localNormal = texture(normalMap, UV).rgb * 2.0 - 1.0;

        localNormal.xy *= u_Material.normalScale;
        localNormal = normalize(localNormal);

        N = normalize(TBN * localNormal);
    } else {
        N = normalize(inNormal);
    }

    vec3 V = normalize(u_Global.cameraPos.rgb - inWorldPos);
    vec3 R = reflect(-V, -N);

    vec3 F0 = DIELECTRIC_F0;
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    int lightCount = min(u_Global.lightCount, MAX_LIGHTS);
    for(int i = 0; i < lightCount; ++i) {
        // calculate per-light radiance
        vec3 L = normalize(u_Global.lights[i].position.rgb - inWorldPos);
        vec3 H = normalize(V + L);
        float distance = length(u_Global.lights[i].position.rgb - inWorldPos);
        float attenuation = 1.0 / (distance * distance);
        float intensity = u_Global.lights[i].color.a;
        vec3 radiance = u_Global.lights[i].color.rgb * intensity * attenuation;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON;
        vec3 specular = numerator / denominator;  

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // --- IBL ---
    float NdotV = max(dot(N, V), 0.0);
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 invN = vec3(N.x, -N.y, N.z);
    vec3 invR = vec3(R.x, -R.y, R.z);

    // Diffuse IBL (Irradiance)
    vec3 irradiance = texture(u_IrradianceMap, invN).rgb;
    vec3 diffuse = irradiance * albedo;

    // Specular IBL (Prefiltered)
    float lod = roughness * float(textureQueryLevels(u_PrefilteredMap) - 1);
    vec3 prefilteredColor = textureLod(u_PrefilteredMap, invR, lod).rgb;
    vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    // Ambient
    vec3 ambient = (kD * diffuse + specular) * occlusion;

    // Final color
    vec3 color = ambient + Lo + emissive;

    color = aces(color);

    outColor = vec4(color, albedoRGBA.a);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
