//
// Created by michaelp on 29/10/25.
//
#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <unordered_map>
#include <map>
#include "Perlin.h"
#include "mesh.h"
#include "Shader.h"
#include <glm/gtx/hash.hpp>
#include <numbers>

#include <imgui-master/imgui.h>
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include <unordered_set>
#include <ThreadPool.h>

constexpr int CHUNK_SIZE = 16;
constexpr int START_RADIUS = 7;
constexpr int OCTAVES = 8;
constexpr unsigned int LOD_COUNT = 5;
constexpr float PI = std::numbers::pi;
constexpr unsigned int RENDER_DISTANCE = 100;
constexpr unsigned int UNLOAD_RADIUS = 4;
constexpr unsigned int CHUNKS_PER_FRAME = 4;

enum blockType {
    AIR,
    DIRT,
    GRASS,
    STONE
};

struct chunkPrio {
    int priority;
    glm::ivec3 chunkPos;
};



struct IVec3Hash {
    std::size_t operator()(const glm::ivec3& v) const noexcept {
        // 64-bit scramble to avoid collisions
        uint64_t h = 0xcbf29ce484222325ULL;
        h = (h ^ (uint64_t)v.x) * 0x100000001b3ULL;
        h = (h ^ (uint64_t)v.y) * 0x100000001b3ULL;
        h = (h ^ (uint64_t)v.z) * 0x100000001b3ULL;
        return (std::size_t)h;
    }
};

struct IVec3Equal {
    bool operator()(const glm::ivec3& a, const glm::ivec3& b) const noexcept {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

enum gridDetail {
    HIGH_DETAIL,
    MEDIUM_DETAIL,
    LOW_DETAIL,
    LOWER_DETAIL,
    LOWEST_DETAIL
};

enum face {
    FRONT,
    BACK,
    TOP,
    BOTTOM,
    RIGHT_SIDE,
    LEFT_SIDE
};

inline constexpr glm::ivec3 directions[6] = {
    glm::ivec3(0, 0, -1), // front
    glm::ivec3(0, 0, 1),   // back
    glm::ivec3(0, 1, 0),  // up
    glm::ivec3(0, -1, 0), // down
    glm::ivec3(1, 0, 0),  // right
    glm::ivec3(-1, 0, 0) // left
};

class ChunkRework {
private:
    const Perlin noise;

    std::vector<std::queue<glm::ivec3>> chunkQueues;

    std::queue<glm::ivec3> chunkGenQueue;
    std::mutex queueMutex;

    std::mutex blockMutex;
    std::mutex meshMutex;
    std::mutex drawMutex;

    std::unordered_map<glm::ivec3, std::vector<std::vector<std::vector<blockType>>>, IVec3Hash, IVec3Equal> globalBlocks;

    std::unordered_map<int, std::unordered_map<glm::ivec3, Vertex_Data>> chunks;

    std::unordered_map<int, std::unordered_map<glm::ivec3, std::unique_ptr<Mesh>>> loadedChunks;

    std::vector<std::vector<glm::ivec3>> chunks_To_Draw;

    std::vector<glm::ivec3> prevPos;

	bool firstIteration = true;

    void generateSurroundingChunks(const glm::ivec3& playerChunk, int LOD);

    void generateBlocks(glm::ivec3 chunkPos);

    void addFace(std::vector<Vertex>& vertices, std::vector<GLuint>& indices,  int x, int y, int z, int face);

    static const glm::ivec3 vertexTemplate[6][4];

    void unloadDistantChunks(const glm::ivec3& playerPos, int LOD);

    void allocateVertexData(int LOD, glm::ivec3 playerPos);

    //std::unordered_map<glm::ivec3, Vertex_Data>& returnMeshChunks(const int LOD);

    static GLushort encodeAngle(float radians);

    static std::vector<GLushort> pitchYaw;

    static int returnLODScale(int LOD);

    bool isBlockTouching(glm::ivec3 worldPos, int face, int LOD);

    void deleteMeshData();

    void generateChunks(int LOD);
public:
    ChunkRework();

    void startUp(const glm::vec3& playerPos, int LOD);

    void mainRoutine(int LOD);

    void allocateMeshData(int LOD, const glm::vec3& playerPos);

    void drawChunks(const Shader& shaderProgram, int LOD);

    std::atomic<bool> generationDone[LOD_COUNT];
    std::atomic<bool> LODReady[LOD_COUNT];

    ~ChunkRework();
};

