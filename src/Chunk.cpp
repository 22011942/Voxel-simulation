#include "Chunk.h"


ThreadPool chunkPool(5);


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

Chunk::Chunk() : chunk_Coord_LOD0{ glm::ivec2(0, 0) }, prev_chunk_Coord_LOD0{ glm::ivec2(0, 0) } {
	for (int i = 0; i < 5; ++i) {
		generationDone[i] = false;
		LODReady[i] = false;
	}
		
}

void Chunk::generateOffsets(const int xLimit, const int zLimit, const int LOD, std::unordered_map<glm::ivec2, std::vector<glm::vec3>>& chunks, glm::vec2& playerPos, Perlin& noise) {
	if (chunks.find(glm::ivec2(xLimit / CHUNK_SIZE, zLimit / CHUNK_SIZE)) == chunks.end()) {
		int gridLOD = pow(2, LOD);

		std::vector<glm::vec3> offsets;
		offsets.reserve(CHUNK_SIZE * CHUNK_SIZE);

		int bleed = 0;

		if (LOD == 0) {
			bleed = 100;
		}

		for (int x = xLimit - bleed; x < xLimit + CHUNK_SIZE + bleed; x += gridLOD) {
			for (int z = zLimit - bleed; z < zLimit + CHUNK_SIZE + bleed; z+= gridLOD) {
				int yval = noise.fractalBrownianMotion(x * 0.05f, z * 0.05f, OCTAVES);

				if (SINK_CULLING) {
					if (LOD > 1) {
						glm::vec2 blockPosition = glm::vec2(x, z);

						float distance = glm::length(blockPosition - playerPos);

						float startDist = startDistMultiplier * LOD;
						float endDist = startDist + endDistAdd * LOD;

						float t = std::clamp((distance - startDist) / std::max(1.0f, endDist - startDist), 0.0f, 1.0f);
						float fade = std::pow(1.0f - glm::smoothstep(0.0f, 1.0f, t), fadePow);

						float sinkStrength = baseSinkStrength * log2(LOD + 1.0);

						float sinkDepth = fade * sinkStrength;

						if (sinkDepth < 1) {
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

void Chunk::generateChunks(const glm::vec3& playerPos, Perlin& noise, const int LOD) {
	glm::ivec2 playerChunk = glm::floor(glm::vec2(playerPos.x, playerPos.z) / (float)CHUNK_SIZE);
	glm::vec2 playerPosition = glm::fvec2(playerPos.x, playerPos.z);


	if (prevPos[LOD] != playerChunk) { 

		if (!firstIteration)
			checkChunkDifference(LOD);
		

		generationDone[LOD] = false;

		auto future = chunkPool.enqueue([=, this]() mutable {
			generateSurroundingChunks(playerChunk, LOD, playerPosition, noise);
			generationDone[LOD].store(true); // mark as finished
		});

		//allocateMeshData(LOD);

		prevPos[LOD] = playerChunk;
		firstIteration = false;

	}
	
}


void Chunk::generateSurroundingChunks(const glm::ivec2& playerChunk, const int LOD, glm::vec2& playerPos, Perlin& noise) {
	if (LOD == 0) {
		chunk_Coord_LOD0 = playerChunk;
		generateOffsets(playerChunk.x * CHUNK_SIZE, playerChunk.y * CHUNK_SIZE, LOD, returnTempChunk(LOD), playerPos, noise);
	}
	else {
		int startX = -LOD;
		int startZ = -LOD;

		int gridSize = 1 + (LOD * 2);

		std::unordered_set<glm::ivec2>& coordsVector = returnChunkCoords(LOD);

		coordsVector.clear();

		for (size_t i = 0; i < gridSize; i++) {
			for (size_t j = 0; j < gridSize; j++) {
				generateOffsets((playerChunk.x + startX) * CHUNK_SIZE, (playerChunk.y + startZ) * CHUNK_SIZE, LOD, returnTempChunk(LOD), playerPos, noise);
				//std::cout << "Data created for lod " << LOD << " Coord: " << playerChunk.x + startX << " " << playerChunk.y + startZ << std::endl;
				coordsVector.insert(glm::ivec2(playerChunk.x + startX, playerChunk.y + startZ));
				startZ++;
			}
			startZ = -LOD;
			startX++;
		}
	}
}

std::unordered_map<glm::ivec2, std::vector<glm::vec3>>& Chunk::returnTempChunk(const int LOD) {
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

std::unordered_set<glm::ivec2>& Chunk::returnPrevChunk(const int LOD) {
	switch (LOD) {
	case MEDIUM_DETAIL:
		return prev_chunk_Coords_LOD1;
	case LOW_DETAIL:
		return prev_chunk_Coords_LOD2;
	case LOWER_DETAIL:
		return prev_chunk_Coords_LOD3;
	case LOWEST_DETAIL:
		return prev_chunk_Coords_LOD4;
	default:
		break;
	}
}

std::unordered_map<glm::ivec2, std::unique_ptr<Mesh>>& Chunk::returnMeshChunks(const int LOD) {
	switch (LOD) {
	case HIGH_DETAIL:
		return chunks_Mesh_LOD0;
	case MEDIUM_DETAIL:
		return chunks_Mesh_LOD1;
	case LOW_DETAIL:
		return chunks_Mesh_LOD2;
	case LOWER_DETAIL:
		return chunks_Mesh_LOD3;
	case LOWEST_DETAIL:
		return chunks_Mesh_LOD4;
	default:
		break;
	}
}

int Chunk::returnLODScale(const int LOD) {
	switch (LOD) {
	case HIGH_DETAIL:
		return 1;
	case MEDIUM_DETAIL:
		return 2;
	case LOW_DETAIL:
		return 4;
	case LOWER_DETAIL:
		return 8;
	case LOWEST_DETAIL:
		return 16;
	default:
		break;
	}
}


void Chunk::drawChunks(Shader& shaderProgram, const int LOD) {

	shaderProgram.setUniform("lodScale", returnLODScale(LOD));

	if (LOD == 0) {
		if (chunks_Mesh_LOD0.find(chunk_Coord_LOD0) == chunks_Mesh_LOD0.end()) {
			//std::cout << "Data missing on LOD 0  Coord: " << chunk_Coord_LOD0.x << " " << chunk_Coord_LOD0.y << std::endl;
		}
		else {
			chunks_Mesh_LOD0[chunk_Coord_LOD0]->Draw(indices);
		}
		
	}
	else {
		for (const auto& coord : returnChunkCoords(LOD)) {
			if (returnMeshChunks(LOD).find(coord) == returnMeshChunks(LOD).end()) {
				//std::cout << "Data missing on LOD "<< LOD << "  Coord: " << coord.x << " " << coord.y << std::endl;
			}
			else {
				returnMeshChunks(LOD)[coord]->Draw(indices);
			}

		}
	}
}

void Chunk::deleteMeshData(const int LOD){

	if (LOD == 0) {
		chunks_Mesh_LOD0.erase(chunk_Coord_LOD0);
	}
	else {
		for (const auto& coord : returnChunkCoords(LOD)) {
			returnMeshChunks(LOD).erase(coord);
		}
	}
}

void Chunk::checkChunkDifference(const int LOD) {

	if (LOD == 0) {
		//std::cout << "lod 0 coord erased: " << prev_chunk_Coord_LOD0.x << " " << prev_chunk_Coord_LOD0.y << std::endl;
		chunks_Mesh_LOD0.erase(prev_chunk_Coord_LOD0);
	}
	else {
		for (const auto& coord : returnPrevChunk(LOD)) {
			if (returnChunkCoords(LOD).find(coord) == returnChunkCoords(LOD).end()) {
				//std::cout << "lod "  << LOD << " coord erased: " << coord.x << " " << coord.y << std::endl;
				returnMeshChunks(LOD).erase(coord);
			}
		}
	}
}

void Chunk::allocateMeshData(const int LOD) {
	//std::cout << "Mesh data allocating... on lod: " << LOD << std::endl;

	for (const auto& coord : returnTempChunk(LOD)) {
		if (returnMeshChunks(LOD).find(coord.first) == returnMeshChunks(LOD).end()) {
			returnMeshChunks(LOD)[coord.first] = std::make_unique<Mesh>(vertices, indices, pitchYaw, coord.second);
			//std::cout << "mesh data allocated for coord: " << coord.first.x << " " << coord.first.y << std::endl;
		}
		else {
			//std::cout << "mesh data already exists for coord: " << coord.first.x << " " << coord.first.y << std::endl;
		}
	}

	//std::cout << "Mesh data finished allocating... on lod: "<< LOD << std::endl;
}

void Chunk::clearTempChunk(const int LOD) {
	returnTempChunk(LOD).clear();
}

void Chunk::prevChunksAssign(const int LOD) {

	if (LOD == 0) {
		prev_chunk_Coord_LOD0 = chunk_Coord_LOD0;
	}
	else {
		returnPrevChunk(LOD) = returnChunkCoords(LOD);
	}

}




Chunk::~Chunk() {
	deleteMeshData(HIGH_DETAIL);
	deleteMeshData(MEDIUM_DETAIL);
	deleteMeshData(LOW_DETAIL);
	deleteMeshData(LOWER_DETAIL);
	deleteMeshData(LOWEST_DETAIL);
}
