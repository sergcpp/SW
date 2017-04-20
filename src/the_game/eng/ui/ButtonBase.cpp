#include "ButtonBase.h"

ui::ButtonBase::ButtonBase(const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent)
    : BaseElement(pos, size, parent), state_(ST_NORMAL) { }

void ui::ButtonBase::Focus(const glm::ivec2 &p) {
    if (state_ != ST_PRESSED) {
        if (Check(p)) {
            state_ = ST_FOCUSED;
        } else {
            state_ = ST_NORMAL;
        }
    }
}

void ui::ButtonBase::Focus(const glm::vec2 &p) {
    if (state_ != ST_PRESSED) {
        if (Check(p)) {
            state_ = ST_FOCUSED;
        } else {
            state_ = ST_NORMAL;
        }
    }
}

void ui::ButtonBase::Press(const glm::ivec2 &p, bool push) {
    if (state_ != ST_NORMAL) {
        if (Check(p)) {
            if (push) {
                state_ = ST_PRESSED;
            } else {
                pressed_signal.FireN();
                state_ = ST_FOCUSED;
            }
        } else {
            state_ = ST_NORMAL;
        }
    }
}

void ui::ButtonBase::Press(const glm::vec2 &p, bool push) {
    if (state_ != ST_NORMAL) {
        if (Check(p)) {
            if (push) {
                state_ = ST_PRESSED;
            } else {
                pressed_signal.FireN();
                state_ = ST_FOCUSED;
            }
        } else {
            state_ = ST_NORMAL;
        }
    }
}