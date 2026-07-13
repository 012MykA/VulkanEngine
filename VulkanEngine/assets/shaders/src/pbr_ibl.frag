#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

#define MAX_LIGHTS 10

struct Light {
    vec4 position;     // [x, y, z, type]
    vec4 color;        // [r, g, b, intensity]
    vec4 direction;    // [x, y, z, range]
    vec4 coneAngles;   // [inner, outer, _padding, _padding]
};

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;

    Light lights[MAX_LIGHTS];
    int lightCount;
} u_Global;

layout(set = 0, binding = 1) uniform samplerCube u_IrradianceMap;
layout(set = 0, binding = 2) uniform samplerCube u_PrefilteredMap;
layout(set = 0, binding = 3) uniform sampler2D u_BrdfLUT;

layout(set = 1, binding = 0) uniform MaterialPBRData {
    vec4 baseColorFactor;

    vec3 emissiveColorFactor;
    float emissiveStrength;

    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;

    float occlusionStrength;
    uint alphaMode; // Opaque = 0, Mask = 1, Blend = 2
    uint doubleSided;
    uint hasBaseColorMap;

    uint hasNormalMap;
    uint hasAoMetallicRoughnessMap;
    uint hasEmissiveMap;
} u_Material;

layout(set = 1, binding = 1) uniform sampler2D baseColorMap;
layout(set = 1, binding = 2) uniform sampler2D emissiveMap;
layout(set = 1, binding = 3) uniform sampler2D aoMetallicRoughnessMap;
layout(set = 1, binding = 4) uniform sampler2D normalMap;

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
    vec2 UV = inTexCoord;

    // Albedo
    vec4 albedoRGBA = u_Material.baseColorFactor;
    if(u_Material.hasBaseColorMap != 0) {
        albedoRGBA *= texture(baseColorMap, UV);
    }
    vec3 albedo = albedoRGBA.rgb;

    // Alpha cutoff
    if(u_Material.alphaMode == 1) { // MASK
        if(albedoRGBA.a < u_Material.alphaCutoff) {
            discard;
        }
    }

    // Emissive
    vec3 emissive = u_Material.emissiveColorFactor;
    if(u_Material.hasEmissiveMap != 0) {
        emissive *= texture(emissiveMap, UV).rgb;
    }
    emissive *= u_Material.emissiveStrength;

    // AO & Metallic & Roughness
    float occlusion = 1.0;
    float metallic = u_Material.metallicFactor;
    float roughness = u_Material.roughnessFactor;

    if(u_Material.hasAoMetallicRoughnessMap != 0) {
        occlusion = 1.0 + u_Material.occlusionStrength * (texture(aoMetallicRoughnessMap, UV).r - 1.0);
        metallic *= texture(aoMetallicRoughnessMap, UV).b;
        roughness *= texture(aoMetallicRoughnessMap, UV).g;
    }

    // Constants
    const vec3 DIELECTRIC_F0 = vec3(0.04);

    // const float BASE_AMBIENT = 1.0;

    const float EPSILON = 0.0001;

    // --- Shading ---
    vec3 N;
    if(u_Material.hasNormalMap != 0) {
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

    // Double sided
    if(u_Material.doubleSided != 0) {
        if(!gl_FrontFacing) {
            N = -N;
        }
    }

    vec3 V = normalize(u_Global.cameraPos.rgb - inWorldPos);
    vec3 R = reflect(-V, -N);

    vec3 F0 = DIELECTRIC_F0;
    F0 = mix(F0, albedo, metallic);

    // Direct Lighting
    vec3 Lo = vec3(0.0);
    int lightCount = min(u_Global.lightCount, MAX_LIGHTS);

    for(int i = 0; i < lightCount; ++i) {
        Light l = u_Global.lights[i];
        int type = int(l.position.w);

        vec3 L;
        float attenuation = 1.0;

        if(type == 0) { // Directional
            L = normalize(-l.direction.xyz);
        } else { // Point or Spot
            L = normalize(l.position.xyz - inWorldPos);
            float dist = length(l.position.xyz - inWorldPos);

            attenuation = 1.0 / (dist * dist + 0.01);

            if(l.direction.w > 0.0) {
                attenuation *= clamp(1.0 - dist / l.direction.w, 0.0, 1.0);
            }

            if(type == 2) { // Spot
                float theta = dot(L, normalize(-l.direction.xyz));
                float epsilon = l.coneAngles.x - l.coneAngles.y;
                float intensity = clamp((theta - l.coneAngles.y) / epsilon, 0.0, 1.0);
                attenuation *= intensity;
            }
        }

        vec3 radiance = l.color.rgb * l.color.a * attenuation;

        vec3 H = normalize(V + L);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // --- Indirect Lighting ---
    float NdotV = max(dot(N, V), 0.0);
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    vec3 R_ibl = reflect(-V, N);

    const MAX_RELECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilteredMap, R_ibl, roughness * MAX_REFLECTION_LOD).rgb;

    vec2 brdf = texture(u_BrdfLUT, vec2(NdotV, roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F * brdf.x, brdf.y);

    vec3 ambient = (kD * diffuseIBL + specularIBL) * occlusion;

    // Final color
    vec3 color = ambient + Lo + emissive;

    // Tone Mappging (ACES)
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
