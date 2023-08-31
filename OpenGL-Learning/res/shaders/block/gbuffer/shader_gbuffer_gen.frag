//shader fragment
#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gUV;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gMisc; // TexId, Reflectiveness

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in float TexIndex;
in float Reflection;

void main()
{    
    gPosition = vec4(FragPos, 1.0);
    gUV = vec4(TexCoords, 1.0, 1.0);
    gNormal = vec4(Normal, 1.0);
    gMisc = vec4(TexIndex, Reflection, 1.0, 1.0);
}