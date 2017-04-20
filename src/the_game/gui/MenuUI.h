#pragma once

#include "../eng/TimedInput.h"
#include "../eng/sys/Signal_.h"
#include "../eng/ui/ButtonText.h"
#include "../eng/ui/ButtonImage.h"
#include "../eng/ui/TypeMesh.h"
#include "../eng/ui/LinearLayout.h"

class GCursor;

class MenuUI : public ui::BaseElement {
	enum eState { Intro, Main } state_;

    std::shared_ptr<ui::BitmapFont> font_;

	ui::TypeMesh		start_text_;

	ui::ButtonText      newgame_text_;
	ui::ButtonText		exit_text_;
public:
	MenuUI(const std::shared_ptr<ui::BitmapFont> &font,
           const std::shared_ptr<GCursor> cursor, const ui::BaseElement *root);

    void Draw(ui::Renderer *r) override;

    void Resize(const ui::BaseElement *parent) override;

    void HandleInput(InputManager::Event evt, const ui::BaseElement *root);

    void Update(int dt_ms);

	sys::Signal<void()> &new_game_signal() { return newgame_text_.pressed_signal; }
	sys::Signal<void()> &exit_signal() { return exit_text_.pressed_signal; }
};
