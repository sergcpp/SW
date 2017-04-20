#pragma once

#include "../eng/TimedInput.h"
#include "../eng/ui/Cursor.h"

class GCursor : public ui::Cursor {
public:
    GCursor(const char *tex_name, const glm::vec2 uvs[2], const glm::vec2 &size, const ui::BaseElement *parent);

	enum eCursorMode { Normal, TextEdit, NumModes };
	void set_mode_uvs(eCursorMode mode, const glm::vec2 uvs[2]) {
		modes_uvs_[mode][0] = uvs[0];
		modes_uvs_[mode][1] = uvs[1];
	}

	void SwitchMode(eCursorMode mode) {
		img_.set_uvs(modes_uvs_[mode]);
	}

    void HandleInput(InputManager::Event evt, const ui::BaseElement *root);
private:
	eCursorMode mode_;
	glm::vec2 modes_uvs_[NumModes][2];
};

