#include "noiseHandler.h"

inline vector<int> MakePermutation() {
	vector<int> p(256);
	for (int i = 0; i < 256; ++i)
		p[i] = i;

	//Utilizza un generatore Mersenne Twister come seme casuale
	random_device rd;
	mt19937 gen(rd());
	shuffle(p.begin(), p.end(), gen);

	p.insert(p.end(), p.begin(), p.end()); //Duplica l'array

	return p;
}

static vector<int> permutation = MakePermutation();

vec2 GetConstantVector(int value) {
	switch (value & 3) {
		case 0: 
			return vec2(1.0f, 1.0f);

		case 1: 
			return vec2(-1.0f, 1.0f);
	
		case 2: 
			return vec2(-1.0f, -1.0f);
	
		default: 
			return vec2(1.0f, -1.0f);
	}
}

float Fade(float t) {
	return ((6 * t - 15) * t + 10) * t * t * t;
}

float Lerp(float t, float a, float b) {
	return a + t * (b - a);
}

float Noise2D(float x, float y) {
	int X = static_cast<int>(floor(x)) & 255;
	int Y = static_cast<int>(floor(y)) & 255;

	float x_decimal = x - floor(x);
	float y_decimal = y - floor(y);

	vec2 topRight = vec2(x_decimal - 1.0, y_decimal - 1.0);
	vec2 topLeft = vec2(x_decimal, y_decimal - 1.0);
	vec2 bottomRight = vec2(x_decimal - 1.0, y_decimal);
	vec2 bottomLeft = vec2(x_decimal, y_decimal);

	// Select a value from the permutation array for each of the 4 corners
	int valueTopRight = permutation[permutation[X + 1] + Y + 1];
	int valueTopLeft = permutation[permutation[X] + Y + 1];
	int valueBottomRight = permutation[permutation[X + 1] + Y];
	int valueBottomLeft = permutation[permutation[X] + Y];

	float dotTopRight = dot(topRight, GetConstantVector(valueTopRight));
	float dotTopLeft = dot(topLeft, GetConstantVector(valueTopLeft));
	float dotBottomRight = dot(bottomRight, GetConstantVector(valueBottomRight));
	float dotBottomLeft = dot(bottomLeft, GetConstantVector(valueBottomLeft));

	float u = Fade(x_decimal);
	float v = Fade(y_decimal);

	return Lerp(u,
		Lerp(v, dotBottomLeft, dotTopLeft),
		Lerp(v, dotBottomRight, dotTopRight)
	);
}

float FractalBrownianMotion(float x, float y, int numOctaves) {
	float result = 0.0f;
	float amplitude = 0.9f; //Definisce l'impatto dell'ottava corrente
	float frequency = 0.005f; //Frequenza bassa (colline e rilievi morbidi), frequenza alta (montagne e rilievi ripidi).
	float gain = 0.5f;
	float lacunarity = 2.0f;

	for (int i = 0; i < numOctaves; ++i) {
		result += amplitude * Noise2D(x * frequency, y * frequency);
		amplitude *= gain;
		frequency *= lacunarity;
	}

	return result;
}

GLuint generateFBMTexture(int width, int height, int numOctaves) {
	vector<float> data(width * height);
	float scale = 512.0f * 2.5;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			float xf = static_cast<float>(x) / width;
			float yf = static_cast<float>(y) / height;
			float value = FractalBrownianMotion(xf * scale, yf * scale, numOctaves);
			data[(y * width) + x] = value;
		}
	}

	// Genera texture OpenGL
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data.data());

	// Impostazioni base
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return textureID;
}
