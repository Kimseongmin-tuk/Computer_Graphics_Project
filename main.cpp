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

// 스카이박스
GLuint skyboxVAO, skyboxVBO;
GLuint skyboxTexture;
GLuint skyboxShaderProgramID;

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

// 게임 상태 (메뉴/게임)
enum class GameState {
    MAIN_MENU,
    CHALLENGE_SELECT,
    PLAYING
};
GameState gameState = GameState::MAIN_MENU;

// UI 버튼 
struct UIButton {
    float x, y, width, height;
    std::string text;
    bool isHovered;
    
    UIButton(float _x = 0, float _y = 0, float _w = 0, float _h = 0, const std::string& _text = "")
        : x(_x), y(_y), width(_w), height(_h), text(_text), isHovered(false) {}
    
    bool contains(float mx, float my) const {
        return mx >= x && mx <= x + width && my >= y && my <= y + height;
    }
};

// 메뉴 버튼
std::vector<UIButton> mainMenuButtons;
std::vector<UIButton> challengeSelectButtons;
int hoveredButtonIndex = -1;

// 마우스 워프 발생 플래그
bool ignoreNextMouseMove = false;

// 키 상태
bool keyStates[256] = { false };
bool spacePressed = false;
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
bool gameStarted = false;  

// 현재 선택된 블록 타입
BlockType selectedBlockType = BlockType::DIRT;

// 챌린지 성공 표시 변수
bool challengeSuccessDisplayed = false;
float challengeSuccessTime = 0.0f;
const float SUCCESS_DISPLAY_DURATION = 3.0f;  // 3초간 표시 후 종료

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

// 스카이박스 버텍스 데이터 
float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
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

// 큐브맵 텍스처 로딩 함수
GLuint loadCubemap(const std::vector<std::string>& faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            std::cout << "큐브맵 면 로드 성공: " << faces[i] << std::endl;
        }
        else {
            std::cerr << "큐브맵 면 로드 실패: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

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
GLuint make_uiTextureShader();  // 텍스처 UI 셰이더 선언 
void initCube();
void drawScene();
void Reshape(int w, int h);
void KeyboardDown(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void MouseWheel(int button, int dir, int x, int y);  // 마우스 휠 함수 
void PassiveMotion(int x, int y);
void updateCamera();
void renderCube(glm::vec3 position, glm::vec3 color, BlockType type);
void drawCrosshair();  // 크로스헤어 그리기
void renderText(const std::string& text, float x, float y);  // 텍스트 렌더링
void renderHotbar();  // 핫바 렌더링
void renderChallengeUI();  // 챌린지 UI
void renderPreview();  // 완성본 미리보기
void selectGameMode();  // 게임 모드 선택
void renderStatsUI();  // 통계 렌더링
void renderMainMenu();  // 메인 메뉴 UI
void renderChallengeSelectMenu();  // 챌린지 선택 UI
void initMenuButtons();  // 메뉴 버튼 초기화
void handleMenuClick(int x, int y);  // 메뉴 클릭 처리
void updateMenuHover(int x, int y);  // 메뉴 호버 업데이트
void initSkybox();  // 스카이박스 초기화

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

// 스카이박스 셰이더 생성 - Equirectangular 텍스처용
GLuint make_skyboxShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        out vec3 WorldPos;
        
        uniform mat4 projection;
        uniform mat4 view;
        
        void main()
        {
            WorldPos = aPos;
            vec4 pos = projection * view * vec4(aPos, 1.0);
            gl_Position = pos.xyww;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 WorldPos;
        
        uniform sampler2D skyboxTexture;
        
        const float PI = 3.14159265359;
        
        vec2 SampleSphericalMap(vec3 v)
        {
            vec3 n = normalize(v);
            vec2 uv;
            uv.x = atan(n.z, n.x) / (2.0 * PI) + 0.5;
            uv.y = asin(-n.y) / PI + 0.5;
            return uv;
        }
        
        void main()
        {    
            vec2 uv = SampleSphericalMap(normalize(WorldPos));
            vec3 color = texture(skyboxTexture, uv).rgb;
            FragColor = vec4(color, 1.0);
        }
    )";

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertShader, 512, NULL, errorLog);
        std::cerr << "ERROR: skybox vertex shader 컴파일 실패\n" << errorLog << std::endl;
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragShader, 512, NULL, errorLog);
        std::cerr << "ERROR: skybox fragment shader 컴파일 실패\n" << errorLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(program, 512, NULL, errorLog);
        std::cerr << "ERROR: skybox shader program 링크 실패\n" << errorLog << std::endl;
    }

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

// 스카이박스 렌더링
void renderSkybox() {
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxShaderProgramID);

    
    glm::mat4 view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

    GLint viewLoc = glGetUniformLocation(skyboxShaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(skyboxShaderProgramID, "projection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skyboxTexture);  // 2D 텍스처 사용
    GLint texLoc = glGetUniformLocation(skyboxShaderProgramID, "skyboxTexture");
    glUniform1i(texLoc, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
    glUseProgram(shaderProgramID);
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

    glUseProgram(currentProgram);
}

// 퀵슬롯 렌더링 함수
void renderHotbar() {
    // 현재 셰더 저장
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

        // 슬롯 배경
        glUseProgram(uiShaderProgramID);

        GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
        GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
        GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
        GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");

        // 선택된 슬롯은 밝은 테두리
        if (i == static_cast<int>(selectedBlockType)) {
            glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 0.9f);  // 흰색
        }
        else {
            glUniform4f(colorLoc, 0.3f, 0.3f, 0.3f, 0.7f);  // 회색
        }

        glUniform2f(posLoc, x, startY);
        glUniform2f(sizeLoc, slotSize, slotSize);
        glUniform2f(screenSizeLoc, (float)width, (float)height);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // 슬롯 안쪽 배경
        glUniform4f(colorLoc, 0.1f, 0.1f, 0.1f, 0.8f);
        glUniform2f(posLoc, x + 4, startY + 4);
        glUniform2f(sizeLoc, slotSize - 8, slotSize - 8);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

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

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    // 셰이더 복원
    glUseProgram(currentProgram);
}

// 통계 UI 렌더링
void renderStatsUI() {
    if (!showStats) return;

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

// 메뉴 버튼 초기화
void initMenuButtons() {
    mainMenuButtons.clear();
    challengeSelectButtons.clear();
    
    // 메인 메뉴 버튼
    float buttonWidth = 300.0f;
    float buttonHeight = 60.0f;
    float centerX = width / 2.0f - buttonWidth / 2.0f;
    float startY = height / 2.0f - 50.0f;
    
    mainMenuButtons.push_back(UIButton(centerX, startY, buttonWidth, buttonHeight, "Free Build Mode"));
    mainMenuButtons.push_back(UIButton(centerX, startY + 80.0f, buttonWidth, buttonHeight, "Challenge Mode"));
    mainMenuButtons.push_back(UIButton(centerX, startY + 160.0f, buttonWidth, buttonHeight, "Exit Game"));
    
    // 챌린지 선택 버튼
    float challengeStartY = height / 2.0f - 100.0f;
    challengeSelectButtons.push_back(UIButton(centerX, challengeStartY, buttonWidth, buttonHeight, "1. First Jump (Easy)"));
    challengeSelectButtons.push_back(UIButton(centerX, challengeStartY + 80.0f, buttonWidth, buttonHeight, "2. Sky Parkour (Medium)"));
    challengeSelectButtons.push_back(UIButton(centerX, challengeStartY + 160.0f, buttonWidth, buttonHeight, "3. Extreme Parkour (Hard)"));
    challengeSelectButtons.push_back(UIButton(centerX, challengeStartY + 260.0f, buttonWidth, buttonHeight, "Back to Menu"));
}

// 버튼 렌더링 헬퍼 함수
void renderButton(const UIButton& button, bool isSelected = false) {
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(uiShaderProgramID);
    
    GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
    GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
    GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
    GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");
    
    // 버튼 배경
    if (button.isHovered || isSelected) {
        glUniform4f(colorLoc, 0.3f, 0.6f, 0.3f, 0.9f);  // 호버 시 초록색
    } else {
        glUniform4f(colorLoc, 0.2f, 0.2f, 0.2f, 0.8f);  // 기본 회색
    }
    
    glUniform2f(posLoc, button.x + button.width / 2, button.y + button.height / 2);
    glUniform2f(sizeLoc, button.width, button.height);
    glUniform2f(screenSizeLoc, (float)width, (float)height);
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // 버튼 테두리
    glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 0.9f);
    glUniform2f(sizeLoc, button.width + 4, button.height + 4);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // 내부 다시 그리기
    if (button.isHovered || isSelected) {
        glUniform4f(colorLoc, 0.3f, 0.6f, 0.3f, 0.9f);
    } else {
        glUniform4f(colorLoc, 0.2f, 0.2f, 0.2f, 0.8f);
    }
    glUniform2f(sizeLoc, button.width, button.height);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    // 버튼 텍스트
    float textX = button.x + button.width / 2 - button.text.length() * 4.5f;
    float textY = height - (button.y + button.height / 2 + 6);
    renderText(button.text, textX, textY);
    
    glUseProgram(currentProgram);
}

// 메인 메뉴 렌더링
void renderMainMenu() {
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    // 배경 어둡게
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(uiShaderProgramID);
    
    GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
    GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
    GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
    GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");
    
    // 반투명 배경
    glUniform2f(posLoc, (float)width / 2, (float)height / 2);
    glUniform2f(sizeLoc, (float)width, (float)height);
    glUniform2f(screenSizeLoc, (float)width, (float)height);
    glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 0.6f);
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    // 타이틀
    std::string title = "=== BLOCK BUILDER ===";
    renderText(title, width / 2 - 100, height - 150);
    
    std::string subtitle = "Build your world!";
    renderText(subtitle, width / 2 - 80, height - 190);
    
    // 버튼들 렌더링
    for (const auto& button : mainMenuButtons) {
        renderButton(button);
    }
    
    // 조작법 안내
    std::string controls1 = "Controls: WASD - Move, Space - Jump, Mouse - Look";
    renderText(controls1, width / 2 - 200, 80);
    
    std::string controls2 = "F - Camera Mode, 1-5 - Select Block, Q - Stats";
    renderText(controls2, width / 2 - 180, 50);
    
    glUseProgram(currentProgram);
}

// 챌린지 선택 메뉴 렌더링
void renderChallengeSelectMenu() {
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    // 배경 어둡게
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(uiShaderProgramID);
    
    GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
    GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
    GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
    GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");
    
    // 반투명 배경
    glUniform2f(posLoc, (float)width / 2, (float)height / 2);
    glUniform2f(sizeLoc, (float)width, (float)height);
    glUniform2f(screenSizeLoc, (float)width, (float)height);
    glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 0.6f);
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    // 타이틀
    std::string title = "=== SELECT CHALLENGE ===";
    renderText(title, width / 2 - 120, height - 120);
    
    // 챌린지 설명
    if (challengeManager) {
        const auto& challenges = challengeManager->getAvailableChallenges();
        float descY = height - 180;
        
        for (size_t i = 0; i < challenges.size() && i < 3; i++) {
            std::string desc = challenges[i].description;

            renderText(desc, width / 2 - 200, descY - i * 80);
        }
    }
    
    // 버튼들 렌더링
    for (const auto& button : challengeSelectButtons) {
        renderButton(button);
    }
    
    glUseProgram(currentProgram);
}

// 메뉴 호버 업데이트
void updateMenuHover(int x, int y) {
    int mouseY = y;
    
    if (gameState == GameState::MAIN_MENU) {
        for (auto& button : mainMenuButtons) {
            button.isHovered = button.contains((float)x, (float)mouseY);
        }
    }
    else if (gameState == GameState::CHALLENGE_SELECT) {
        for (auto& button : challengeSelectButtons) {
            button.isHovered = button.contains((float)x, (float)mouseY);
        }
    }
}

// 메뉴 클릭 처리
void handleMenuClick(int x, int y) {
    int mouseY = y;
    
    if (gameState == GameState::MAIN_MENU) {
        for (size_t i = 0; i < mainMenuButtons.size(); i++) {
            if (mainMenuButtons[i].contains((float)x, (float)mouseY)) {
                switch (i) {
                case 0:  // Free Build Mode
                    std::cout << "자유 모드" << std::endl;
                    challengeManager->setMode(GameMode::FREE_BUILD);
                    if (player) {
                        player->setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
                    }
                    gameState = GameState::PLAYING;
                    gameStarted = true;
                    glutSetCursor(GLUT_CURSOR_NONE);
                    glutWarpPointer(width / 2, height / 2);
                    break;
                    
                case 1:  // Challenge Mode
                    std::cout << "챌린지 모드" << std::endl;
                    gameState = GameState::CHALLENGE_SELECT;
                    break;
                    
                case 2:  // Exit Game
                    std::cout << "게임 종료 중..." << std::endl;
                    exit(0);
                    break;
                }
                return;
            }
        }
    }
    else if (gameState == GameState::CHALLENGE_SELECT) {
        for (size_t i = 0; i < challengeSelectButtons.size(); i++) {
            if (challengeSelectButtons[i].contains((float)x, (float)mouseY)) {
                if (i < 3) {
                    // 챌린지 선택 (1, 2, 3)
                    int challengeId = (int)i + 1;
                    std::cout << "Challenge " << challengeId << " selected!" << std::endl;
                    
                    challengeManager->setMode(GameMode::CHALLENGE);
                    challengeManager->loadChallenge(challengeId);
                    challengeManager->startChallenge();
                    
                    // 챌린지별 시작 위치 설정
                    if (player) {
                        switch (challengeId) {
                        case 1: // First Jump
                            player->setPosition(glm::vec3(0.0f, 6.0f, 0.0f));
                            break;
                        case 2: // Sky Parkour
                            player->setPosition(glm::vec3(0.0f, 11.0f, 0.0f));
                            break;
                        case 3: // Extreme Parkour
                            player->setPosition(glm::vec3(0.0f, 16.0f, 0.0f));
                            break;
                        }
                    }
                    
                    gameState = GameState::PLAYING;
                    gameStarted = true;
                    glutSetCursor(GLUT_CURSOR_NONE);
                    glutWarpPointer(width / 2, height / 2);
                }
                else {
                    // Back to Menu
                    std::cout << "Back to main menu" << std::endl;
                    gameState = GameState::MAIN_MENU;
                }
                return;
            }
        }
    }
}

void renderChallengeUI() {
    if (!challengeManager || !challengeManager->isChallengeMode()) return;
    if (!challengeManager->isChallengeStarted()) return;

    auto* info = challengeManager->getCurrentChallengeInfo();
    if (!info) return;

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    // ==========================================
    // 챌린지 완료 - 대형 성공 화면
    // ==========================================
    if (challengeManager->isChallengeCompleted()) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(uiShaderProgramID);

        // 반투명 초록색 배경 (전체 화면)
        GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
        GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
        GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
        GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");

        glUniform2f(posLoc, (float)width / 2, (float)height / 2);
        glUniform2f(sizeLoc, (float)width, (float)height);
        glUniform2f(screenSizeLoc, (float)width, (float)height);
        glUniform4f(colorLoc, 0.0f, 0.5f, 0.0f, 0.7f);  // 초록색 배경

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 성공 패널 (중앙 박스)
        glUniform2f(posLoc, (float)width / 2, (float)height / 2);
        glUniform2f(sizeLoc, 600.0f, 400.0f);
        glUniform4f(colorLoc, 0.1f, 0.1f, 0.1f, 0.9f);  // 검은 패널

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        // 성공 메시지
        std::string successTitle = "=== CHALLENGE COMPLETED! ===";
        renderText(successTitle, width / 2 - 200, height / 2 + 150);

        std::string challengeName = "Challenge: " + info->name;
        renderText(challengeName, width / 2 - 150, height / 2 + 100);

        int totalTime = (int)challengeManager->getChallengeTime();
        int minutes = totalTime / 60;
        int seconds = totalTime % 60;
        std::string timeText = "Time: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
        renderText(timeText, width / 2 - 100, height / 2 + 50);

        std::string scoreText = "Checkpoints: " + std::to_string(challengeManager->getCurrentScore()) +
            "/" + std::to_string(challengeManager->getTargetScore());
        renderText(scoreText, width / 2 - 100, height / 2);

        std::string exitText = "Game will exit in " +
            std::to_string((int)(SUCCESS_DISPLAY_DURATION - challengeSuccessTime + 1)) + " seconds...";
        renderText(exitText, width / 2 - 150, height / 2 - 100);

        std::string pressEscText = "Press ESC to exit now";
        renderText(pressEscText, width / 2 - 120, height / 2 - 150);

        glUseProgram(currentProgram);
        return;  // 완료 화면 표시 중에는 다른 UI 표시 안 함
    }

    // ==========================================
    // 챌린지 실패 - 대형 실패 화면
    // ==========================================
    if (challengeManager->isChallengeFailed()) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(uiShaderProgramID);

        // 반투명 빨간색 배경
        GLint posLoc = glGetUniformLocation(uiShaderProgramID, "position");
        GLint sizeLoc = glGetUniformLocation(uiShaderProgramID, "size");
        GLint screenSizeLoc = glGetUniformLocation(uiShaderProgramID, "screenSize");
        GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "color");

        glUniform2f(posLoc, (float)width / 2, (float)height / 2);
        glUniform2f(sizeLoc, (float)width, (float)height);
        glUniform2f(screenSizeLoc, (float)width, (float)height);
        glUniform4f(colorLoc, 0.5f, 0.0f, 0.0f, 0.7f);  // 빨간색 배경

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 실패 패널
        glUniform2f(posLoc, (float)width / 2, (float)height / 2);
        glUniform2f(sizeLoc, 600.0f, 300.0f);
        glUniform4f(colorLoc, 0.1f, 0.1f, 0.1f, 0.9f);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        // 실패 메시지
        std::string failTitle = "=== CHALLENGE FAILED ===";
        renderText(failTitle, width / 2 - 180, height / 2 + 100);

        std::string reason = "Time's up!";
        renderText(reason, width / 2 - 80, height / 2 + 50);

        std::string scoreText = "Progress: " + std::to_string(challengeManager->getCurrentScore()) +
            "/" + std::to_string(challengeManager->getTargetScore());
        renderText(scoreText, width / 2 - 100, height / 2);

        std::string exitText = "Press ESC to exit";
        renderText(exitText, width / 2 - 100, height / 2 - 100);

        glUseProgram(currentProgram);
        return;
    }

    // ==========================================
    // 진행 중 - 작은 UI 표시
    // ==========================================
    float timeRemaining = challengeManager->getTimeRemaining();
    float progress = challengeManager->getProgress();

    // 시간 표시
    std::string timeText = "Time: " + std::to_string((int)timeRemaining) + "s";
    renderText(timeText, 10, height - 30);

    // 진행도 표시
    std::string progressText = "Progress: " + std::to_string((int)progress) + "%";
    renderText(progressText, 10, height - 60);

    // 체크포인트 표시
    std::string checkpointText = "Checkpoints: " + std::to_string(challengeManager->getCurrentScore()) +
        "/" + std::to_string(challengeManager->getTargetScore());
    renderText(checkpointText, 10, height - 90);

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
    // UI 기반 모드 선택으로 변경 - 메뉴 상태로 시작
    gameState = GameState::MAIN_MENU;
    gameStarted = false;
    
    // 메뉴에서는 마우스 커서 보이기
    glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
    
    std::cout << "========================================" << std::endl;
    std::cout << "   BlockBuilder - 메인 메뉴   " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "게임 모드 선택 대기중" << std::endl;
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
        bool collided = blockManager->checkCollision(playerAABB);

        if (!collided && challengeManager && challengeManager->isChallengeMode()) {
            collided = challengeManager->checkPlatformCollision(playerAABB);
        }

        if (collided) {
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

    if (!player->getIsFlying()) {
        player->applyGravity(deltaTime);

        glm::vec3 playerPos = player->getPosition();
        glm::vec3 oldPos = playerPos;

        float verticalMovement = player->getVerticalVelocity() * deltaTime;
        playerPos.y += verticalMovement;

        player->setPosition(playerPos);

        AABB playerAABB = player->getAABB();
        bool collided = blockManager->checkCollision(playerAABB);

        if (!collided && challengeManager && challengeManager->isChallengeMode()) {
            collided = challengeManager->checkPlatformCollision(playerAABB);
        }

        if (collided) {
            if (player->getVerticalVelocity() < 0) {
                player->setPosition(oldPos);
                player->setVerticalVelocity(0.0f);
                player->setOnGround(true);
            }
            else {
                player->setPosition(oldPos);
                player->setVerticalVelocity(0.0f);
            }
        }
        else {
            player->setOnGround(false);
        }

        if (!player->getIsOnGround()) {
            glm::vec3 testPos = playerPos;
            testPos.y -= 0.01f;

            player->setPosition(testPos);
            AABB testAABB = player->getAABB();

            bool onGround = blockManager->checkCollision(testAABB);

            if (!onGround && challengeManager && challengeManager->isChallengeMode()) {
                onGround = challengeManager->checkPlatformCollision(testAABB);
            }

            if (onGround) {
                player->setOnGround(true);
                player->setVerticalVelocity(0.0f);
            }

            player->setPosition(playerPos);
        }
    }
    else {
        if (keyStates[' ']) {
            glm::vec3 playerPos = player->getPosition();
            glm::vec3 oldPos = playerPos;
            playerPos.y += cameraSpeed;
            player->setPosition(playerPos);

            AABB playerAABB = player->getAABB();
            bool collided = blockManager->checkCollision(playerAABB);

            if (!collided && challengeManager && challengeManager->isChallengeMode()) {
                collided = challengeManager->checkPlatformCollision(playerAABB);
            }

            if (collided) {
                player->setPosition(oldPos);
            }
        }

        glm::vec3 playerPos = player->getPosition();
        glm::vec3 oldPos = playerPos;

        glm::vec3 testPos = playerPos;
        testPos.y -= 0.05f;

        player->setPosition(testPos);
        AABB testAABB = player->getAABB();

        bool onGround = blockManager->checkCollision(testAABB);

        if (!onGround && challengeManager && challengeManager->isChallengeMode()) {
            onGround = challengeManager->checkPlatformCollision(testAABB);
        }

        if (onGround) {
            player->setFlying(false);
            player->setOnGround(true);
            player->setVerticalVelocity(0.0f);
            std::cout << "블록에 착지! 걷기 모드로 자동 전환" << std::endl;
        }

        player->setPosition(oldPos);
    }

    if (keyStates['c'] || keyStates['C']) {
        if (player->getIsFlying()) {
            glm::vec3 playerPos = player->getPosition();
            glm::vec3 oldPos = playerPos;
            playerPos.y -= cameraSpeed;
            player->setPosition(playerPos);

            AABB playerAABB = player->getAABB();
            bool collided = blockManager->checkCollision(playerAABB);

            if (!collided && challengeManager && challengeManager->isChallengeMode()) {
                collided = challengeManager->checkPlatformCollision(playerAABB);
            }

            if (collided) {
                player->setPosition(oldPos);
            }
        }
    }

    player->updateAnimation(deltaTime);
    updateCamera();
}

void drawScene() {
    // 메뉴 상태에서는 다른 렌더링
    if (gameState == GameState::MAIN_MENU || gameState == GameState::CHALLENGE_SELECT) {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 스카이박스 렌더링 (배경용)
        renderSkybox();
        
        // 메뉴 UI 렌더링
        if (gameState == GameState::MAIN_MENU) {
            renderMainMenu();
        }
        else if (gameState == GameState::CHALLENGE_SELECT) {
            renderChallengeSelectMenu();
        }
        
        glutSwapBuffers();
        return;
    }
    
    // 게임 플레이 중
    if (!gameStarted) return;
    
    processInput();
    updateCamera();

    if (challengeManager && challengeManager->isChallengeMode()) {
        glm::vec3 playerPos = player ? player->getPosition() : glm::vec3(0.0f);
        challengeManager->update(deltaTime, playerPos);

        if (playerPos.y < 0 && player) {
            challengeManager->respawnAtCheckpoint(playerPos);
            player->setPosition(playerPos);
            player->setVerticalVelocity(0.0f);
            player->setOnGround(true);
            std::cout << "Respawning at checkpoint..." << std::endl;
        }

        if (challengeManager->isChallengeCompleted()) {
            if (!challengeSuccessDisplayed) {
                challengeSuccessDisplayed = true;
                challengeSuccessTime = 0.0f;
                std::cout << "\n========================================" << std::endl;
                std::cout << "   CHALLENGE COMPLETED!   " << std::endl;
                std::cout << "========================================" << std::endl;
                std::cout << "Time: " << challengeManager->getChallengeTime() << "s" << std::endl;
                std::cout << "Score: " << challengeManager->getCurrentScore() << "/"
                    << challengeManager->getTargetScore() << std::endl;
                std::cout << "Game will exit in 3 seconds..." << std::endl;
                std::cout << "========================================\n" << std::endl;
            }
            else {
                challengeSuccessTime += deltaTime;

                if (challengeSuccessTime >= SUCCESS_DISPLAY_DURATION) {
                    std::cout << "Exiting game..." << std::endl;
                    exit(0);
                }
            }
        }
    }

    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSkybox();

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

    if (challengeManager && challengeManager->isChallengeMode()) {
        for (const auto& platformBlock : challengeManager->getPlatformBlocks()) {
            renderCube(platformBlock.position, platformBlock.color, platformBlock.type);
        }

        for (const auto& targetBlock : challengeManager->getTargetBlocks()) {
            glm::vec3 ghostColor = targetBlock.color * 0.5f + glm::vec3(0.3f);
            renderCube(targetBlock.position, ghostColor, targetBlock.type);
        }
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
    
    // 창 크기 변경 시 메뉴 버튼 위치 업데이트
    initMenuButtons();
}

void KeyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    if (key == 27) {
        if (challengeManager && challengeManager->isChallengeMode()) {
            if (challengeManager->isChallengeCompleted() ||
                challengeManager->isChallengeFailed()) {
                std::cout << "Exiting game..." << std::endl;
                exit(0);
            }
        }
        exit(0);
    }

    // 게임 플레이 중이 아니면 키 입력 무시
    if (gameState != GameState::PLAYING) return;

    if (key == ' ') {
        if (!spacePressed) {
            spacePressed = true;
            float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

            if (player) {
                player->handleSpaceTap(currentTime);

                if (!player->getIsFlying() && player->getIsOnGround()) {
                    player->jump();
                }
            }
        }
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

    if (key == ' ') {
        spacePressed = false;
    }
}

void Mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        // 메뉴 상태에서는 메뉴 클릭 처리
        if (gameState == GameState::MAIN_MENU || gameState == GameState::CHALLENGE_SELECT) {
            if (button == GLUT_LEFT_BUTTON) {
                handleMenuClick(x, y);
            }
            return;
        }
        
        // 게임 플레이 중
        if (challengeManager && challengeManager->isChallengeMode()) {
            if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
                std::cout << "챌린지 모드에서는 블록을 배치하거나 제거할 수 없습니다!" << std::endl;
                return;
            }
        }

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

            }
            else if (button == GLUT_RIGHT_BUTTON) {
                bool removed = blockManager->removeBlock(hitBlock->getPosition());
                if (removed) blocksDestroyed++;
            }
        }
    }
    glutPostRedisplay();
}

void MouseWheel(int button, int dir, int x, int y) {
    // 메뉴에서는 마우스 휠 무시
    if (gameState != GameState::PLAYING) return;
    
    const int blockTypeCount = 5;
    int currentType = static_cast<int>(selectedBlockType);
    if (dir < 0) {
        currentType++;
        if (currentType >= blockTypeCount) currentType = 0;
    }
    else {
        currentType--;
        if (currentType < 0) currentType = blockTypeCount - 1;
    }
    selectedBlockType = static_cast<BlockType>(currentType);
    const char* blockNames[] = { "DIRT", "BRICKS", "COBBLESTONE", "MUDBLOCK", "QUARTZ" };
    std::cout << "선택된 블록: " << blockNames[currentType] << std::endl;
}

void PassiveMotion(int x, int y) {
    // 메뉴 상태에서는 호버 처리
    if (gameState == GameState::MAIN_MENU || gameState == GameState::CHALLENGE_SELECT) {
        updateMenuHover(x, y);
        glutPostRedisplay();
        return;
    }
    
    // 게임 플레이 중 카메라 회전
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

void initSkybox() {
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

const int maxHeight = 20;
const float noiseScale = 0.12f;

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
    // 메뉴에서 시작하므로 커서 보이기
    glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();
    initCube();
    initCrosshair();
    initQuad();
    initSkybox();
    crosshairShaderProgramID = make_crosshairShader();
    uiShaderProgramID = make_uiShader();
    uiTextureShaderProgramID = make_uiTextureShader();
    skyboxShaderProgramID = make_skyboxShader();

    dirtTexture = loadTexture("textures/dirt.png");
    bricksTexture = loadTexture("textures/bricks.png");
    cobblestoneTexture = loadTexture("textures/cobblestone.png");
    mudblockTexture = loadTexture("textures/mudblock.png");
    quartzTexture = loadTexture("textures/quartz.png");

    // 스카이박스 텍스처 로딩
    skyboxTexture = loadTexture("textures/sky_minecraft.png");

    blockManager = new BlockManager();
    player = new Player(glm::vec3(0.0f, 15.0f, 0.0f));
    challengeManager = new ChallengeManager();
    initMenuButtons();
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
