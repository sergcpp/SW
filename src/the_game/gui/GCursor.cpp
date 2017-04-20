#include "GCursor.h"

#include "../eng/ui/Utils.h"

GCursor::GCursor(const char *tex_name, const glm::vec2 uvs[2], const glm::vec2 &size, const ui::BaseElement *parent)
        : ui::Cursor(tex_name, uvs, size, parent) {
	set_mode_uvs(Normal, uvs);
}

void GCursor::HandleInput(InputManager::Event evt, const ui::BaseElement *root) {
    switch (evt.type) {
        case InputManager::RAW_INPUT_P1_DOWN:
        case InputManager::RAW_INPUT_P2_DOWN: {
            glm::vec2 p = ui::MapPointToScreen({evt.point.x, evt.point.y}, root->size_px());
            this->set_clicked(true);
            this->SetPos(p, root);
        } break;
        case InputManager::RAW_INPUT_P1_UP:
        case InputManager::RAW_INPUT_P2_UP: {
            glm::vec2 p = ui::MapPointToScreen({evt.point.x, evt.point.y}, root->size_px());
            this->set_clicked(false);
            this->SetPos(p, root);
        } break;
        case InputManager::RAW_INPUT_P1_MOVE:
        case InputManager::RAW_INPUT_P2_MOVE: {
            glm::vec2 p = ui::MapPointToScreen({evt.point.x, evt.point.y}, root->size_px());
            this->SetPos(p, root);
        } break;
        default:
            break;
    }
}
