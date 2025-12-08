#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include "Common.h"
#include "Block.h"
#include "RayCast.h"

struct GridPosition {
    int x, y, z;

    GridPosition(int _x = 0, int _y = 0, int _z = 0) : x(_x), y(_y), z(_z) {}
    GridPosition(const glm::vec3& worldPos);

    bool operator==(const GridPosition& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    glm::vec3 toWorldPosition() const;
};

namespace std {
    template <>
    struct hash<GridPosition> {
        size_t operator()(const GridPosition& pos) const {
            return hash<int>()(pos.x) ^ (hash<int>()(pos.y) << 1) ^ (hash<int>()(pos.z) << 2);
        }
    };
}

class BlockManager {
private:
    std::unordered_map<GridPosition, Block> blocks;
    BlockType selectedBlockType;

public:
    BlockManager();

    bool addBlock(const glm::vec3& worldPos, BlockType type = BlockType::CUBE, const glm::vec3& upDirection = glm::vec3(0.0f, 1.0f, 0.0f));
    bool addBlock(const GridPosition& gridPos, BlockType type = BlockType::CUBE, const glm::vec3& upDirection = glm::vec3(0.0f, 1.0f, 0.0f));

    bool removeBlock(const glm::vec3& worldPos);
    bool removeBlock(const GridPosition& gridPos);

    bool hasBlockAt(const glm::vec3& worldPos) const;
    bool hasBlockAt(const GridPosition& gridPos) const;

    Block* getBlock(const glm::vec3& worldPos);
    Block* getBlock(const GridPosition& gridPos);

    bool raycastBlock(const Ray& ray, RaycastHit& hit, Block** hitBlock = nullptr);

    bool checkCollision(const AABB& playerAABB) const;

    int getBlockCount() const { return blocks.size(); }
    void clearAll() { blocks.clear(); }

    void generateRandomTerrain();
    void generateFlatTerrain();

    const std::unordered_map<GridPosition, Block>& getAllBlocks() const { return blocks; }
};

#endif // BLOCKMANAGER_H