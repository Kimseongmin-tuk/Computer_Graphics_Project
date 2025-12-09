#include "ChallengeManager.h"
#include <iostream>

ChallengeManager::ChallengeManager()
    : currentMode(GameMode::FREE_BUILD), currentChallenge(nullptr),
    challengeTime(0.0f), maxTime(300.0f),
    challengeStarted(false), challengeCompleted(false), challengeFailed(false),
    currentScore(0), checkpointsReached(0), previewRotation(0.0f),
    lastCheckpoint(0.0f, 0.0f, 0.0f) {
    initializeChallenges();
}

ChallengeManager::~ChallengeManager() {
    if (currentChallenge) {
        delete currentChallenge;
    }
}

void ChallengeManager::initializeChallenges() {
    availableChallenges.clear();

    // 파쿠르 챌린지만 유지
    availableChallenges.push_back(ChallengeInfo(
        1, "First Jump",
        "점프의 기초를 배워보세요! 간단한 플랫폼을 뛰어넘으세요.",
        ChallengeType::PARKOUR, ChallengeDifficulty::EASY, 60.0f, 3
    ));

    availableChallenges.push_back(ChallengeInfo(
        2, "Sky Parkour",
        "하늘 높이 솟은 플랫폼들을 건너세요! 떨어지면 체크포인트로 돌아갑니다.",
        ChallengeType::PARKOUR, ChallengeDifficulty::MEDIUM, 120.0f, 8
    ));

    availableChallenges.push_back(ChallengeInfo(
        3, "Extreme Parkour",
        "극한의 점프 실력을 시험하세요! 정밀한 조작이 필요합니다.",
        ChallengeType::PARKOUR, ChallengeDifficulty::HARD, 180.0f, 12
    ));
}

void ChallengeManager::setMode(GameMode mode) {
    currentMode = mode;
    if (mode == GameMode::CHALLENGE) {
        reset();
    }
}

void ChallengeManager::loadChallenge(int challengeId) {
    reset();

    // 챌린지 정보 찾기
    for (const auto& challenge : availableChallenges) {
        if (challenge.id == challengeId) {
            if (currentChallenge) delete currentChallenge;
            currentChallenge = new ChallengeInfo(challenge);
            maxTime = challenge.timeLimit;
            break;
        }
    }

    if (!currentChallenge) {
        std::cout << "챌린지를 찾을 수 없습니다!" << std::endl;
        return;
    }

    // 파쿠르 챌린지만 지원
    setupParkourChallenge(challengeId);

    std::cout << "챌린지 로드 완료: " << currentChallenge->name << std::endl;
}

void ChallengeManager::setupParkourChallenge(int id) {
    targetBlocks.clear();
    platformBlocks.clear();
    checkpoints.clear();

    switch (id) {
    case 1: { // First Jump - 쉬운 파쿠르
        // 시작 플랫폼
        for (int x = -2; x <= 2; x++) {
            for (int z = -2; z <= 2; z++) {
                platformBlocks.push_back(ChallengeBlock(
                    glm::vec3(x, 5, z), BlockType::COBBLESTONE, glm::vec3(0.5f, 0.5f, 0.5f)
                ));
            }
        }

        // 체크포인트 1 (2블록 점프)
        for (int x = 4; x <= 6; x++) {
            for (int z = -1; z <= 1; z++) {
                platformBlocks.push_back(ChallengeBlock(
                    glm::vec3(x, 5, z), BlockType::BRICKS, glm::vec3(0.8f, 0.3f, 0.3f)
                ));
            }
        }
        checkpoints.push_back(glm::vec3(5, 6, 0));

        // 체크포인트 2 (3블록 점프)
        for (int x = 9; x <= 11; x++) {
            for (int z = -1; z <= 1; z++) {
                platformBlocks.push_back(ChallengeBlock(
                    glm::vec3(x, 5, z), BlockType::BRICKS, glm::vec3(0.3f, 0.8f, 0.3f)
                ));
            }
        }
        checkpoints.push_back(glm::vec3(10, 6, 0));

        // 골 플랫폼
        for (int x = 14; x <= 16; x++) {
            for (int z = -1; z <= 1; z++) {
                platformBlocks.push_back(ChallengeBlock(
                    glm::vec3(x, 6, z), BlockType::QUARTZ, glm::vec3(0.3f, 0.3f, 0.8f)
                ));
            }
        }
        checkpoints.push_back(glm::vec3(15, 7, 0));

        lastCheckpoint = glm::vec3(0, 6, 0);
        break;
    }

    case 2: { // Sky Parkour - 중간 난이도
        // 시작 플랫폼
        for (int x = -1; x <= 1; x++) {
            for (int z = -1; z <= 1; z++) {
                platformBlocks.push_back(ChallengeBlock(
                    glm::vec3(x, 10, z), BlockType::COBBLESTONE
                ));
            }
        }
        checkpoints.push_back(glm::vec3(0, 11, 0));
        lastCheckpoint = glm::vec3(0, 11, 0);

        // 지그재그 플랫폼 생성
        int currentX = 3;
        int currentZ = 0;
        int currentY = 10;

        for (int i = 0; i < 7; i++) {
            // 플랫폼 생성
            for (int x = 0; x < 2; x++) {
                for (int z = 0; z < 2; z++) {
                    platformBlocks.push_back(ChallengeBlock(
                        glm::vec3(currentX + x, currentY, currentZ + z),
                        BlockType::BRICKS
                    ));
                }
            }

            // 체크포인트
            if (i % 2 == 1) {
                checkpoints.push_back(glm::vec3(currentX + 0.5f, currentY + 1, currentZ + 0.5f));
            }

            // 다음 위치 계산 (지그재그)
            currentX += 3;
            currentZ = (i % 2 == 0) ? 3 : -3;
            currentY += (i % 3 == 0) ? 1 : 0;  // 가끔 높이 증가
        }

        // 골 플랫폼
        for (int x = 0; x < 3; x++) {
            for (int z = 0; z < 3; z++) {
                platformBlocks.push_back(ChallengeBlock(
                    glm::vec3(currentX + 3 + x, currentY + 2, z - 1),
                    BlockType::QUARTZ, glm::vec3(1.0f, 0.84f, 0.0f)
                ));
            }
        }
        checkpoints.push_back(glm::vec3(currentX + 4.5f, currentY + 3, 0.5f));
        break;
    }

    case 3: { // Extreme Parkour - 어려움
        // 시작
        platformBlocks.push_back(ChallengeBlock(glm::vec3(0, 15, 0), BlockType::COBBLESTONE));
        checkpoints.push_back(glm::vec3(0, 16, 0));
        lastCheckpoint = glm::vec3(0, 16, 0);

        // 1블록 점프 연속
        for (int i = 1; i <= 10; i++) {
            platformBlocks.push_back(ChallengeBlock(
                glm::vec3(i * 2, 15 + (i % 3), 0), BlockType::BRICKS
            ));
            if (i % 3 == 0) {
                checkpoints.push_back(glm::vec3(i * 2, 16 + (i % 3), 0));
            }
        }

        // 나선형 구조
        float angle = 0.0f;
        float radius = 5.0f;
        int currentY = 18;

        for (int i = 0; i < 12; i++) {
            float x = 20 + radius * cos(angle);
            float z = radius * sin(angle);
            platformBlocks.push_back(ChallengeBlock(
                glm::vec3(x, currentY, z), BlockType::MUDBLOCK
            ));

            if (i % 3 == 2) {
                checkpoints.push_back(glm::vec3(x, currentY + 1, z));
            }

            angle += 0.5f;
            currentY += 0.5f;
        }

        // 골
        platformBlocks.push_back(ChallengeBlock(glm::vec3(20, currentY + 2, 0),
            BlockType::QUARTZ, glm::vec3(1.0f, 0.84f, 0.0f)));
        checkpoints.push_back(glm::vec3(20, currentY + 3, 0));
        break;
    }
    }
}

void ChallengeManager::startChallenge() {
    challengeStarted = true;
    challengeTime = 0.0f;
    challengeCompleted = false;
    challengeFailed = false;
    currentScore = 0;
    checkpointsReached = 0;

    std::cout << "챌린지 시작: " << currentChallenge->name << std::endl;
    std::cout << "제한 시간: " << maxTime << "초" << std::endl;
}

void ChallengeManager::update(float deltaTime, const glm::vec3& playerPos) {
    if (!challengeStarted || challengeCompleted || challengeFailed) return;

    challengeTime += deltaTime;
    previewRotation += deltaTime * 30.0f;
    if (previewRotation > 360.0f) {
        previewRotation -= 360.0f;
    }

    // 시간 체크
    if (challengeTime >= maxTime) {
        challengeFailed = true;
        std::cout << "시간 초과! 챌린지 실패" << std::endl;
        return;
    }

    // 체크포인트 체크
    checkCheckpoint(playerPos);

    // 추락 체크 (y < 0)
    if (playerPos.y < 0) {
        std::cout << "추락! 마지막 체크포인트로 이동" << std::endl;
        // 리스폰은 main에서 처리
    }
}

bool ChallengeManager::checkCheckpoint(const glm::vec3& playerPos) {
    if (checkpointsReached >= checkpoints.size()) {
        // 모든 체크포인트 도달 = 완료
        if (!challengeCompleted) {
            challengeCompleted = true;
            std::cout << "챌린지 완료! 시간: " << challengeTime << "초" << std::endl;
        }
        return false;
    }

    glm::vec3 targetCheckpoint = checkpoints[checkpointsReached];
    float distance = glm::length(playerPos - targetCheckpoint);

    if (distance < 1.5f) {  // 체크포인트 반경 1.5블록
        checkpointsReached++;
        currentScore = checkpointsReached;
        lastCheckpoint = targetCheckpoint;
        std::cout << "체크포인트 " << checkpointsReached << "/" << checkpoints.size() << " 도달!" << std::endl;
        return true;
    }

    return false;
}

void ChallengeManager::respawnAtCheckpoint(glm::vec3& playerPos) {
    playerPos = lastCheckpoint;
}

bool ChallengeManager::checkPlatformCollision(const AABB& playerAABB) const {
    // 모든 플랫폼 블록과 충돌 체크
    for (const auto& platform : platformBlocks) {
        // 플랫폼 블록의 AABB 생성
        glm::vec3 blockMin = platform.position - glm::vec3(0.5f);
        glm::vec3 blockMax = platform.position + glm::vec3(0.5f);
        AABB blockAABB(blockMin, blockMax);

        // AABB 충돌 검사
        if (playerAABB.min.x < blockAABB.max.x && playerAABB.max.x > blockAABB.min.x &&
            playerAABB.min.y < blockAABB.max.y && playerAABB.max.y > blockAABB.min.y &&
            playerAABB.min.z < blockAABB.max.z && playerAABB.max.z > blockAABB.min.z) {
            return true;  // 충돌 발생
        }
    }

    return false;  // 충돌 없음
}

float ChallengeManager::getProgress() const {
    if (!currentChallenge) return 0.0f;

    if (currentChallenge->targetScore == 0) return 0.0f;

    return (float)currentScore / (float)currentChallenge->targetScore * 100.0f;
}

int ChallengeManager::getTargetScore() const {
    return currentChallenge ? currentChallenge->targetScore : 0;
}

std::string ChallengeManager::getChallengeTypeString(ChallengeType type) const {
    switch (type) {
    case ChallengeType::PARKOUR: return "Parkour";
    case ChallengeType::BUILD: return "Build";
    case ChallengeType::COLLECTION: return "Collection";
    case ChallengeType::TIME_TRIAL: return "Time Trial";
    default: return "Unknown";
    }
}

std::string ChallengeManager::getDifficultyString(ChallengeDifficulty diff) const {
    switch (diff) {
    case ChallengeDifficulty::EASY: return "Easy";
    case ChallengeDifficulty::MEDIUM: return "Medium";
    case ChallengeDifficulty::HARD: return "Hard";
    case ChallengeDifficulty::EXPERT: return "Expert";
    default: return "Unknown";
    }
}

void ChallengeManager::reset() {
    challengeTime = 0.0f;
    challengeStarted = false;
    challengeCompleted = false;
    challengeFailed = false;
    currentScore = 0;
    checkpointsReached = 0;
    previewRotation = 0.0f;
    targetBlocks.clear();
    platformBlocks.clear();
    checkpoints.clear();
    lastCheckpoint = glm::vec3(0.0f);
}