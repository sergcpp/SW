#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "TimedInput.h"

class GameState {
public:
    virtual ~GameState() { }

    //Вызавается каждый раз при возвращении
    //в это состояние
    virtual void Enter() { };

    //Вызавается каждый раз при выходе
    //из этого состояния
    virtual void Exit() { };

    //Рисование
    virtual void Draw(float dt_s) { };

    //Вызавается UPDATE_RATE раз в секунду (обычно 60)
    virtual void Update(int dt_ms) { };

    virtual void HandleInput(InputManager::Event) { };
};



#endif /* GAMESTATE_H */
