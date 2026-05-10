#version 330 core

// Atributos de los vértices (deben coincidir con el setupMesh en Mesh.h)
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in ivec4 boneIds;  // IDs de los huesos
layout (location = 4) in vec4 weights;   // Pesos de los huesos

// Salidas hacia el Fragment Shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// Uniforms
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 finalBonesMatrices[MAX_BONES]; // Transformaciones de la animación

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    bool hasBones = false;

    // Calcular la posición y normal influenciada por los huesos
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;

        if(boneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(position, 1.0f);
            break;
        }

        hasBones = true;
        
        // Posición afectada por el hueso
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(position, 1.0f);
        totalPosition += localPosition * weights[i];

        // Normal afectada por el hueso
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * normal;
        totalNormal += localNormal * weights[i];
    }

    // Si el modelo no tiene huesos (o es una malla estática), usamos su posición original
    if(!hasBones)
    {
        totalPosition = vec4(position, 1.0f);
        totalNormal = normal;
    }

    // Calcular normales en el espacio del mundo para la iluminación
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * totalNormal;

    FragPos = vec3(model * totalPosition);
    TexCoords = texCoords;

    gl_Position = projection * view * model * totalPosition;
}