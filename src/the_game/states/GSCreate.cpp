#include "GSCreate.h"

#include <stdexcept>

#include "GSDrawTest.h"
#include "GSLoading.h"
#include "GSMenu.h"
#include "GSRunning.h"
#include "GSTransition.h"

std::shared_ptr<GameState> GSCreate(eGameState state, GameBase *game) {
	if (state == GS_MENU) {
		return std::make_shared<GSMenu>(game);
	} else if (state == GS_LOADING) {
		return std::make_shared<GSLoading>(game);
	} else if (state == GS_RUNNING) {
		return std::make_shared<GSRunning>(game);
	} else if (state == GS_TRANSITION) {
		return std::make_shared<GSTransition>(game);
	} else if (state == GS_DRAW_TEST) {
		return std::make_shared<GSDrawTest>(game);
	}

	throw std::invalid_argument("Unknown game state!");
}