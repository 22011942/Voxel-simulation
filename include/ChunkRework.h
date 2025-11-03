//
// Created by michaelp on 29/10/25.
//
#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
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
constexpr int START_RADIUS = 5;
constexpr int OCTAVES = 8;
constexpr float PI = std::numbers::pi;

enum blockType {
    AIR,
    DIRT,
    GRASS,
    STONE
};

struct IVec3Hash {
    size_t operator()(const glm::ivec3& v) const noexcept {
        // Using a more robust hash combination
        return (static_cast<size_t>(v.x) * 73856093) ^
               (static_cast<size_t>(v.y) * 19349663) ^
               (static_cast<size_t>(v.z) * 83492791);
    }
};

struct IVec3Equal {
    bool operator()(const glm::ivec3& a, const glm::ivec3& b) const noexcept {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

struct vertexData {
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;
    std::vector<GLushort> pitchYaw;
};

enum gridDetail {
    HIGH_DETAIL,
    MEDIUM_DETAIL,
    LOW_DETAIL,
    LOWER_DETAIL,
    LOWEST_DETAIL
};

class ChunkRework {
private:
    const Perlin noise;

    std::unordered_map<glm::ivec3, std::vector<std::vector<std::vector<blockType>>>, IVec3Hash, IVec3Equal> globalBlocks;

    std::unordered_map<glm::ivec3, vertexData> CHUNK_LOD_0;
    std::unordered_map<glm::ivec3, vertexData> CHUNK_LOD_1;
    std::unordered_map<glm::ivec3, vertexData> CHUNK_LOD_2;
    std::unordered_map<glm::ivec3, vertexData> CHUNK_LOD_3;
    std::unordered_map<glm::ivec3, vertexData> CHUNK_LOD_4;

    std::vector<glm::vec3> chunkPositions_LOD_0;
    std::vector<glm::vec3> chunkPositions_LOD_1;
    std::vector<glm::vec3> chunkPositions_LOD_2;
    std::vector<glm::vec3> chunkPositions_LOD_3;
    std::vector<glm::vec3> chunkPositions_LOD_4;

    std::vector<glm::ivec3> prevPos;

	bool firstIteration = true;

    std::atomic<bool> generationDone[8];

    void generateSurroundingChunks(const glm::ivec3& playerChunk, int LOD) const;

    void generateBlocks(glm::ivec3 chunkPos) const;

    void addFace(std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices,  int x, int y, int z, int face);

    static const glm::vec3 vertexTemplate[6][4];

    void unloadDistantChunks(const glm::vec3& playerPos, const int LOD);

    void allocateMeshData(const int LOD, glm::vec3 playerPos);

    std::unordered_map<glm::ivec3, vertexData>& returnMeshChunks(const int LOD);

    static GLushort encodeAngle(float radians);

    static std::vector<GLushort> pitchYaw;
public:
    ChunkRework();

    void generateChunks(const glm::vec3& playerPos, int LOD);

    ~ChunkRework();
};

