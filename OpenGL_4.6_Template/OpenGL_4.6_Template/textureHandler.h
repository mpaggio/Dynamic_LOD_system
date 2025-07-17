#pragma once
#include "lib.h"
#include "strutture.h"

GLuint loadSkybox();
vector<GLuint> loadAllTextures();
GLuint loadSingleTexture(const string& path);