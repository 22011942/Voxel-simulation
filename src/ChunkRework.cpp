//
// Created by michaelp on 29/10/25.
//

#include "ChunkRework.h"

ThreadPool chunkPool(4);

ChunkRework::ChunkRework() {
    prevPos.resize(LOD_COUNT, glm::ivec3(10, 10, 10));
    chunks_To_Draw.resize(LOD_COUNT);
    chunkQueues.resize(LOD_COUNT);
}

// void ChunkRework::startUp(const glm::vec3& playerPos, const int LOD) {
//     glm::ivec3 playerChunk = glm::floor(playerPos / static_cast<float>(CHUNK_SIZE));
//
//     std::vector<glm::ivec3> chunksToGenerate;
//
//     constexpr int step = RENDER_DISTANCE / 4;
//
//     const int halfGrid = ((LOD * step) / 2) + 4;
//     const int prevHalfGrid = (((LOD - 1) * step) / 2) + 4;
//
//     for (int x = -halfGrid; x <= halfGrid; ++x) {
//         for (int y = -halfGrid; y <= halfGrid; ++y) {
//             for (int z = -halfGrid; z <= halfGrid; ++z) {
//
//
//                 if (LOD > 0 &&
//                     x >= -prevHalfGrid && x <= prevHalfGrid &&
//                     y >= -prevHalfGrid && y <= prevHalfGrid &&
//                     z >= -prevHalfGrid && z <= prevHalfGrid) {
//
//                     continue; // skip full generation for inner chunks
//                     }
//
//                 glm::ivec3 chunkPos = playerChunk + glm::ivec3(x, y, z);
//
//                 //std::cout << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << std::endl;
//
//                 chunksToGenerate.push_back(chunkPos);
//             }
//         }
//     }
//
//
//     generateChunks(LOD, chunksToGenerate);
// }

void ChunkRework::startUp(const glm::vec3& playerPos, const int LOD) {
    glm::ivec3 playerChunk = glm::floor(playerPos / static_cast<float>(CHUNK_SIZE));

    constexpr int step = RENDER_DISTANCE / 4;
    const int halfGrid = ((LOD * step) / 2) + 4;

    std::vector<chunkPrio> chunk_Prio;

    for (int x = -halfGrid; x <= halfGrid; ++x) {
        for (int y = -halfGrid; y <= halfGrid; ++y) {
            for (int z = -halfGrid; z <= halfGrid; ++z) {

                glm::ivec3 chunkPos = playerChunk + glm::ivec3(x, y, z);
                const glm::ivec3 d = chunkPos - playerChunk;
                int priority = d.x*d.x + d.y*d.y + d.z*d.z;

                chunk_Prio.push_back({priority, chunkPos});
            }
        }
    }

    std::ranges::sort(chunk_Prio, [](const chunkPrio &a, const chunkPrio &b) {return a.priority < b.priority; });

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (auto const & c : chunk_Prio) {
            chunkQueues[LOD].push(c.chunkPos);
        }
    }

}

void ChunkRework::mainRoutine(const int LOD) {

    generateChunks(LOD);
}

//void ChunkRework::generateChunks(const glm::vec3& playerPos, const int LOD) {
   // glm::ivec3 playerChunk = glm::floor(playerPos / static_cast<float>(CHUNK_SIZE));

    //glm::ivec3 playerChunk = glm::ivec3(0,0,0);

   // if (prevPos[LOD] != playerChunk) {

        //if (!firstIteration)
        //    checkChunkDifference(LOD);


     //   generationDone[LOD] = false;
    //    LODReady[LOD] = false;

     //   chunks_To_Draw[LOD].clear();

     //   auto future = chunkPool.enqueue([=, this]() mutable {
     //       generateSurroundingChunks(playerChunk, LOD);
     //       generationDone[LOD].store(true); // mark as finished
    //    });

        //allocateMeshData(LOD);

     //   unloadDistantChunks(playerChunk, LOD);

     //   prevPos[LOD] = playerChunk;
        //firstIteration = false;
    //}
//}

void ChunkRework::generateChunks(const int LOD) {

    int generated = 0;

    while (generated < CHUNKS_PER_FRAME) {
        glm::ivec3 chunkPos;

        generationDone[LOD] = false;
        LODReady[LOD] = false;

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (chunkQueues[LOD].empty()) break;
            chunkPos = chunkQueues[LOD].front();
            chunkQueues[LOD].pop();
        }

        auto future = chunkPool.enqueue([=, this]() mutable {
            generateBlocks(chunkPos);
            allocateVertexData(LOD, chunkPos);
        });

        generated++;
    }

    generationDone[LOD].store(true);

    //if (!chunksToGenerate.empty()) {
        //generationDone[LOD] = false;
        //LODReady[LOD] = false;
        //auto future = chunkPool.enqueue([=, this]() mutable {
          //  for (auto& chunkPos : chunksToGenerate) {
        //        generateBlocks(chunkPos);
         //       allocateVertexData(LOD, chunkPos);
         //   }
        //    generationDone[LOD].store(true);
        //});
    //}

    //unloadDistantChunks(playerChunk, LOD);
    //prevPos[LOD] = playerChunk;
}



void ChunkRework::generateSurroundingChunks(const glm::ivec3& playerChunk, int LOD) {

    //if (LOD == 0) {
     //   generateBlocks(glm::ivec3(0,0,0));

    //    allocateVertexData(LOD, glm::ivec3(0,0,0));
    //}

    const int gridSize = START_RADIUS + (LOD * 6);
    const int prevGridSize = START_RADIUS + ((LOD - 1) * 6);

    const int halfGrid = gridSize / 2;
    const int prevHalfGrid = prevGridSize / 2;

    for (int x = -halfGrid; x <= halfGrid; ++x) {
        for (int y = -halfGrid; y <= halfGrid; ++y) {
            for (int z = -halfGrid; z <= halfGrid; ++z) {
                glm::ivec3 chunkPos = playerChunk + glm::ivec3(x, y, z);

                // Generate all chunks first
                generateBlocks(chunkPos);

                // Then skip inner chunks for higher LODs
                if (LOD > 0 &&
                    x >= -prevHalfGrid && x <= prevHalfGrid &&
                    y >= -prevHalfGrid && y <= prevHalfGrid &&
                    z >= -prevHalfGrid && z <= prevHalfGrid) {

                    if (!chunks[LOD].contains(chunkPos)) {
                        allocateVertexData(LOD, chunkPos);
                    }
                    continue; // skip full generation for inner chunks
                    }

                allocateVertexData(LOD, chunkPos);
            }
        }
    }

}

void ChunkRework::generateBlocks(glm::ivec3 chunkPos) {

    std::lock_guard<std::mutex> lock(blockMutex);
    //std::cout << "generateBlocks" << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << std::endl;

    if (globalBlocks.contains(chunkPos)) {
        return;
    }

    std::vector<std::vector<std::vector<blockType>>> block;

    block.resize(CHUNK_SIZE);
    for (auto& plane : block) {
        plane.resize(CHUNK_SIZE);
        for (auto& column : plane) {
            column.resize(CHUNK_SIZE, AIR);
        }
    }

    const int xDist = chunkPos.x * CHUNK_SIZE;
    const int zDist = chunkPos.z * CHUNK_SIZE;
    const int yDist = chunkPos.y * CHUNK_SIZE;

    for (int lx = 0; lx < CHUNK_SIZE; lx++) {
        int wx = xDist + lx;

        for (int lz = 0; lz < CHUNK_SIZE; lz++) {
            int wz = zDist + lz;

            int highest = static_cast<int>(noise.fractalBrownianMotion(wx * 0.05f, wz * 0.05f, OCTAVES));

            for (int ly = 0; ly < CHUNK_SIZE; ly++) {
                int wy = yDist + ly;

                //std::cout << "(" << lx << ", " << ly << ", " << lz << ")" << std::endl;

                if (wy == highest) {
                    block[lx][ly][lz] = GRASS;
                } else if (wy < highest) {
                    block[lx][ly][lz] = DIRT;
                }

                //std::cout << "block type: " << block[lx][ly][lz] << " at Local: " << lx << ", " << ly << ", " << lz << " World pos: " << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << std::endl;
            }
        }
    }

    //std::cout << "blocks generated" << std::endl;

    globalBlocks[chunkPos] = block;
}

void ChunkRework::allocateVertexData(const int LOD, glm::ivec3 chunkPos) {
    std::lock_guard<std::mutex> lock(blockMutex);
    std::unordered_map<glm::ivec3, Vertex_Data>& chunk = chunks[LOD];

    //std::cout<< chunkPos.x << "\t" << chunkPos.y << "\t" << chunkPos.z << std::endl;

    chunks_To_Draw[LOD].emplace_back(chunkPos);
    if (chunk.contains(chunkPos)) {
        return;
    }

    Vertex_Data vertexData;
    //std::cout << "allocating data to: " << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << " On LOD: " << LOD << std::endl;

    for (int x = 0; x < CHUNK_SIZE; x += returnLODScale(LOD)) {
        for (int y = 0; y < CHUNK_SIZE; y += returnLODScale(LOD)) {
            for (int z = 0; z < CHUNK_SIZE; z += returnLODScale(LOD)) {
                //std::cout << x << " " << y << " " << z << std::endl;

                if (!globalBlocks.contains(chunkPos)) {
                    std::cout << "Error global blocks chunk does not exist" << std::endl;
                    return;
                }

                if (globalBlocks.find(chunkPos)->second[x][y][z] != AIR) {

                    //std::cout << "Allocating data on lod: " << LOD << " At pos: " << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << std::endl;

                    glm::ivec3 worldPos = (chunkPos * CHUNK_SIZE) + glm::ivec3(x, y, z);

                    if (!isBlockTouching(worldPos, FRONT, LOD)) {
                        addFace(vertexData.vertices, vertexData.indices, x, y, z, FRONT);
                    }

                    if (!isBlockTouching(worldPos, BACK, LOD)) {
                        addFace(vertexData.vertices, vertexData.indices, x, y, z, BACK);
                    }

                    if (!isBlockTouching(worldPos, TOP, LOD)) {
                        addFace(vertexData.vertices, vertexData.indices, x, y, z, TOP);
                    }

                    if (!isBlockTouching(worldPos, BOTTOM, LOD)) {
                        addFace(vertexData.vertices, vertexData.indices, x, y, z, BOTTOM);
                    }

                    if (!isBlockTouching(worldPos, RIGHT_SIDE, LOD)) {
                        addFace(vertexData.vertices, vertexData.indices, x, y, z, RIGHT_SIDE);
                    }

                    if (!isBlockTouching(worldPos, LEFT_SIDE, LOD)) {
                        addFace(vertexData.vertices, vertexData.indices, x, y, z, LEFT_SIDE);
                    }

                    //addFace(vertexData.vertices, vertexData.indices, x, y, z, 0);
                }
            }
        }
    }


    chunk[chunkPos] = std::move(vertexData);
}

inline int floorDiv(int a, int b) {
    return (a >= 0) ? (a / b) : ((a - (b - 1)) / b);
}

bool ChunkRework::isBlockTouching(glm::ivec3 worldPos, int face, int LOD) {

    //std::cout << directions[face].x << " " << directions[face].y << " " << directions[face].z << std::endl;

    glm::ivec3 neighborBlock = worldPos + (directions[face] * returnLODScale(LOD));

    //std::cout << neighborBlock.x << ", " << neighborBlock.y << ", " << neighborBlock.z << std::endl;

    glm::ivec3 neighborChunk = {
        floorDiv(neighborBlock.x, CHUNK_SIZE),
        floorDiv(neighborBlock.y, CHUNK_SIZE),
        floorDiv(neighborBlock.z, CHUNK_SIZE)
    };


    glm::ivec3 local = neighborBlock - (neighborChunk * CHUNK_SIZE);

    if (globalBlocks.find(neighborChunk) == globalBlocks.end()) {
        return false;
    }

    //std::cout << local.x << ", " << local.y << ", " << local.z << std::endl;

    if (globalBlocks.find(neighborChunk)->second[local.x][local.y][local.z] != AIR) {
        return true;
    }

    return false;
}

void ChunkRework::addFace(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, int x, int y, int z, int face) {

    GLuint baseIndex = vertices.size();

    Vertex vertex;

    GLushort pitch = pitchYaw[face * 2];
    GLushort yaw   = pitchYaw[face * 2 + 1];

    for (size_t indx = 0; indx < 4; ++indx) {
        //vertices.push_back(vertexTemplate[face][indx] + glm::vec3(x, y, z));

        //std::cout << x << " " << y <<  " "  << z << std::endl;

        //vertex.pos = vertexTemplate[face][indx];
        vertex.x = vertexTemplate[face][indx].x;
        vertex.y = vertexTemplate[face][indx].y;
        vertex.z = vertexTemplate[face][indx].z;
        vertex.pitch = pitch;
        vertex.yaw   = yaw;
        vertex.localPos = glm::u8vec3(x, y, z);

        //pitch_yaw.push_back(pitch);
        //pitch_yaw.push_back(yaw);

        vertices.push_back(vertex);
    }

    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);

    //std::cout << "Added face " << face << " with pitch=" << pitch << " yaw=" << yaw << "\n";
}

// void ChunkRework::allocateMeshData(const int LOD, const glm::vec3& playerPos) {
//     std::lock_guard<std::mutex> lock(meshMutex);
//     glm::ivec3 playerChunk = glm::floor(playerPos / static_cast<float>(CHUNK_SIZE));
//     std::unordered_map<glm::ivec3, Vertex_Data>& chunk = chunks[LOD];
//
//     const int step = RENDER_DISTANCE / 4;
//
//     const int halfGrid = ((LOD * step) / 2) + 4;
//
//     for (int x = -halfGrid; x <= halfGrid; ++x) {
//         for (int y = -halfGrid; y <= halfGrid; ++y) {
//             for (int z = -halfGrid; z <= halfGrid; ++z) {
//                 glm::ivec3 chunkPos = playerChunk + glm::ivec3(x, y, z);
//
//                 //std::cout << " Positions: " << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << std::endl;
//                 if (!chunk.contains(chunkPos)) {
//                     std::cout << "Chunk does not exist" <<std::endl;
//                     continue;
//                 }
//
//                 Vertex_Data vertexDataInChunk = chunk.find(chunkPos)->second;
//
//                 //std::cout << "mesh " << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << std::endl;
//                 if (!vertexDataInChunk.vertices.empty() && !vertexDataInChunk.indices.empty()) {
//                     //std::cout << "Empty data sent into mesh from LOD: " << LOD << " At pos: " << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << std::endl;
//                     if (!loadedChunks[LOD].contains(chunkPos)) {
//                         //std::cout << chunkPos.x << " " << chunkPos.y << " " << chunkPos.z << std::endl;
//                         loadedChunks[LOD].emplace(chunkPos, std::make_unique<Mesh>(vertexDataInChunk));
//
//                     }
//                 }
//
//                 //loadedChunks[LOD][chunkPos] = std::make_unique<Mesh>(vertexDataInChunk);
//
//             }
//         }
//     }
// }

void ChunkRework::allocateMeshData(const int LOD, const glm::vec3& playerPos) {
    std::lock_guard<std::mutex> lock(meshMutex);

    for (auto & [chunkPos, vertexData] : chunks[LOD]) {

        if (vertexData.vertices.empty() && vertexData.indices.empty()) {
            continue;
        }

        if (loadedChunks[LOD].contains(chunkPos)) {
            continue;
        }

        loadedChunks[LOD].emplace(chunkPos, std::make_unique<Mesh>(vertexData));
    }
}


void ChunkRework::unloadDistantChunks(const glm::ivec3& playerPos, const int LOD) {
    const int range = START_RADIUS + (LOD * 6);

    const int xRangePos = playerPos.x + range;
    const int zRangePos = playerPos.z + range;
    const int yRangePos = playerPos.y + range;

    const int xRangeNeg = playerPos.x - range;
    const int zRangeNeg = playerPos.z - range;
    const int yRangeNeg = playerPos.y - range;

    std::vector<glm::ivec3> chunksToUnload;

    for (const auto & chunk: loadedChunks[LOD]) {
        if (
            chunk.first.x >= xRangePos || chunk.first.x <= xRangeNeg ||
            chunk.first.y >= yRangePos || chunk.first.y <= yRangeNeg ||
            chunk.first.z >= zRangePos || chunk.first.z <= zRangeNeg
        ) {
            //loadedChunks[LOD].erase(chunk.first);
            chunksToUnload.emplace_back(chunk.first);
        }
    }

    for (const glm::ivec3 & chunk: chunksToUnload) {
        //std::cout << "Chunk unloaded" << std::endl;
        loadedChunks[LOD].erase(chunk);
    }
}

bool once = false;

void ChunkRework::drawChunks(const Shader& shaderProgram, int LOD) {

    shaderProgram.setUniform("lodScale", returnLODScale(LOD));

    for (const auto & chunk : chunks_To_Draw[LOD]) {

        if (loadedChunks[LOD].contains(chunk)) {
            //std::cout << "chunk drawn" << std::endl;
            //std::cout << "Drawing chunk lod: " << LOD << " At pos: " << chunk.x << ", " << chunk.y << ", " << chunk.z << std::endl;
            //std::cout << " chunk INDICES size: " << chunks[LOD][chunk].indices.size() << " Chunk vertices size: " << chunks[LOD][chunk].vertices.size() << std::endl;

            //std::cout << "indices size: " << loadedChunks[LOD][chunk]->indices.size() << " vertices size: " << loadedChunks[LOD][chunk]->vertices.size() << std::endl;
            //loadedChunks[LOD][chunk]->Draw();
            glm::vec3 convertedChunk = chunk;
            shaderProgram.setUniform("chunkWorldPos", convertedChunk);
            loadedChunks[LOD].find(chunk)->second->Draw();
        }
    }

}

int ChunkRework::returnLODScale(const int LOD) {
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
            std::cout << "Unknown LOD Detected" << std::endl;
            return 0;
    }
}


const glm::ivec3 ChunkRework::vertexTemplate[6][4] = {
    //Front face
    // Front
    {{0,1,1}, {1,1,1}, {1,0,1}, {0,0,1}},
    // Back
    {{1,1,0}, {0,1,0}, {0,0,0}, {1,0,0}},
    // Top
    {{0,1,1}, {1,1,1}, {1,1,0}, {0,1,0}},
    // Bottom
    {{0,0,0}, {1,0,0}, {1,0,1}, {0,0,1}},
    // Right
    {{1,0,1}, {1,0,0}, {1,1,0}, {1,1,1}},
    // Left
    {{0,0,0}, {0,0,1}, {0,1,1}, {0,1,0}}
};

std::vector<GLushort> ChunkRework::pitchYaw = {
    encodeAngle(0.0f),       encodeAngle( PI / 2), // front  (+Z)
     encodeAngle(0.0f),       encodeAngle(-PI / 2), // back   (-Z)

     encodeAngle(-PI / 2),    encodeAngle(0.0f),    // top    (+Y)
     encodeAngle( PI / 2),    encodeAngle(0.0f),    // bottom (-Y)

     encodeAngle(0.0f),       encodeAngle(0.0f),    // right  (+X)
     encodeAngle(0.0f),       encodeAngle( PI),     // left   (-X)
};

GLushort ChunkRework::encodeAngle(const float radians) {
    return static_cast<GLushort>((radians + PI) / (2.0f * PI) * 65535.0f);
}

void ChunkRework::deleteMeshData() {
    loadedChunks.clear();
}

ChunkRework::~ChunkRework() {
    deleteMeshData();
}


