#ifndef CURSOR_H
#define CURSOR_H

#include "Image.h"

namespace ui {
    class Cursor : public BaseElement {
	protected:
        Image		img_;
        bool		clicked_;
        glm::vec2	offset_;
    public:
        Cursor(const R::Texture2DRef &tex, const glm::vec2 uvs[2], const glm::vec2 &size, const BaseElement *parent);
        Cursor(const char *tex_name, const glm::vec2 uvs[2], const glm::vec2 &size, const BaseElement *parent);

        void set_clicked(bool b) { clicked_ = b; }
        void set_offset(const glm::vec2 &offset) { offset_ = offset; }

        void SetPos(const glm::vec2 &pos, const BaseElement *parent);

        void Draw(Renderer *r) override;
    };
}

#endif // CURSOR_H
