//
// Created by michaelp on 29/10/25.
//

#include "ChunkRework.h"

ThreadPool chunkPool(5);

ChunkRework::ChunkRework() {
    prevPos.resize(8, glm::ivec3(10, 10, 10));
}

void ChunkRework::generateChunks(const glm::vec3& playerPos, const int LOD) {
    glm::ivec3 playerChunk = glm::floor(playerPos / static_cast<float>(CHUNK_SIZE));

    if (prevPos[LOD] != playerChunk) {

        //if (!firstIteration)
        //    checkChunkDifference(LOD);


        generationDone[LOD] = false;

        auto future = chunkPool.enqueue([=, this]() mutable {
            generateSurroundingChunks(playerChunk, LOD);

            generationDone[LOD].store(true); // mark as finished
        });

        //allocateMeshData(LOD);

        prevPos[LOD] = playerChunk;
        firstIteration = false;
    }
}

void ChunkRework::generateSurroundingChunks(const glm::ivec3& playerChunk, int LOD) const {
    const int gridSize = START_RADIUS + (LOD * 6);
    const int prevGridSize = START_RADIUS + ((LOD - 1) * 6);

    const int halfGrid = gridSize / 2;
    const int prevHalfGrid = prevGridSize / 2;

    for (int x = -halfGrid; x <= halfGrid; ++x) {
        for (int y = -halfGrid; y <= halfGrid; ++y) {
            for (int z = -halfGrid; z <= halfGrid; ++z) {
                glm::ivec3 chunkPos = playerChunk + glm::ivec3(x, y, z);

                if (LOD > 0) {
                    if (x >= -prevHalfGrid && x <= prevHalfGrid &&
                        y >= -prevHalfGrid && y <= prevHalfGrid &&
                        z >= -prevHalfGrid && z <= prevHalfGrid) {
                        continue;
                        }
                }

                generateBlocks(chunkPos);
            }
        }
    }
}

void ChunkRework::generateBlocks(glm::ivec3 chunkPos) const {
    if (!globalBlocks.contains(chunkPos)) {
        return;
    }

    std::vector<std::vector<std::vector<blockType>>> block;

    block.resize(CHUNK_SIZE);
    for (auto& plane : block) {
        plane.resize(CHUNK_SIZE);
        for (auto& block : plane) {
            block.resize(CHUNK_SIZE, AIR);
        }
    }

    const int xDist = chunkPos.x * CHUNK_SIZE;
    const int zDist = chunkPos.y * CHUNK_SIZE;
    const int yDist = chunkPos.z * CHUNK_SIZE;

    for (size_t x = xDist; x < xDist + CHUNK_SIZE; x++) {
        for (size_t z = zDist; z < zDist + CHUNK_SIZE; z++) {
            int highestBlock = noise.fractalBrownianMotion(x * 0.05f, z * 0.05f, OCTAVES);

            for (size_t y = yDist; y < yDist + CHUNK_SIZE; y++) {

                if (y <= highestBlock) {
                    block[x][y][z] = GRASS;
                }

            }
        }
    }

}

void ChunkRework::allocateMeshData(const int LOD, glm::vec3 playerPos) {
    std::unordered_map<glm::ivec3, vertexData> chunk = returnMeshChunks(LOD);


}

std::unordered_map<glm::ivec3, vertexData>& ChunkRework::returnMeshChunks(const int LOD) {
    switch (LOD) {
        case HIGH_DETAIL:
            return CHUNK_LOD_0;
        case MEDIUM_DETAIL:
            return CHUNK_LOD_1;
        case LOW_DETAIL:
            return CHUNK_LOD_2;
        case LOWER_DETAIL:
            return CHUNK_LOD_3;
        case LOWEST_DETAIL:
            return CHUNK_LOD_4;
    }

    //Should not happen
    std::cout << "Unrecognized LOD " << LOD << std::endl;
    return CHUNK_LOD_0;
}


void ChunkRework::addFace(std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices,  int x, int y, int z, int face) {

    GLuint baseIndex = vertices.size();
    for (size_t indx = 0; indx < 4; ++indx) {
        vertices.push_back(vertexTemplate[face][indx] + glm::vec3(x, y, z));
    }

    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
}

void ChunkRework::unloadDistantChunks(const glm::vec3& playerPos, const int LOD) {

}

const glm::vec3 ChunkRework::vertexTemplate[6][4] = {
    //Front face
    {{-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}},
    //Back face
    {{0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, {0.5f,  0.5f, -0.5f}},
    //Top face
    {{-0.5f,  0.5f,  0.5f}, {0.5f,  0.5f,  0.5f}, {0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}},
    //Bottom face
    {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f}},
    //Right face
    {{0.5f, -0.5f,  0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f,  0.5f, -0.5f}, {0.5f,  0.5f,  0.5f}},
    //Left face
    {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f}}
};

std::vector<GLushort> ChunkRework::pitchYaw = {
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

GLushort ChunkRework::encodeAngle(const float radians) {
    return static_cast<GLushort>((radians + PI) / (2.0f * PI) * 65535.0f);
}

