#version 460 core

layout(vertices = 3) out; // Patch di 3 vertici per triangoli

in vec3 vsCenter[]; //Dal vertex shader
out vec3 tcCenter[]; //Al TES

uniform vec3 cameraPosition;
uniform sampler2D u_fbmTexture;
uniform float sphereYOffset;

const int MIN_TES = 1;
const int MAX_TES = 16;
const float MAX_DIST = 4.0;

void main() {
    // Passa i vertici in output senza modifiche
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcCenter[gl_InvocationID] = vsCenter[gl_InvocationID];

    // Solo un thread decide i livelli di tessellazione
    if (gl_InvocationID == 0) {
        vec3 center = vsCenter[0];

        float dist = length(cameraPosition - center);
        int tessLevel;
        if (dist > MAX_DIST) {
            tessLevel = MIN_TES;  // minimo livello di tessellazione oltre max distanza
        }
        else {
            float lodFactor = dist / MAX_DIST;
            tessLevel = int(mix(MAX_TES, MIN_TES, lodFactor));
        }

        //Outer
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        
        //Inner
        gl_TessLevelInner[0] = tessLevel;
    }
}
