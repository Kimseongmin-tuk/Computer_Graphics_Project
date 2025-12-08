#include "BlockManager.h"
#include <ctime>

GridPosition::GridPosition(const glm::vec3& worldPos) {
    x = static_cast<int>(std::round(worldPos.x / Constants::BLOCK_SIZE));
    y = static_cast<int>(std::round(worldPos.y / Constants::BLOCK_SIZE));
    z = static_cast<int>(std::round(worldPos.z / Constants::BLOCK_SIZE));
}

glm::vec3 GridPosition::toWorldPosition() const {
    return glm::vec3(
        x * Constants::BLOCK_SIZE,
        y * Constants::BLOCK_SIZE,
        z * Constants::BLOCK_SIZE
    );
}

BlockManager::BlockManager() : selectedBlockType(BlockType::CUBE) {
    generateRandomTerrain();
}

bool BlockManager::addBlock(const glm::vec3& worldPos, BlockType type, const glm::vec3& upDirection) {
    return addBlock(GridPosition(worldPos), type, upDirection);
}

bool BlockManager::addBlock(const GridPosition& gridPos, BlockType type, const glm::vec3& upDirection) {
    if (hasBlockAt(gridPos)) {
        return false;
    }

    glm::vec3 worldPos = gridPos.toWorldPosition();
    Block newBlock(worldPos, Constants::BLOCK_SIZE, type, upDirection);

    blocks.insert({ gridPos, newBlock });

    return true;
}

bool BlockManager::removeBlock(const glm::vec3& worldPos) {
    return removeBlock(GridPosition(worldPos));
}

bool BlockManager::removeBlock(const GridPosition& gridPos) {
    auto it = blocks.find(gridPos);
    if (it != blocks.end()) {
        blocks.erase(it);
        return true;
    }
    return false;
}

bool BlockManager::hasBlockAt(const glm::vec3& worldPos) const {
    return hasBlockAt(GridPosition(worldPos));
}

bool BlockManager::hasBlockAt(const GridPosition& gridPos) const {
    return blocks.find(gridPos) != blocks.end();
}

Block* BlockManager::getBlock(const glm::vec3& worldPos) {
    return getBlock(GridPosition(worldPos));
}

Block* BlockManager::getBlock(const GridPosition& gridPos) {
    auto it = blocks.find(gridPos);
    if (it != blocks.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool BlockManager::raycastBlock(const Ray& ray, RaycastHit& hit, Block** hitBlock) {
    float closestDistance = Constants::MAX_RAYCAST_DISTANCE;
    bool foundHit = false;
    RaycastHit closestHit;
    Block* closestBlock = nullptr;

    for (auto& pair : blocks) {
        Block& block = pair.second;
        AABB aabb = block.getAABB();

        RaycastHit tempHit;
        if (RayCast::rayAABBIntersection(ray, aabb, tempHit)) {
            if (tempHit.distance < closestDistance) {
                closestDistance = tempHit.distance;
                closestHit = tempHit;
                closestBlock = &block;
                foundHit = true;
            }
        }
    }

    if (foundHit) {
        hit = closestHit;
        if (hitBlock != nullptr) {
            *hitBlock = closestBlock;
        }
    }

    return foundHit;
}

static bool AABBIntersect(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
        (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
        (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool BlockManager::checkCollision(const AABB& playerAABB) const {
    for (const auto& pair : blocks) {
        const Block& block = pair.second;
        AABB blockAABB = block.getAABB();

        if (AABBIntersect(playerAABB, blockAABB)) {
            return true;
        }
    }
    return false;
}

void BlockManager::generateRandomTerrain() {
    srand(static_cast<unsigned>(time(nullptr)));

    int terrainSize = 10;
    int maxHeight = 3;

    for (int x = -terrainSize; x <= terrainSize; x++) {
        for (int z = -terrainSize; z <= terrainSize; z++) {
            float noise = (sin(x * 0.3f) + cos(z * 0.3f)) * 0.5f + 0.5f;
            noise += (rand() % 100) / 200.0f;
            noise = glm::clamp(noise, 0.0f, 1.0f);
            int height = static_cast<int>(noise * maxHeight) + 1;

            for (int y = -1; y < height; y++) {
                addBlock(GridPosition(x, y, z));
            }
        }
    }
}

void BlockManager::generateFlatTerrain() {
    // 5x5 평면 지형 (챌린지 모드용)
    int terrainSize = 2;

    for (int x = -terrainSize; x <= terrainSize; x++) {
        for (int z = -terrainSize; z <= terrainSize; z++) {
            // y=0에만 블록 생성 (평면)
            addBlock(GridPosition(x, 0, z));
        }
    }
}