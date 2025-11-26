#ifndef RAYCAST_H
#define RAYCAST_H

#include "Common.h"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {
    }

    glm::vec3 getPoint(float t) const {
        return origin + direction * t;
    }
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& minPoint, const glm::vec3& maxPoint)
        : min(minPoint), max(maxPoint) {
    }

    glm::vec3 getCenter() const {
        return (min + max) * 0.5f;
    }

    glm::vec3 getSize() const {
        return max - min;
    }
};

struct RaycastHit {
    bool hit;
    float distance;
    glm::vec3 point;
    glm::vec3 normal;

    RaycastHit() : hit(false), distance(0.0f), point(0.0f), normal(0.0f) {}
};

class RayCast {
public:
    static Ray screenToWorldRay(
        double mouseX, double mouseY,
        int screenWidth, int screenHeight,
        const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos
    );

    static bool rayAABBIntersection(
        const Ray& ray,
        const AABB& box,
        RaycastHit& hit,
        float maxDistance = Constants::MAX_RAYCAST_DISTANCE
    );

private:
    static glm::vec3 calculateFaceNormal(int axis, bool positive);
};

#endif // RAYCAST_H