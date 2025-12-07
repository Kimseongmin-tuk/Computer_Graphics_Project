#include "Player.h"

Player::Player(const glm::vec3& pos)
    : position(pos), yaw(0.0f), pitch(0.0f),
    walkCycle(0.0f), armSwing(0.0f), legSwing(0.0f), isWalking(false)
{

    float scale = 0.0625f; // 1픽셀 = 0.0625 유닛

    size.head = glm::vec3(8.0f, 8.0f, 8.0f) * scale;      // 0.5 x 0.5 x 0.5
    size.body = glm::vec3(8.0f, 12.0f, 4.0f) * scale;     // 0.5 x 0.75 x 0.25
    size.arm = glm::vec3(4.0f, 12.0f, 4.0f) * scale;      // 0.25 x 0.75 x 0.25
    size.leg = glm::vec3(4.0f, 12.0f, 4.0f) * scale;      // 0.25 x 0.75 x 0.25

    // 색상 설정
    skinColor = glm::vec3(0.96f, 0.8f, 0.69f);   // 살색
    shirtColor = glm::vec3(0.2f, 0.6f, 0.9f);    // 파란 셔츠
    pantsColor = glm::vec3(0.3f, 0.3f, 0.5f);    // 청바지
}

AABB Player::getAABB() const {
    // 캐릭터의 정확한 충돌 박스 - 마진 없이 완전히 정확하게

    // 1. 각 부위의 실제 Y 위치 계산
    // 다리: position.y를 중심으로 위아래로 size.leg.y/2 만큼
    float legBottom = position.y - size.leg.y * 0.5f;  // 발 끝
    float legTop = position.y + size.leg.y * 0.5f;     // 골반

    // 몸통: 다리 위에서 시작
    float bodyBottom = legTop;
    float bodyTop = bodyBottom + size.body.y;

    // 머리: 몸통 위에서 시작
    float headBottom = bodyTop;
    float headTop = headBottom + size.head.y;

    // 2. 충돌 박스 Y 범위 (마진 없음)
    float minY = legBottom;   // 발 끝
    float maxY = headTop;     // 머리 끝

    // 3. 너비와 깊이 (마진 없음)
    float width = std::max(size.head.x, size.body.x);  // 0.5
    float depth = width;  // 회전 고려하여 정사각형

    // 4. AABB 생성 (완전히 정확하게)
    glm::vec3 aabbMin(
        position.x - width * 0.5f,
        minY,
        position.z - depth * 0.5f
    );

    glm::vec3 aabbMax(
        position.x + width * 0.5f,
        maxY,
        position.z + depth * 0.5f
    );

    return AABB(aabbMin, aabbMax);
}

float Player::getTotalHeight() const {
    // 머리 + 몸통 + 다리
    return size.head.y + size.body.y + size.leg.y;
}

void Player::updateAnimation(float deltaTime) {
    if (isWalking) {
        walkCycle += deltaTime * 8.0f; // 걷기 속도

        // 팔 흔들기 (-30도 ~ 30도)
        armSwing = glm::sin(walkCycle) * 30.0f;

        // 다리 흔들기 (-45도 ~ 45도)
        legSwing = glm::sin(walkCycle) * 45.0f;
    }
    else {
        // 정지 시 부드럽게 원위치
        armSwing *= 0.9f;
        legSwing *= 0.9f;

        if (std::abs(armSwing) < 0.1f) armSwing = 0.0f;
        if (std::abs(legSwing) < 0.1f) legSwing = 0.0f;
    }
}

glm::mat4 Player::getHeadTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);

    // 플레이어 위치로 이동
    transform = glm::translate(transform, position);

    // 플레이어 회전
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 머리 위치
    float headY = size.body.y + size.head.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(0.0f, headY, 0.0f));


    transform = glm::scale(transform, size.head);

    return transform;
}

glm::mat4 Player::getBodyTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::translate(transform, position);

    // 플레이어 회전
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 몸통 위치
    float bodyY = size.body.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(0.0f, bodyY, 0.0f));

    transform = glm::scale(transform, size.body);

    return transform;
}

glm::mat4 Player::getLeftArmTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::translate(transform, position);

    // 플레이어 회전 
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 왼팔 어깨 위치
    float armX = -(size.body.x * 0.5f + size.arm.x * 0.5f);
    float armY = size.body.y - size.arm.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(armX, armY, 0.0f));

    // 어깨 관절을 중심으로 회전
    transform = glm::translate(transform, glm::vec3(0.0f, size.arm.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(-armSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.arm.y * 0.5f, 0.0f));

    transform = glm::scale(transform, size.arm);

    return transform;
}

glm::mat4 Player::getRightArmTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::translate(transform, position);

    // 플레이어 회전
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 오른팔 어깨 위치
    float armX = size.body.x * 0.5f + size.arm.x * 0.5f;
    float armY = size.body.y - size.arm.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(armX, armY, 0.0f));

    // 어깨 관절을 중심으로 회전
    transform = glm::translate(transform, glm::vec3(0.0f, size.arm.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(armSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.arm.y * 0.5f, 0.0f));

    transform = glm::scale(transform, size.arm);

    return transform;
}

glm::mat4 Player::getLeftLegTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::translate(transform, position);

    // 플레이어 회전 
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 왼다리 위치
    float legX = -size.body.x * 0.25f;
    float legY = -size.leg.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(legX, legY, 0.0f));

    // 골반 관절을 중심으로 회전
    transform = glm::translate(transform, glm::vec3(0.0f, size.leg.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.leg.y * 0.5f, 0.0f));

    transform = glm::scale(transform, size.leg);

    return transform;
}

glm::mat4 Player::getRightLegTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::translate(transform, position);

    // 플레이어 회전
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 오른다리 위치 
    float legX = size.body.x * 0.25f;
    float legY = -size.leg.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(legX, legY, 0.0f));

    // 골반 관절을 중심으로 회전 (왼다리와 반대)
    transform = glm::translate(transform, glm::vec3(0.0f, size.leg.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(-legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.leg.y * 0.5f, 0.0f));

    transform = glm::scale(transform, size.leg);

    return transform;
}