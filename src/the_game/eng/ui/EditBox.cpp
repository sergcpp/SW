#include "EditBox.h"

#include "Renderer.h"

namespace EditBoxConstants{
    const unsigned long long default_flags =
                    (1 << ui::Integers) |
                    (1 << ui::Chars) |
                    (1 << ui::Floats) |
                    (1 << ui::Signed) |
                    (1 << ui::Multiline);

	const int padding = 10;
    const int cursor_offset = 12;
}

ui::EditBox::EditBox(const char *frame_tex_name, const glm::vec2 &frame_offsets,
                     BitmapFont *font,
                     const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent)
    : EditBox({frame_tex_name, frame_offsets, {-1, -1}, {2, 2}, this}, font, pos, size, parent) {}

ui::EditBox::EditBox(const Frame &frame, BitmapFont *font,
                     const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent)
    : BaseElement(pos, size, parent),
      cursor_("|", font, {0, 0}, this),
	  lay_({ -1 + 2.0f * EditBoxConstants::padding / parent->size_px().x, -1 }, { 2, 2 }, this),
      frame_(frame), font_(font), edit_flags_(EditBoxConstants::default_flags), focused_(false),
      current_line_(0), current_char_(0) {
	lay_.set_vetical(true);
	frame_.Resize(this);
}

void ui::EditBox::Resize(const BaseElement *parent) {
	BaseElement::Resize(parent);
	lay_.Resize({ -1 + 2.0f * EditBoxConstants::padding / parent->size_px().x, -1 }, { 2, 2 }, this);
	frame_.Resize(this);

    UpdateCursor();
}

void ui::EditBox::Press(const glm::vec2 &p, bool push) {
    if (!push) return;

    if (Check(p)) {
        focused_ = true;
        for (auto it = lines_.begin(); it != lines_.end(); ++it) {
            if (it->Check(p)) {
                current_line_ = std::distance(lines_.begin(), it);
                const auto &pos = it->positions();
                for (unsigned i = 0; i < pos.size(); i += 4 * 3) {
                    if ((i == 0 && p.x > pos[i]) || (i > 0 && p.x > 0.5f * (pos[i] + pos[i - 3]))) {
                        current_char_ = i / 12;
                    }
                }
                UpdateCursor();
                break;
            } else if (p.y >= it->pos().y && p.y <= it->pos().y + it->size().y) {
                current_line_ = std::distance(lines_.begin(), it);
                current_char_ = lines_[current_line_].text().length();
                UpdateCursor();
                break;
            }
        }
    } else {
        focused_ = false;
    }
}

void ui::EditBox::Draw(Renderer *r) {
	auto &cur = r->GetParams();
	r->EmplaceParams(cur.col(), cur.z_val(), cur.blend_mode(), dims_px_);

    frame_.Draw(r);
    lay_.Draw(r);

    r->EmplaceParams(glm::vec3(0.75f, 0.75f, 0.75f), cur.z_val(), cur.blend_mode(), dims_px_);
    if (focused_) {
        cursor_.Draw(r);
    }
    r->PopParams();

	r->PopParams();
}

int ui::EditBox::AddLine(const std::string &text) {
    if (!edit_flags_[Multiline] && !lines_.empty()) return -1;

    lines_.emplace_back(text, font_, glm::vec2{0, 0}, this);

    // pointers could be invalidated after reallocation, so...
    UpdateLayout();

    return (int)lines_.size() - 1;
}

int ui::EditBox::InsertLine(const std::string &text) {
    if (!edit_flags_[Multiline]) return -1;

    lines_.insert(lines_.begin() + current_line_, { text, font_, glm::vec2{ 0, 0 }, this });

    UpdateLayout();

    return current_line_;
}

void ui::EditBox::DeleteLine(unsigned line) {
    lines_.erase(lines_.begin() + line);
    UpdateLayout();
}

void ui::EditBox::UpdateLayout() {
    lay_.Clear();
    for (auto &l : lines_) {
        lay_.AddElement(&l);
    }
    lay_.Resize(this);
}

void ui::EditBox::UpdateCursor() {
    using namespace EditBoxConstants;

    if (current_line_ >= (int)lines_.size()) return;
    const auto &cur_line = lines_[current_line_];

    glm::vec2 cur_pos = { 0, cur_line.pos().y };
    if (current_char_ < (int)line_text(current_line_).length()) {
        cur_pos.x = cur_line.positions()[current_char_ * 12];
    } else {
        cur_pos.x = cur_line.pos().x + cur_line.size().x;
    }

    cur_pos = 2.0f * (cur_pos - pos()) / size() - glm::vec2(1, 1);
    cur_pos.x -= float(cursor_offset) / size_px().x;

    cursor_.Move(cur_pos, this);
}

void ui::EditBox::AddChar(int c) {
    if (current_line_ >= (int)lines_.size()) return;

    switch (c) {
        case 191:
            c = '0' - 1;
            break;
        case 190:
            if (!(edit_flags_[Chars] || edit_flags_[Floats])) return;
            c = 46;
            break;
        case 188:
            if (!edit_flags_[Chars]) return;
            c = 44;
            break;
        default:
            if (((c == ' ' || (c >= 'A' && c <= 'z') || (c >= 160 && c <= 255)) && (edit_flags_[Chars])) ||
                ((c == '.' || c == '-' || c == '=' || c == '+' || c == '/' || c == '{' || c == '}' || (c >= '0' && c <= '9')) &&
                (edit_flags_[Integers] || edit_flags_[Floats]))) {
            } else {
                //LOGE("No %i", (int)s);
                return;
            }
    }

    std::string text = lines_[current_line_].text();
    text.insert(text.begin() + current_char_, c);

    lines_[current_line_] = TypeMesh(text, font_, glm::vec2{ 0, 0 }, this);
    current_char_++;

    UpdateLayout();
    UpdateCursor();
}

void ui::EditBox::DeleteChar() {
    if (current_line_ >= (int)lines_.size()) return;

    std::string text = lines_[current_line_].text();

    int ch = current_char_ - 1;
    if (ch < 0 || ch >= (int)text.length()) return;
    text.erase(text.begin() + ch);

    lines_[current_line_] = TypeMesh(text, font_, glm::vec2{ 0, 0 }, this);
    current_char_--;

    UpdateLayout();
    UpdateCursor();
}

bool ui::EditBox::MoveCursorH(int m) {
    if (current_line_ >= (int)lines_.size()) return false;

    current_char_ += m;

    int len = (int)lines_[current_line_].text().length();
    
    if (current_char_ < 0) {
        current_char_ = 0;
        return false;
    } else if (current_char_ > len) {
        current_char_ = len;
        return false;
    }

    UpdateCursor();

    return true;
}

bool ui::EditBox::MoveCursorV(int m) {
    if (current_line_ >= (int)lines_.size()) return false;

    bool res = true;

    current_line_ += m;

    if (current_line_ < 0) {
        current_line_ = 0;
        res = false;
    } else if (current_line_ >= (int)lines_.size()) {
        current_line_ = lines_.size() - 1;
        res = false;
    }

    int len = (int)lines_[current_line_].text().length();

    if (current_char_ < 0) {
        current_char_ = 0;
        return false;
    } else if (current_char_ > len) {
        current_char_ = len;
        return false;
    }

    UpdateCursor();

    return res;
}