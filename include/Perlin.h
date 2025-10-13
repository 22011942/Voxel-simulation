#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iterator>
#include <random>
#include <algorithm>
#include <iostream>

class Perlin {
private:
	std::vector<int> permutation;

	std::vector<int> makePermutation();

	glm::vec2 getConstantVector(int v) const;

	static float fade(float t);

	glm::mat2 rotation(float angle) const;
public:
	Perlin();

	float fractalBrownianMotion(float x, float y, int octaves) const;

	float Noise2D(float x, float y) const;

	~Perlin() = default;
};