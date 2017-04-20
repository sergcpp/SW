#ifndef IMAGE_H
#define IMAGE_H

#include <ren/Texture.h>

#include "BaseElement.h"

namespace ui {
    class Image : public BaseElement {
	protected:
        R::Texture2DRef tex_;
        glm::vec2       uvs_[2];
    public:
        Image(const R::Texture2DRef &tex, const glm::vec2 uvs[2],
              const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent);
        Image(const char *tex_name, const glm::vec2 uvs[2],
              const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent);

		void set_uvs(const glm::vec2 uvs[2]) {
			uvs_[0] = uvs[0];
			uvs_[1] = uvs[1];
		}

        void Draw(Renderer *r) override;
    };
}

#endif // IMAGE_H
