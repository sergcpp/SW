#ifndef BUTTONBASE_H
#define BUTTONBASE_H

#include <sys/Signal_.h>

#include "BaseElement.h"

namespace ui {
    class ButtonBase : public BaseElement {
    protected:
        enum eState { ST_NORMAL, ST_FOCUSED, ST_PRESSED };
        eState state_;
    public:
        ButtonBase(const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent);

        void Focus(const glm::ivec2 &p) override;
        void Focus(const glm::vec2 &p) override;

        void Press(const glm::ivec2 &p, bool push) override;
        void Press(const glm::vec2 &p, bool push) override;

        sys::Signal<void()> pressed_signal;
    };
}

#endif // BUTTONBASE_H
