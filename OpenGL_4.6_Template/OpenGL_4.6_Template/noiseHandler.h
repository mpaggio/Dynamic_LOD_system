#pragma once
#include "lib.h"
#include "strutture.h"

float FractalBrownianMotion(float x, float y, int numOctaves);
GLuint generateFBMTexture(int width, int height, int numOctaves);