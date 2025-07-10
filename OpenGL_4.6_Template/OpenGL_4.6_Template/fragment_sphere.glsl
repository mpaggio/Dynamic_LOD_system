#version 460 core

in vec4 gs_worldPos; //Input proveniente dal Geometry Shader
out vec4 FragColor; //Output del Fragment Shader, ovvero il colore finale del pixel (variabile mandata direttamente al frame buffer)

void main() {
    FragColor = vec4(0.95f, 0.9f, 0.1f, 0.0f); //Imposta il colore RGBA del pixel
}