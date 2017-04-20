#ifndef LINEARLAYOUT_H
#define LINEARLAYOUT_H

#include <vector>

#include "BaseElement.h"

namespace ui {
    class LinearLayout : public BaseElement {
    protected:
        std::vector<BaseElement *>  elements_;
        bool                        vertical_;
    public:
        LinearLayout(const glm::vec2 &start, const glm::vec2 &size, const BaseElement *parent) :
                BaseElement(start, size, parent), vertical_(false) {}

        bool vertical() const { return vertical_; }

        void set_vetical(bool v) { vertical_ = v; }

        template<class T>
        T *AddElement(T *el) {
            elements_.push_back(el);
            return el;
        }

        template<class T>
        T *InsertElement(T *el, size_t pos) {
            elements_.insert(elements_.begin() + pos, el);
            return el;
        }

        template<class T>
        T *ReplaceElement(T *el, size_t pos) {
            if (pos == elements_.size()) {
                elements_.push_back(el);
            } else {
                elements_[pos] = el;
            }
        }

        void Clear() {
            elements_.clear();
        }

        void Resize(const BaseElement *parent) override;
        void Resize(const glm::vec2 &start, const glm::vec2 &size, const BaseElement *parent) override;

        bool Check(const glm::ivec2 &p) const override;
        bool Check(const glm::vec2 &p) const override;

        void Focus(const glm::ivec2 &p) override;
        void Focus(const glm::vec2 &p) override;

        void Press(const glm::ivec2 &p, bool push) override;
        void Press(const glm::vec2 &p, bool push) override;

        void Draw(Renderer *r) override;
    };
}

#endif // LINEARLAYOUT_H
