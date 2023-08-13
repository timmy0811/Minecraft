//shader fragment
#version 330 core
precision highp float;

layout(location = 0) out vec4 o_Color;

in vec2 v_UV;
in vec3 v_Normal;
flat in int v_TexIndex;
in vec3 v_FragPos;
in float v_Reflection;

in vec4 v_LightViewPosition;

uniform sampler2D u_ShadowMap;

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

// Fog
uniform float u_FogAffectDistance;
uniform float u_FogDensity;

uniform vec3 u_SkyBoxColor;

// Lights
uniform DirectionalLight u_DirLight;

vec3 AffectDirectionallight(DirectionalLight DirLight, vec3 normal, vec3 viewDirection);
float CalculateShadow();

void main(){
    vec3 I = normalize(v_FragPos - u_ViewPosition);
    vec3 Rf = reflect(I, normalize(v_Normal));
    vec4 color = texture(u_TextureMap, v_UV);
    if(color.a <= 0.1) discard;

    // Fog
    float distanceToCamera = length(v_FragPos - u_ViewPosition);
    float fogFactor = (1.0 / (1 + pow(2.71828183, (-u_FogDensity * (distanceToCamera - u_FogAffectDistance)))));

    float ratio = 1.00 / 1.52;
    vec3 Rr = refract(I, normalize(v_Normal), ratio);
   
    vec4 blockColor = (color * (1.0 - v_Reflection) + vec4(texture(u_Cubemap, Rr).rgb, 1.0) * v_Reflection) * (1.0 - 0.1 * u_Refraction)
                + vec4(texture(u_Cubemap, Rr).rgb, 1.0) * u_Refraction * 0.1;

    vec3 viewDirection = normalize(u_ViewPosition - v_FragPos);

    //o_Color = (blockColor * vec4(AffectDirectionallight(u_DirLight, v_Normal, viewDirection), 1.0)) * (1.0 - fogFactor) + (vec4(u_SkyBoxColor * fogFactor, 1.0));
    o_Color = blockColor * vec4(AffectDirectionallight(u_DirLight, v_Normal, viewDirection), 1.0) * (1.0 - fogFactor) * (1.0 - CalculateShadow() * 0.35) + (vec4(u_SkyBoxColor * fogFactor, 1.0));
}

// Light Functions
vec3 AffectDirectionallight(DirectionalLight DirLight, vec3 normal, vec3 viewDirection){
    vec3 lightDir = normalize(-DirLight.direction);
    float diffAngle = max(dot(normal, lightDir), 0.0);

    vec3 reflectDirection = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 16);

    vec3 diffuse = diffAngle * DirLight.diffuse * 0.5 + 0.5;
    vec3 specular = spec * DirLight.specular;
    vec3 ambient = DirLight.ambient;

    return (diffuse + specular + ambient);
}

float CalculateShadow(){
    vec3 coord = v_LightViewPosition.xyz / v_LightViewPosition.w;
    coord = coord * 0.5 + 0.5;
    float closestDepth = texture(u_ShadowMap, coord.xy).r;
    float currentDepth = coord.z;

    // Double check bias
    float bias = max(0.05 * (1.0 - abs(dot(normalize(v_Normal), u_DirLight.direction))), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);

    for(int x = -1; x <= 1; ++x){
        for(int y = -1; y <= 1; ++y){
            float pcfDepth = texture(u_ShadowMap, coord.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - 0.002 > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    if(coord.z >= 1.0) shadow = 0.0;

    return shadow;
}