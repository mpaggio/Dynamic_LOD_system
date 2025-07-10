#pragma once
#include "lib.h"
#include "strutture.h"

int getYIndex(int x, int z, int division);
float randomFloat(float min, float max);
vec3 randomPosition(float L);
vector<vec3> generateSphericalBase(const vec3& center, float radius);