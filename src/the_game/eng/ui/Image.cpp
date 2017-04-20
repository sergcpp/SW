#include "Image.h"

#include <memory>

#include <sys/AssetFile.h>

#include "Renderer.h"

ui::Image::Image(const R::Texture2DRef &tex, const glm::vec2 uvs[2],
                 const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent) :
        BaseElement(pos, size, parent), tex_(tex) {
    uvs_[0] = uvs[0]; uvs_[1] = uvs[1];
}

ui::Image::Image(const char *tex_name, const glm::vec2 uvs[2],
                 const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent) :
        BaseElement(pos, size, parent) {
    uvs_[0] = uvs[0]; uvs_[1] = uvs[1];

    sys::AssetFile in_file(tex_name, sys::AssetFile::IN);
    size_t in_file_size = in_file.size();
    std::unique_ptr<char[]> data(new char[in_file_size]);
    in_file.Read(data.get(), in_file_size);

    tex_ = R::LoadTexture2D(tex_name, data.get());
}

void ui::Image::Draw(Renderer *r) {
    r->DrawImageQuad(tex_, dims_, uvs_);
}
