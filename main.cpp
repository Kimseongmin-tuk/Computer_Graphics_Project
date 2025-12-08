#define _CRT_SECURE_NO_WARNINGS 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

// 텍스처
GLuint dirtTexture;
GLuint bricksTexture;
GLuint cobblestoneTexture;
GLuint mudblockTexture;
GLuint quartzTexture;

GLuint quadVAO, quadVBO;
GLuint uiShaderProgramID;
GLuint uiTextureShaderProgramID;  // UI

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

// 통계 정보
int blocksPlaced = 0;
int blocksDestroyed = 0;
float totalPlayTime = 0.0f;
bool showStats = false;

// 게임 시작 여부
bool gameStarted = true;  // 임시로 true로 설정 

// 현재 선택된 블록 타입
BlockType selectedBlockType = BlockType::DIRT;

// 정육면체 버텍스 데이터
// 육면체 정점 데이터 (위치, 법선, UV)
float cubeVertices[] = {
    // 앞면 
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

    // 뒷면 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,

    // 왼쪽면
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,

    // 오른쪽면 
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

    // 아래면 
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    // 윗면
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

// 텍스처 로딩 함수
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 텍스처 필터링 설정 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
        std::cout << "텍스처 로드 성공: " << path << std::endl;
    }
    else {
        std::cerr << "텍스처 로드 실패: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// BlockType에 따라 텍스처 반환
GLuint getTextureForBlockType(BlockType type) {
    switch (type) {
    case BlockType::DIRT:
        return dirtTexture;
    case BlockType::BRICKS:
        return bricksTexture;
    case BlockType::COBBLESTONE:
        return cobblestoneTexture;
    case BlockType::MUDBLOCK:
        return mudblockTexture;
    case BlockType::QUARTZ:
        return quartzTexture;
    default:
        return dirtTexture;
    }
}

// 함수 선언
char* filetobuf(const char* file);
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLuint make_uiTextureShader();  // 텍스처 UI 셰이더 선언 추가
void initCube();
void drawScene();
void Reshape(int w, int h);
void KeyboardDown(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void MouseWheel(int button, int dir, int x, int y);  // 마우스 휠 함수 추가
void PassiveMotion(int x, int y);
void updateCamera();
void renderCube(glm::vec3 position, glm::vec3 color, BlockType type);
void drawCrosshair();  // 크로스헤어 그리기
void renderText(const std::string& text, float x, float y);  // 텍스트 렌더링
void renderHotbar();  // 핫바 렌더링 추가
void renderChallengeUI();  // 챌린지 UI
void renderPreview();  // 완성본 미리보기
void selectGameMode();  // 게임 모드 선택
void renderStatsUI();  // 통계 렌더링

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

    // 위치 속성 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 법선 속성 (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UV 좌표 속성 (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

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

// UI 쿼드 초기화
void initQuad() {
    float quadVertices[] = {
        // 위치 (x, y)
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

// UI 셰이더 생성
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

// 텍스처를 가진 UI 셰이더 생성
GLuint make_uiTextureShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        out vec2 TexCoord;
        uniform vec2 position;
        uniform vec2 size;
        uniform vec2 screenSize;
        void main()
        {
            vec2 pos = (aPos * size + position) / screenSize * 2.0 - 1.0;
            pos.y = -pos.y;
            gl_Position = vec4(pos, 0.0, 1.0);
            TexCoord = (aPos + 1.0) * 0.5;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D uiTexture;
        void main()
        {
            FragColor = texture(uiTexture, TexCoord);
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

void renderCube(glm::vec3 position, glm::vec3 color, BlockType type) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // 텍스처 사용 설정
    GLint useTextureLoc = glGetUniformLocation(shaderProgramID, "useTexture");
    glUniform1i(useTextureLoc, 1);  // true - 텍스처 사용

    // BlockType에 따라 텍스처 선택
    GLuint texture = getTextureForBlockType(type);
    
    // 텍스처 바인딩
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLint texLoc = glGetUniformLocation(shaderProgramID, "blockTexture");
    glUniform1i(texLoc, 0);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// 박스 렌더링
void renderBox(const glm::mat4& modelMatrix, const glm::vec3& color) {
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // 단색 사용 설정
    GLint useTextureLoc = glGetUniformLocation(shaderProgramID, "useTexture");
    glUniform1i(useTextureLoc, 0);  

    // 단색 설정
    GLint solidColorLoc = glGetUniformLocation(shaderProgramID, "solidColor");
    glUniform3fv(solidColorLoc, 1, glm::value_ptr(color));

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

// 텍스처 렌더링 함수
void renderTexture(GLuint textureID, float x, float y, float width, float height) {
    glUseProgram(uiShaderProgramID);

    // 사각형 위치 및 크기 설정
    GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
    GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
    GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
    GLint texLoc = glGetUniformLocation(uiShaderProgramID, "texture1");

    glUniform2f(posLoc, x, y);
    glUniform2f(sizeLoc, width, height);
    glUniform2f(screenSizeLoc, (float)width, (float)height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(texLoc, 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// 텍스트 렌더링 함수
void renderText(const std::string& text, float x, float y) {
    // 현재 셰이더 저장
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    glDisable(GL_DEPTH_TEST);
    glUseProgram(0); 
    
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
    
    // 이전 셰이더 복원
    glUseProgram(currentProgram);
}

// 퀵슬롯 렌더링 함수
void renderHotbar() {
    // 현재 셰이더 저장
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float slotSize = 64.0f;
    const float slotSpacing = 64.0f;  
    const float totalWidth = (slotSize * 5) + (slotSpacing * 4);
    const float startX = (width - totalWidth) / 2.0f;
    const float startY = 20.0f;

    // 각 슬롯 렌더링
    for (int i = 0; i < 5; i++) {
        float x = startX + i * (slotSize + slotSpacing);
        
        // 슬롯 배경 (회색 테두리)
        glUseProgram(uiShaderProgramID);
        
        GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
        GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
        GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
        GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");
        
        // 선택된 슬롯은 밝은 테두리
        if (i == static_cast<int>(selectedBlockType)) {
            glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 0.9f);  // 흰색
        } else {
            glUniform4f(colorLoc, 0.3f, 0.3f, 0.3f, 0.7f);  // 회색
        }
        
        glUniform2f(posLoc, x, startY);
        glUniform2f(sizeLoc, slotSize, slotSize);
        glUniform2f(screenSizeLoc, (float)width, (float)height);
        
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // 슬롯 안쪽 배경
        glUniform4f(colorLoc, 0.1f, 0.1f, 0.1f, 0.8f);
        glUniform2f(posLoc, x + 4, startY + 4);
        glUniform2f(sizeLoc, slotSize - 8, slotSize - 8);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // 블록 텍스처 렌더링
        glUseProgram(uiTextureShaderProgramID);
        
        posLoc = glGetUniformLocation(uiTextureShaderProgramID, "position");
        sizeLoc = glGetUniformLocation(uiTextureShaderProgramID, "size");
        screenSizeLoc = glGetUniformLocation(uiTextureShaderProgramID, "screenSize");
        GLint texLoc = glGetUniformLocation(uiTextureShaderProgramID, "uiTexture");
        
        glUniform2f(posLoc, x + 8, startY + 8);
        glUniform2f(sizeLoc, slotSize - 16, slotSize - 16);
        glUniform2f(screenSizeLoc, (float)width, (float)height);
        
        // 블록 타입에 따라 텍스처 선택
        GLuint texture = getTextureForBlockType(static_cast<BlockType>(i));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(texLoc, 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    // 셰이더 복원
    glUseProgram(currentProgram);
}

// 통계 UI 렌더링
void renderStatsUI() {
    if (!showStats) return;

    // 현재 셰이더 저장
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    // 반투명 검은 배경
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(uiShaderProgramID);

    GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
    GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
    GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
    GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");

    glUniform2f(posLoc, (float)width / 2, (float)height / 2);
    glUniform2f(sizeLoc, 200.0f, 150.0f);
    glUniform2f(screenSizeLoc, (float)width, (float)height);
    glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 0.7f);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    // 통계 텍스트 표시
    std::string title = "=== STATISTICS ===";
    renderText(title, width / 2 - 80, height / 2 + 100);

    int minutes = (int)(totalPlayTime / 60.0f);
    int seconds = (int)totalPlayTime % 60;
    std::string timeText = "Play Time: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    renderText(timeText, width / 2 - 80, height / 2 + 50);

    std::string placedText = "Blocks Placed: " + std::to_string(blocksPlaced);
    renderText(placedText, width / 2 - 80, height / 2 + 20);

    std::string destroyedText = "Blocks Destroyed: " + std::to_string(blocksDestroyed);
    renderText(destroyedText, width / 2 - 80, height / 2 - 10);

    int currentBlocks = blockManager->getBlockCount() - 25;
    if (currentBlocks < 0) currentBlocks = 0;
    std::string currentText = "Current Blocks: " + std::to_string(currentBlocks);
    renderText(currentText, width / 2 - 80, height / 2 - 40);

    std::string closeText = "Press Q to close";
    renderText(closeText, width / 2 - 80, height / 2 - 100);
    
    // 셰이더 복원
    glUseProgram(currentProgram);
}

void updateSelectedBlockType(int direction) {
    const int blockTypeCount = 5;
    int currentType = static_cast<int>(selectedBlockType);
    currentType += direction;
    if (currentType >= blockTypeCount) currentType = 0;
    else if (currentType < 0) currentType = blockTypeCount - 1;
    selectedBlockType = static_cast<BlockType>(currentType);
    const char* blockNames[] = { "DIRT", "BRICKS", "COBBLESTONE", "MUDBLOCK", "QUARTZ" };
    std::cout << "선택된 블록: " << blockNames[currentType] << std::endl;
}

void renderChallengeUI() {
    if (!challengeManager || !challengeManager->isChallengeMode()) return;
    if (!challengeManager->isChallengeStarted()) return;
    float timeRemaining = challengeManager->getTimeRemaining();
    float progress = challengeManager->getProgress(blockManager->getAllBlocks());
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    std::string timeText = "Time: " + std::to_string((int)timeRemaining) + "s";
    renderText(timeText, 10, height - 30);
    std::string progressText = "Progress: " + std::to_string((int)progress) + "%";
    renderText(progressText, 10, height - 60);
    if (challengeManager->isChallengeCompleted()) {
        std::string completedText = "CHALLENGE COMPLETED!";
        renderText(completedText, width / 2 - 100, height / 2);
    }
    glUseProgram(currentProgram);
}

void renderPreview() {
    if (!challengeManager || !challengeManager->isChallengeMode()) return;
    if (!challengeManager->isChallengeStarted()) return;
    glViewport(width - 250, height - 250, 200, 200);
    float rotation = challengeManager->getPreviewRotation();
    float rad = glm::radians(rotation);
    float camX = 5.0f * cos(rad);
    float camZ = 5.0f * sin(rad);
    glm::mat4 previewView = glm::lookAt(glm::vec3(camX, 5.0f, camZ), glm::vec3(0.5f, 1.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 previewProjection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(previewView));
    GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(previewProjection));
    for (const auto& targetBlock : challengeManager->getTargetBlocks()) {
        renderCube(targetBlock.position, targetBlock.color, BlockType::DIRT);
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
    } else if (choice == 2) {
        std::cout << "Challenge Mode selected!" << std::endl;
        challengeManager->setMode(GameMode::CHALLENGE);
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
    } else {
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
        } else if (cameraMode == CameraMode::THIRD_PERSON) {
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
    if (!showStats && challengeManager && challengeManager->getMode() == GameMode::FREE_BUILD) {
        totalPlayTime += deltaTime;
    }
    if (!player) return;
    bool isMoving = false;
    glm::vec3 moveDirection(0.0f);
    glm::vec3 forward = cameraFront;
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f) forward = glm::normalize(forward);
    glm::vec3 right = glm::cross(forward, cameraUp);
    if (glm::length(right) > 0.0f) right = glm::normalize(right);
    if (keyStates['w'] || keyStates['W']) { moveDirection += forward; isMoving = true; }
    if (keyStates['s'] || keyStates['S']) { moveDirection -= forward; isMoving = true; }
    if (keyStates['a'] || keyStates['A']) { moveDirection -= right; isMoving = true; }
    if (keyStates['d'] || keyStates['D']) { moveDirection += right; isMoving = true; }
    if (isMoving && glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
        glm::vec3 playerPos = player->getPosition();
        glm::vec3 oldPos = playerPos;
        float currentY = playerPos.y;
        playerPos += moveDirection * cameraSpeed;
        playerPos.y = currentY;
        player->setPosition(playerPos);
        AABB playerAABB = player->getAABB();
        if (blockManager->checkCollision(playerAABB)) {
            player->setPosition(oldPos);
            player->setWalking(false);
        } else {
            player->setWalking(true);
        }
    } else {
        player->setWalking(false);
    }
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
    player->updateAnimation(deltaTime);
    updateCamera();
}

void drawScene() {
    if (!gameStarted) return;
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
        renderCube(block.getPosition(), block.getColor(), block.getType());
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
    renderStatsUI();
    renderChallengeUI();
    renderHotbar();
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
    if (key == 27) exit(0);
    if (key == 'f' || key == 'F') {
        if (cameraMode == CameraMode::FIRST_PERSON) {
            cameraMode = CameraMode::THIRD_PERSON;
            std::cout << "3인칭 모드" << std::endl;
        } else {
            cameraMode = CameraMode::FIRST_PERSON;
            std::cout << "1인칭 모드" << std::endl;
        }
    }
    if ((key == 'q' || key == 'Q') && challengeManager && challengeManager->getMode() == GameMode::FREE_BUILD) {
        showStats = !showStats;
        if (showStats) std::cout << "Statistics displayed" << std::endl;
        else std::cout << "Statistics hidden" << std::endl;
    }
    if (key >= '1' && key <= '5') {
        int index = key - '1';
        selectedBlockType = static_cast<BlockType>(index);
        const char* blockNames[] = { "Dirt", "Bricks", "Cobblestone", "Mudblock", "Quartz" };
        std::cout << "선택된 블록: " << blockNames[index] << std::endl;
    }
}

void KeyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void Mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
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
                bool added = blockManager->addBlock(newPos, selectedBlockType);
                if (added) blocksPlaced++;
                if (added && challengeManager && challengeManager->isChallengeMode()) {
                    GridPosition gridPos(newPos);
                    Block* newBlock = blockManager->getBlock(gridPos);
                    for (const auto& targetBlock : challengeManager->getTargetBlocks()) {
                        if (glm::length(targetBlock.position - newPos) < 0.1f) {
                            if (newBlock) newBlock->setColor(targetBlock.color);
                            break;
                        }
                    }
                }
            } else if (button == GLUT_RIGHT_BUTTON) {
                bool removed = blockManager->removeBlock(hitBlock->getPosition());
                if (removed) blocksDestroyed++;
            }
        }
    }
    glutPostRedisplay();
}

void MouseWheel(int button, int dir, int x, int y) {
    const int blockTypeCount = 5;
    int currentType = static_cast<int>(selectedBlockType);
    if (dir > 0) {
        currentType++;
        if (currentType >= blockTypeCount) currentType = 0;
    } else {
        currentType--;
        if (currentType < 0) currentType = blockTypeCount - 1;
    }
    selectedBlockType = static_cast<BlockType>(currentType);
    const char* blockNames[] = { "DIRT", "BRICKS", "COBBLESTONE", "MUDBLOCK", "QUARTZ" };
    std::cout << "선택된 블록: " << blockNames[currentType] << std::endl;
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
    if (abs(xoffset) < 1 && abs(yoffset) < 1) return;
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

int main(int argc, char** argv) {
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
    initQuad();
    crosshairShaderProgramID = make_crosshairShader();
    uiShaderProgramID = make_uiShader();
    uiTextureShaderProgramID = make_uiTextureShader();
    dirtTexture = loadTexture("textures/dirt.png");
    bricksTexture = loadTexture("textures/bricks.png");
    cobblestoneTexture = loadTexture("textures/cobblestone.png");
    mudblockTexture = loadTexture("textures/mudblock.png");
    quartzTexture = loadTexture("textures/quartz.png");
    blockManager = new BlockManager();
    player = new Player(glm::vec3(0.0f, 2.0f, 0.0f));
    challengeManager = new ChallengeManager();
    selectGameMode();
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(KeyboardDown);
    glutKeyboardUpFunc(KeyboardUp);
    glutMouseFunc(Mouse);
    glutMouseWheelFunc(MouseWheel);
    glutPassiveMotionFunc(PassiveMotion);
    glutIdleFunc(Idle);
    glutMainLoop();
    return 0;
}
