#ifndef CHALLENGEMANAGER_H
#define CHALLENGEMANAGER_H

#include "Common.h"
#include "Block.h"
#include "BlockManager.h"
#include <vector>

enum class GameMode {
    FREE_BUILD,
    CHALLENGE
};

struct ChallengeBlock {
    glm::vec3 position;
    glm::vec3 color;

    ChallengeBlock(const glm::vec3& pos, const glm::vec3& col)
        : position(pos), color(col) {
    }
};

class ChallengeManager {
private:
    GameMode currentMode;
    std::vector<ChallengeBlock> targetBlocks;
    float challengeTime;
    float maxTime;
    bool challengeStarted;
    bool challengeCompleted;
    float previewRotation;

public:
    ChallengeManager();

    void setMode(GameMode mode);
    GameMode getMode() const { return currentMode; }

    void startChallenge();
    void update(float deltaTime);

    void loadChallenge(int challengeNumber);

    bool isChallengeMode() const { return currentMode == GameMode::CHALLENGE; }
    bool isChallengeStarted() const { return challengeStarted; }
    bool isChallengeCompleted() const { return challengeCompleted; }

    float getTimeRemaining() const { return maxTime - challengeTime; }
    float getProgress(const std::unordered_map<GridPosition, Block>& playerBlocks) const;

    const std::vector<ChallengeBlock>& getTargetBlocks() const { return targetBlocks; }
    float getPreviewRotation() const { return previewRotation; }

    void reset();
};

#endif // CHALLENGEMANAGER_H
