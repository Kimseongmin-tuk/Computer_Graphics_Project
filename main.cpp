#define _CRT_SECURE_NO_WARNINGS 
#include <stdlib.h>
#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include "Common.h"
#include "RayCast.h"
#include "Block.h"
#include "BlockManager.h"
#include "Player.h"
#include "ChallengeManager.h"

// 전역 변수
GLint width = 1280, height = 720;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
GLuint cubeVAO, cubeVBO;
GLuint pyramidVAO, pyramidVBO;
GLuint crosshairVAO, crosshairVBO;
GLuint crosshairShaderProgramID;
GLuint quadVAO, quadVBO;
GLuint uiShaderProgramID;

// 카메라
glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(-1.0f, -1.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -135.0f;
float pitch = -35.0f;
float lastX = 640.0f;
float lastY = 360.0f;
bool firstMouse = true;

// 카메라 모드 (1인칭/3인칭)
enum class CameraMode {
    FIRST_PERSON,
    THIRD_PERSON
};
CameraMode cameraMode = CameraMode::THIRD_PERSON;
float thirdPersonDistance = 5.0f;

// 마우스 워프 이벤트 플래그
bool ignoreNextMouseMove = false;

// 키 상태
bool keyStates[256] = { false };
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 게임 매니저
BlockManager* blockManager = nullptr;
Player* player = nullptr;
ChallengeManager* challengeManager = nullptr;

// 통계 정보
int blocksPlaced = 0;
int blocksDestroyed = 0;
int cubesPlaced = 0;
int pyramidsPlaced = 0;
int cubesDestroyed = 0;
int pyramidsDestroyed = 0;
float totalPlayTime = 0.0f;
bool showStats = false;

// 게임 시작 여부
bool gameStarted = false;

// 블록 선택 변수
int selectedBlockIndex = 0;
BlockType availableBlocks[] = { BlockType::CUBE, BlockType::PYRAMID };

// 정육면체 버텍스 데이터
float cubeVertices[] = {
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
};

// 사각뿔 버텍스 데이터 (바닥면 y=-0.5, 꼭지점 y=0.5)
float pyramidVertices[] = {
    // 바닥 (y = -0.5)
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    // 앞면 (+Z)
    -0.5f, -0.5f,  0.5f,  0.0f,  0.447f,  0.894f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.447f,  0.894f,
     0.0f,  0.5f,  0.0f,  0.0f,  0.447f,  0.894f,

     // 오른쪽면 (+X)
      0.5f, -0.5f,  0.5f,  0.894f,  0.447f,  0.0f,
      0.5f, -0.5f, -0.5f,  0.894f,  0.447f,  0.0f,
      0.0f,  0.5f,  0.0f,  0.894f,  0.447f,  0.0f,

      // 뒷면 (-Z)
       0.5f, -0.5f, -0.5f,  0.0f,  0.447f, -0.894f,
      -0.5f, -0.5f, -0.5f,  0.0f,  0.447f, -0.894f,
       0.0f,  0.5f,  0.0f,  0.0f,  0.447f, -0.894f,

       // 왼쪽면 (-X)
       -0.5f, -0.5f, -0.5f, -0.894f,  0.447f,  0.0f,
       -0.5f, -0.5f,  0.5f, -0.894f,  0.447f,  0.0f,
        0.0f,  0.5f,  0.0f, -0.894f,  0.447f,  0.0f
};

// 함수 선언
char* filetobuf(const char* file);
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
void initCube();
void initPyramid();
void drawScene();
void Reshape(int w, int h);
void KeyboardDown(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void PassiveMotion(int x, int y);
void updateCamera();
void renderCube(glm::vec3 position, glm::vec3 color);
void drawCrosshair();
void renderText(const std::string& text, float x, float y);
void renderChallengeUI();
void renderPreview();
void selectGameMode();
void renderStatsUI();
void renderBlockSelectionUI();

char* filetobuf(const char* file) {
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb");
    if (!fptr)
        return NULL;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length + 1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;
    return buf;
}

void make_vertexShaders() {
    GLchar* vertexSource = filetobuf("vertex.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

void make_fragmentShaders() {
    GLchar* fragmentSource = filetobuf("fragment.glsl");
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
    }
}

GLuint make_shaderProgram() {
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 링크 실패\n" << errorLog << std::endl;
        return false;
    }

    glUseProgram(shaderID);
    return shaderID;
}

GLuint make_crosshairShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform vec2 screenSize;
        void main()
        {
            vec2 pos = aPos.xy / screenSize * 2.0;
            gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    )";

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertShader);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

void initCube() {
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void initPyramid() {
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);

    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void initCrosshair() {
    float crosshairVertices[] = {
        -15.0f, 0.0f, 0.0f,
         15.0f, 0.0f, 0.0f,
          0.0f, -15.0f, 0.0f,
          0.0f,  15.0f, 0.0f
    };

    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void initQuad() {
    float quadVertices[] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

GLuint make_uiShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform vec2 position;
        uniform vec2 size;
        uniform vec2 screenSize;
        void main()
        {
            vec2 pos = (aPos * size + position) / screenSize * 2.0 - 1.0;
            pos.y = -pos.y;
            gl_Position = vec4(pos, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 color;
        void main()
        {
            FragColor = color;
        }
    )";

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertShader);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

void renderCube(glm::vec3 position, glm::vec3 color) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    GLint colorLoc = glGetUniformLocation(shaderProgramID, "blockColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderPyramid(glm::vec3 position, glm::vec3 color, glm::vec3 upDirection) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    glm::vec3 defaultUp(0.0f, 1.0f, 0.0f);
    if (glm::length(upDirection - defaultUp) > 0.001f) {
        glm::vec3 rotationAxis = glm::cross(defaultUp, upDirection);
        if (glm::length(rotationAxis) > 0.001f) {
            float angle = acos(glm::dot(defaultUp, upDirection));
            model = glm::rotate(model, angle, glm::normalize(rotationAxis));
        }
        else if (glm::dot(defaultUp, upDirection) < 0) {
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    GLint colorLoc = glGetUniformLocation(shaderProgramID, "blockColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    glBindVertexArray(pyramidVAO);
    glDrawArrays(GL_TRIANGLES, 0, 21);
    glBindVertexArray(0);
}

void renderBlock(const Block& block) {
    if (block.getType() == BlockType::PYRAMID) {
        renderPyramid(block.getPosition(), block.getColor(), block.getUpDirection());
    }
    else {
        renderCube(block.getPosition(), block.getColor());
    }
}

void renderBox(const glm::mat4& modelMatrix, const glm::vec3& color) {
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint colorLoc = glGetUniformLocation(shaderProgramID, "blockColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderPlayer() {
    if (!player) return;

    renderBox(player->getHeadTransform(), player->getHeadColor());
    renderBox(player->getBodyTransform(), player->getBodyColor());
    renderBox(player->getLeftArmTransform(), player->getArmColor());
    renderBox(player->getRightArmTransform(), player->getArmColor());
    renderBox(player->getLeftLegTransform(), player->getLegColor());
    renderBox(player->getRightLegTransform(), player->getLegColor());
}

void drawCrosshair() {
    glDisable(GL_DEPTH_TEST);

    glUseProgram(crosshairShaderProgramID);

    GLint screenSizeLoc = glGetUniformLocation(crosshairShaderProgramID, "screenSize");
    glUniform2f(screenSizeLoc, (float)width, (float)height);

    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(shaderProgramID);
}

void renderText(const std::string& text, float x, float y) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

void renderStatsUI() {
    if (!showStats) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(uiShaderProgramID);

    GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
    GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
    GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
    GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");

    glUniform2f(posLoc, (float)width / 2, (float)height / 2);
    glUniform2f(sizeLoc, 200.0f, 200.0f);
    glUniform2f(screenSizeLoc, (float)width, (float)height);
    glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 0.7f);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProgramID);

    std::string title = "=== STATISTICS ===";
    renderText(title, width / 2 - 80, height / 2 + 120);

    int minutes = (int)(totalPlayTime / 60.0f);
    int seconds = (int)totalPlayTime % 60;
    std::string timeText = "Play Time: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    renderText(timeText, width / 2 - 80, height / 2 + 70);

    std::string cubesPlacedText = "Cubes Placed: " + std::to_string(cubesPlaced);
    renderText(cubesPlacedText, width / 2 - 80, height / 2 + 40);

    std::string pyramidsPlacedText = "Pyramids Placed: " + std::to_string(pyramidsPlaced);
    renderText(pyramidsPlacedText, width / 2 - 80, height / 2 + 10);

    std::string cubesDestroyedText = "Cubes Destroyed: " + std::to_string(cubesDestroyed);
    renderText(cubesDestroyedText, width / 2 - 80, height / 2 - 20);

    std::string pyramidsDestroyedText = "Pyramids Destroyed: " + std::to_string(pyramidsDestroyed);
    renderText(pyramidsDestroyedText, width / 2 - 80, height / 2 - 50);

    int currentBlocks = blockManager->getBlockCount() - 25;
    if (currentBlocks < 0) currentBlocks = 0;
    std::string currentText = "Current Blocks: " + std::to_string(currentBlocks);
    renderText(currentText, width / 2 - 80, height / 2 - 80);

    std::string closeText = "Press Q to close";
    renderText(closeText, width / 2 - 80, height / 2 - 120);
}

void renderBlockSelectionUI() {
    if (!challengeManager || challengeManager->getMode() != GameMode::FREE_BUILD) return;

    std::string blockTypeText = "Selected: ";

    if (selectedBlockIndex == 0) {
        blockTypeText += "Cube (1)";
    }
    else {
        blockTypeText += "Pyramid (2)";
    }

    renderText(blockTypeText, 20, height - 30);
}

void renderChallengeUI() {
    if (!challengeManager || !challengeManager->isChallengeMode()) return;
    if (!challengeManager->isChallengeStarted()) return;

    float timeRemaining = challengeManager->getTimeRemaining();
    float progress = challengeManager->getProgress(blockManager->getAllBlocks());

    std::string timeText = "Time: " + std::to_string((int)timeRemaining) + "s";
    renderText(timeText, 10, height - 30);

    std::string progressText = "Progress: " + std::to_string((int)progress) + "%";
    renderText(progressText, 10, height - 60);

    if (challengeManager->isChallengeCompleted()) {
        std::string completedText = "CHALLENGE COMPLETED!";
        renderText(completedText, width / 2 - 100, height / 2);
    }
}

void renderPreview() {
    if (!challengeManager || !challengeManager->isChallengeMode()) return;
    if (!challengeManager->isChallengeStarted()) return;

    glViewport(width - 250, height - 250, 200, 200);

    float rotation = challengeManager->getPreviewRotation();

    float rad = glm::radians(rotation);
    float camX = 5.0f * cos(rad);
    float camZ = 5.0f * sin(rad);

    glm::mat4 previewView = glm::lookAt(
        glm::vec3(camX, 5.0f, camZ),
        glm::vec3(0.5f, 1.0f, 0.5f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 previewProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(previewView));

    GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(previewProjection));

    for (const auto& targetBlock : challengeManager->getTargetBlocks()) {
        renderCube(targetBlock.position, targetBlock.color);
    }

    glViewport(0, 0, width, height);
}

void selectGameMode() {
    std::cout << "========================================" << std::endl;
    std::cout << "   BlockBuilder - Game Mode Selection   " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Free Build Mode - Build freely!" << std::endl;
    std::cout << "2. Challenge Mode - Build the target shape!" << std::endl;
    std::cout << "Select mode (1 or 2): ";

    int choice;
    std::cin >> choice;

    if (choice == 1) {
        std::cout << "Free Build Mode selected!" << std::endl;
        challengeManager->setMode(GameMode::FREE_BUILD);
    }
    else if (choice == 2) {
        std::cout << "Challenge Mode selected!" << std::endl;
        challengeManager->setMode(GameMode::CHALLENGE);

        // 추가 평면 지형 생성
        blockManager->clearAll();                    // 기존 랜덤 지형 제거
        blockManager->generateFlatTerrain();         // 5x5 평면 지형 생성
        std::cout << "Flat terrain generated for challenge mode." << std::endl;

        std::cout << "\n========================================" << std::endl;
        std::cout << "   Select Challenge   " << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "1. Simple Tower (Easy) - 60s" << std::endl;
        std::cout << "2. Pyramid (Medium) - 180s" << std::endl;
        std::cout << "3. Stairs (Hard) - 120s" << std::endl;
        std::cout << "Select challenge (1-3): ";

        int challengeChoice;
        std::cin >> challengeChoice;

        if (challengeChoice < 1 || challengeChoice > 3) {
            std::cout << "Invalid choice. Defaulting to Simple Tower." << std::endl;
            challengeChoice = 0;
        }

        challengeManager->loadChallenge(challengeChoice);
        challengeManager->startChallenge();
        std::cout << "Challenge " << challengeChoice << " loaded!" << std::endl;
    }
    else {
        std::cout << "Invalid choice. Defaulting to Free Build Mode." << std::endl;
        challengeManager->setMode(GameMode::FREE_BUILD);
    }

    gameStarted = true;
    std::cout << "Game starting..." << std::endl;
}

void updateCamera() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    if (player) {
        player->setYaw(yaw);
        player->setPitch(pitch);
    }

    if (player) {
        glm::vec3 playerPos = player->getPosition();

        if (cameraMode == CameraMode::FIRST_PERSON) {
            cameraPos = playerPos;
            cameraPos.y += 1.0f;
        }
        else if (cameraMode == CameraMode::THIRD_PERSON) {
            playerPos.y += 1.0f;
            cameraPos = playerPos - cameraFront * thirdPersonDistance;
        }
    }
}

void processInput() {
    float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float cameraSpeed = 5.0f * deltaTime;

    if (challengeManager && challengeManager->getMode() == GameMode::FREE_BUILD) {
        totalPlayTime += deltaTime;
    }

    if (!player) return;

    bool isMoving = false;
    glm::vec3 moveDirection(0.0f);

    glm::vec3 forward = cameraFront;
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f) {
        forward = glm::normalize(forward);
    }

    glm::vec3 right = glm::cross(forward, cameraUp);
    if (glm::length(right) > 0.0f) {
        right = glm::normalize(right);
    }

    if (keyStates['w'] || keyStates['W']) {
        moveDirection += forward;
        isMoving = true;
    }
    if (keyStates['s'] || keyStates['S']) {
        moveDirection -= forward;
        isMoving = true;
    }
    if (keyStates['a'] || keyStates['A']) {
        moveDirection -= right;
        isMoving = true;
    }
    if (keyStates['d'] || keyStates['D']) {
        moveDirection += right;
        isMoving = true;
    }

    if (isMoving && glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
        glm::vec3 playerPos = player->getPosition();
        glm::vec3 oldPos = playerPos;

        if (player->getFlyingMode()) {
            playerPos += moveDirection * cameraSpeed;
        }
        else {
            float currentY = playerPos.y;
            playerPos += moveDirection * cameraSpeed;
            playerPos.y = currentY;
        }

        player->setPosition(playerPos);

        AABB playerAABB = player->getAABB();
        if (blockManager->checkCollision(playerAABB)) {
            player->setPosition(oldPos);
            player->setWalking(false);
        }
        else {
            player->setWalking(true);
        }
    }
    else {
        player->setWalking(false);
    }

    if (player->getFlyingMode()) {
        if (keyStates[' ']) {
            glm::vec3 playerPos = player->getPosition();
            glm::vec3 oldPos = playerPos;

            playerPos.y += cameraSpeed;
            player->setPosition(playerPos);

            AABB playerAABB = player->getAABB();
            if (blockManager->checkCollision(playerAABB)) {
                player->setPosition(oldPos);
            }
        }
        if (keyStates['c'] || keyStates['C']) {
            glm::vec3 playerPos = player->getPosition();
            glm::vec3 oldPos = playerPos;

            playerPos.y -= cameraSpeed;
            player->setPosition(playerPos);

            AABB playerAABB = player->getAABB();
            if (blockManager->checkCollision(playerAABB)) {
                player->setPosition(oldPos);
            }
        }
    }
    else {
        glm::vec3 velocity = player->getVelocity();
        velocity.y -= 25.0f * deltaTime;

        glm::vec3 playerPos = player->getPosition();
        glm::vec3 oldPos = playerPos;
        playerPos += velocity * deltaTime;
        player->setPosition(playerPos);

        AABB playerAABB = player->getAABB();
        if (blockManager->checkCollision(playerAABB)) {
            player->setPosition(oldPos);

            if (velocity.y < 0) {
                player->setIsOnGround(true);
                velocity.y = 0.0f;
            }
            else {
                velocity.y = 0.0f;
            }
        }
        else {
            player->setIsOnGround(false);
        }

        player->setVelocity(velocity);
    }

    player->updateAnimation(deltaTime);
    updateCamera();
}

void drawScene() {
    if (!gameStarted) {
        return;
    }

    processInput();
    updateCamera();

    if (challengeManager && challengeManager->isChallengeMode()) {
        challengeManager->update(deltaTime);
    }

    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
    GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
    glUniform3f(lightPosLoc, 10.0f, 10.0f, 10.0f);

    GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

    GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

    for (const auto& pair : blockManager->getAllBlocks()) {
        const Block& block = pair.second;
        renderBlock(block);
    }

    if (cameraMode == CameraMode::THIRD_PERSON) {
        renderPlayer();
    }

    renderPreview();

    glm::mat4 mainView = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mainView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    if (!showStats) {
        drawCrosshair();
    }

    renderChallengeUI();
    renderStatsUI();
    renderBlockSelectionUI();

    glutSwapBuffers();
}

void Idle() {
    glutPostRedisplay();
}

void Reshape(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

void KeyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    if (key == 27) {
        exit(0);
    }

    if (key == 'f' || key == 'F') {
        if (cameraMode == CameraMode::FIRST_PERSON) {
            cameraMode = CameraMode::THIRD_PERSON;
            std::cout << "3인칭 모드" << std::endl;
        }
        else {
            cameraMode = CameraMode::FIRST_PERSON;
            std::cout << "1인칭 모드" << std::endl;
        }
    }

    if ((key == 'q' || key == 'Q') && challengeManager && challengeManager->getMode() == GameMode::FREE_BUILD) {
        showStats = !showStats;
        if (showStats) {
            std::cout << "Statistics displayed" << std::endl;
        }
        else {
            std::cout << "Statistics hidden" << std::endl;
        }
    }

    if (key == ' ' && player) {
        float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        player->handleSpaceKeyPress(currentTime);
    }

    if (key >= '1' && key <= '2' && challengeManager && challengeManager->getMode() == GameMode::FREE_BUILD) {
        selectedBlockIndex = key - '1';
        std::cout << "Selected block type: " << (selectedBlockIndex == 0 ? "Cube" : "Pyramid") << std::endl;
    }
}

void KeyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void Mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN && gameStarted) {
        int centerX = width / 2;
        int centerY = height / 2;

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
        glm::vec3 rayOrigin = cameraPos;

        if (player) {
            rayOrigin = player->getPosition();
            rayOrigin.y += 1.0f;
        }

        Ray ray = RayCast::screenToWorldRay(centerX, centerY, width, height, view, projection, rayOrigin);
        RaycastHit hit;
        Block* hitBlock = nullptr;

        if (blockManager->raycastBlock(ray, hit, &hitBlock)) {
            if (button == GLUT_LEFT_BUTTON) {
                glm::vec3 newPos = hitBlock->getPosition() - hit.normal * Constants::BLOCK_SIZE;
                BlockType selectedType = availableBlocks[selectedBlockIndex];

                if (selectedType == BlockType::PYRAMID) {
                    if (hitBlock->getType() != BlockType::CUBE) {
                        std::cout << "Pyramids can only be placed on cubes!" << std::endl;
                        return;
                    }
                }

                glm::vec3 upDirection = (selectedType == BlockType::PYRAMID) ? -hit.normal : glm::vec3(0.0f, 1.0f, 0.0f);
                bool added = blockManager->addBlock(newPos, selectedType, upDirection);

                if (added) {
                    blocksPlaced++;
                    if (selectedType == BlockType::CUBE) {
                        cubesPlaced++;
                    }
                    else if (selectedType == BlockType::PYRAMID) {
                        pyramidsPlaced++;
                    }
                }

                if (added && challengeManager && challengeManager->isChallengeMode()) {
                    GridPosition gridPos(newPos);
                    Block* newBlock = blockManager->getBlock(gridPos);

                    for (const auto& targetBlock : challengeManager->getTargetBlocks()) {
                        if (glm::length(targetBlock.position - newPos) < 0.1f) {
                            if (newBlock) {
                                newBlock->setColor(targetBlock.color);
                            }
                            break;
                        }
                    }
                }
            }
            else if (button == GLUT_RIGHT_BUTTON) {
                BlockType removedType = hitBlock->getType();
                bool removed = blockManager->removeBlock(hitBlock->getPosition());

                if (removed) {
                    blocksDestroyed++;
                    if (removedType == BlockType::CUBE) {
                        cubesDestroyed++;
                    }
                    else if (removedType == BlockType::PYRAMID) {
                        pyramidsDestroyed++;
                    }
                }
            }
        }
    }

    glutPostRedisplay();
}

void PassiveMotion(int x, int y) {
    if (ignoreNextMouseMove) {
        ignoreNextMouseMove = false;
        return;
    }

    if (firstMouse) {
        lastX = width / 2;
        lastY = height / 2;
        firstMouse = false;
        return;
    }

    float xoffset = x - width / 2;
    float yoffset = height / 2 - y;

    if (abs(xoffset) < 1 && abs(yoffset) < 1) {
        return;
    }

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCamera();

    ignoreNextMouseMove = true;
    glutWarpPointer(width / 2, height / 2);

    glutPostRedisplay();
}

void main(int argc, char** argv) {
    width = 1280;
    height = 720;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("BlockBuilder");

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);

    glutSetCursor(GLUT_CURSOR_NONE);

    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();

    initCube();
    initPyramid();
    initCrosshair();
    crosshairShaderProgramID = make_crosshairShader();
    initQuad();
    uiShaderProgramID = make_uiShader();

    blockManager = new BlockManager();
    player = new Player(glm::vec3(0.0f, 5.0f, 0.0f));
    challengeManager = new ChallengeManager();

    selectGameMode();

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(KeyboardDown);
    glutKeyboardUpFunc(KeyboardUp);
    glutMouseFunc(Mouse);
    glutPassiveMotionFunc(PassiveMotion);
    glutIdleFunc(Idle);

    glutMainLoop();
}