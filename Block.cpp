#include "Block.h"

Block::Block(const glm::vec3& pos, float blockSize, BlockType blockType, const glm::vec3& up)
    : position(pos), size(blockSize), type(blockType),
    color(0.8f, 0.8f, 0.8f), isSpecial(false), upDirection(glm::normalize(up))
{
}

AABB Block::getAABB() const {
    glm::vec3 halfSize(size * 0.5f);
    return AABB(position - halfSize, position + halfSize);
}