#shader fragment
#version 330 core
precision highp float;

layout(location = 0) out vec4 o_Color;

in vec2 v_UV;
in vec3 v_Normal;
flat in int v_TexIndex;
in vec3 v_FragPos;
in float v_Reflection;

// Structs
struct DirectionalLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Uniforms
uniform sampler2D u_TextureMap;
uniform samplerCube u_Cubemap;
uniform float u_Refraction;

uniform vec3 u_ViewPosition;

// Lights
uniform DirectionalLight u_DirLight;

vec3 AffectDirectionallight(DirectionalLight DirLight, vec3 normal, vec3 viewDirection);

void main(){
    vec3 I = normalize(v_FragPos - u_ViewPosition);
    vec3 Rf = reflect(I, normalize(v_Normal));
    vec4 color = texture(u_TextureMap, v_UV);

    float ratio = 1.00 / 1.52;
    vec3 Rr = refract(I, normalize(v_Normal), ratio);
   
    vec4 blockColor = (color * (1.0 - v_Reflection) + vec4(texture(u_Cubemap, Rr).rgb, 1.0) * v_Reflection) * (1.0 - 0.1 * u_Refraction)
                + vec4(texture(u_Cubemap, Rr).rgb, 1.0) * u_Refraction * 0.1;

    vec3 viewDirection = normalize(u_ViewPosition - v_FragPos);

    o_Color = blockColor * vec4(AffectDirectionallight(u_DirLight, v_Normal, viewDirection), 1.0);
}

// Light Functions
vec3 AffectDirectionallight(DirectionalLight DirLight, vec3 normal, vec3 viewDirection){
    vec3 lightDir = normalize(-DirLight.direction);
    float diffAngle = max(dot(normal, lightDir), 0.0);

    vec3 reflectDirection = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);

    vec3 diffuse = diffAngle * DirLight.diffuse;
    vec3 specular = spec * DirLight.specular;
    vec3 ambient = DirLight.ambient;

    return (diffuse + specular + ambient);
}
