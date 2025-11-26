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

// 마우스 움직임 무시 플래그
bool ignoreNextMouseMove = false;

// 키 상태
bool keyStates[256] = { false };
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 블록 매니저
BlockManager* blockManager = nullptr;

// 육면체 정점 데이터
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
void drawCrosshair();  // 조준선 그리기

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
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
    }

    glUseProgram(shaderID);
    return shaderID;
}

// 조준선용 간단한 셰이더 생성 함수 추가
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

// 조준선 초기화 함수 추가
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

// drawCrosshair 함수 수정 (셰이더 프로그램 끄고 그리기)
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

void updateCamera() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// 키 입력 처리 함수 추가
void processInput() {
    float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float cameraSpeed = 5.0f * deltaTime;  // 초당 5 유닛

    if (keyStates['w'] || keyStates['W']) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (keyStates['s'] || keyStates['S']) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (keyStates['a'] || keyStates['A']) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (keyStates['d'] || keyStates['D']) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (keyStates[' ']) {
        cameraPos += cameraSpeed * cameraUp;
    }
    if (keyStates['c'] || keyStates['C']) {  // C키로 아래로 이동
        cameraPos -= cameraSpeed * cameraUp;
    }
}

void drawScene() {
    processInput();

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
        renderCube(block.getPosition(), block.getColor());
    }

    // 조준선 그리기 (새로 추가)
    drawCrosshair();

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
}

void KeyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

//void Keyboard(unsigned char key, int x, int y) {
//    float cameraSpeed = 0.2f;
//
//    switch (key) {
//    case 'w': case 'W':
//        cameraPos += cameraSpeed * cameraFront;
//        break;
//    case 's': case 'S':
//        cameraPos -= cameraSpeed * cameraFront;
//        break;
//    case 'a': case 'A':
//        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
//        break;
//    case 'd': case 'D':
//        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
//        break;
//    case ' ':
//        cameraPos += cameraSpeed * cameraUp;
//        break;
//    case 27:
//        exit(0);
//        break;
//    }
//
//    glutPostRedisplay();
//}

// Mouse 함수에 디버그 출력 추가
void Mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        int centerX = width / 2;
        int centerY = height / 2;

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

        Ray ray = RayCast::screenToWorldRay(centerX, centerY, width, height, view, projection, cameraPos);

        RaycastHit hit;
        Block* hitBlock = nullptr;

        if (blockManager->raycastBlock(ray, hit, &hitBlock)) {
            if (button == GLUT_LEFT_BUTTON) {
                glm::vec3 newPos = hitBlock->getPosition() - hit.normal * Constants::BLOCK_SIZE;
                blockManager->addBlock(newPos);
            }
            else if (button == GLUT_RIGHT_BUTTON) {
                blockManager->removeBlock(hitBlock->getPosition());
            }
        }
    }

    glutPostRedisplay();
}

void PassiveMotion(int x, int y) {
    // glutWarpPointer로 인한 이벤트 무시
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

    // 움직임이 너무 작으면 무시
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

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(KeyboardDown);      // 수정
    glutKeyboardUpFunc(KeyboardUp);      // 추가
    glutMouseFunc(Mouse);
    glutPassiveMotionFunc(PassiveMotion);
    glutIdleFunc(Idle);                  // 추가

    glutMainLoop();
}