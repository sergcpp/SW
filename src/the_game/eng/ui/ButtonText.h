#ifndef BUTTONTEXT_H
#define BUTTONTEXT_H

#include <string>

#include "ButtonBase.h"
#include "TypeMesh.h"

namespace ui {
    class ButtonText : public ButtonBase {
    protected:
        TypeMesh type_mesh_;
    public:
        ButtonText(const std::string &text, BitmapFont *font, const glm::vec2 &pos, const BaseElement *parent);
	
        bool Check(const glm::vec2 &p) const override;
        bool Check(const glm::ivec2 &p) const override;

		void Move(const glm::vec2 &pos, const BaseElement *parent);

        void Draw(Renderer *r) override;
    };
}

#endif // BUTTONTEXT_H
