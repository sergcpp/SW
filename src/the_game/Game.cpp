#include "Game.h"

#include <sstream>

#include <glm/vec2.hpp>

#include "eng/GameStateManager.h"
#include "eng/sys/AssetFile.h"
#include "eng/sys/Log.h"
#include "eng/sys/Json.h"

#include "managers/Renderer.h"
#include "states/GSCreate.h"
#include "gui/FontStorage.h"
#include "gui/GBackground.h"
#include "gui/GCursor.h"

namespace GameConstants {
    const int CURSOR_SIZE = 32;
	const char CURSOR_FILENAME[] = "game_assets/textures/cursor_03.tga";
	const char CONFIG_FILENAME[] = "game_assets/config.json";
}

Game::Game(int w, int h, const char *local_dir) : GameBase(w, h, local_dir) {
    using namespace GameConstants; using namespace glm;

    auto ui_root = GetComponent<ui::RootElement>(UI_ROOT_KEY);

    vec2 uvs_normal[2] = { { 0, 0 }, { 0.5f, 1 } },
		 uvs_text_edit[2] = { { 0.5f, 0 }, { 1, 1 } };
    vec2 size = { CURSOR_SIZE / float(w), CURSOR_SIZE / float(h) };
	auto ui_cursor = std::make_shared<GCursor>(CURSOR_FILENAME, uvs_normal, size, ui_root.get());
	ui_cursor->set_mode_uvs(GCursor::TextEdit, uvs_text_edit);
	ui_cursor->set_offset({ -1, -1 });
    SetComponent(UI_CURSOR_KEY, ui_cursor);

    auto ui_background = std::make_shared<GBackground>(vec2{1.5f, -0.75f}, vec2{-0.5f, 0}, ui_root.get());
    SetComponent(UI_BACKGROUND_KEY, ui_background);

    JsObject main_config;
    
    // load config
	{   sys::AssetFile config_file(CONFIG_FILENAME, sys::AssetFile::IN);
        size_t config_file_size = config_file.size();
        std::unique_ptr<char[]> buf(new char[config_file_size]);
        config_file.Read(buf.get(), config_file_size);

        std::stringstream ss;
        ss.write(buf.get(), config_file_size);
    
        if (!main_config.Read(ss)) {
            throw std::runtime_error("Unable to load main config!");
        }
    }

    const JsObject &gfx_settings = (const JsObject &)main_config.at("gfx_settings");

    {   // create renderer for player etc.
        const JsObject &general_settings = (const JsObject &)gfx_settings.at("general");
        auto renderer = std::make_shared<Renderer>(general_settings);
        SetComponent(RENDERER_KEY, renderer);
    }

    const JsObject &ui_settings = (const JsObject &)main_config.at("ui_settings");

    {   // load fonts
        auto font_storage = std::make_shared<FontStorage>();
        SetComponent(UI_FONTS_KEY, font_storage);

        const JsObject &fonts = (const JsObject &)ui_settings.at("fonts");
        for (auto &el : fonts.elements) {
            const std::string &name = el.first;
            const JsString &file_name = (const JsString &)el.second;

            font_storage->LoadFont(name, file_name.val);
        }
    }

    auto state_manager = GetComponent<GameStateManager>(STATE_MANAGER_KEY);
	state_manager->Push(GSCreate(GS_MENU, this));
}

void Game::Resize(int w, int h) {
    using namespace GameConstants;
    using namespace glm; using namespace ui;

    GameBase::Resize(w, h);

    auto ui_root	   = GetComponent<RootElement>(UI_ROOT_KEY);
    auto ui_cursor	   = GetComponent<GCursor>(UI_CURSOR_KEY);
    auto ui_background = GetComponent<GBackground>(UI_BACKGROUND_KEY);

    ui_root->set_zone({w, h});
    
    vec2 size = { CURSOR_SIZE / float(w), CURSOR_SIZE / float(h) };
    vec2 pos = ui_cursor->pos();
    ui_cursor->Resize(pos, size, ui_root.get());

    ui_background->Resize(ui_root.get());
}

