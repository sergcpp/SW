#ifndef EDITBOX_H
#define EDITBOX_H

#include <bitset>

#include "Frame.h"
#include "LinearLayout.h"
#include "TypeMesh.h"

namespace ui {
    enum eEditBoxFlags {
        Integers,
        Chars,
        Floats,
        Signed,
        Multiline
    };
    
    class EditBox : public BaseElement {
        TypeMesh cursor_;
        LinearLayout lay_;
        Frame frame_;
        std::vector<TypeMesh> lines_;
        BitmapFont *font_;
        std::bitset<32> edit_flags_;
		bool focused_;
        int current_line_, current_char_;

        void UpdateLayout();
        void UpdateCursor();
    public:
        EditBox(const char *frame_tex_name, const glm::vec2 &frame_offsets,
                BitmapFont *font,
                const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent);
        EditBox(const Frame &frame, BitmapFont *font,
                const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent);

        const Frame &frame() const { return frame_; }

        const std::string &line_text(unsigned line) const { return lines_[line].text(); }

		bool focused() const { return focused_; }

        void set_focused(bool b) { focused_ = b; UpdateCursor(); }

        void set_flag(eEditBoxFlags flag, bool enabled) { edit_flags_.set(flag, enabled); }

		void Resize(const BaseElement *parent) override;

        void Press(const glm::vec2 &p, bool push) override;

        void Draw(Renderer *r) override;

        int AddLine(const std::string &text);
        int InsertLine(const std::string &text);
        void DeleteLine(unsigned line);

        void AddChar(int c);
        void DeleteChar();

        bool MoveCursorH(int m);
        bool MoveCursorV(int m);
    };
}

#endif // EDITBOX_H