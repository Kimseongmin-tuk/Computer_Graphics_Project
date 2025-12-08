#ifndef BLOCK_H
#define BLOCK_H

#include "Common.h"
#include "RayCast.h"

enum class BlockType {
    DIRT,
    BRICKS,
    COBBLESTONE,
    MUDBLOCK,
    QUARTZ
};

class Block {
private:
    glm::vec3 position;
    float size;
    BlockType type;
    glm::vec3 color;
    bool isSpecial;

public:
    Block(const glm::vec3& pos, float blockSize = Constants::BLOCK_SIZE,
        BlockType blockType = BlockType::DIRT);

    glm::vec3 getPosition() const { return position; }
    float getSize() const { return size; }
    BlockType getType() const { return type; }
    glm::vec3 getColor() const { return color; }
    bool getIsSpecial() const { return isSpecial; }

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setColor(const glm::vec3& col) { color = col; }
    void setSpecial(bool special) { isSpecial = special; }
    void setType(BlockType newType) { type = newType; }

    AABB getAABB() const;
    glm::vec3 getMin() const { return position - glm::vec3(size * 0.5f); }
    glm::vec3 getMax() const { return position + glm::vec3(size * 0.5f); }
};

#endif 