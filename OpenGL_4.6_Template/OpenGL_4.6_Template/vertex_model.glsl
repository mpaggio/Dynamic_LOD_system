#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4 aWeights;

out vec3 FragColor;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 bones[128]; // Numero massimo di bone supportati

void main() {
    mat4 skinMatrix =
        aWeights[0] * bones[aBoneIDs[0]] +
        aWeights[1] * bones[aBoneIDs[1]] +
        aWeights[2] * bones[aBoneIDs[2]] +
        aWeights[3] * bones[aBoneIDs[3]];

    vec4 skinnedPos = skinMatrix * vec4(aPos, 1.0);

    FragPos = vec3(model * skinnedPos);
    Normal = normalize(mat3(skinMatrix) * aNormal);
    FragColor = aColor;

    gl_Position = proj * view * vec4(FragPos, 1.0);
}