#version 460 core

layout(location = 0) in vec3 aPos; //Attributo posizione (vettore a 2 componenti)

void main() {
    gl_Position = vec4(aPos.x, aPos.z, aPos.y, 1.0); //Trasforma l'attributo in un vettore a 4 componenti (le due componenti già presenti in aPos, il livello di profondità e il valore per essere una coordinata omogenea)
}