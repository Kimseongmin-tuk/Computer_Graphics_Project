#include "ChallengeManager.h"
#include "BlockManager.h"

ChallengeManager::ChallengeManager()
    : currentMode(GameMode::FREE_BUILD), challengeTime(0.0f), maxTime(300.0f),
    challengeStarted(false), challengeCompleted(false), previewRotation(0.0f) {
}

void ChallengeManager::setMode(GameMode mode) {
    currentMode = mode;
    if (mode == GameMode::CHALLENGE) {
        reset();
    }
}

void ChallengeManager::startChallenge() {
    challengeStarted = true;
    challengeTime = 0.0f;
    challengeCompleted = false;
}

void ChallengeManager::update(float deltaTime) {
    if (!challengeStarted || challengeCompleted) return;

    challengeTime += deltaTime;
    previewRotation += deltaTime * 30.0f;
    if (previewRotation > 360.0f) {
        previewRotation -= 360.0f;
    }

    if (challengeTime >= maxTime) {
        std::cout << "챌린지 시간 종료!" << std::endl;
        challengeCompleted = true;
    }
}

void ChallengeManager::loadChallenge(int challengeNumber) {
    targetBlocks.clear();

    switch (challengeNumber) {
    case 1:
        // 1번: 간단한 탑 (Easy) - 기본 블록 위에 놓임
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 1, 0), glm::vec3(0.7f, 0.3f, 0.3f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 2, 0), glm::vec3(0.3f, 0.7f, 0.3f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 3, 0), glm::vec3(0.3f, 0.3f, 0.7f)));
        maxTime = 60.0f;
        break;

    case 2:
        // 2번: 피라미드 (Medium) - 기본 블록 위에 놓임
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 1, 0), glm::vec3(0.8f, 0.2f, 0.2f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(1, 1, 0), glm::vec3(0.8f, 0.2f, 0.2f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 1, 1), glm::vec3(0.8f, 0.2f, 0.2f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(1, 1, 1), glm::vec3(0.8f, 0.2f, 0.2f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 2, 0), glm::vec3(0.2f, 0.8f, 0.2f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(1, 2, 0), glm::vec3(0.2f, 0.8f, 0.2f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 2, 1), glm::vec3(0.2f, 0.8f, 0.2f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(1, 2, 1), glm::vec3(0.2f, 0.8f, 0.2f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 3, 0), glm::vec3(0.2f, 0.2f, 0.8f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(1, 3, 0), glm::vec3(0.2f, 0.2f, 0.8f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 3, 1), glm::vec3(0.2f, 0.2f, 0.8f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(1, 3, 1), glm::vec3(0.2f, 0.2f, 0.8f)));
        maxTime = 180.0f;
        break;

    case 3:
        // 3번: 계단 구조 (Hard) - 기본 블록 위에 놓임
        for (int i = 0; i < 5; i++) {
            targetBlocks.push_back(ChallengeBlock(glm::vec3(i, i + 1, 0), glm::vec3(0.9f, 0.6f, 0.2f)));
        }
        maxTime = 120.0f;
        break;

    default:
        // 기본값: 간단한 탑 - 기본 블록 위에 놓임
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 1, 0), glm::vec3(0.7f, 0.3f, 0.3f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 2, 0), glm::vec3(0.3f, 0.7f, 0.3f)));
        targetBlocks.push_back(ChallengeBlock(glm::vec3(0, 3, 0), glm::vec3(0.3f, 0.3f, 0.7f)));
        maxTime = 60.0f;
        break;
    }
}

float ChallengeManager::getProgress(const std::unordered_map<GridPosition, Block>& playerBlocks) const {
    if (targetBlocks.empty()) return 0.0f;

    int correctBlocks = 0;
    for (const auto& target : targetBlocks) {
        GridPosition gridPos(target.position);
        auto it = playerBlocks.find(gridPos);

        if (it != playerBlocks.end()) {
            const Block& block = it->second;
            glm::vec3 blockColor = block.getColor();
            glm::vec3 targetColor = target.color;

            float colorDiff = glm::length(blockColor - targetColor);
            if (colorDiff < 0.1f) {
                correctBlocks++;
            }
        }
    }

    float progress = (float)correctBlocks / (float)targetBlocks.size() * 100.0f;

    if (progress >= 100.0f && !challengeCompleted) {
        const_cast<ChallengeManager*>(this)->challengeCompleted = true;
        std::cout << "챌린지 완료!" << std::endl;
    }

    return progress;
}

void ChallengeManager::reset() {
    challengeTime = 0.0f;
    challengeStarted = false;
    challengeCompleted = false;
    previewRotation = 0.0f;
    targetBlocks.clear();
}
