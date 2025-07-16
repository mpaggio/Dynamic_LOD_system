#pragma once
#include "lib.h"
#include "strutture.h"

void loadModel(string modelPath, ModelState state);
void updateBoneTransforms(float animationTime, ModelState state);