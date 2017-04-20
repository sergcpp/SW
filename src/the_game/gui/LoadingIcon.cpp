#include "LoadingIcon.h"

LoadingIcon::LoadingIcon(const glm::vec2 &pos, const glm::vec2 &size, const ui::BaseElement *parent)
	: ui::BaseElement(pos, size, parent) {
	Init();
}

void LoadingIcon::Draw(ui::Renderer *r) {
	DrawCurve(dims_[0], dims_[1], time_s_);
    DrawCurve(dims_[0] + glm::vec2(0.005f, 0.005f), dims_[1], time_s_ + 0.05f);
    DrawCurve(dims_[0] + glm::vec2(0.01f, 0.01f), dims_[1], time_s_ + 0.1f);
}

void LoadingIcon::Update(int dt_ms) {
    time_s_ += 0.001f * dt_ms;
}