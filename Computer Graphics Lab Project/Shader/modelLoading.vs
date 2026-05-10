#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 boneIds; 
layout (location = 4) in vec4 weights;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    mat4 boneTransform = finalBonesMatrices[boneIds[0]] * weights[0];
    boneTransform     += finalBonesMatrices[boneIds[1]] * weights[1];
    boneTransform     += finalBonesMatrices[boneIds[2]] * weights[2];
    boneTransform     += finalBonesMatrices[boneIds[3]] * weights[3];

    vec4 totalPosition = boneTransform * vec4(aPos, 1.0f);

    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * totalPosition;
}