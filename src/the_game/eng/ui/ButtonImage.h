#ifndef BUTTONIMAGE_H
#define BUTTONIMAGE_H

#include <memory>

#include "ButtonBase.h"
#include "Image.h"

namespace ui {
    class ButtonImage : public ButtonBase {
    protected:
		Image image_normal_, image_focused_, image_pressed_;
        std::unique_ptr<BaseElement> additional_element_;
    public:
        ButtonImage(const char *tex_normal, const glm::vec2 uvs_normal[2],
                    const char *tex_focused, const glm::vec2 uvs_focused[2],
                    const char *tex_pressed, const glm::vec2 uvs_pressed[2],
                    const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent);

        BaseElement *element() const { return additional_element_.get(); }

        void SetElement(std::unique_ptr<BaseElement> &&el) { additional_element_ = std::move(el); additional_element_->Resize(this); }

        void Resize(const BaseElement *parent) override;
        //void Resize(const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent) override;

        void Draw(Renderer *r) override;
    };
}

#endif // BUTTONIMAGE_H
