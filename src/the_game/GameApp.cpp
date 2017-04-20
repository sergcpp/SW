#include "GameApp.h"

#if !defined(__ANDROID__)
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_keyboard.h>
	#include <SDL2/SDL_video.h>
#endif

#include <SDL2/SDL_events.h>

#include <SW/SW.h>

#include "eng/TimedInput.h"
#include "eng/ren/RenderState.h"
#include "eng/sys/Log.h"
#include "eng/sys/Time_.h"

#include "Game.h"

namespace {
    GameApp *g_app = nullptr;
}


GameApp::GameApp() {
    g_app = this;
}

GameApp::~GameApp() {

}

int GameApp::Init(int w, int h) {
#if !defined(__ANDROID__)
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return -1;
	}

	window_ = SDL_CreateWindow("View", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_RESIZABLE);

    SDL_ShowCursor(SDL_DISABLE);

    sw_ctx_ = swCreateContext(w, h);

    swDisable(SW_PERSPECTIVE_CORRECTION);
    swEnable(SW_FAST_PERSPECTIVE_CORRECTION);
    swEnable(SW_DEPTH_TEST);

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        const char *s = SDL_GetError();
        printf("%s\n", s);
    }
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!texture_) {
        const char *s = SDL_GetError();
        printf("%s\n", s);
    }
#endif
    R::Init(w, h);

    try {
        game_.reset(new Game(w, h, "./"));
    } catch (std::exception &e) {
        fprintf(stderr, "%s", e.what());
        return -1;
    }

    return 0;
}

void GameApp::Destroy() {
    game_.reset();
    R::Deinit();

    swDeleteContext(sw_ctx_);

#if !defined(__ANDROID__)
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);

    SDL_DestroyWindow(window_);
    SDL_Quit();
#endif
}

void GameApp::Frame() {
    //R::ClearColorAndDepth(0.1f, 0.75f, 0.75f, 1);
    game_->Frame();
}

bool GameApp::terminated() const {
	return game_->terminated;
}

#if !defined(__ANDROID__)
int GameApp::Run(const std::vector<std::string> &args) {
	//const int w = 640;  const int h = 360;
	const int w = 1024;  const int h = 576;
    //const int w = 1024;  const int h = 512;

    if (Init(w, h) < 0) {
        return -1;
    }

    while (!terminated()) {
        PollEvents();

        Frame();

        const void *pixels = swGetPixelDataRef(swGetCurFramebuffer());

        SDL_UpdateTexture(texture_, NULL, pixels, game_->width * sizeof(Uint32));
        SDL_RenderClear(renderer_);
        SDL_RenderCopy(renderer_, texture_, NULL, NULL);
        SDL_RenderPresent(renderer_);
    }

    Destroy();

    return 0;
}

bool GameApp::ConvertToRawButton(int32_t key, InputManager::RawInputButton &button) {
    switch (key) {
        case SDLK_UP:
            button = InputManager::RAW_INPUT_BUTTON_UP;
            break;
        case SDLK_DOWN:
            button = InputManager::RAW_INPUT_BUTTON_DOWN;
            break;
        case SDLK_LEFT:
            button = InputManager::RAW_INPUT_BUTTON_LEFT;
            break;
        case SDLK_RIGHT:
            button = InputManager::RAW_INPUT_BUTTON_RIGHT;
            break;
        case SDLK_ESCAPE:
            button = InputManager::RAW_INPUT_BUTTON_EXIT;
            break;
        case SDLK_TAB:
            button = InputManager::RAW_INPUT_BUTTON_TAB;
            break;
        case SDLK_BACKSPACE:
            button = InputManager::RAW_INPUT_BUTTON_BACKSPACE;
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            button = InputManager::RAW_INPUT_BUTTON_SHIFT;
            break;
        case SDLK_DELETE:
            button = InputManager::RAW_INPUT_BUTTON_DELETE;
            break;
        default:
            button = InputManager::RAW_INPUT_BUTTON_OTHER;
            break;
    }
    return true;
}

void GameApp::PollEvents() {
    auto input_manager = game_->GetComponent<InputManager>(INPUT_MANAGER_KEY);
    if (!input_manager) return;

    SDL_Event e;
    InputManager::RawInputButton button;
    InputManager::Event evt;
    while (SDL_PollEvent(&e)) {
        evt.type = InputManager::RAW_INPUT_NONE;
        switch (e.type) {
        case SDL_KEYDOWN: {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
				if (game_) game_->Quit();
                return;
            } /*else if (e.key.keysym.sym == SDLK_TAB) {
                bool is_fullscreen = bool(SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN);
                SDL_SetWindowFullscreen(window_, is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
                return;
            }*/ else if (ConvertToRawButton(e.key.keysym.sym, button)) {
                evt.type = InputManager::RAW_INPUT_KEY_DOWN;
                evt.key = button;
                evt.raw_key = e.key.keysym.sym;
            }
        } break;
        case SDL_KEYUP:
            if (ConvertToRawButton(e.key.keysym.sym, button)) {
                evt.type = InputManager::RAW_INPUT_KEY_UP;
                evt.key = button;
                evt.raw_key = e.key.keysym.sym;
            }
            break;
        case SDL_FINGERDOWN:
            evt.type = e.tfinger.fingerId == 0 ? InputManager::RAW_INPUT_P1_DOWN : InputManager::RAW_INPUT_P2_DOWN;
            evt.point.x = e.tfinger.x * game_->width;
            evt.point.y = e.tfinger.y * game_->height;
            break;
        case SDL_MOUSEBUTTONDOWN:
            evt.type = InputManager::RAW_INPUT_P1_DOWN;
            evt.point.x = (float) e.motion.x;
            evt.point.y = (float) e.motion.y;
            break;
        case SDL_FINGERUP:
            evt.type = e.tfinger.fingerId == 0 ? InputManager::RAW_INPUT_P1_UP : InputManager::RAW_INPUT_P2_UP;
            evt.point.x = e.tfinger.x * game_->width;
            evt.point.y = e.tfinger.y * game_->height;
            break;
        case SDL_MOUSEBUTTONUP:
            evt.type = InputManager::RAW_INPUT_P1_UP;
            evt.point.x = (float) e.motion.x;
            evt.point.y = (float) e.motion.y;
            break;
        case SDL_QUIT: {
			if (game_) game_->Quit();
            return;
        }
        case SDL_FINGERMOTION:
            evt.type = e.tfinger.fingerId == 0 ? InputManager::RAW_INPUT_P1_MOVE : InputManager::RAW_INPUT_P2_MOVE;
            evt.point.x = e.tfinger.x * game_->width;
            evt.point.y = e.tfinger.y * game_->height;
            evt.move.dx = e.tfinger.dx * game_->width;
            evt.move.dy = e.tfinger.dy * game_->height;
            break;
        case SDL_MOUSEMOTION:
            evt.type = InputManager::RAW_INPUT_P1_MOVE;
            evt.point.x = (float) e.motion.x;
            evt.point.y = (float) e.motion.y;
            evt.move.dx = (float) e.motion.xrel;
            evt.move.dy = (float) e.motion.yrel;
            break;
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
                e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                evt.type = InputManager::RAW_INPUT_RESIZE;
                evt.point.x = (float)e.window.data1;
                evt.point.y = (float)e.window.data2;
                // TODO: ???

                game_->Resize(e.window.data1, e.window.data2);
                SDL_RenderPresent(renderer_);

                SDL_DestroyTexture(texture_);
                texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                             e.window.data1, e.window.data2);
            }
            break;
        default:
            return;
        }
        if (evt.type != InputManager::RAW_INPUT_NONE) {
            evt.time_stamp = sys::GetTicks() - (SDL_GetTicks() - e.common.timestamp);
            input_manager->AddRawInputEvent(evt);
        }
    }
}

#endif
