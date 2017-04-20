#pragma once

#include "eng/GameBase.h"

const char UI_CURSOR_KEY[]              = "ui_cursor";
const char UI_BACKGROUND_KEY[]          = "ui_background";
const char UI_FONTS_KEY[]               = "ui_fonts";
const char RENDERER_KEY[]               = "renderer";
const char SCENE_MANAGER_KEY[]          = "scene_manager";
const char CURRENT_LEVEL_KEY[]			= "current_level";

struct LevelInfo {
	std::string name;
    std::string door_name;
    bool revert = false;
};

class Game : public GameBase {
public:
    Game(int w, int h, const char *local_dir);

    void Resize(int w, int h) override;
};

