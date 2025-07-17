#pragma once
#include "lib.h"
#include "strutture.h"

BufferPair INIT_PLANE_BUFFERS(vector<float> planeVertices);
BufferPair INIT_QUAD_BUFFERS(float* vertices, size_t count);
BufferPair INIT_SPHERE_BUFFERS(vector<vec3> instancePositions, vector<vec3> allCenters);
ModelBufferPair INIT_MODEL_BUFFERS();
BufferPair INIT_SKYBOX_BUFFERS(vector<float> skyboxVertices);