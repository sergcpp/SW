#pragma once

#include "../eng/ren/Program.h"
#include "../eng/ui/BaseElement.h"

class LoadingIcon : public ui::BaseElement {
	R::ProgramRef curve_program_;

	float time_s_ = 0;

	void Init();
	void DrawCurve(const glm::vec2 &pos, const glm::vec2 &size, float t);
public:
	LoadingIcon(const glm::vec2 &pos, const glm::vec2 &size, const ui::BaseElement *parent);

	void Draw(ui::Renderer *r);

    void Update(int dt_ms);
};