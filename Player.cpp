#include "Player.h"

Player::Player(const glm::vec3& pos)
    : position(pos), yaw(0.0f), pitch(0.0f),
    walkCycle(0.0f), armSwing(0.0f), legSwing(0.0f), isWalking(false)
{
    // 마인크래프트 스타일 크기 설정 
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
    
    // 플레이어 회전 (yaw) - Y축 기준
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // 머리 위치 (몸통 위)
    float headY = size.body.y + size.head.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(0.0f, headY, 0.0f));
    
    // 머리 크기
    transform = glm::scale(transform, size.head);
    
    return transform;
}

glm::mat4 Player::getBodyTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    
    transform = glm::translate(transform, position);
    
    // 플레이어 회전 (yaw) - Y축 기준
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // 몸통 위치 (중심)
    float bodyY = size.body.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(0.0f, bodyY, 0.0f));
    
    transform = glm::scale(transform, size.body);
    
    return transform;
}

glm::mat4 Player::getLeftArmTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    
    transform = glm::translate(transform, position);
    
    // 플레이어 회전 (yaw) - Y축 기준
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
    
    // 플레이어 회전 (yaw) - Y축 기준
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // 오른팔 어깨 위치
    float armX = size.body.x * 0.5f + size.arm.x * 0.5f;
    float armY = size.body.y - size.arm.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(armX, armY, 0.0f));
    
    // 어깨 관절을 중심으로 회전 (왼팔과 반대)
    transform = glm::translate(transform, glm::vec3(0.0f, size.arm.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(armSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.arm.y * 0.5f, 0.0f));
    
    transform = glm::scale(transform, size.arm);
    
    return transform;
}

glm::mat4 Player::getLeftLegTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    
    transform = glm::translate(transform, position);
    
    // 플레이어 회전 (yaw) - Y축 기준
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // 왼다리 위치 (몸통 아래)
    float legX = -size.body.x * 0.25f;
    float legY = -size.leg.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(legX, legY, 0.0f));
    
    // 힙 관절을 중심으로 회전
    transform = glm::translate(transform, glm::vec3(0.0f, size.leg.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.leg.y * 0.5f, 0.0f));
    
    transform = glm::scale(transform, size.leg);
    
    return transform;
}

glm::mat4 Player::getRightLegTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    
    transform = glm::translate(transform, position);
    
    // 플레이어 회전 (yaw) - Y축 기준
    transform = glm::rotate(transform, glm::radians(-yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // 오른다리 위치 (몸통 아래)
    float legX = size.body.x * 0.25f;
    float legY = -size.leg.y * 0.5f;
    transform = glm::translate(transform, glm::vec3(legX, legY, 0.0f));
    
    // 힙 관절을 중심으로 회전 (왼다리와 반대)
    transform = glm::translate(transform, glm::vec3(0.0f, size.leg.y * 0.5f, 0.0f));
    transform = glm::rotate(transform, glm::radians(-legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -size.leg.y * 0.5f, 0.0f));
    
    transform = glm::scale(transform, size.leg);
    
    return transform;
}
