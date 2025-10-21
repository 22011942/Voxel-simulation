#pragma once
#define GLM_ENABLE_EXPERIMENTAL

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


const int CHUNK_SIZE = 500;
const int OCTAVES = 8;
const double PI = std::numbers::pi;
const bool SINK_CULLING = false;

struct IVec2Hash {
	size_t operator()(const glm::ivec2& v) const {
		return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
	}
};

struct IVec2Equal {
	bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
		return a.x == b.x && a.y == b.y;
	}
};

namespace std {
	template <>
	struct hash<glm::ivec2> {
		std::size_t operator()(const glm::ivec2& k) const {
			return std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1);
		}
	};
}

enum gridDetail {
	HIGH_DETAIL,
	MEDIUM_DETAIL,
	LOW_DETAIL,
	LOWER_DETAIL,
	LOWEST_DETAIL
};

class Chunk {
private:
	std::unordered_map<glm::ivec2, std::unique_ptr<Mesh>> chunks_Mesh_LOD0;
	std::unordered_map<glm::ivec2, std::unique_ptr<Mesh>> chunks_Mesh_LOD1;
	std::unordered_map<glm::ivec2, std::unique_ptr<Mesh>> chunks_Mesh_LOD2;
	std::unordered_map<glm::ivec2, std::unique_ptr<Mesh>> chunks_Mesh_LOD3;
	std::unordered_map<glm::ivec2, std::unique_ptr<Mesh>> chunks_Mesh_LOD4;

	std::unordered_map<glm::ivec2, std::vector<glm::vec3>> temp_chunks_Mesh_LOD0;
	std::unordered_map<glm::ivec2, std::vector<glm::vec3>> temp_chunks_Mesh_LOD1;
	std::unordered_map<glm::ivec2, std::vector<glm::vec3>> temp_chunks_Mesh_LOD2;
	std::unordered_map<glm::ivec2, std::vector<glm::vec3>> temp_chunks_Mesh_LOD3;
	std::unordered_map<glm::ivec2, std::vector<glm::vec3>> temp_chunks_Mesh_LOD4;

	glm::ivec2 chunk_Coord_LOD0;
	std::unordered_set<glm::ivec2> chunk_Coords_LOD1;
	std::unordered_set<glm::ivec2> chunk_Coords_LOD2;
	std::unordered_set<glm::ivec2> chunk_Coords_LOD3;
	std::unordered_set<glm::ivec2> chunk_Coords_LOD4;

	glm::ivec2 prev_chunk_Coord_LOD0;
	std::unordered_set<glm::ivec2> prev_chunk_Coords_LOD1;
	std::unordered_set<glm::ivec2> prev_chunk_Coords_LOD2;
	std::unordered_set<glm::ivec2> prev_chunk_Coords_LOD3;
	std::unordered_set<glm::ivec2> prev_chunk_Coords_LOD4;


	static std::vector<GLfloat> vertices;
	static std::vector<GLuint> indices;
	static GLushort encodeAngle(float radians);
	static std::vector<GLushort> pitchYaw;

	void generateOffsets(const int xLimit, const int zLimit, const int LOD, std::unordered_map<glm::ivec2, std::vector<glm::vec3>>& chunks, glm::vec2& playerPos, Perlin& noise);

	void generateSurroundingChunks(const glm::ivec2& playerChunk, int LOD, glm::vec2& playerPos, Perlin& noise);

	std::unordered_map<glm::ivec2, std::vector<glm::vec3>>& returnTempChunk(const int LOD);

	glm::ivec2 prevPos[5] = {
		glm::ivec2(10, 10),
		glm::ivec2(10, 10),
		glm::ivec2(10, 10),
		glm::ivec2(10, 10),
		glm::ivec2(10, 10)
	};

	void checkChunkDifference(const int LOD);

	bool firstIteration = true;

	void deleteMeshData(const int LOD);

	std::unordered_set<glm::ivec2>& returnPrevChunk(const int LOD);

	std::unordered_map<glm::ivec2, std::unique_ptr<Mesh>>& returnMeshChunks(const int LOD);

	int returnLODScale(const int LOD);
public:
	Chunk();

	std::atomic<bool> generationDone[5];

	void allocateMeshData(const int LOD);

	void clearTempChunk(const int LOD);

	void prevChunksAssign(const int LOD);

	void generateChunks(const glm::vec3& playerPos, Perlin& noise, const int LOD, std::vector<bool>& LODReady);

	std::unordered_set<glm::ivec2>& returnChunkCoords(const int LOD);

	void drawChunks(Shader& shaderProgram, const int LOD);

	~Chunk();
};