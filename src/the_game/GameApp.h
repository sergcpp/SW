#ifndef TESTAPP_H
#define TESTAPP_H

#include <memory>
#include <string>
#include <vector>

#include "eng/TimedInput.h"

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

struct SWcontext;

class Game;

class GameApp {
    SDL_Renderer    *renderer_;
    SDL_Texture     *texture_;
    SDL_Window		*window_;
    SWcontext       *sw_ctx_;

#if !defined(__ANDROID__)
	bool ConvertToRawButton(int32_t key, InputManager::RawInputButton &button);
	void PollEvents();
#endif

    std::unique_ptr<Game> game_;
public:
	GameApp();
	~GameApp();

	int Init(int w, int h);
	void Destroy();

	void Frame();

#if !defined(__ANDROID__)
	int Run(const std::vector<std::string> &args);
#endif

	bool terminated() const;
};

#endif /* TESTAPP_H */
