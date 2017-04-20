#pragma once

#include <memory>

enum eGameState { GS_MENU, GS_LOADING, GS_RUNNING, GS_TRANSITION, GS_DRAW_TEST };

class GameBase;
class GameState;

std::shared_ptr<GameState> GSCreate(eGameState state, GameBase *game);