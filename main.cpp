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
GLuint crosshairVAO, crosshairVBO;
GLuint crosshairShaderProgramID;

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

// 마우스 워프시 발생 플래그
bool ignoreNextMouseMove = false;

// 키 상태
bool keyStates[256] = { false };
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 게임 매니저
BlockManager* blockManager = nullptr;
Player* player = nullptr;
ChallengeManager* challengeManager = nullptr;

// 게임 시작 여부
bool gameStarted = false;

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

// 함수 선언
char* filetobuf(const char* file);
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
void initCube();
void drawScene();
void Reshape(int w, int h);
void KeyboardDown(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void PassiveMotion(int x, int y);
void updateCamera();
void renderCube(glm::vec3 position, glm::vec3 color);
void drawCrosshair();  // 크로스헤어 그리기
void renderText(const std::string& text, float x, float y);  // 텍스트 렌더링
void renderChallengeUI();  // 챌린지 UI
void renderPreview();  // 완성본 미리보기
void selectGameMode();  // 게임 모드 선택

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

// 크로스헤어를 위한 쉐이더 생성 함수
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

// 크로스헤어 초기화 함수 추가
void initCrosshair() {
    float crosshairVertices[] = {
        // 가로선
        -15.0f, 0.0f, 0.0f,
         15.0f, 0.0f, 0.0f,
         // 세로선
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

// 박스렌더 렌더링
void renderBox(const glm::mat4& modelMatrix, const glm::vec3& color) {
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint colorLoc = glGetUniformLocation(shaderProgramID, "blockColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// 플레이어 렌더링 함수
void renderPlayer() {
    if (!player) return;

    // 머리
    renderBox(player->getHeadTransform(), player->getHeadColor());

    // 몸통
    renderBox(player->getBodyTransform(), player->getBodyColor());

    // 왼팔
    renderBox(player->getLeftArmTransform(), player->getArmColor());

    // 오른팔
    renderBox(player->getRightArmTransform(), player->getArmColor());

    // 왼다리
    renderBox(player->getLeftLegTransform(), player->getLegColor());

    // 오른다리
    renderBox(player->getRightLegTransform(), player->getLegColor());
}

// drawCrosshair 함수
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

// 텍스트 렌더링 함수
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

// 챌린지 UI 렌더링
void renderChallengeUI() {
    if (!challengeManager || !challengeManager->isChallengeMode()) return;
    if (!challengeManager->isChallengeStarted()) return;

    float timeRemaining = challengeManager->getTimeRemaining();
    float progress = challengeManager->getProgress(blockManager->getAllBlocks());

    // 타이머 표시
    std::string timeText = "Time: " + std::to_string((int)timeRemaining) + "s";
    renderText(timeText, 10, height - 30);

    // 진행도 표시
    std::string progressText = "Progress: " + std::to_string((int)progress) + "%";
    renderText(progressText, 10, height - 60);

    if (challengeManager->isChallengeCompleted()) {
        std::string completedText = "CHALLENGE COMPLETED!";
        renderText(completedText, width / 2 - 100, height / 2);
    }
}

// 완성본 미리보기 렌더링
void renderPreview() {
    if (!challengeManager || !challengeManager->isChallengeMode()) return;
    if (!challengeManager->isChallengeStarted()) return;

    // 오른쪽 위에 미니 뷰포트 설정
    glViewport(width - 250, height - 250, 200, 200);

    // 회전 각도
    float rotation = challengeManager->getPreviewRotation();

    // 회전하는 카메라 위치 계산
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

    // 블록들을 그대로 렌더링 (회전은 카메라가 함)
    for (const auto& targetBlock : challengeManager->getTargetBlocks()) {
        renderCube(targetBlock.position, targetBlock.color);
    }

    // 원래 뷰포트로 복원
    glViewport(0, 0, width, height);
}

// 게임 모드 선택 함수
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

        // 챌린지 선택
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

    // 플레이어 회전 업데이트
    if (player) {
        player->setYaw(yaw);
        player->setPitch(pitch);
    }

    // 카메라 위치 업데이트
    if (player) {
        glm::vec3 playerPos = player->getPosition();

        if (cameraMode == CameraMode::FIRST_PERSON) {
            // 1인칭
            cameraPos = playerPos;
            cameraPos.y += 1.0f;
        }
        else if (cameraMode == CameraMode::THIRD_PERSON) {
            // 3인칭
            playerPos.y += 1.0f;
            cameraPos = playerPos - cameraFront * thirdPersonDistance;
        }
    }
}

// 키 입력 처리 함수
void processInput() {
    float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float cameraSpeed = 5.0f * deltaTime;

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

    // WASD 입력 처리
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

    // 플레이어 수평 이동 (충돌 체크 포함)
    if (isMoving && glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
        glm::vec3 playerPos = player->getPosition();
        glm::vec3 oldPos = playerPos;  // 이전 위치 저장

        float currentY = playerPos.y;
        playerPos += moveDirection * cameraSpeed;
        playerPos.y = currentY;

        // 임시로 새 위치 설정
        player->setPosition(playerPos);

        // 충돌 체크
        AABB playerAABB = player->getAABB();
        if (blockManager->checkCollision(playerAABB)) {
            // 충돌 발생 -> 이전 위치로 되돌림
            player->setPosition(oldPos);
            player->setWalking(false);
        }
        else {
            // 충돌 없음 -> 이동 성공
            player->setWalking(true);
        }
    }
    else {
        player->setWalking(false);
    }

    // Space/C 키로 플레이어 수직 이동 (충돌 체크 포함)
    if (keyStates[' ']) {
        glm::vec3 playerPos = player->getPosition();
        glm::vec3 oldPos = playerPos;  // 이전 위치 저장

        playerPos.y += cameraSpeed;
        player->setPosition(playerPos);

        // 충돌 체크
        AABB playerAABB = player->getAABB();
        if (blockManager->checkCollision(playerAABB)) {
            // 충돌 발생 -> 이전 위치로 되돌림
            player->setPosition(oldPos);
        }
    }
    if (keyStates['c'] || keyStates['C']) {
        glm::vec3 playerPos = player->getPosition();
        glm::vec3 oldPos = playerPos;  // 이전 위치 저장

        playerPos.y -= cameraSpeed;
        player->setPosition(playerPos);

        // 충돌 체크
        AABB playerAABB = player->getAABB();
        if (blockManager->checkCollision(playerAABB)) {
            // 충돌 발생 -> 이전 위치로 되돌림
            player->setPosition(oldPos);
        }
    }

    // 플레이어 애니메이션 업데이트
    player->updateAnimation(deltaTime);

    // 카메라 업데이트
    updateCamera();
}

void drawScene() {
    if (!gameStarted) {
        return;
    }

    processInput();
    updateCamera();  // 매 프레임 카메라 위치 업데이트

    // 챌린지 업데이트
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

    // 블록 렌더링
    for (const auto& pair : blockManager->getAllBlocks()) {
        const Block& block = pair.second;
        renderCube(block.getPosition(), block.getColor());
    }

    // 플레이어 렌더링
    if (cameraMode == CameraMode::THIRD_PERSON) {
        renderPlayer();
    }

    // 완성본 미리보기 렌더링
    renderPreview();

    // 뷰 매트릭스 복원
    glm::mat4 mainView = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mainView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // 크로스헤어 그리기
    drawCrosshair();

    // 챌린지 UI 렌더링
    renderChallengeUI();

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

// Keyboard 함수를 KeyboardDown, KeyboardUp으로 분리
void KeyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    if (key == 27) {  // ESC
        exit(0);
    }

    // F 키로 카메라 모드 전환
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
}

void KeyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

// Mouse 함수에 블록 설치 기능 추가
void Mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        int centerX = width / 2;
        int centerY = height / 2;

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

        // 모든 경우에 플레이어 눈 위치에서 광선 발사
        glm::vec3 rayOrigin = cameraPos;
        if (player) {
            rayOrigin = player->getPosition();
            rayOrigin.y += 1.0f;  // 플레이어 눈높이
        }

        Ray ray = RayCast::screenToWorldRay(centerX, centerY, width, height, view, projection, rayOrigin);

        RaycastHit hit;
        Block* hitBlock = nullptr;

        if (blockManager->raycastBlock(ray, hit, &hitBlock)) {
            if (button == GLUT_LEFT_BUTTON) {
                // 좌클릭: 블록 추가
                glm::vec3 newPos = hitBlock->getPosition() - hit.normal * Constants::BLOCK_SIZE;
                bool added = blockManager->addBlock(newPos);

                // 챌린지 모드에서 블록 색상 설정
                if (added && challengeManager && challengeManager->isChallengeMode()) {
                    GridPosition gridPos(newPos);
                    Block* newBlock = blockManager->getBlock(gridPos);

                    // 해당 위치의 타겟 블록에 맞는 색상 찾기
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
                // 우클릭: 블록 제거
                blockManager->removeBlock(hitBlock->getPosition());
            }
        }
    }

    glutPostRedisplay();
}

void PassiveMotion(int x, int y) {
    // glutWarpPointer로 발생한 이벤트 무시
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

    // 너무작은 움직임은 무시
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

    // 마우스를 화면 중앙으로 되돌림
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
    initCrosshair();
    crosshairShaderProgramID = make_crosshairShader();

    blockManager = new BlockManager();

    // 플레이어 초기화 (플레이어 시작 위치)
    player = new Player(glm::vec3(0.0f, 2.0f, 0.0f));

    // 챌린지 매니저 초기화
    challengeManager = new ChallengeManager();

    // 게임 모드 선택
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
