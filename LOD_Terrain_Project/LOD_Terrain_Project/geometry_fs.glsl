#version 460 core

layout(triangles) in;
layout(points, max_vertices = 3) out;

layout(location = 0) out vec3 characterPositionTransformFeedback;
layout(location = 1) out vec3 characterNormalTransformFeedback;

in vec4 worldPos[];

float MAX_DISTANCE = 4.0f;

uniform bool useCharacterToTess;
uniform vec3 cameraPosition;
uniform vec3 characterPosition;
uniform mat4 view; // Simula la camera
uniform mat4 proj; // Matrice di proiezione

bool isPointInTriangle(vec2 p, vec2 a, vec2 b, vec2 c) {
    // Calcolo dei vettori
    vec2 v0 = c - a;
    vec2 v1 = b - a;
    vec2 v2 = p - a;

    // Calcolo dei dot product
    float dot00 = dot(v0, v0);
    float dot01 = dot(v0, v1);
    float dot02 = dot(v0, v2);
    float dot11 = dot(v1, v1);
    float dot12 = dot(v1, v2);

    // Calcolo baricentriche
    float invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0.0) && (v >= 0.0) && (u + v <= 1.0);
}

void main() {
    // Calcolo centro triangolo
    vec3 center = (worldPos[0].xyz + worldPos[1].xyz + worldPos[2].xyz) / 3.0;

    // Setting dell'altezza del personaggio effettiva
    vec3 p0 = worldPos[0].xyz;
    vec3 p1 = worldPos[1].xyz;
    vec3 p2 = worldPos[2].xyz;
    vec2 charXZ = characterPosition.xz;
    bool inside = isPointInTriangle(charXZ, p0.xz, p1.xz, p2.xz);
    float interpolatedY = -100.0;

    if (inside) {
        vec2 v0 = p1.xz - p0.xz;
        vec2 v1 = p2.xz - p0.xz;
        vec2 v2 = charXZ - p0.xz;

        float d00 = dot(v0, v0);
        float d01 = dot(v0, v1);
        float d11 = dot(v1, v1);
        float d20 = dot(v2, v0);
        float d21 = dot(v2, v1);

        float denom = d00 * d11 - d01 * d01;
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0 - v - w;

        interpolatedY = u * p0.y + v * p1.y + w * p2.y;

        vec3 edge1 = p1 - p0;
        vec3 edge2 = p2 - p0;
        vec3 normal = normalize(cross(edge1, edge2));

        characterPositionTransformFeedback = vec3(characterPosition.x, interpolatedY, characterPosition.z);
        characterNormalTransformFeedback = normal;

        EmitVertex();
        EndPrimitive();
    }
    
}
