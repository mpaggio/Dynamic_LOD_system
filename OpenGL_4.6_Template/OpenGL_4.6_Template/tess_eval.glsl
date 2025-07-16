#version 460 core

//UNA ESECUZIONE PER OGNI VERTICE GENERATO DAL TCS

layout(quads, fractional_odd_spacing, cw) in; //Specifica che il patch è un triangolo che usa una spaziatura uguale e definendo la direzione dell'ordinamento dei vertici (clockwise)

uniform float terrainSize_tes;
uniform mat4 model; //Matrice di trasformazione da coordinate locali a coordinate globali 3D
uniform sampler2D u_fbmTexture;

const float UV_SCALE = 1.0f / terrainSize_tes;
const float HEIGHT_SCALE = 1.5;

out vec4 worldPos; //Da inviare al Geometry Shader

void main() {

    /* --- gl_in è un array che contiene le posizioni dei vertici originali del patch --- */
    /* --- gl_TessCoord contiene le coordinate baricentriche del vertice generato dalla tessellazione attualmente elaborato dal TES --- */

    //Prende le posizioni 3D di ciascun vertice originale
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 p3 = gl_in[3].gl_Position.xyz;

    //Prende le coordinate baricentriche del vertice generato dalla tessellazione (le coordinate del punto normalizzate in [0,1])
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    //Calcola la posizione del nuovo vertice come interpolazione bilineare dei vertici originali
    vec3 pos = ((1.0 - u) * (1.0 - v) * p0) + (u * (1.0 - v) * p1) + (u * v * p2) + ((1.0 - u) * v * p3); 
    
    vec2 uv = pos.xz * UV_SCALE; // Normalizza le coordinate UV
    float height = texture(u_fbmTexture, uv).r; // Displacement dell'altezza
    pos.y += height * HEIGHT_SCALE; // Applica l'altezza con uno scaling

    worldPos = model * vec4(pos, 1.0); //Trasforma in coordinate del mondo 3D a partire da quelle locali
}