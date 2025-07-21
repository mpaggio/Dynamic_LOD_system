#include "geometryHandler.h"


// --- SKYBOX --- //
vector<float> generateSkyboxCube() {
    vector<float> skyboxVertices = vector<float>{
        -1.0f,  1.0f, -1.0f,  // fronte
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  // retro
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,  // destra
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  // sinistra
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,  // alto
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,  // basso
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    return skyboxVertices;
}




// --- PLANE --- //
vector<float> simplePlane(int division, float width) {
	vector<float> plane;
	float triangleSide = width / division;

	for (int row = 0; row < division + 1; row++) {
		for (int col = 0; col < division + 1; col++) {
			vec3 vertex = vec3(col * triangleSide, 0.0, row * -triangleSide);
			plane.push_back(vertex.x);
			plane.push_back(vertex.z);
			plane.push_back(vertex.y);
		}
	}
	return plane;
}




// --- PLANE PATCHES --- //
vector<float> generatePatches(const vector<float>& plane, int division) {
    vector<float> patches;
    int rowLength = division + 1;

    for (int row = 0; row < division; ++row) {
        for (int col = 0; col < division; ++col) {
            int v0 = ((row + 1) * rowLength + col) * 3;       // bottom-left
            int v1 = ((row + 1) * rowLength + col + 1) * 3;   // bottom-right
            int v2 = (row * rowLength + col + 1) * 3;         // top-right
            int v3 = (row * rowLength + col) * 3;             // top-left

            patches.push_back(plane[v0]);
            patches.push_back(plane[v0 + 1]);
            patches.push_back(plane[v0 + 2]);

            patches.push_back(plane[v1]);
            patches.push_back(plane[v1 + 1]);
            patches.push_back(plane[v1 + 2]);

            patches.push_back(plane[v2]);
            patches.push_back(plane[v2 + 1]);
            patches.push_back(plane[v2 + 2]);

            patches.push_back(plane[v3]);
            patches.push_back(plane[v3 + 1]);
            patches.push_back(plane[v3 + 2]);
        }
    }

    return patches;
}




// --- SPHERES --- //
vector<vec3> sphereCorners = {
    vec3(0,  1,  0),
    vec3(0,  -1,  0),
    vec3(0, 0,  1),
    vec3(0, 0,  -1),
    vec3(1,  0, 0),
    vec3(-1,  0, 0)
};

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