#ifndef PLAYER_H
#define PLAYER_H

#include "Common.h"
#include "RayCast.h"

class Player {
private:
    glm::vec3 position;
    float yaw;
    float pitch;

    struct {
        glm::vec3 head;
        glm::vec3 body;
        glm::vec3 arm;
        glm::vec3 leg;
    } size;

    float walkCycle;
    float armSwing;
    float legSwing;
    bool isWalking;

    glm::vec3 skinColor;
    glm::vec3 shirtColor;
    glm::vec3 pantsColor;

    bool isFlyingMode;
    glm::vec3 velocity;
    bool isOnGround;
    float jumpVelocity;
    int spaceKeyPressCount;
    float lastSpaceKeyTime;

public:
    Player(const glm::vec3& pos = glm::vec3(0.0f));

    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3& pos) { position = pos; }

    float getYaw() const { return yaw; }
    void setYaw(float y) { yaw = y; }

    float getPitch() const { return pitch; }
    void setPitch(float p) { pitch = p; }

    void setWalking(bool walking) { isWalking = walking; }

    bool getFlyingMode() const { return isFlyingMode; }
    void setFlyingMode(bool flying) { isFlyingMode = flying; }

    glm::vec3 getVelocity() const { return velocity; }
    void setVelocity(const glm::vec3& vel) { velocity = vel; }

    bool getIsOnGround() const { return isOnGround; }
    void setIsOnGround(bool onGround) { isOnGround = onGround; }

    void handleSpaceKeyPress(float currentTime);
    void jump();

    AABB getAABB() const;
    float getTotalHeight() const;
    void updateAnimation(float deltaTime);

    glm::mat4 getHeadTransform() const;
    glm::mat4 getBodyTransform() const;
    glm::mat4 getLeftArmTransform() const;
    glm::mat4 getRightArmTransform() const;
    glm::mat4 getLeftLegTransform() const;
    glm::mat4 getRightLegTransform() const;

    glm::vec3 getHeadColor() const { return skinColor; }
    glm::vec3 getBodyColor() const { return shirtColor; }
    glm::vec3 getArmColor() const { return skinColor; }
    glm::vec3 getLegColor() const { return pantsColor; }

    glm::vec3 getHeadSize() const { return size.head; }
    glm::vec3 getBodySize() const { return size.body; }
    glm::vec3 getArmSize() const { return size.arm; }
    glm::vec3 getLegSize() const { return size.leg; }
};

#endif
