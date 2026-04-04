#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;

// Global UBO
layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 proj;
    vec4 lightDir;    // rgb - direction, a - unused
    vec4 lightColor; // rgb - color,     a - intencity
    vec4 cameraPos;  // rgb - position   a - unused
} u_Global;

// Material UBO
layout(set = 1, binding = 0) uniform MaterialUBO
{
    vec4  ambientColor;
    vec4  diffuseColor;
    vec4  specularColor; // rgb - color, a = shininess
} u_Material;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 N = normalize(inNormal);
    vec3 L = normalize(-u_Global.lightDir.rgb);
    vec3 V = normalize(u_Global.cameraPos.rgb - inWorldPos);

    float lightIntensity = u_Global.lightColor.a;
    vec3  lightColor     = u_Global.lightColor.rgb * lightIntensity;

    // --- Ambient ---
    vec3 ambient = u_Material.ambientColor.rgb * lightColor;

    // --- Diffuse ---
    float diff   = max(dot(N, L), 0.0);
    vec3  diffuse = diff * u_Material.diffuseColor.rgb * lightColor;

    // --- Specular ---
    vec3  R         = reflect(-L, N);
    float shininess = u_Material.specularColor.a;
    float spec      = pow(max(dot(V, R), 0.0), shininess);
    vec3  specular  = spec * u_Material.specularColor.rgb * lightColor;

    vec3 result = ambient + diffuse + specular;

    outColor = vec4(result, 1.0);
}
