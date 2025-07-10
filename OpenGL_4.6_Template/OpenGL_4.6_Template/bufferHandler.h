#pragma once
#include "lib.h"
#include "strutture.h"

BufferPair INIT_PLANE_BUFFERS(vector<float> planeVertices);
BufferPair INIT_QUAD_BUFFERS(float* vertices, size_t count);
BufferPair INIT_SPHERE_BUFFERS(vector<vec3> instancePositions, vector<vec3> allCenters);
BufferPair INIT_DEER_BUFFERS(vector<Vertex> vertices);
BufferPair INIT_SIMPLE_MODEL_BUFFERS(vector<SimpleVertex> vertices);