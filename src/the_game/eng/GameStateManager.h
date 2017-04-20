#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include <memory>
#include <vector>

#include "GameState.h"
#include "TimedInput.h"

class GameState;

class GameStateManager {
    std::vector<std::shared_ptr<GameState>> states_;
public:
    virtual ~GameStateManager();

    std::shared_ptr<GameState> Peek();

    void Push(const std::shared_ptr<GameState> &state);

    std::shared_ptr<GameState> Pop();

    std::shared_ptr<GameState> Switch(const std::shared_ptr<GameState> &state);

    void Clear();

    void Update(int dt_ms);

    void Draw(float dt_s);

    void HandleInput(InputManager::Event &);
};


#endif /* GAMESTATEMANAGER_H */
