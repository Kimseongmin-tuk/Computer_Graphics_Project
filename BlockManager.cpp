#include "BlockManager.h"

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

BlockManager::BlockManager() : selectedBlockType(BlockType::DIRT) {
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            addBlock(GridPosition(x, 0, z));
        }
    }
}

bool BlockManager::addBlock(const glm::vec3& worldPos, BlockType type) {
    return addBlock(GridPosition(worldPos), type);
}

bool BlockManager::addBlock(const GridPosition& gridPos, BlockType type) {
    if (hasBlockAt(gridPos)) {
        return false;
    }

    glm::vec3 worldPos = gridPos.toWorldPosition();
    Block newBlock(worldPos, Constants::BLOCK_SIZE, type);

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

// AABB 충돌 검사 헬퍼 함수
static bool AABBIntersect(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
        (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
        (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

// 충돌 감지 함수 구현
bool BlockManager::checkCollision(const AABB& playerAABB) const {
    for (const auto& pair : blocks) {
        const Block& block = pair.second;
        AABB blockAABB = block.getAABB();

        if (AABBIntersect(playerAABB, blockAABB)) {
            return true;  // 충돌 발생
        }
    }
    return false;  // 충돌 없음
}