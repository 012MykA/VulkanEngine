#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

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

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main() {
    const float gamma = 2.2;

    // Albedo
    vec4 albedoRGBA = u_Material.baseColorFactor;
    if(u_Material.baseColorTextureIdx != -1) {
        vec4 sampledColor = texture(baseColorMap, inTexCoord);
        albedoRGBA *= vec4(pow(sampledColor.rgb, vec3(gamma)), sampledColor.a);
    }
    vec3 albedo = albedoRGBA.rgb;

    // Metallic
    float metallic = u_Material.metallicFactor;
    if(u_Material.metallicTextureIdx != -1) {
        metallic *= texture(metallicMap, inTexCoord).r;
    }

    float roughness = u_Material.roughnessFactor;
    if(u_Material.roughnessTextureIdx != -1) {
        roughness *= texture(roughnessMap, inTexCoord).g;
    }

    vec3 emissive = u_Material.emissiveFactor;
    if(u_Material.emissiveTextureIdx != -1) {
        emissive *= pow(texture(emissiveMap, inTexCoord).rgb, vec3(gamma));
    }

    vec3 N = normalize(inNormal);
    vec3 V = normalize(u_Global.cameraPos.xyz - inWorldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Light
    vec3 L = normalize(u_Global.light.position.xyz - inWorldPos);
    vec3 H = normalize(V + L);

    float distance = length(u_Global.light.position.xyz - inWorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = u_Global.light.color.rgb * u_Global.light.color.a * attenuation;

    // BRDF Cook-Torrance
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // Energy balance
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);

    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.03) * albedo;

    vec3 color = ambient + Lo + emissive;

    // HDR Tonemapping
    color = color / (color + vec3(1.0));
    // Gamma Correction
    color = pow(color, vec3(1.0 / gamma));

    outColor = vec4(color, albedoRGBA.a);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
