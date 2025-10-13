#include "Perlin.h"

Perlin::Perlin() : permutation(makePermutation()) {

}

std::vector<int> Perlin::makePermutation() {
	std::vector<int> perm(256);

	for (int indx = 0; indx < 256; indx++) {
		perm[indx] = indx;
	}

	// Shuffle using a random engine
	std::random_device rd;
	std::mt19937 g(rd()); // Mersenne Twister engine
	std::shuffle(perm.begin(), perm.end(), g);

	for (int i = 0; i < 256; i++) { //to prevent an out of bounds later
		perm.push_back(perm[i]);
	}

	return perm;
}

glm::vec2 Perlin::getConstantVector(int v) const {
	int h = v & 3; // keep only last 2 bits (0-3)

	switch (h) {
	case 0:
		return glm::vec2(1.0f, 1.0f);
	case 1:
		return glm::vec2(-1.0f, 1.0f);
	case 2:
		return glm::vec2(-1.0f, -1.0f);
	case 3:
		return glm::vec2(-1.0f, -1.0f);
	default:
		return glm::vec2(1.0f, -1.0f);
	}
}

float Perlin::fade(float t) {
	return ((6.0f * t - 15.0f) * t + 10.0f) * t * t * t;
}

glm::mat2 Perlin::rotation(float angle) const {
	float c = cos(angle);
	float s = sin(angle);
	return glm::mat2(c, -s, s, c);
}

float Perlin::Noise2D(float x, float y) const {
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;

	float xf = x - floor(x);
	float yf = y - floor(y);

	glm::vec2 topRight(xf - 1.0f, yf - 1.0f);
	glm::vec2 topLeft(xf, yf - 1.0f);
	glm::vec2 bottomRight(xf - 1.0f, yf);
	glm::vec2 bottomLeft(xf, yf);

	int valueTopRight = permutation[permutation[X + 1] + Y + 1];
	int valueTopLeft = permutation[permutation[X] + Y + 1];
	int valueBottomRight = permutation[permutation[X + 1] + Y];
	int valueBottomLeft = permutation[permutation[X] + Y];

	float dotTopRight = glm::dot(topRight, getConstantVector(valueTopRight));
	float dotTopLeft = glm::dot(topLeft, getConstantVector(valueTopLeft));
	float dotBottomRight = glm::dot(bottomRight, getConstantVector(valueBottomRight));
	float dotBottomLeft = glm::dot(bottomLeft, getConstantVector(valueBottomLeft));

	float u = fade(xf);
	float v = fade(yf);

	float val = glm::mix(glm::mix(dotBottomLeft, dotTopLeft, v), glm::mix(dotBottomRight, dotTopRight, v), u);
	
	return val;
}

float Perlin::fractalBrownianMotion(float x, float y, int octaves) const {
	glm::vec2 pos(x, y);
	int scale = 35;
	float result = 0.0f;
	float amplitude = 65.0f;
	float frequency = 1.0f / scale;

	glm::mat2 rot = rotation(glm::radians(30.0f));

	for (int octave = 0; octave < octaves; ++octave) {
		glm::vec2 rotated = rot * (pos * frequency);

		float n = amplitude * Noise2D(rotated.x, rotated.y);
		result += n;

		amplitude *= 0.5f;  // reduce amplitude each octave Decay
		frequency *= 2.0f;  // increase frequency each octave lacunarity

		rot = rotation(glm::radians(30.0f * (octave + 1)));
	}
	
	return result;
}