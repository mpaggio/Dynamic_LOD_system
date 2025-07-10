#pragma once
#include "lib.h"

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
    int textureIndex;
} Vertex;

typedef struct {
    vec3 position;
    vec3 normal;
    vec3 color;
    int boneIDs[4];
    float weights[4];
} SimpleVertex;

typedef struct {
    vec3 position; // Posizione della camera nello spazio 3D
    vec3 target; // Punto verso cui la camera è puntata
    vec3 upVector; // Vettore che indica la direzione "up" della camera
    vec3 direction; // Vettore che indica la direzione di visione della camera
} ViewSetup;

//gestione proiezione
typedef struct {
    float fovY; // Campo visivo verticale in gradi
    float aspect; // Rapporto tra larghezza e altezza del viewport
    float near_plane; // Distanza del piano di clipping vicino
    float far_plane; // Distanza del piano di clipping lontano
} PerspectiveSetup;

//gestione buffer
typedef struct{
    unsigned int vao;
    unsigned int vbo;
    unsigned int centerVBO;
} BufferPair;