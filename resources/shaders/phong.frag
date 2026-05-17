#version 330 core

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 FragColor;

uniform vec3 u_LightPos;
uniform vec3 u_LightColor;
uniform vec3 u_ViewPos;

uniform vec3 u_Ambient;
uniform vec3 u_Diffuse;
uniform vec3 u_Specular;
uniform float u_Shininess;

uniform sampler2D u_DiffuseMap;
uniform bool u_HasDiffuseMap;

void main() {
    vec3 normal = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightPos - v_FragPos);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    // ambient
    vec3 ambient = u_Ambient * 0.3;

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * u_Diffuse * u_LightColor;

    // specular (Blinn-Phong)
    float spec = pow(max(dot(normal, halfDir), 0.0), u_Shininess);
    vec3 specular = spec * u_Specular * u_LightColor;

    vec3 baseColor = u_Diffuse;
    if (u_HasDiffuseMap) {
        baseColor = texture(u_DiffuseMap, v_TexCoord).rgb;
    }

    vec3 result = ambient * baseColor + diffuse * baseColor + specular;
    FragColor = vec4(result, 1.0);
}
