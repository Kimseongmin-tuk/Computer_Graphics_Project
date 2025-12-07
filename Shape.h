#ifndef SHAPE_H
#define SHAPE_H

#include "Common.h"
#include <vector>

enum class ShapeType {
    CUBE,
    PYRAMID,
    SPHERE
};

class Shape {
protected:
    glm::vec3 position;
    glm::vec3 color;
    float size;
    ShapeType type;

    // VAO/VBO 데이터
    GLuint VAO;
    GLuint VBO;
    std::vector<float> vertices;
    int vertexCount;

public:
    Shape(const glm::vec3& pos, float sz, const glm::vec3& col, ShapeType t);
    virtual ~Shape();

    // 순수 가상 함수 - 각 도형이 구현해야 함
    virtual void generateVertices() = 0;
    virtual void setupBuffers() = 0;

    // 렌더링
    void render(GLuint shaderProgramID);

    // Getter
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getColor() const { return color; }
    float getSize() const { return size; }
    ShapeType getType() const { return type; }
    int getVertexCount() const { return vertexCount; }

    // Setter
    void setPosition(const glm::vec3& pos) { position = pos; }
    void setColor(const glm::vec3& col) { color = col; }
    void setSize(float sz) { size = sz; }
};

// 정육면체
class CubeShape : public Shape {
public:
    CubeShape(const glm::vec3& pos, float sz, const glm::vec3& col);
    void generateVertices() override;
    void setupBuffers() override;
};

// 사각뿔
class PyramidShape : public Shape {
public:
    PyramidShape(const glm::vec3& pos, float sz, const glm::vec3& col);
    void generateVertices() override;
    void setupBuffers() override;
};

// 구
class SphereShape : public Shape {
private:
    int sectorCount;  // 경도 분할 수
    int stackCount;   // 위도 분할 수

public:
    SphereShape(const glm::vec3& pos, float sz, const glm::vec3& col, int sectors = 36, int stacks = 18);
    void generateVertices() override;
    void setupBuffers() override;
};

#endif // SHAPE_H
