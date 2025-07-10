#include "utilities.h"

vector<vec3> sphereCorners = {
    vec3(0,  1,  0),
    vec3(0,  -1,  0),
    vec3(0, 0,  1),
    vec3(0, 0,  -1),
    vec3(1,  0, 0),
    vec3(-1,  0, 0)
};

// Ogni ottante usa 3 dei vertici sopra per formare un triangolo grezzo
const int ottanteTriangles[8][3] = {
    {0, 2, 4}, // Ottante 1
    {0, 3, 4}, // Ottante 2
    {0, 3, 5}, // Ottante 3
    {0, 2, 5}, // Ottante 4
    {1, 2, 4}, // Ottante 5
    {1, 3, 4}, // Ottante 6
    {1, 3, 5}, // Ottante 7
    {1, 2, 5}  // Ottante 8
};

int getYIndex(int x, int z, int division) {
    return (z * division + x) * 3 + 1; // +1 per la y
}

// Funzione per generare float casuale tra min e max
float randomFloat(float min, float max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

// Genera posizione random dentro quadrato di lato L
vec3 randomPosition(float L) {
    return vec3(
        randomFloat(0, L), // solo coordinate positive in X
        randomFloat(-L, 0), // y rimane come preferisci (ad esempio altezza)
        randomFloat(L/5, L/2) // coordinate negative in Z (perché la mappa va verso -Z)
    );
}

vector<vec3> generateSphericalBase(const vec3& center, float radius) {
    vector<vec3> verts;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 3; ++j) {
            vec3 dir = normalize(sphereCorners[ottanteTriangles[i][j]]);
            vec3 offset = dir * radius;
            verts.push_back(center + offset);
        }
    }

    return verts;
}
