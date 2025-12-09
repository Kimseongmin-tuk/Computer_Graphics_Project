#ifndef CHALLENGEMANAGER_H
#define CHALLENGEMANAGER_H

#include "Common.h"
#include "Block.h"
#include "BlockManager.h"
#include <vector>
#include <string>

enum class GameMode {
    FREE_BUILD,
    CHALLENGE
};

enum class ChallengeType {
    PARKOUR,        // 점프 파쿠르 챌린지
    BUILD,          // 건축 챌린지
    COLLECTION,     // 수집 챌린지
    TIME_TRIAL      // 타임 어택
};

enum class ChallengeDifficulty {
    EASY,
    MEDIUM,
    HARD,
    EXPERT
};

// 목표 블록 구조체
struct ChallengeBlock {
    glm::vec3 position;
    BlockType type;
    glm::vec3 color;
    bool isCheckpoint;  // 파쿠르용 체크포인트

    ChallengeBlock(const glm::vec3& pos, BlockType blockType = BlockType::DIRT,
        const glm::vec3& col = glm::vec3(0.8f, 0.8f, 0.8f), bool checkpoint = false)
        : position(pos), type(blockType), color(col), isCheckpoint(checkpoint) {
    }
};

// 챌린지 정보 구조체
struct ChallengeInfo {
    int id;
    std::string name;
    std::string description;
    ChallengeType type;
    ChallengeDifficulty difficulty;
    float timeLimit;
    int targetScore;  // 목표 점수 (블록 개수, 체크포인트 개수 등)

    ChallengeInfo(int _id, const std::string& _name, const std::string& _desc,
        ChallengeType _type, ChallengeDifficulty _diff, float _time, int _score)
        : id(_id), name(_name), description(_desc), type(_type),
        difficulty(_diff), timeLimit(_time), targetScore(_score) {
    }
};

class ChallengeManager {
private:
    GameMode currentMode;
    ChallengeInfo* currentChallenge;

    // 챌린지 데이터
    std::vector<ChallengeBlock> targetBlocks;      // 목표 블록들
    std::vector<ChallengeBlock> platformBlocks;    // 고정 플랫폼 (파쿠르용)
    std::vector<glm::vec3> checkpoints;            // 체크포인트 위치들

    // 상태 변수
    float challengeTime;
    float maxTime;
    bool challengeStarted;
    bool challengeCompleted;
    bool challengeFailed;

    // 진행 상황
    int currentScore;           // 현재 점수
    int checkpointsReached;     // 도달한 체크포인트 수
    glm::vec3 lastCheckpoint;   // 마지막 체크포인트 위치

    // UI/시각화
    float previewRotation;

    // 사용 가능한 챌린지 목록
    std::vector<ChallengeInfo> availableChallenges;

    // 초기화 함수들
    void initializeChallenges();
    void setupParkourChallenge(int id);

public:
    ChallengeManager();
    ~ChallengeManager();

    // 모드 관리
    void setMode(GameMode mode);
    GameMode getMode() const { return currentMode; }

    // 챌린지 관리
    void loadChallenge(int challengeId);
    void startChallenge();
    void update(float deltaTime, const glm::vec3& playerPos);
    void reset();

    // 챌린지 상태
    bool isChallengeMode() const { return currentMode == GameMode::CHALLENGE; }
    bool isChallengeStarted() const { return challengeStarted; }
    bool isChallengeCompleted() const { return challengeCompleted; }
    bool isChallengeFailed() const { return challengeFailed; }

    // 시간 관리
    float getTimeRemaining() const { return maxTime - challengeTime; }
    float getChallengeTime() const { return challengeTime; }

    // 진행 상황
    float getProgress() const;
    int getCurrentScore() const { return currentScore; }
    int getTargetScore() const;
    int getCheckpointsReached() const { return checkpointsReached; }

    // 블록 관련
    const std::vector<ChallengeBlock>& getTargetBlocks() const { return targetBlocks; }
    const std::vector<ChallengeBlock>& getPlatformBlocks() const { return platformBlocks; }

    // 체크포인트
    bool checkCheckpoint(const glm::vec3& playerPos);
    void respawnAtCheckpoint(glm::vec3& playerPos);

    // 플랫폼 충돌 체크
    bool checkPlatformCollision(const AABB& playerAABB) const;

    // 챌린지 정보
    ChallengeInfo* getCurrentChallengeInfo() const { return currentChallenge; }
    const std::vector<ChallengeInfo>& getAvailableChallenges() const { return availableChallenges; }
    std::string getChallengeTypeString(ChallengeType type) const;
    std::string getDifficultyString(ChallengeDifficulty diff) const;

    // UI
    float getPreviewRotation() const { return previewRotation; }
};

#endif // CHALLENGEMANAGER_Hy