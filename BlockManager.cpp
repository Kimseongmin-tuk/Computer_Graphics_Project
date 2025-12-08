#include "BlockManager.h"
#include <cmath>

// Perlin Noise 헬퍼 함수들
namespace {
    // 선형 보간
    float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    // 부드러운 보간 (smoothstep)
    float smoothstep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }

    // 간단한 해시 함수 (의사 랜덤)
    float hash(int x, int z, int seed = 12345) {
        int n = x + z * 57 + seed * 131;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    }

    // 2D Perlin-like Noise
    float noise2D(float x, float z, int seed = 12345) {
        int xi = (int)floor(x);
        int zi = (int)floor(z);
        
        float xf = x - xi;
        float zf = z - zi;
        
        // 4개의 코너 값
        float n00 = hash(xi, zi, seed);
        float n10 = hash(xi + 1, zi, seed);
        float n01 = hash(xi, zi + 1, seed);
        float n11 = hash(xi + 1, zi + 1, seed);
        
        // 부드러운 보간
        float sx = smoothstep(xf);
        float sz = smoothstep(zf);
        
        float nx0 = lerp(n00, n10, sx);
        float nx1 = lerp(n01, n11, sx);
        
        return lerp(nx0, nx1, sz);
    }

    // 다중 옥타브 노이즈 (자연스러운 지형)
    float fractalNoise(float x, float z, int octaves = 4, float persistence = 0.5f) {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;
        
        for (int i = 0; i < octaves; i++) {
            total += noise2D(x * frequency, z * frequency, i) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2.0f;
        }
        
        return total / maxValue;
    }
}

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
    // 지형 생성 파라미터
    const int terrainSize = 20;  // 20x20 지형
    const float noiseScale = 0.1f;  // 노이즈 스케일 (작을수록 완만함)
    const int maxHeight = 5;  // 최대 높이
    const int minHeight = 0;  // 최소 높이
    
    // Perlin Noise 기반 지형 생성
    for (int x = -terrainSize; x <= terrainSize; x++) {
        for (int z = -terrainSize; z <= terrainSize; z++) {
            // Perlin Noise로 높이 계산
            float noiseValue = fractalNoise(x * noiseScale, z * noiseScale, 4, 0.5f);
            
            // 0~1 범위를 minHeight~maxHeight로 매핑
            int height = minHeight + (int)((noiseValue * 0.5f + 0.5f) * (maxHeight - minHeight));
            
            // 높이에 따라 블록 타입 결정
            BlockType blockType = BlockType::DIRT;
            if (height <= 1) {
                blockType = BlockType::COBBLESTONE;  // 낮은 지대
            } else if (height <= 3) {
                blockType = BlockType::DIRT;  // 중간 지대
            } else {
                blockType = BlockType::MUDBLOCK;  // 높은 지대
            }
            
            // 해당 높이에 블록 배치
            addBlock(GridPosition(x, height, z), blockType);
            
            // 아래층 채우기 (더 견고한 지형)
            for (int y = minHeight; y < height; y++) {
                addBlock(GridPosition(x, y, z), BlockType::COBBLESTONE);
            }
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