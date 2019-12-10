#include "generate_noise.h"

static vec2 random_offset{ 0.0f, 0.0f };
void randomize_noise()
{
	random_offset.x = rand() * rand() / (float)RAND_MAX;
	random_offset.y = rand() * rand() / (float)RAND_MAX ;
}

map2d<float> generate_noise(size_t size, float scale, int iterations, float persistance, float lacunarity)
{
	map2d<float> noise_map{ size, size };

	if (scale <= 0.0f)
		scale = FLT_EPSILON;

	float min_value{ FLT_MAX }, max_value{ -FLT_MAX };
	noise_map.loop(
		[&](size_t x, size_t y, float) -> float
		{
			float amplitude{ 1.0f };
			float frequency{ 1.0f };
			float noise_value{ 0.0f };

			for (int i = 0; i < iterations; ++i)
			{
				float sampler_x = x / (size - 1.0f) * frequency;
				float sampler_y = y / (size - 1.0f) * frequency;

				noise_value += coef(-1.0f, 1.0f, glm::perlin(random_offset + scale * vec2{ sampler_x, sampler_y })) * amplitude;

				amplitude *= persistance;
				frequency *= lacunarity;
			}

			if (noise_value < min_value)
				min_value = noise_value;
			if (noise_value > max_value)
				max_value = noise_value;
			return noise_value;
		}
	);

	noise_map.loop(
		[&](size_t, size_t, float prev) -> float
		{
			return glm::pow(map(prev, min_value, max_value, 0.0f, 1.0f),4);
		}
	);
	return noise_map;
}
