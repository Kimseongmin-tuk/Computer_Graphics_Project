#include "RayCast.h"

Ray RayCast::screenToWorldRay(
    double mouseX, double mouseY,
    int screenWidth, int screenHeight,
    const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& cameraPos
) {
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    float z = 1.0f;
    glm::vec3 ray_nds(x, y, z);

    glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
    ray_world = glm::normalize(ray_world);

    return Ray(cameraPos, ray_world);
}

bool RayCast::rayAABBIntersection(
    const Ray& ray,
    const AABB& box,
    RaycastHit& hit,
    float maxDistance
) {
    float tMin = 0.0f;
    float tMax = maxDistance;

    int hitAxis = -1;
    bool hitPositive = false;

    for (int i = 0; i < 3; i++) {
        if (std::abs(ray.direction[i]) < 0.0001f) {
            if (ray.origin[i] < box.min[i] || ray.origin[i] > box.max[i]) {
                return false;
            }
        }
        else {
            float t1 = (box.min[i] - ray.origin[i]) / ray.direction[i];
            float t2 = (box.max[i] - ray.origin[i]) / ray.direction[i];

            bool swapped = false;
            if (t1 > t2) {
                std::swap(t1, t2);
                swapped = true;
            }

            if (t1 > tMin) {
                tMin = t1;
                hitAxis = i;
                hitPositive = !swapped;
            }

            if (t2 < tMax) {
                tMax = t2;
            }

            if (tMin > tMax) {
                return false;
            }
        }
    }

    if (tMin >= 0 && tMin <= maxDistance) {
        hit.hit = true;
        hit.distance = tMin;
        hit.point = ray.getPoint(tMin);
        hit.normal = calculateFaceNormal(hitAxis, hitPositive);
        return true;
    }

    return false;
}

glm::vec3 RayCast::calculateFaceNormal(int axis, bool positive) {
    glm::vec3 normal(0.0f);
    normal[axis] = positive ? 1.0f : -1.0f;
    return normal;
}