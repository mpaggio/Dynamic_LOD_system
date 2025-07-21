#pragma once
#include "lib.h"
#include "strutture.h"

vector<float> generateSkyboxCube();
vector<float> simplePlane(int division, float width);
vector<vec3> generateSphericalBase(const vec3& center, float radius);
vector<float> generatePatches(const vector<float>& plane, int division);