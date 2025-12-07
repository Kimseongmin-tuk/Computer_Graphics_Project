#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include "Common.h"
#include "Shape.h"
#include "RayCast.h"
#include <unordered_map>
#include <memory>

// 그리드 위치 해시 함수 (Shape용)
struct ShapeGridPosition {
    int x, y, z;

    ShapeGridPosition(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
    ShapeGridPosition(const glm::vec3& worldPos) {
        x = static_cast<int>(std::round(worldPos.x));
        y = static_cast<int>(std::round(worldPos.y));
        z = static_cast<int>(std::round(worldPos.z));
    }

    glm::vec3 toWorldPosition() const {
        return glm::vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    }

    bool operator==(const ShapeGridPosition& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {
    template <>
    struct hash<ShapeGridPosition> {
        size_t operator()(const ShapeGridPosition& pos) const {
            return hash<int>()(pos.x) ^ (hash<int>()(pos.y) << 1) ^ (hash<int>()(pos.z) << 2);
        }
    };
}

class ShapeManager {
private:
    std::unordered_map<ShapeGridPosition, std::unique_ptr<Shape>> shapes;
    ShapeType selectedShapeType;  // 현재 선택된 도형 타입
    glm::vec3 defaultColor;
    float defaultSize;

public:
    ShapeManager();
    ~ShapeManager();

    // 도형 타입 선택
    void setSelectedShapeType(ShapeType type);
    ShapeType getSelectedShapeType() const { return selectedShapeType; }

    // 도형 타입 순환 (1, 2, 3 키로 전환)
    void selectCube() { selectedShapeType = ShapeType::CUBE; }
    void selectPyramid() { selectedShapeType = ShapeType::PYRAMID; }
    void selectSphere() { selectedShapeType = ShapeType::SPHERE; }
    void cycleShapeType();  // 다음 도형 타입으로 전환

    // 도형 추가/제거
    bool addShape(const glm::vec3& worldPos);
    bool addShape(const glm::vec3& worldPos, ShapeType type, const glm::vec3& color);
    bool removeShape(const glm::vec3& worldPos);

    // 도형 조회
    bool hasShapeAt(const glm::vec3& worldPos) const;
    Shape* getShape(const glm::vec3& worldPos);

    // 레이캐스팅
    bool raycastShape(const Ray& ray, RaycastHit& hit, Shape** hitShape);

    // 렌더링
    void renderAll(GLuint shaderProgramID);

    // 색상/크기 설정
    void setDefaultColor(const glm::vec3& color) { defaultColor = color; }
    void setDefaultSize(float size) { defaultSize = size; }

    // 도형 개수
    int getShapeCount() const { return shapes.size(); }

    // 모든 도형 제거
    void clear() { shapes.clear(); }

    // 현재 선택된 도형 타입 이름 반환
    std::string getSelectedShapeTypeName() const;
};

#endif // SHAPEMANAGER_H
