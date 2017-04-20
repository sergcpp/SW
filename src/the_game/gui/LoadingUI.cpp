#include "LoadingUI.h"

LoadingUI::LoadingUI(const ui::BaseElement *root) : BaseElement({ -1, -1 }, { 2, 2 }, root),
													loading_icon_({ 0, 0 }, { 1, 1 }, root) {
	Resize(root);
}

void LoadingUI::Draw(ui::Renderer *r) {
	loading_icon_.Draw(r);
}

void LoadingUI::Resize(const ui::BaseElement *parent) {
	float k = float(parent->size_px().x) / parent->size_px().y;
	loading_icon_.Resize({ 0.65f, -0.75f }, { 0.2f, k * 0.1f }, parent);
}

void LoadingUI::Update(int dt_ms) {
    loading_icon_.Update(dt_ms);
}