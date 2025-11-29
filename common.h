#ifndef COMMON_H
#define COMMON_H

#include <gl/glew.h>
#include <gl/freeglut.h>
/*
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
*/

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>

namespace Constants {
    const unsigned int SCREEN_WIDTH = 1280;
    const unsigned int SCREEN_HEIGHT = 720;
    const float BLOCK_SIZE = 1.0f;
    const float CAMERA_SPEED = 5.0f;
    const float MOUSE_SENSITIVITY = 0.1f;
    const float MAX_RAYCAST_DISTANCE = 10.0f;
}

namespace Utils {
    inline void printVec3(const glm::vec3& v, const std::string& name = "") {
        if (!name.empty()) {
            std::cout << name << ": ";
        }
        std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
    }
}

#endif // COMMON_H