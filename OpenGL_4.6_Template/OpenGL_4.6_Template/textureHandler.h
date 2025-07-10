#pragma once
#include "lib.h"
#include "strutture.h"

vector<GLuint> loadAllTextures();
GLuint loadTextureFromMaterial(aiMaterial* material, aiTextureType type, const string& directory);