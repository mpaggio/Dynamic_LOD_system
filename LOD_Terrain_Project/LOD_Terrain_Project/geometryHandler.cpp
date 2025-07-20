#include "geometryHandler.h"

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

vector<float> generateSkyboxCube() {
    vector<float> skyboxVertices = vector<float>{
        // Positions          
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