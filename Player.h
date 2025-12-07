#ifndef PLAYER_H
#define PLAYER_H

#include "Common.h"
#include "RayCast.h"  // AABB 구조체 사용

class Player {
private:
    glm::vec3 position;
    float yaw;          // 좌우 회전
    float pitch;        // 상하 회전

    // 플레이어 크기
    struct {
        glm::vec3 head;         // 머리: 8x8x8
        glm::vec3 body;         // 몸통: 8x12x4
        glm::vec3 arm;          // 팔:   4x12x4
        glm::vec3 leg;          // 다리: 4x12x4
    } size;

    // 애니메이션 변수
    float walkCycle;
    float armSwing;
    float legSwing;
    bool isWalking;

    // 색상
    glm::vec3 skinColor;
    glm::vec3 shirtColor;
    glm::vec3 pantsColor;

public:
    Player(const glm::vec3& pos = glm::vec3(0.0f));

    // Getter/Setter
    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3& pos) { position = pos; }

    float getYaw() const { return yaw; }
    void setYaw(float y) { yaw = y; }

    float getPitch() const { return pitch; }
    void setPitch(float p) { pitch = p; }

    void setWalking(bool walking) { isWalking = walking; }

    // 충돌 감지용 AABB 반환
    AABB getAABB() const;

    // 전체 캐릭터 높이
    float getTotalHeight() const;

    // 애니메이션 업데이트
    void updateAnimation(float deltaTime);

    // 렌더링 (각 부위별 변환 행렬 반환)
    glm::mat4 getHeadTransform() const;
    glm::mat4 getBodyTransform() const;
    glm::mat4 getLeftArmTransform() const;
    glm::mat4 getRightArmTransform() const;
    glm::mat4 getLeftLegTransform() const;
    glm::mat4 getRightLegTransform() const;

    // 색상 반환
    glm::vec3 getHeadColor() const { return skinColor; }
    glm::vec3 getBodyColor() const { return shirtColor; }
    glm::vec3 getArmColor() const { return skinColor; }
    glm::vec3 getLegColor() const { return pantsColor; }

    // 크기 반환
    glm::vec3 getHeadSize() const { return size.head; }
    glm::vec3 getBodySize() const { return size.body; }
    glm::vec3 getArmSize() const { return size.arm; }
    glm::vec3 getLegSize() const { return size.leg; }
};

#endif