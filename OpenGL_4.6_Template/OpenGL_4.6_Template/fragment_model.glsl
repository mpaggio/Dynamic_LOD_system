#version 460 core

in vec3 FragColor;
out vec4 color;

void main() {
    color = vec4(FragColor, 1.0);
    //color = vec4(1.0, 1.0, 1.0, 1.0);
}