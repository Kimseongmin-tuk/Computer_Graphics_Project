#include "Player.h"

Player::Player(const glm::vec3& pos)
    : position(pos), yaw(0.0f), pitch(0.0f),
    walkCycle(0.0f), armSwing(0.0f), legSwing(0.0f), isWalking(false),
    isFlying(true), lastSpaceTapTime(-1.0f),  // ?? 비행 모드로 시작
    isJumping(false), isOnGround(false), verticalVelocity(0.0f)
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

void Player::handleSpaceTap(float currentTime) {
    // 더블탭 감지
    if (lastSpaceTapTime > 0 && (currentTime - lastSpaceTapTime) <= doubleTapDelay) {
        // 더블탭 성공! 비행 모드 토글
        toggleFlying();
        if (isFlying) {
            std::cout << "비행 모드 활성화!" << std::endl;
            // 비행 모드로 전환 시 점프 상태 초기화
            isJumping = false;
            verticalVelocity = 0.0f;
        }
        else {
            std::cout << "걷기 모드로 전환!" << std::endl;
        }
        lastSpaceTapTime = -1.0f;  // 리셋
    }
    else {
        // 첫 번째 탭 기록
        lastSpaceTapTime = currentTime;
    }
}

void Player::jump() {
    // 땅에 있고, 비행 모드가 아닐 때만 점프 가능
    if (isOnGround && !isFlying) {
        verticalVelocity = jumpForce;
        isJumping = true;
        isOnGround = false;
        std::cout << "점프!" << std::endl;
    }
}

void Player::applyGravity(float deltaTime) {
    // 비행 모드가 아닐 때만 중력 적용
    if (!isFlying) {
        verticalVelocity -= gravity * deltaTime;

        // 최대 낙하 속도 제한
        if (verticalVelocity < -20.0f) {
            verticalVelocity = -20.0f;
        }
    }
}

AABB Player::getAABB() const {
    // 작은 여유 공간을 두어 충돌 과민 반응을 방지
    float margin = 0.02f;   // 2cm 정도의 마진

    // 1. 각 부위 Y 위치 계산
    // position.y는 다리의 중심이므로, 실제 발 끝은 한 다리 길이만큼 더 아래
    float legBottom = position.y - size.leg.y;  // 실제 발 끝 위치
    float legTop = position.y;  // 다리 상단 = 몸통 시작점

    float bodyBottom = legTop;
    float bodyTop = bodyBottom + size.body.y;

    float headBottom = bodyTop;
    float headTop = headBottom + size.head.y;

    // 2. 충돌 박스 Y 범위 (여유 공간 추가)
    float minY = legBottom + margin;     // 발 끝에서 약간 위로
    float maxY = headTop - margin;       // 머리는 약간 아래로

    // 3. 너비·깊이
    float width = std::max(size.head.x, size.body.x);
    float depth = width;

    // 4. AABB 생성 (좌우·앞뒤에도 마진 적용)
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

    // 걸음 관절을 중심으로 회전
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

    // 걸음 관절을 중심으로 회전 (왼다리와 반대)
    transform = glm::translate(transform, glm::vec3(0.0f, size.leg.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(-legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.leg.y * 0.5f, 0.0f));

    transform = glm::scale(transform, size.leg);

    return transform;
}