#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4 worldPos[]; //Input dal Tessellation Evaluation Shader
out vec4 gs_worldPos; //Output per il Fragment Shader

uniform float u_time;

uniform mat4 view; //Simula la camera (sposta e ruota il mondo per simulare il punto di vista dell'osservatore)
uniform mat4 proj; //converte le coordinate da mondo 3D alla visione 2D (matrice di proiezione)

void main() {

    vec4 center = (worldPos[0] + worldPos[1] + worldPos[2]) / 3.0; //Calcola il centro del triangolo
    float strength = sin(u_time) * 0.2; //Fattore di animazione da -0.2 a 0.2

    for (int i = 0; i < 3; ++i) {
        vec4 offset = normalize(worldPos[i] - center);
        vec4 newWorldPos = worldPos[i]; //+ offset * strength;

        gl_Position = proj * view * newWorldPos; //Trasforma da coordinate 3D del mondo nelle coordinate di vista della telecamera e infine in coordinate in 2D
        gs_worldPos = newWorldPos;

        EmitVertex();
    }
    EndPrimitive();

}
