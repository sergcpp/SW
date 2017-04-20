#pragma once

#include "../eng/TimedInput.h"

#include "LoadingIcon.h"

class GCursor;

class LoadingUI : public ui::BaseElement {
	LoadingIcon	loading_icon_;
public:
	LoadingUI(const ui::BaseElement *root);

	void Draw(ui::Renderer *r) override;

	void Resize(const ui::BaseElement *parent) override;

	void Update(int dt_ms);
};
