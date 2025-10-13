#include "Chunk.h"

std::vector<GLfloat> Chunk::vertices = {
	// positions                    // front face
	-0.5f, -0.5f,  0.5f,     // bottom left
	 0.5f, -0.5f,  0.5f,      // bottom right
	 0.5f,  0.5f,  0.5f,       // top right
	-0.5f,  0.5f,  0.5f,       // top left 

	// back face
	 0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,

	 // top face
	 -0.5f,  0.5f,  0.5f,
	  0.5f,  0.5f,  0.5f,
	  0.5f,  0.5f, -0.5f,
	 -0.5f,  0.5f, -0.5f,

	 // bottom face
	 -0.5f, -0.5f, -0.5f,
	  0.5f, -0.5f, -0.5f,
	  0.5f, -0.5f,  0.5f,
	 -0.5f, -0.5f,  0.5f,

	 // right face
	 0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f,  0.5f,

	 // left face
	 -0.5f, -0.5f, -0.5f,
	 -0.5f, -0.5f,  0.5f,
	 -0.5f,  0.5f,  0.5f,
	 -0.5f,  0.5f, -0.5f,
};

std::vector<GLuint> Chunk::indices = {
	0, 1, 3,
	1, 2, 3,

	4, 5, 7,
	5, 6, 7,

	8, 9, 11,
	9, 10, 11,

	12, 13, 15,
	13, 14, 15,

	16, 17, 19,
	17, 18, 19,

	20, 21, 23,
	21, 22, 23
};

std::vector<GLushort> Chunk::pitchYaw = {
	encodeAngle(0.0f), encodeAngle(PI / 2),   // front face
	encodeAngle(0.0f), encodeAngle(PI / 2),
	encodeAngle(0.0f), encodeAngle(PI / 2),
	encodeAngle(0.0f), encodeAngle(PI / 2),

	encodeAngle(0.0f), encodeAngle(-PI / 2),  // back face
	encodeAngle(0.0f), encodeAngle(-PI / 2),
	encodeAngle(0.0f), encodeAngle(-PI / 2),
	encodeAngle(0.0f), encodeAngle(-PI / 2),

	encodeAngle(PI / 2), encodeAngle(0.0f),   // right face
	encodeAngle(PI / 2), encodeAngle(0.0f),
	encodeAngle(PI / 2), encodeAngle(0.0f),
	encodeAngle(PI / 2), encodeAngle(0.0f),

	encodeAngle(-PI / 2), encodeAngle(0.0f),  // left face
	encodeAngle(-PI / 2), encodeAngle(0.0f),
	encodeAngle(-PI / 2), encodeAngle(0.0f),
	encodeAngle(-PI / 2), encodeAngle(0.0f),

	encodeAngle(0.0f), encodeAngle(0.0f),       // top face
	encodeAngle(0.0f), encodeAngle(0.0f),
	encodeAngle(0.0f), encodeAngle(0.0f),
	encodeAngle(0.0f), encodeAngle(0.0f),

	encodeAngle(0.0f), encodeAngle(PI),       // bottom face
	encodeAngle(0.0f), encodeAngle(PI),
	encodeAngle(0.0f), encodeAngle(PI),
	encodeAngle(0.0f), encodeAngle(PI)
};

GLushort Chunk::encodeAngle(float radians) {
	return static_cast<GLushort>((radians + PI) / (2.0f * PI) * 65535.0f);
}

Chunk::Chunk() {
	
}

void Chunk::generateOffsets(const int xLimit, const int zLimit, const int LOD, std::unordered_map<glm::ivec2, std::vector<glm::vec3>>& chunks, glm::vec2& playerPos, Perlin& noise) {
	if (chunks.find(glm::ivec2(xLimit / CHUNK_SIZE, zLimit / CHUNK_SIZE)) == chunks.end()) {
		int gridLOD = pow(2, LOD);

		std::vector<glm::vec3> offsets;
		offsets.reserve(CHUNK_SIZE * CHUNK_SIZE);

		for (int x = xLimit; x < xLimit + CHUNK_SIZE; x += gridLOD) {
			for (int z = zLimit; z < zLimit + CHUNK_SIZE; z+= gridLOD) {
				int yval = noise.fractalBrownianMotion(x * 0.05f, z * 0.05f, OCTAVES);

				if (SINK_CULLING) {
					if (LOD > 1) {
						glm::vec2 blockPosition = glm::vec2(x, z);

						float distance = glm::length(blockPosition - playerPos);

						float startDist = CHUNK_SIZE * (LOD * 0.5);
						float endDist = startDist + 300.0 * LOD;

						float t = std::clamp((distance - startDist) / (endDist - startDist), 0.0f, 1.0f);
						float fade = 1.0 - glm::smoothstep(0.0f, 1.0f, t);

						float sinkStrength = 8.0 * log2(LOD + 1.0);

						float sinkDepth = fade * sinkStrength;

						if (sinkDepth < 13) {
							offsets.emplace_back(x, yval, z);
						}
					}
					else {
						offsets.emplace_back(x, yval, z);
					}
				}
				else {
					offsets.emplace_back(x, yval, z);
				}
			}
		}

		chunks[glm::ivec2(xLimit / CHUNK_SIZE, zLimit / CHUNK_SIZE)] = offsets;
	}
}

void Chunk::generateChunks(const glm::vec3& playerPos, Perlin& noise) {
	glm::ivec2 playerChunk = glm::floor(glm::vec2(playerPos.x, playerPos.z) / (float)CHUNK_SIZE);
	glm::vec2 playerPosition = glm::fvec2(playerPos.x, playerPos.z);

	if (prevPos != playerChunk) {

		if (!firstIteration) {
			checkChunkDifference();
		}

		std::jthread t0(&Chunk::generateSurroundingChunks, this, playerChunk, HIGH_DETAIL, std::ref(playerPosition), std::ref(noise));
		std::jthread t1(&Chunk::generateSurroundingChunks, this, playerChunk, MEDIUM_DETAIL, std::ref(playerPosition), std::ref(noise));
		std::jthread t2(&Chunk::generateSurroundingChunks, this, playerChunk, LOW_DETAIL, std::ref(playerPosition), std::ref(noise));
		std::jthread t3(&Chunk::generateSurroundingChunks, this, playerChunk, LOWER_DETAIL, std::ref(playerPosition), std::ref(noise));
		std::jthread t4(&Chunk::generateSurroundingChunks, this, playerChunk, LOWEST_DETAIL, std::ref(playerPosition), std::ref(noise));

		t0.join();
		t1.join();
		t2.join();
		t3.join();
		t4.join();

		allocateMeshData();

		temp_chunks_Mesh_LOD0.clear();
		temp_chunks_Mesh_LOD1.clear();
		temp_chunks_Mesh_LOD2.clear();
		temp_chunks_Mesh_LOD3.clear();
		temp_chunks_Mesh_LOD4.clear();


		prevPos = playerChunk;

		prev_chunk_Coord_LOD0 = chunk_Coord_LOD0;
		prev_chunk_Coords_LOD1 = chunk_Coords_LOD1;
		prev_chunk_Coords_LOD2 = chunk_Coords_LOD2;
		prev_chunk_Coords_LOD3 = chunk_Coords_LOD3;
		prev_chunk_Coords_LOD4 = chunk_Coords_LOD4;

		firstIteration = false;
	}
	
}


void Chunk::generateSurroundingChunks(const glm::ivec2& playerChunk, const int LOD, glm::vec2& playerPos, Perlin& noise) {
	if (LOD == 0) {
		chunk_Coord_LOD0 = playerChunk;
		generateOffsets(playerChunk.x * CHUNK_SIZE, playerChunk.y * CHUNK_SIZE, LOD, returnLODChunk(LOD), playerPos, noise);
	}
	else {
		int startX = -LOD;
		int startZ = -LOD;

		int gridSize = 1 + (LOD * 2);

		//std::vector<glm::ivec2>& coordsVector = returnChunkCoords(LOD);

		std::unordered_set<glm::ivec2>& coordsVector = returnChunkCoords(LOD);

		coordsVector.clear();

		for (size_t i = 0; i < gridSize; i++) {
			for (size_t j = 0; j < gridSize; j++) {
				generateOffsets((playerChunk.x + startX) * CHUNK_SIZE, (playerChunk.y + startZ) * CHUNK_SIZE, LOD, returnLODChunk(LOD), playerPos, noise);
				coordsVector.insert(glm::ivec2(playerChunk.x + startX, playerChunk.y + startZ));
				startZ++;
			}
			startZ = -LOD;
			startX++;
		}
	}
}

std::unordered_map<glm::ivec2, std::vector<glm::vec3>>& Chunk::returnLODChunk(const int LOD) {
	switch (LOD) {
	case HIGH_DETAIL:
		return temp_chunks_Mesh_LOD0;
	case MEDIUM_DETAIL:
		return temp_chunks_Mesh_LOD1;
	case LOW_DETAIL:
		return temp_chunks_Mesh_LOD2;
	case LOWER_DETAIL:
		return temp_chunks_Mesh_LOD3;
	case LOWEST_DETAIL:
		return temp_chunks_Mesh_LOD4;
	default:
		break;
	}
}

std::unordered_set<glm::ivec2>& Chunk::returnChunkCoords(const int LOD) {
	switch (LOD) {
	case MEDIUM_DETAIL:
		return chunk_Coords_LOD1;
	case LOW_DETAIL:
		return chunk_Coords_LOD2;
	case LOWER_DETAIL:
		return chunk_Coords_LOD3;
	case LOWEST_DETAIL:
		return chunk_Coords_LOD4;
	default:
		break;
	}
}

void Chunk::drawChunks(Shader& shaderProgram){
	shaderProgram.setUniform("lodScale", 1);
	chunks_Mesh_LOD0[chunk_Coord_LOD0]->Draw(indices);

	shaderProgram.setUniform("lodScale", 2);
	for (const auto& coord : chunk_Coords_LOD1) {
		chunks_Mesh_LOD1[coord]->Draw(indices);
	}

	shaderProgram.setUniform("lodScale", 4);
	for (const auto& coord : chunk_Coords_LOD2) {
		chunks_Mesh_LOD2[coord]->Draw(indices);
	}

	shaderProgram.setUniform("lodScale", 8);
	for (const auto& coord : chunk_Coords_LOD3) {
		chunks_Mesh_LOD3[coord]->Draw(indices);
	}

	shaderProgram.setUniform("lodScale", 16);
	for (const auto & coord : chunk_Coords_LOD4) {
		chunks_Mesh_LOD4[coord]->Draw(indices);
	}
}

void Chunk::deleteMeshData(){
	chunks_Mesh_LOD0.erase(chunk_Coord_LOD0);

	for (const auto& coord : chunk_Coords_LOD1) {
		chunks_Mesh_LOD1.erase(coord);
	}

	for (const auto& coord : chunk_Coords_LOD2) {
		chunks_Mesh_LOD2.erase(coord);
	}
	
	for (const auto& coord : chunk_Coords_LOD3) {
		chunks_Mesh_LOD3.erase(coord);
	}

	for (const auto& coord : chunk_Coords_LOD4) {
		chunks_Mesh_LOD4.erase(coord);
	}
}

void Chunk::checkChunkDifference() {
	chunks_Mesh_LOD0.erase(prev_chunk_Coord_LOD0);

	for (const auto& coord : prev_chunk_Coords_LOD1) {
		if (chunk_Coords_LOD1.find(coord) == chunk_Coords_LOD1.end()) {
			chunks_Mesh_LOD1.erase(coord);
		}
	}

	for (const auto& coord : prev_chunk_Coords_LOD2) {
		if (chunk_Coords_LOD2.find(coord) == chunk_Coords_LOD2.end()) {
			chunks_Mesh_LOD2.erase(coord);
		}
	}

	for (const auto& coord : prev_chunk_Coords_LOD3) {
		if (chunk_Coords_LOD3.find(coord) == chunk_Coords_LOD3.end()) {
			chunks_Mesh_LOD3.erase(coord);
		}
	}

	for (const auto& coord : prev_chunk_Coords_LOD4) {
		if (chunk_Coords_LOD4.find(coord) == chunk_Coords_LOD4.end()) {
			chunks_Mesh_LOD4.erase(coord);
		}
	}
}

void Chunk::allocateMeshData() {

	for (const auto& coord : temp_chunks_Mesh_LOD0) {

		if (chunks_Mesh_LOD0.find(coord.first) == chunks_Mesh_LOD0.end())
			chunks_Mesh_LOD0[coord.first] = std::make_unique<Mesh>(vertices, indices, pitchYaw, coord.second);
	}

	for (const auto& coord : temp_chunks_Mesh_LOD1) {
		if (chunks_Mesh_LOD1.find(coord.first) == chunks_Mesh_LOD1.end())
			chunks_Mesh_LOD1[coord.first] = std::make_unique<Mesh>(vertices, indices, pitchYaw, coord.second);
	}

	for (const auto& coord : temp_chunks_Mesh_LOD2) {
		if (chunks_Mesh_LOD2.find(coord.first) == chunks_Mesh_LOD2.end())
			chunks_Mesh_LOD2[coord.first] = std::make_unique<Mesh>(vertices, indices, pitchYaw, coord.second);
	}

	for (const auto& coord : temp_chunks_Mesh_LOD3) {
		if (chunks_Mesh_LOD3.find(coord.first) == chunks_Mesh_LOD3.end())
		chunks_Mesh_LOD3[coord.first] = std::make_unique<Mesh>(vertices, indices, pitchYaw, coord.second);
	}

	for (const auto& coord : temp_chunks_Mesh_LOD4) {
		if (chunks_Mesh_LOD4.find(coord.first) == chunks_Mesh_LOD4.end())
			chunks_Mesh_LOD4[coord.first] = std::make_unique<Mesh>(vertices, indices, pitchYaw, coord.second);
	}

}


Chunk::~Chunk() {
	deleteMeshData();
}
