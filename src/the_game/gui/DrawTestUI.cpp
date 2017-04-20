#include "DrawTestUI.h"

#include "../eng/ui/BitmapFont.h"

namespace DrawTestUIInternal {
	const char PRIMS[] = "PRIMITIVES",
			   CORRE[] = "ATTRIBUTE INTERPOLATION";

	const char LINE[] = "line",
			   TRI[] = "tri",
			   CURVE[] = "curve";

	const char AFF[] = "affine",
			   CORR[] = "correct",
			   FAST1[] = "fast (correct in",
			   FAST2[] = "block corners)";
}

DrawTestUI::DrawTestUI(const std::shared_ptr<ui::BitmapFont> &font, const ui::BaseElement *root)
	: BaseElement({ -1, -1 }, { 2, 2 }, root), font_(font) {
	state_ = 0;
}

void DrawTestUI::Draw(ui::Renderer *r) {
	using namespace DrawTestUIInternal;

	if (state_ == 1) {
		float w = font_->GetWidth(PRIMS, this);
		font_->DrawText(r, PRIMS, { 0 - 0.5f * w, 0.7f }, this);

		w = font_->GetWidth(LINE, this);
		font_->DrawText(r, LINE, { -0.625f - 0.5f * w, -0.7f }, this);

		w = font_->GetWidth(TRI, this);
		font_->DrawText(r, TRI, { 0 - 0.5f * w, -0.7f }, this);

		w = font_->GetWidth(CURVE, this);
		font_->DrawText(r, CURVE, { 0.625f - 0.5f * w, -0.7f }, this);
	} else if (state_ == 2) {
		float w = font_->GetWidth(CORRE, this);
		font_->DrawText(r, CORRE, { 0 - 0.5f * w, 0.7f }, this);

		w = font_->GetWidth(AFF, this);
		font_->DrawText(r, AFF, { -0.625f - 0.5f * w, -0.75f }, this);

		w = font_->GetWidth(CORR, this);
		font_->DrawText(r, CORR, { 0 - 0.5f * w, -0.75f }, this);

		w = font_->GetWidth(FAST1, this);
		font_->DrawText(r, FAST1, { 0.625f - 0.5f * w, -0.75f }, this);
		w = font_->GetWidth(FAST2, this);
		font_->DrawText(r, FAST2, { 0.625f - 0.5f * w, -0.85f }, this);
	}
}

void DrawTestUI::Resize(const ui::BaseElement *parent) {
	BaseElement::Resize(parent);
}

void DrawTestUI::Update(int dt_ms) {

}