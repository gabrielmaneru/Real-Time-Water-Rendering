#pragma once
#include "map2d.h"
#include "math_utils.h"
#include <glm/glm.h>
void randomize_noise();
map2d<float> generate_noise(size_t size, float scale, int iterations, float persistance, float lacunarity, float lowerbound, float upperbound, vec2 falloff);