#include "Player.h"

Player::Player(const glm::vec3& pos)
    : position(pos), yaw(0.0f), pitch(0.0f),
    walkCycle(0.0f), armSwing(0.0f), legSwing(0.0f), isWalking(false),
    isFlyingMode(true), velocity(0.0f), isOnGround(false), jumpVelocity(8.0f),
    spaceKeyPressCount(0), lastSpaceKeyTime(0.0f)
{
    float scale = 0.0625f;

    size.head = glm::vec3(8.0f, 8.0f, 8.0f) * scale;
    size.body = glm::vec3(8.0f, 12.0f, 4.0f) * scale;
    size.arm = glm::vec3(4.0f, 12.0f, 4.0f) * scale;
    size.leg = glm::vec3(4.0f, 12.0f, 4.0f) * scale;

    skinColor = glm::vec3(0.96f, 0.8f, 0.69f);
    shirtColor = glm::vec3(0.2f, 0.6f, 0.9f);
    pantsColor = glm::vec3(0.3f, 0.3f, 0.5f);
}

AABB Player::getAABB() const {
    float margin = 0.02f;

    float legBottom = position.y - size.leg.y * 0.5f;
    float legTop = position.y + size.leg.y * 0.5f;
    float bodyBottom = legTop;
    float bodyTop = bodyBottom + size.body.y;
    float headBottom = bodyTop;
    float headTop = headBottom + size.head.y;

    float minY = legBottom + margin;
    float maxY = headTop - margin;

    float width = std::max(size.head.x, size.body.x);
    float depth = width;

    glm::vec3 aabbMin(
        position.x - width * 0.5f + margin,
        minY,
        position.z - depth * 0.5f + margin
    );

    glm::vec3 aabbMax(
        position.x + width * 0.5f - margin,
        maxY,
        position.z + depth * 0.5f - margin
    );

    return AABB(aabbMin, aabbMax);
}

float Player::getTotalHeight() const {
    return size.head.y + size.body.y + size.leg.y;
}

void Player::updateAnimation(float deltaTime) {
    if (isWalking) {
        walkCycle += deltaTime * 8.0f;
        armSwing = glm::sin(walkCycle) * 30.0f;
        legSwing = glm::sin(walkCycle) * 45.0f;
    }
    else {
        armSwing *= 0.9f;
        legSwing *= 0.9f;
        if (std::abs(armSwing) < 0.1f) armSwing = 0.0f;
        if (std::abs(legSwing) < 0.1f) legSwing = 0.0f;
    }
}

glm::mat4 Player::getHeadTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float headY = size.body.y + size.head.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(0.0f, headY, 0.0f));
    transform = glm::scale(transform, size.head);
    return transform;
}

glm::mat4 Player::getBodyTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float bodyY = size.body.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(0.0f, bodyY, 0.0f));
    transform = glm::scale(transform, size.body);
    return transform;
}

glm::mat4 Player::getLeftArmTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float armX = -(size.body.x * 0.5f + size.arm.x * 0.5f);
    float armY = size.body.y - size.arm.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(armX, armY, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, size.arm.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(-armSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.arm.y * 0.5f, 0.0f));
    transform = glm::scale(transform, size.arm);
    return transform;
}

glm::mat4 Player::getRightArmTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float armX = size.body.x * 0.5f + size.arm.x * 0.5f;
    float armY = size.body.y - size.arm.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(armX, armY, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, size.arm.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(armSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.arm.y * 0.5f, 0.0f));
    transform = glm::scale(transform, size.arm);
    return transform;
}

glm::mat4 Player::getLeftLegTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float legX = -size.body.x * 0.25f;
    float legY = -size.leg.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(legX, legY, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, size.leg.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.leg.y * 0.5f, 0.0f));
    transform = glm::scale(transform, size.leg);
    return transform;
}

glm::mat4 Player::getRightLegTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float legX = size.body.x * 0.25f;
    float legY = -size.leg.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(legX, legY, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, size.leg.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(-legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.leg.y * 0.5f, 0.0f));
    transform = glm::scale(transform, size.leg);
    return transform;
}

void Player::handleSpaceKeyPress(float currentTime) {
    float doubleClickThreshold = 0.25f;  // 0.25초 이내 더블탭

    if (currentTime - lastSpaceKeyTime < doubleClickThreshold) {
        // 더블탭 감지
        spaceKeyPressCount++;
        if (spaceKeyPressCount >= 1) {
            isFlyingMode = !isFlyingMode;
            velocity.y = 0.0f;
            spaceKeyPressCount = 0;
        }
    }
    else {
        // 단일 탭
        spaceKeyPressCount = 0;
        if (!isFlyingMode && isOnGround) {
            jump();  // 걷기 모드에서만 점프
        }
    }

    lastSpaceKeyTime = currentTime;
}

void Player::jump() {
    if (isOnGround && !isFlyingMode) {
        velocity.y = jumpVelocity;
        isOnGround = false;
    }
}
