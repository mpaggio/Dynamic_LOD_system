#pragma once
#include "lib.h"
#include "strutture.h"

tuple<vector<Vertex>, vector<GLuint>> loadFBX();
vector<SimpleVertex> loadSimpleFBX(string modelPath);