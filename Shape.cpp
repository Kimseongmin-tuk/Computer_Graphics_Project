#include "Shape.h"
#include <cmath>

// Shape 기본 클래스 구현
Shape::Shape(const glm::vec3& pos, float sz, const glm::vec3& col, ShapeType t)
    : position(pos), size(sz), color(col), type(t), VAO(0), VBO(0), vertexCount(0) {
}

Shape::~Shape() {
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
    }
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
    }
}

void Shape::render(GLuint shaderProgramID) {
    // 모델 변환 행렬
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    GLint colorLoc = glGetUniformLocation(shaderProgramID, "blockColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

// CubeShape 구현
CubeShape::CubeShape(const glm::vec3& pos, float sz, const glm::vec3& col)
    : Shape(pos, sz, col, ShapeType::CUBE) {
    generateVertices();
    setupBuffers();
}

void CubeShape::generateVertices() {
    float half = size * 0.5f;

    vertices = {
        // 앞면 (Z+)
        -half, -half,  half,  0.0f,  0.0f,  1.0f,
         half, -half,  half,  0.0f,  0.0f,  1.0f,
         half,  half,  half,  0.0f,  0.0f,  1.0f,
         half,  half,  half,  0.0f,  0.0f,  1.0f,
        -half,  half,  half,  0.0f,  0.0f,  1.0f,
        -half, -half,  half,  0.0f,  0.0f,  1.0f,

        // 뒷면 (Z-)
        -half, -half, -half,  0.0f,  0.0f, -1.0f,
         half,  half, -half,  0.0f,  0.0f, -1.0f,
         half, -half, -half,  0.0f,  0.0f, -1.0f,
         half,  half, -half,  0.0f,  0.0f, -1.0f,
        -half, -half, -half,  0.0f,  0.0f, -1.0f,
        -half,  half, -half,  0.0f,  0.0f, -1.0f,

        // 왼쪽면 (X-)
        -half,  half,  half, -1.0f,  0.0f,  0.0f,
        -half,  half, -half, -1.0f,  0.0f,  0.0f,
        -half, -half, -half, -1.0f,  0.0f,  0.0f,
        -half, -half, -half, -1.0f,  0.0f,  0.0f,
        -half, -half,  half, -1.0f,  0.0f,  0.0f,
        -half,  half,  half, -1.0f,  0.0f,  0.0f,

        // 오른쪽면 (X+)
         half,  half,  half,  1.0f,  0.0f,  0.0f,
         half, -half, -half,  1.0f,  0.0f,  0.0f,
         half,  half, -half,  1.0f,  0.0f,  0.0f,
         half, -half, -half,  1.0f,  0.0f,  0.0f,
         half,  half,  half,  1.0f,  0.0f,  0.0f,
         half, -half,  half,  1.0f,  0.0f,  0.0f,

        // 아래면 (Y-)
        -half, -half, -half,  0.0f, -1.0f,  0.0f,
         half, -half, -half,  0.0f, -1.0f,  0.0f,
         half, -half,  half,  0.0f, -1.0f,  0.0f,
         half, -half,  half,  0.0f, -1.0f,  0.0f,
        -half, -half,  half,  0.0f, -1.0f,  0.0f,
        -half, -half, -half,  0.0f, -1.0f,  0.0f,

        // 위면 (Y+)
        -half,  half, -half,  0.0f,  1.0f,  0.0f,
         half,  half,  half,  0.0f,  1.0f,  0.0f,
         half,  half, -half,  0.0f,  1.0f,  0.0f,
         half,  half,  half,  0.0f,  1.0f,  0.0f,
        -half,  half, -half,  0.0f,  1.0f,  0.0f,
        -half,  half,  half,  0.0f,  1.0f,  0.0f
    };

    vertexCount = 36;
}

void CubeShape::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 위치 속성
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 법선 속성
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// PyramidShape 구현
PyramidShape::PyramidShape(const glm::vec3& pos, float sz, const glm::vec3& col)
    : Shape(pos, sz, col, ShapeType::PYRAMID) {
    generateVertices();
    setupBuffers();
}

void PyramidShape::generateVertices() {
    float half = size * 0.5f;
    float height = size;

    // 꼭대기 정점
    glm::vec3 apex(0.0f, height * 0.5f, 0.0f);

    // 바닥 정점들
    glm::vec3 base1(-half, -height * 0.5f,  half);
    glm::vec3 base2( half, -height * 0.5f,  half);
    glm::vec3 base3( half, -height * 0.5f, -half);
    glm::vec3 base4(-half, -height * 0.5f, -half);

    // 법선 계산 함수
    auto calcNormal = [](const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
        return glm::normalize(glm::cross(v2 - v1, v3 - v1));
    };

    // 앞면
    glm::vec3 n1 = calcNormal(base1, base2, apex);
    vertices.insert(vertices.end(), {
        base1.x, base1.y, base1.z, n1.x, n1.y, n1.z,
        base2.x, base2.y, base2.z, n1.x, n1.y, n1.z,
        apex.x, apex.y, apex.z, n1.x, n1.y, n1.z
    });

    // 오른쪽면
    glm::vec3 n2 = calcNormal(base2, base3, apex);
    vertices.insert(vertices.end(), {
        base2.x, base2.y, base2.z, n2.x, n2.y, n2.z,
        base3.x, base3.y, base3.z, n2.x, n2.y, n2.z,
        apex.x, apex.y, apex.z, n2.x, n2.y, n2.z
    });

    // 뒷면
    glm::vec3 n3 = calcNormal(base3, base4, apex);
    vertices.insert(vertices.end(), {
        base3.x, base3.y, base3.z, n3.x, n3.y, n3.z,
        base4.x, base4.y, base4.z, n3.x, n3.y, n3.z,
        apex.x, apex.y, apex.z, n3.x, n3.y, n3.z
    });

    // 왼쪽면
    glm::vec3 n4 = calcNormal(base4, base1, apex);
    vertices.insert(vertices.end(), {
        base4.x, base4.y, base4.z, n4.x, n4.y, n4.z,
        base1.x, base1.y, base1.z, n4.x, n4.y, n4.z,
        apex.x, apex.y, apex.z, n4.x, n4.y, n4.z
    });

    // 바닥면 (2개 삼각형)
    glm::vec3 n5(0.0f, -1.0f, 0.0f);
    vertices.insert(vertices.end(), {
        base1.x, base1.y, base1.z, n5.x, n5.y, n5.z,
        base3.x, base3.y, base3.z, n5.x, n5.y, n5.z,
        base2.x, base2.y, base2.z, n5.x, n5.y, n5.z,

        base1.x, base1.y, base1.z, n5.x, n5.y, n5.z,
        base4.x, base4.y, base4.z, n5.x, n5.y, n5.z,
        base3.x, base3.y, base3.z, n5.x, n5.y, n5.z
    });

    vertexCount = 18; // 4면 * 3 정점 + 바닥 2삼각형 * 3 정점
}

void PyramidShape::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 위치 속성
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 법선 속성
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// SphereShape 구현
SphereShape::SphereShape(const glm::vec3& pos, float sz, const glm::vec3& col, int sectors, int stacks)
    : Shape(pos, sz, col, ShapeType::SPHERE), sectorCount(sectors), stackCount(stacks) {
    generateVertices();
    setupBuffers();
}

void SphereShape::generateVertices() {
    float radius = size * 0.5f;

    const float PI = 3.14159265359f;
    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * stackStep; // -π/2 에서 π/2
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * sectorStep; // 0 에서 2π

            // 정점 위치
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            // 법선 (구의 중심에서 정점으로의 방향)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }

    // 인덱스를 사용하지 않고 삼각형 정점 직접 생성
    std::vector<float> triangleVertices;

    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                // 첫 번째 삼각형
                for (int v = 0; v < 6; ++v) {
                    triangleVertices.push_back(vertices[k1 * 6 + v]);
                }
                for (int v = 0; v < 6; ++v) {
                    triangleVertices.push_back(vertices[k2 * 6 + v]);
                }
                for (int v = 0; v < 6; ++v) {
                    triangleVertices.push_back(vertices[(k1 + 1) * 6 + v]);
                }
            }

            if (i != (stackCount - 1)) {
                // 두 번째 삼각형
                for (int v = 0; v < 6; ++v) {
                    triangleVertices.push_back(vertices[(k1 + 1) * 6 + v]);
                }
                for (int v = 0; v < 6; ++v) {
                    triangleVertices.push_back(vertices[k2 * 6 + v]);
                }
                for (int v = 0; v < 6; ++v) {
                    triangleVertices.push_back(vertices[(k2 + 1) * 6 + v]);
                }
            }
        }
    }

    vertices = triangleVertices;
    vertexCount = vertices.size() / 6;
}

void SphereShape::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 위치 속성
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 법선 속성
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
