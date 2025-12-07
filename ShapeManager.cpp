#include "ShapeManager.h"

ShapeManager::ShapeManager()
    : selectedShapeType(ShapeType::CUBE),
      defaultColor(glm::vec3(0.8f, 0.8f, 0.8f)),
      defaultSize(1.0f) {
}

ShapeManager::~ShapeManager() {
    clear();
}

void ShapeManager::setSelectedShapeType(ShapeType type) {
    selectedShapeType = type;
}

void ShapeManager::cycleShapeType() {
    switch (selectedShapeType) {
        case ShapeType::CUBE:
            selectedShapeType = ShapeType::PYRAMID;
            break;
        case ShapeType::PYRAMID:
            selectedShapeType = ShapeType::SPHERE;
            break;
        case ShapeType::SPHERE:
            selectedShapeType = ShapeType::CUBE;
            break;
    }
}

bool ShapeManager::addShape(const glm::vec3& worldPos) {
    return addShape(worldPos, selectedShapeType, defaultColor);
}

bool ShapeManager::addShape(const glm::vec3& worldPos, ShapeType type, const glm::vec3& color) {
    ShapeGridPosition gridPos(worldPos);

    // 이미 도형이 있는지 확인
    if (hasShapeAt(worldPos)) {
        return false;
    }

    // 도형 타입에 따라 생성
    std::unique_ptr<Shape> newShape;
    switch (type) {
        case ShapeType::CUBE:
            newShape = std::make_unique<CubeShape>(gridPos.toWorldPosition(), defaultSize, color);
            break;
        case ShapeType::PYRAMID:
            newShape = std::make_unique<PyramidShape>(gridPos.toWorldPosition(), defaultSize, color);
            break;
        case ShapeType::SPHERE:
            newShape = std::make_unique<SphereShape>(gridPos.toWorldPosition(), defaultSize, color);
            break;
        default:
            return false;
    }

    shapes[gridPos] = std::move(newShape);
    return true;
}

bool ShapeManager::removeShape(const glm::vec3& worldPos) {
    ShapeGridPosition gridPos(worldPos);
    auto it = shapes.find(gridPos);
    if (it != shapes.end()) {
        shapes.erase(it);
        return true;
    }
    return false;
}

bool ShapeManager::hasShapeAt(const glm::vec3& worldPos) const {
    ShapeGridPosition gridPos(worldPos);
    return shapes.find(gridPos) != shapes.end();
}

Shape* ShapeManager::getShape(const glm::vec3& worldPos) {
    ShapeGridPosition gridPos(worldPos);
    auto it = shapes.find(gridPos);
    if (it != shapes.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool ShapeManager::raycastShape(const Ray& ray, RaycastHit& hit, Shape** hitShape) {
    float closestDistance = 1000.0f;  // MAX_RAYCAST_DISTANCE
    bool foundHit = false;
    RaycastHit closestHit;
    Shape* closestShape = nullptr;

    for (auto& pair : shapes) {
        Shape* shape = pair.second.get();
        glm::vec3 pos = shape->getPosition();
        float size = shape->getSize();
        float half = size * 0.5f;

        // 간단한 AABB 충돌 검사
        AABB aabb;
        aabb.min = pos - glm::vec3(half);
        aabb.max = pos + glm::vec3(half);

        RaycastHit tempHit;
        if (RayCast::rayAABBIntersection(ray, aabb, tempHit)) {
            if (tempHit.distance < closestDistance) {
                closestDistance = tempHit.distance;
                closestHit = tempHit;
                closestShape = shape;
                foundHit = true;
            }
        }
    }

    if (foundHit) {
        hit = closestHit;
        if (hitShape != nullptr) {
            *hitShape = closestShape;
        }
    }

    return foundHit;
}

void ShapeManager::renderAll(GLuint shaderProgramID) {
    for (auto& pair : shapes) {
        pair.second->render(shaderProgramID);
    }
}

std::string ShapeManager::getSelectedShapeTypeName() const {
    switch (selectedShapeType) {
        case ShapeType::CUBE:
            return "Cube";
        case ShapeType::PYRAMID:
            return "Pyramid";
        case ShapeType::SPHERE:
            return "Sphere";
        default:
            return "Unknown";
    }
}
